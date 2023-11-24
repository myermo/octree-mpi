#include "handlers.hpp"
#include "main_options.hpp"
#include "octree.hpp"
#include <filesystem> // Only C++17 and beyond
#include <iomanip>
#include <iostream>
#include "TimeWatcher.hpp"
#include "NeighborKernels/KernelFactory.hpp"

namespace fs = std::filesystem;

double density_cube(const Octree& gOctree, double radius, const std::vector<Lpoint>& points)
{
    double tot_dens = 0;

    for (const Lpoint& point : points) {
        tot_dens += gOctree.searchNeighbors3D(point, radius).size()/std::pow(radius*2,3);
    }

    return tot_dens/points.size();
}

// con numNeighbors o tempo de cálculo é aprox. 80% que con searchNeighbors().size()
double density_cube_2(const Octree& gOctree, double radius, const std::vector<Lpoint>& points)
{
    double tot_dens = 0;

    for (const Lpoint& point : points) {
        tot_dens += gOctree.numNeighbors<Kernel_t::cube>(point, radius)/std::pow(radius*2,3);
    }

    return tot_dens/points.size();
}

double density_sphere(const Octree& gOctree, double radius, const std::vector<Lpoint>& points)
{
    double tot_dens = 0;

    for (const Lpoint& point : points) {
        tot_dens += gOctree.searchSphereNeighbors(point, radius).size()/(M_PI*std::pow(radius,2));
    }

    return tot_dens/points.size();
}

double density_square(const Octree& gOctree, double radius, const std::vector<Lpoint>& points)
{
    double tot_dens = 0;

    for (const Lpoint& point : points) {
        tot_dens += gOctree.searchNeighbors2D(point, radius).size()/std::pow(radius*2,3);
    }

    return tot_dens/points.size();
}

double density_square_2(const Octree& gOctree, double radius, const std::vector<Lpoint>& points)
{
    double tot_dens = 0;

    for (const Lpoint& point : points) {
        tot_dens += gOctree.numNeighbors<Kernel_t::square>(point, radius)/std::pow(radius*2,3);
    }

    return tot_dens/points.size();
}

void time_diff(const Octree& gOctree, double radius, const std::vector<Lpoint>& points)
{
    TimeWatcher tw;

    tw.start();
    std::vector<Lpoint *> search_result;
    for (const Lpoint& point : points) {
        search_result = gOctree.searchNeighbors3D(point, radius);
    }
    tw.stop();
    double time_search = tw.getElapsedDecimalSeconds();

    tw.start();
    unsigned long num_result;
    for (const Lpoint& point : points) {
        num_result = gOctree.numNeighbors<Kernel_t::cube>(point, radius);
    }
    tw.stop();
    double time_num = tw.getElapsedDecimalSeconds();

    std::cout << "Time to search neighbors: " << time_search << std::endl;
    std::cout << "Time to get num neighbors: " << time_num << std::endl;

    std::cout << search_result.size() - num_result << std::endl;
}

void tests(const Octree& gOctree, double radius, const std::vector<Lpoint>& points)
{
	TimeWatcher tw;
    double dense;
    std::cout << std::endl;

    tw.start();
    dense = density_cube(gOctree, radius, points);
    std::cout << "Cube density with a " << radius << "m radius: " << dense << std::endl;
    tw.stop();
	std::cout << "Time to get density: " << tw.getElapsedDecimalSeconds() << " seconds\n";

    tw.start();
    dense = density_cube_2(gOctree, radius, points);
    std::cout << "Cube density 2 with a " << radius << "m radius: " << dense << std::endl;
    tw.stop();
	std::cout << "Time to get density: " << tw.getElapsedDecimalSeconds() << " seconds\n";

    tw.start();
    dense = density_sphere(gOctree, radius, points);
    std::cout << "Sphere density with a "<< radius <<"m radius: " << dense << std::endl;
    tw.stop();
	std::cout << "Time to get density: " << tw.getElapsedDecimalSeconds() << " seconds\n";

    tw.start();
    dense = density_square(gOctree, radius, points);
    std::cout << "Square density with a "<< radius <<"m radius: " << dense << std::endl;
    tw.stop();
	std::cout << "Time to get density: " << tw.getElapsedDecimalSeconds() << " seconds\n";

    tw.start();
    dense = density_square_2(gOctree, radius, points);
    std::cout << "Square density 2 with a "<< radius <<"m radius: " << dense << std::endl;
    tw.stop();
	std::cout << "Time to get density: " << tw.getElapsedDecimalSeconds() << " seconds\n";
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

    tests(gOctree, .5, points);
    tests(gOctree, 1, points);
    tests(gOctree, 2, points);

    for (int i = 0; i < 5; i++) { time_diff(gOctree, i + 1, points); }

	return EXIT_SUCCESS;
}
