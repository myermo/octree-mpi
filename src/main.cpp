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
#include <omp.h>

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
    return {x / points.size(), y / points.size(), z / points.size()};
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

std::vector<double> features(const std::vector<Lpoint*>& neigh, const Lpoint& p) {
        std::vector<double> feats(18);

        // get eigenvalues
        auto A = cov(neigh);
        arma::vec eigval;
        arma::mat eigvec;
        arma::eig_sym(eigval, eigvec, A);
        arma::vec z = { 0, 0, 1 };

        double neighInv = 1.0 / neigh.size();

        feats[0] = neigh.size();                                                 // number of neighbors
        feats[1] = accu(eigval);                                                 // sum eigenvalues
        feats[2] = pow(fabs((eigval[0] * eigval[1] * eigval[2])), 1.0 / 3.0);    // omnivariance
        feats[3] = -accu(eigval % arma::log(eigval));                            // eigenentropy
        feats[4] = (eigval[2] - eigval[1]) / eigval[2];                          // linearity
        feats[5] = (eigval[1] - eigval[0]) / eigval[2];                          // planarity
        feats[6] = eigval[0] / eigval[2];                                        // sphericity
        feats[7] = eigval[0] / (eigval[0] + eigval[1] + eigval[2]);              // change of curvature
        // verticality
        feats[8] = fabs(M_PI / 2 - acos(arma::dot(eigvec.col(0), z) / sqrt(arma::dot(eigvec.col(0), eigvec.col(0)) * arma::dot(z, z))));
        feats[9] = fabs(M_PI / 2 - acos(arma::dot(eigvec.col(2), z) / sqrt(arma::dot(eigvec.col(2), eigvec.col(2)) * arma::dot(z, z))));
        // TODO:
        // absolute moment 1 - 6, vertical moment 1 - 2
        // https://isprs-annals.copernicus.org/articles/III-3/177/2016/isprs-annals-III-3-177-2016.pdf
        std::vector<double> abs(6);
        std::vector<double> vert(2);
        for (const Lpoint* n : neigh)
        {
            arma::vec v = { n->getX() - p.getX(), n->getY() - p.getY(), n->getZ() - p.getZ() };
            abs[0] += arma::dot(v, eigvec.col(0));
            abs[1] += pow(arma::dot(v, eigvec.col(0)), 2);
            abs[2] += arma::dot(v, eigvec.col(1));
            abs[3] += pow(arma::dot(v, eigvec.col(1)), 2);
            abs[4] += arma::dot(v, eigvec.col(2));
            abs[5] += pow(arma::dot(v, eigvec.col(2)), 2);
            vert[0] += arma::dot(v, z);
            vert[1] += pow(arma::dot(v, z), 2);
        }
        feats[10] = neighInv * fabs(abs[0]);
        feats[11] = neighInv * fabs(abs[1]);
        feats[12] = neighInv * fabs(abs[2]);
        feats[13] = neighInv * fabs(abs[3]);
        feats[14] = neighInv * fabs(abs[4]);
        feats[15] = neighInv * fabs(abs[5]);
        feats[16] = neighInv * vert[0];
        feats[17] = neighInv * vert[1];

        return feats;
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
    mainOptions.outputDirName = mainOptions.outputDirName / (fileName + ".txt");

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
    std::string deb_str;
    if (node == 0)
    {
        boxes = genBoxes(getBoundingBox(mainOptions.inputFile), npes);
        deb_str = std::to_string(npes) + ", " + std::to_string(omp_get_max_threads()) + ", " 
            + mainOptions.inputFile.filename().c_str() + ", " + std::to_string(mainOptions.radius) + ", ";
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
    double localTime = tw.getElapsedDecimalSeconds();
    double meanTime = 0;
    MPI_Reduce(&localTime, &meanTime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (node == 0) { deb_str += std::to_string(meanTime / npes) + ", "; }

    unsigned int numOvl = 0;
    for (const Lpoint& p : lPoints) {
        if (p.isOverlap()) { numOvl++; }
    }

    std::cout << "Node " << node << " Number of read points: " << lPoints.size() << "\n"
        << "    Number of overlapping points: " << numOvl << "\n"
        << "    Number of unique points: " << lPoints.size() - numOvl << "\n"
        << "    Time to read points: " << localTime << " seconds\n"
        << "    Box dimensions: " << minMax.first << ", " << minMax.second << "\n\n";

    MPI_Barrier(MPI_COMM_WORLD);

    // Local Octree Creation
    tw.start();
    Octree lOctree(lPoints);
    tw.stop();
    localTime = tw.getElapsedDecimalSeconds();
    std::cout << "Node " << node << " Time to build local octree: " << localTime << " seconds\n";
    MPI_Reduce(&localTime, &meanTime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (node == 0) { deb_str += std::to_string(meanTime / npes) + ", "; }

    unsigned totalPoints, totalOvl;
    unsigned localPoints = lPoints.size();
    MPI_Reduce(&localPoints, &totalPoints, 1, MPI_UNSIGNED, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&numOvl, &totalOvl, 1, MPI_UNSIGNED, MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_File file;
    MPI_File_open(MPI_COMM_WORLD, mainOptions.outputDirName.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);
    // header for txt file (helps in CloudCompare)
    if (node == 0) {
        std::string str = "x y z node numNeigh sumEig omnivariance eigenentropy linearity planarity sphericity changeCurvature "
            "verticality0 verticality2 absMom0 absMom0^2 absMom1 absMom1^2 absMom2 absMom2^2 vertMom vertMom^2\n";
        MPI_File_write_shared(file, str.c_str(), str.size(), MPI_CHAR, MPI_STATUS_IGNORE);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    tw.start();
    double localNeigh = 0;
    int count = 0;
    std::string str;
    #pragma omp parallel reduction(+:localNeigh) private(count, str)
    {
        #pragma omp for
        for (const Lpoint& p : lPoints)
        {
            if (p.isOverlap()) { continue; }
            auto neigh = lOctree.searchNeighbors3D(p, mainOptions.radius);
            if (neigh.size() == 0) { continue; }

            auto feats = features(neigh, p);

            localNeigh += neigh.size();

            // print coordinates and other info to file "out.txt"
            str += std::to_string(p.getX()) + " " + std::to_string(p.getY()) + " " + std::to_string(p.getZ()) + " " + std::to_string(node);
            for (double f : feats) { str += " " + std::to_string(f); }
            str += "\n";
            count++;

            if (count == 500) {
                #pragma omp critical
                MPI_File_write_shared(file, str.c_str(), str.size(), MPI_CHAR, MPI_STATUS_IGNORE);
                str.clear();
                count = 0;
            }
        }
        #pragma omp critical
        MPI_File_write_shared(file, str.c_str(), str.size(), MPI_CHAR, MPI_STATUS_IGNORE);
        str.clear();
    }
    tw.stop();

    MPI_File_close(&file);

    localTime = tw.getElapsedDecimalSeconds();
    std::cout << "Node " << node << " Density for radius " << mainOptions.radius << ": " << localNeigh / (lPoints.size() - numOvl) << "\n"
        << "    Time to calculate features: " << localTime << " seconds\n\n";

    double totNeigh = 0;
    MPI_Reduce(&localNeigh, &totNeigh, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&localTime, &meanTime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (node == 0)
    {
        std::cout << "Density for radius " << mainOptions.radius << ": " << totNeigh / (totalPoints - totalOvl) << "\n"
            << "    Mean time to calculate features: " << meanTime / npes << " seconds\n";
        deb_str += std::to_string(meanTime / npes) + "\n";
        std::fstream deb(mainOptions.debugFile, std::fstream::app);
        deb << deb_str;
        deb.close();
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

        #pragma omp parallel for reduction(+:totNeigh)
        for (const Lpoint& p : points)
        {
            auto neigh = gOctree.searchNeighbors3D(p, mainOptions.radius);
            if (neigh.size() == 0) { continue; }

            auto feats = features(neigh, p);

            totNeigh += neigh.size();
        }
        tw.stop();
        std::cout << "Density for radius " << mainOptions.radius << ": " << static_cast<double>(totNeigh) / points.size() << "\n"
            << "    Time to calculate features: " << tw.getElapsedDecimalSeconds() << " seconds\n";
    }

    MPI_Finalize();

	return EXIT_SUCCESS;
}
