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

// TODO: typedef std::pair<Point, Point>, maybe
std::vector<std::pair<Point, Point>> genBoxes(const std::pair<Point, Point>& minMax, int n)
{
    // FIFO (biggest is always first)
    std::queue<std::pair<Point, Point>> boxes;
    boxes.emplace(minMax);

    for (int i = n - 1; i > 0; i--)
    {
        std::pair<Point, Point> b = boxes.front();
        boxes.pop();

        // get max length and find max axis
        double max_l = std::max({ b.second.getX() - b.first.getX(), b.second.getY() - b.first.getY(), b.second.getZ() - b.first.getZ() });
        char axis = 'z';
        if (fabs(max_l - (b.second.getX() - b.first.getX())) < 0.001) { axis = 'x'; }
        else if (fabs(max_l - (b.second.getY() - b.first.getY())) < 0.001) { axis = 'y'; }

        double split = max_l / 2.0;

        // split box
        if (i == 2 && n % 2 == 1) {
            // split in 3
            split = max_l / 3.0;
            if (axis == 'x')
            {
                boxes.emplace(b.first, Point(b.first.getX() + split, b.second.getY(), b.second.getZ()));
                boxes.emplace(Point(b.first.getX() + split, b.first.getY(), b.first.getZ()), Point(b.first.getX() + 2 * split, b.second.getY(), b.second.getZ()));
                boxes.emplace(Point(b.first.getX() + 2 * split, b.first.getY(), b.first.getZ()), b.second);
            } else if (axis == 'y')
            {
                boxes.emplace(b.first, Point(b.second.getX(), b.first.getY() + split, b.second.getZ()));
                boxes.emplace(Point(b.first.getX(), b.first.getY() + split, b.first.getZ()), Point(b.second.getX(), b.first.getY() + 2 * split, b.second.getZ()));
                boxes.emplace(Point(b.first.getX(), b.first.getY() + 2 * split, b.first.getZ()), b.second);
            } else if (axis == 'z')
            {
                boxes.emplace(b.first, Point(b.second.getX(), b.second.getY(), b.first.getZ() + split));
                boxes.emplace(Point(b.first.getX(), b.first.getY(), b.first.getZ() + split), Point(b.second.getX(), b.second.getY(), b.first.getZ() + 2 * split));
                boxes.emplace(Point(b.first.getX(), b.first.getY(), b.first.getZ() + 2 * split), b.second);
            }
            break;
        }

        // check if quicker (or tidier) option
        if (axis == 'x')
        {
            boxes.emplace(b.first, Point(b.first.getX() + split, b.second.getY(), b.second.getZ()));
            boxes.emplace(Point(b.first.getX() + split, b.first.getY(), b.first.getZ()), b.second);
        } else if (axis == 'y')
        {
            boxes.emplace(b.first, Point(b.second.getX(), b.first.getY() + split, b.second.getZ()));
            boxes.emplace(Point(b.first.getX(), b.first.getY() + split, b.first.getZ()), b.second);
        } else if (axis == 'z')
        {
            boxes.emplace(b.first, Point(b.second.getX(), b.second.getY(), b.first.getZ() + split));
            boxes.emplace(Point(b.first.getX(), b.first.getY(), b.first.getZ() + split), b.second);
        }
    }

    // convert to vector (contiguous in memory, needed for scatter)
    std::vector<std::pair<Point, Point>> ret;
    while (!boxes.empty())
    {
        ret.push_back(boxes.front());
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

    int node = 0, npes = 1;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &node);

    std::vector<std::pair<Point, Point>> boxes;
    if (node == 0)
    {
        std::cout << "npes: " << npes << "\n";

        boxes = genBoxes(getBoundingBox(mainOptions.inputFile), npes);
        std::cout << "Bounding boxes:\n";
        for (auto b : boxes) {
            std::cout << "Min: " << b.first << " Max: " << b.second << "\n";
        }
    }

    std::pair<Point, Point> minMax;
    MPI_Datatype pointType = createPointType();
    MPI_Scatter(boxes.data(), 2, pointType, &minMax, 2, pointType, 0, MPI_COMM_WORLD);
    std::cout << "Node " << node << " min: " << minMax.first << " max: " << minMax.second << "\n";

    // TODO: read points, octree construction

    if (node == 0)
    {
        TimeWatcher tw;

        tw.start();
        std::vector<Lpoint> points = readPointCloud(mainOptions.inputFile);
        tw.stop();
        std::cout << "Number of read points: " << points.size() << "\n";
        std::cout << "Time to read points: " << tw.getElapsedDecimalSeconds() << " seconds\n";

        // Global Octree Creation
        std::cout << "Building global octree..." << "\n";
        tw.start();
        Octree gOctree(points);
        tw.stop();
        std::cout << "Time to build global octree: " << tw.getElapsedDecimalSeconds() << " seconds\n";
    }

    MPI_Finalize();

	return EXIT_SUCCESS;
}
