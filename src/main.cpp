#include "handlers.hpp"
#include "main_options.hpp"
#include "octree.hpp"
#include <filesystem> // Only C++17 and beyond
#include <iomanip>
#include <iostream>
#include "TimeWatcher.hpp"
#include <cmath>
#include <mpi.h>

#include "Box.hpp"  // temp

namespace fs = std::filesystem;

/* using Pair = std::pair<Point, Point>; */

// TODO: typedef std::pair<Point, Point>, maybe
// move to handles.cpp, maybe
std::vector<std::pair<Point, Point>> genBoxes(const std::pair<Point, Point>& minMax, int n)
{
    const double epsilon = 0.000000001;
    // FIFO (biggest is always first)
    std::queue<std::pair<Point, Point>> boxes;
    // better solution to this would be not using epsilon and instead passing to
    // isInside() some kind of option telling if that specific box shares sides with
    // the min of the main box or not, and if it does do a <=
    // and do a >= to everyone
    // TODO: do this^, as current implementation is flawed
    boxes.emplace(minMax.first - (epsilon, epsilon, epsilon), minMax.second);

    for (int i = n - 1; i > 0; i--)
    {
        std::pair<Point, Point> b = boxes.front();
        boxes.pop();

        // get max length and find max axis
        double max_l = std::max({ b.second.getX() - b.first.getX(), b.second.getY() - b.first.getY(), b.second.getZ() - b.first.getZ() });
        char axis = 'z';
        if (fabs(max_l - (b.second.getX() - b.first.getX())) < epsilon) { axis = 'x'; }
        else if (fabs(max_l - (b.second.getY() - b.first.getY())) < epsilon) { axis = 'y'; }

        // times 2 for split/3
        bool last = i == 2 && n % 2 == 1;
        double split = (last) ? max_l / 3.0 : max_l / 2.0;
        int mult = (last) ? 2 : 1;

        if (axis == 'x')
        {
            boxes.emplace(b.first, Point(b.first.getX() + split, b.second.getY(), b.second.getZ()));
            if (last) { boxes.emplace(Point(b.first.getX() + split, b.first.getY(), b.first.getZ()), Point(b.first.getX() + mult * split, b.second.getY(), b.second.getZ())); }
            boxes.emplace(Point(b.first.getX() + mult * split, b.first.getY(), b.first.getZ()), b.second);
        } else if (axis == 'y')
        {
            boxes.emplace(b.first, Point(b.second.getX(), b.first.getY() + split, b.second.getZ()));
            if (last) { boxes.emplace(Point(b.first.getX(), b.first.getY() + split, b.first.getZ()), Point(b.second.getX(), b.first.getY() + mult * split, b.second.getZ())); }
            boxes.emplace(Point(b.first.getX(), b.first.getY() + mult * split, b.first.getZ()), b.second);
        } else
        {
            boxes.emplace(b.first, Point(b.second.getX(), b.second.getY(), b.first.getZ() + split));
            if (last) { boxes.emplace(Point(b.first.getX(), b.first.getY(), b.first.getZ() + split), Point(b.second.getX(), b.second.getY(), b.first.getZ() + mult * split)); }
            boxes.emplace(Point(b.first.getX(), b.first.getY(), b.first.getZ() + mult * split), b.second);
        }

        if (last) { break; }
    }

    // convert to vector (contiguous in memory, needed for scatter)
    std::vector<std::pair<Point, Point>> ret;
    while (!boxes.empty())
    {
        ret.emplace_back(boxes.front().first, boxes.front().second + (epsilon, epsilon, epsilon));
        boxes.pop();
    }

    return ret;
}

// creates MPI type for points
MPI_Datatype createPointType()
{
    MPI_Datatype pointType;
    MPI_Datatype types[4] = { MPI_UNSIGNED, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE };
    int blockLengths[4] = { 1, 1, 1, 1 };

    // there's probably a cleaner way
    MPI_Aint offsets[4];
    std::array<size_t, 4> off = Point::getOffsets();

    for (int i = 0; i < 4; i++) { offsets[i] = off[i]; }

    MPI_Type_create_struct(4, blockLengths, offsets, types, &pointType);
    MPI_Type_commit(&pointType);

    return pointType;
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
    std::vector<std::pair<Point, Point>> boxes;
    if (node == 0)
    {
        boxes = genBoxes(getBoundingBox(mainOptions.inputFile), npes);
    }

    std::pair<Point, Point> minMax;
    MPI_Datatype pointType = createPointType();
    MPI_Scatter(boxes.data(), 2, pointType, &minMax, 2, pointType, 0, MPI_COMM_WORLD);

    Point radius(mainOptions.radius, mainOptions.radius, mainOptions.radius);
    Box b(minMax);
    Box overlap(std::pair<Point, Point>{ minMax.first - radius, minMax.second + radius });

    // Read point cloud
    TimeWatcher tw;
    tw.start();
    std::vector<Lpoint> l_points = readPointCloudOverlap(mainOptions.inputFile, b, overlap);
    tw.stop();

    unsigned int numOvl = 0;
    for (const Lpoint& p : l_points) {
        if (p.isOverlap()) { numOvl++; }
    }

    std::cout << "Node " << node << " Number of read points: " << l_points.size() << "\n"
        << "    Number of overlapping points: " << numOvl << "\n"
        << "    Number of unique points: " << l_points.size() - numOvl << "\n"
        << "    Time to read points: " << tw.getElapsedDecimalSeconds() << " seconds\n"
        << "    Box dimensions: " << minMax.first << ", " << minMax.second << "\n\n";

    MPI_Barrier(MPI_COMM_WORLD);

    // Local Octree Creation
    tw.start();
    Octree lOctree(l_points);
    tw.stop();
    std::cout << "Node " << node << " Time to build local octree: " << tw.getElapsedDecimalSeconds() << " seconds\n";

    unsigned totalPoints, totalOvl;
    unsigned localPoints = l_points.size();
    MPI_Reduce(&localPoints, &totalPoints, 1, MPI_UNSIGNED, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&numOvl, &totalOvl, 1, MPI_UNSIGNED, MPI_SUM, 0, MPI_COMM_WORLD);

    tw.start();
    double localNeigh = 0;
    for (const Lpoint& p : l_points)
    {
        if (p.isOverlap()) { continue; }
        localNeigh += lOctree.searchNeighbors3D(p, mainOptions.radius).size();
    }
    tw.stop();

    std::cout << "Node " << node << " Density for radius " << mainOptions.radius << ": " << localNeigh / (l_points.size() - numOvl) << "\n"
        << "    Time to calculate density: " << tw.getElapsedDecimalSeconds() << " seconds\n";

    double totNeigh = 0;
    MPI_Reduce(&localNeigh, &totNeigh, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (node == 0)
    {
        std::cout << "Density for radius " << mainOptions.radius << ": " << totNeigh / (totalPoints - totalOvl) << "\n";
    }

    // Sequential
    if (node == 0)
    {
        // sort globalIds
        /* std::sort(globalIds.begin(), globalIds.end()); */

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
