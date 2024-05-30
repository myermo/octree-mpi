//
// Created by miguelyermo on 6/8/21.
//

#pragma once

#include "Box.hpp"
#include "FileReader.hpp"
#include "Lpoint.hpp"
#include <lasreader.hpp>

/**
 * @author Miguel Yermo
 * @brief Specialization of FileRead to read .las/.laz files
 */
class LasFileReader : public FileReader
{
	public:
	// ***  CONSTRUCTION / DESTRUCTION  *** //
	// ************************************ //
	LasFileReader(const fs::path& path) : FileReader(path){};
	~LasFileReader(){};

	/**
	 * @brief Reads the points contained in the .las/.laz file
	 * @return Vector of Lpoint
	 */
	std::vector<Lpoint> read();

    /**
     * @brief Reads the points contained in the .las/.laz file that are inside the box and overlap
     * @return Vector of Lpoint
     */
    std::vector<Lpoint> readOverlap(const Box& box, const Box& overlap);

    /**
     * @brief Get the minimum and maximum points of the point cloud
     * @return Pair of Point
     */
    std::pair<Point, Point> getMinMax();
};
