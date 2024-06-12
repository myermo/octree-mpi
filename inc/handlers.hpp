//
// Created by miguelyermo on 1/3/20.
//

/*
* FILENAME :  handlers.h  
* PROJECT  :  rule-based-classifier-cpp
* DESCRIPTION :
*  
*
*
*
*
* AUTHOR :    Miguel Yermo        START DATE : 03:07 1/3/20
*
*/

#ifndef CPP_HANDLERS_H
#define CPP_HANDLERS_H

#include "main_options.hpp"
#include "point.hpp"
#include "Lpoint.hpp"
#include "Box.hpp"
#include <filesystem> // File extensions
#include <mpi.h>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using PointPair = std::pair<Point, Point>;

void handleNumberOfPoints(std::vector<Lpoint>& points);

unsigned int getNumberOfCols(const fs::path& filePath);

void createDirectory(const fs::path& dirname);

std::vector<Lpoint> readPointCloudOverlap(const fs::path& fileName, const Box& box, const Box& overlap);

std::vector<Lpoint> readPointCloud(const fs::path& fileName);

PointPair getBoundingBox(const fs::path& filename);

std::vector<PointPair> genBoxes(const PointPair& minMax, int n);

MPI_Datatype createPointType();

#endif //CPP_HANDLERS_H
