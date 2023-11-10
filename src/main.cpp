#include "handlers.hpp"
#include "main_options.hpp"
#include "octree.hpp"
#include <filesystem> // Only C++17 and beyond
#include <iomanip>
#include <iostream>
#include "TimeWatcher.hpp"

namespace fs = std::filesystem;

double density(const Octree* gOctree, double radius, const std::vector<Lpoint>* points)
{
    unsigned long tot_neigh = 0;

    for (Lpoint point : *points) {
        tot_neigh += gOctree->searchNeighbors3D(point, radius).size();
    }

    return tot_neigh/(double)points->size();
}

double density_2(const Octree* gOctree, double radius, const std::vector<Lpoint>* points)
{
    unsigned long tot_neigh = 0;

    for (Lpoint point : *points) {
        tot_neigh += gOctree->numNeighbors(&point, radius);  // nesta funcion da erro
    }

    return tot_neigh/(double)points->size();
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

	TimeWatcher tw;

	tw.start();
	std::vector<Lpoint> points = readPointCloud(mainOptions.inputFile);
	tw.stop();
	std::cout << "Number of read points: " << points.size() << "\n";
	std::cout << "Time to read points: " << tw.getElapsedDecimalSeconds() << " seconds\n";

	// Global Octree Creation
	std::cout << "Building global octree..." << std::endl;
	tw.start();
	Octree gOctree(points);
	tw.stop();
	std::cout << "Time to build global octree: " << tw.getElapsedDecimalSeconds() << " seconds\n";

    tw.start();
    double dense = density(&gOctree, 1, &points);
    tw.stop();
    std::cout << "density with a 1m radius: " << dense << std::endl;
	std::cout << "Time to get density: " << tw.getElapsedDecimalSeconds() << " seconds\n";

    tw.start();
    dense = density_2(&gOctree, 1, &points);
    tw.stop();
    std::cout << "density with a 1m radius: " << dense << std::endl;
	std::cout << "Time to get density: " << tw.getElapsedDecimalSeconds() << " seconds\n";

	return EXIT_SUCCESS;
}
