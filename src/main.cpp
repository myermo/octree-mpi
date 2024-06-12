#include "handlers.hpp"
#include "main_options.hpp"
#include "octree.hpp"
#include <filesystem> // Only C++17 and beyond
#include <iomanip>
#include <iostream>
#include "TimeWatcher.hpp"
#include <cmath>
#include <mpi.h>
#include <armadillo>

#include "Box.hpp"  // temp

namespace fs = std::filesystem;

using PointPair = std::pair<Point, Point>;

Lpoint centroid(const std::vector<Lpoint*>& points)
{
    double x = 0, y = 0, z = 0;
    for (const Lpoint* p : points)
    {
        x += p->getX();
        y += p->getY();
        z += p->getZ();
    }
    return Lpoint(x / points.size(), y / points.size(), z / points.size());
}

arma::mat cov(const std::vector<Lpoint*>& points)
{
    auto c = centroid(points);
    arma::mat cov(3, 3, arma::fill::zeros);
    for (const Lpoint* p : points)
    {
        arma::vec v = { p->getX() - c.getX(), p->getY() - c.getY(), p->getZ() - c.getZ() };
        cov += v * v.t();
    }
    return cov / points.size();
}

int main(int argc, char* argv[])
{
	setDefaults();
	processArgs(argc, argv);

	fs::path    inputFile = mainOptions.inputFile;
	std::string fileName  = inputFile.stem();

	if (!mainOptions.outputDirName.empty()) { mainOptions.outputDirName = mainOptions.outputDirName / fileName; }

	// Handling Directories
	createDirectory(mainOptions.outputDirName);

	// Print three decimals
	std::cout << std::fixed;
	std::cout << std::setprecision(3);

    // MPI init
    int node = 0, npes = 1;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &node);

    // Boxes generation
    std::vector<PointPair> boxes;
    if (node == 0)
    {
        boxes = genBoxes(getBoundingBox(mainOptions.inputFile), npes);
    }

    PointPair minMax;
    MPI_Datatype pointType = createPointType();
    MPI_Scatter(boxes.data(), 2, pointType, &minMax, 2, pointType, 0, MPI_COMM_WORLD);

    Point radius(mainOptions.radius, mainOptions.radius, mainOptions.radius);
    Box b(minMax);
    Box overlap(PointPair{ minMax.first - radius, minMax.second + radius });

    // Read point cloud
    TimeWatcher tw;
    tw.start();
    std::vector<Lpoint> lPoints = readPointCloudOverlap(mainOptions.inputFile, b, overlap);
    tw.stop();

    unsigned int numOvl = 0;
    for (const Lpoint& p : lPoints) {
        if (p.isOverlap()) { numOvl++; }
    }

    std::cout << "Node " << node << " Number of read points: " << lPoints.size() << "\n"
        << "    Number of overlapping points: " << numOvl << "\n"
        << "    Number of unique points: " << lPoints.size() - numOvl << "\n"
        << "    Time to read points: " << tw.getElapsedDecimalSeconds() << " seconds\n"
        << "    Box dimensions: " << minMax.first << ", " << minMax.second << "\n\n";

    MPI_Barrier(MPI_COMM_WORLD);

    // Local Octree Creation
    tw.start();
    Octree lOctree(lPoints);
    tw.stop();
    std::cout << "Node " << node << " Time to build local octree: " << tw.getElapsedDecimalSeconds() << " seconds\n";

    unsigned totalPoints, totalOvl;
    unsigned localPoints = lPoints.size();
    MPI_Reduce(&localPoints, &totalPoints, 1, MPI_UNSIGNED, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&numOvl, &totalOvl, 1, MPI_UNSIGNED, MPI_SUM, 0, MPI_COMM_WORLD);

    tw.start();
    double localNeigh = 0;
    double sumEigVal = 0;
    double omnivariance = 0;
    for (const Lpoint& p : lPoints)
    {
        if (p.isOverlap()) { continue; }
        auto neigh = lOctree.searchNeighbors3D(p, mainOptions.radius);

        // get eigenvalues
        auto A = cov(neigh);
        arma::cx_vec c_eigval;
        arma::eig_gen(c_eigval, A);

        // sort eigval and discard complex part
        // doing it as arma::real(arma::sort(c_eigval)) raises an error
        c_eigval = arma::sort(c_eigval);
        arma::vec eigval = arma::real(c_eigval);

        // some points had eigenvalues as [matrix size: 0x1]
        if (eigval.n_rows != 3) { continue; }

        sumEigVal += accu(eigval);
        double lOmni = pow((eigval[0] * eigval[1] * eigval[2]), 1.0 / 3.0);
        // else, result is -nan always
        if (std::isnan(lOmni)) { continue; }
        omnivariance += lOmni;
        localNeigh += neigh.size();
    }
    tw.stop();

    std::cout << "Node " << node << " Density for radius " << mainOptions.radius << ": " << localNeigh / (lPoints.size() - numOvl) << "\n"
        << "    Mean sum eigenvalues: " << sumEigVal / (lPoints.size() - numOvl) << "\n"
        << "    Mean omnivariance: " << omnivariance / (lPoints.size() - numOvl) << "\n"
        << "    Time to calculate density: " << tw.getElapsedDecimalSeconds() << " seconds\n\n";

    double totNeigh = 0;
    MPI_Reduce(&localNeigh, &totNeigh, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    double totSumEigVal = 0;
    MPI_Reduce(&sumEigVal, &totSumEigVal, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    double totOmni = 0;
    MPI_Reduce(&omnivariance, &totOmni, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (node == 0)
    {
        std::cout << "Density for radius " << mainOptions.radius << ": " << totNeigh / (totalPoints - totalOvl) << "\n"
            << "Mean sum eigenvalues: " << totSumEigVal / (totalPoints - totalOvl) << "\n"
            << "Mean omnivariance: " << totOmni / (totalPoints - totalOvl) << "\n";
    }

    // Sequential
    if (node == 0)
    {
        std::cout << "\nTotal number of read points: " << totalPoints << "\n"
            << "Total number of overlapping points: " << totalOvl << "\n"
            << "Total number of unique points: " << totalPoints - totalOvl << "\n";

        std::cout << "\n-------------------------------------------------\n\n";

        tw.start();
        std::vector<Lpoint> points = readPointCloud(mainOptions.inputFile);
        tw.stop();
        std::cout << "Number of read points: " << points.size() << "\n";
        std::cout << "    Time to read points: " << tw.getElapsedDecimalSeconds() << " seconds\n";

        // Global Octree Creation
        std::cout << "Building global octree..." << "\n";
        tw.start();
        Octree gOctree(points);
        tw.stop();
        std::cout << "    Time to build global octree: " << tw.getElapsedDecimalSeconds() << " seconds\n";

        // Density
        tw.start();
        unsigned long totNeigh = 0;
        for (const Lpoint& p : points)
        {
            totNeigh += gOctree.searchNeighbors3D(p, mainOptions.radius).size();
        }
        tw.stop();
        std::cout << "Density for radius " << mainOptions.radius << ": " << static_cast<double>(totNeigh) / points.size() << "\n"
            << "    Time to calculate density: " << tw.getElapsedDecimalSeconds() << " seconds\n";
    }

    MPI_Finalize();

	return EXIT_SUCCESS;
}
