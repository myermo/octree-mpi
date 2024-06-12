//
// Created by miguelyermo on 1/3/20.
//

#include "handlers.hpp"
#include "FileReaderFactory.hpp"
#include "TxtFileReader.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <lasreader.hpp>
#include <queue>
#include <random>

#include "Box.hpp"

void createDirectory(const fs::path& dirName)
/**
 * This function creates a directory if it does not exist.
 * @param dirname
 * @return
 */
{
	if (!fs::is_directory(dirName)) { fs::create_directories(dirName); }
}

void writePoints(fs::path& filename, std::vector<Lpoint>& points)
{
	std::ofstream f(filename);
	f << std::fixed << std::setprecision(2);

	for (Lpoint& p : points)
	{
		f << p << "\n";
	}

	f.close();
}

std::vector<Lpoint> readPointCloudOverlap(const fs::path& filename, const Box& box, const Box& overlap)
{
    auto fExt = filename.extension();

    FileReader_t readerType = chooseReaderType(fExt);

    if (readerType == err_t || readerType == txt_t)
    {
        std::cout << "Uncompatible file format\n";
        exit(-1);
    }

    std::shared_ptr<FileReader> fileReader = FileReaderFactory::makeReader(readerType, filename);

    std::vector<Lpoint> points = fileReader->readOverlap(box, overlap);

    return points;
}

std::vector<Lpoint> readPointCloud(const fs::path& filename)
{
	// Get Input File extension
	auto fExt = filename.extension();

	FileReader_t readerType = chooseReaderType(fExt);

	// asdf
	if (readerType == err_t)
	{
		std::cout << "Uncompatible file format\n";
		exit(-1);
	}

	std::shared_ptr<FileReader> fileReader = FileReaderFactory::makeReader(readerType, filename);

	std::vector<Lpoint> points = fileReader->read();
	// Decimation. Implemented here because, tbh, I don't want to implement it for each reader type.
	std::cout << "Point cloud size: " << points.size() << "\n";

	return points;
}

PointPair getBoundingBox(const fs::path& filename)
{
    auto fExt = filename.extension();

    FileReader_t readerType = chooseReaderType(fExt);

    // TODO: support for txt files
    if (readerType == err_t || readerType == txt_t)
    {
        std::cout << "Uncompatible file format\n";
        exit(-1);
    }

    std::shared_ptr<FileReader> fileReader = FileReaderFactory::makeReader(readerType, filename);

    auto minMax = fileReader->getMinMax();

    std::cout << "Min: " << minMax.first << " Max: " << minMax.second << "\n";
    std::cout << "X: " << minMax.second.getX() - minMax.first.getX();
    std::cout << " Y: " << minMax.second.getY() - minMax.first.getY();
    std::cout << " Z: " << minMax.second.getZ() - minMax.first.getZ() << "\n";

    return minMax;
}

std::vector<PointPair> genBoxes(const PointPair& minMax, int n)
{
    const double epsilon = 0.000000001;
    // FIFO (biggest is always first)
    std::queue<PointPair> boxes;
    // better solution to this would be not using epsilon and instead passing to
    // isInside() some kind of option telling if that specific box shares sides with
    // the min of the main box or not, and if it does do a <=
    // and do a >= to everyone
    // TODO: do this^, as current implementation is flawed
    boxes.emplace(minMax.first - (epsilon, epsilon, epsilon), minMax.second);

    for (int i = n - 1; i > 0; i--)
    {
        PointPair b = boxes.front();
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
    std::vector<PointPair> ret;
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
