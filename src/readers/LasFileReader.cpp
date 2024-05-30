//
// Created by miguelyermo on 6/8/21.
//

#include "LasFileReader.hpp"
#include "main_options.hpp"

std::vector<Lpoint> LasFileReader::read()
{
	std::vector<Lpoint> points;

	// LAS File reading
	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(path.c_str());
	LASreader* lasreader = lasreadopener.open();

	// TODO: Use extended_point_records para LAS v1.4
	// https://gitlab.citius.usc.es/oscar.garcia/las-shapefile-classifier/-/blob/master/lasreader_wrapper.cc

	// Scale factors for each coordinate
	double xScale = lasreader->header.x_scale_factor;
	double yScale = lasreader->header.y_scale_factor;
	double zScale = lasreader->header.z_scale_factor;

	double xOffset = lasreader->header.x_offset;
	double yOffset = lasreader->header.y_offset;
	double zOffset = lasreader->header.z_offset;

	// Index of the read point
	unsigned int idx = 0;

	// Main bucle
	while (lasreader->read_point())
	{
		points.emplace_back(idx++, static_cast<double>(lasreader->point.get_X() * xScale + xOffset),
		                    static_cast<double>(lasreader->point.get_Y() * yScale + yOffset),
		                    static_cast<double>(lasreader->point.get_Z() * zScale + zOffset),
		                    static_cast<double>(lasreader->point.get_intensity()),
		                    static_cast<unsigned short>(lasreader->point.get_return_number()),
		                    static_cast<unsigned short>(lasreader->point.get_number_of_returns()),
		                    static_cast<unsigned short>(lasreader->point.get_scan_direction_flag()),
		                    static_cast<unsigned short>(lasreader->point.get_edge_of_flight_line()),
		                    static_cast<unsigned short>(lasreader->point.get_classification()),
		                    static_cast<char>(lasreader->point.get_scan_angle_rank()),
		                    static_cast<unsigned short>(lasreader->point.get_user_data()),
		                    static_cast<unsigned short>(lasreader->point.get_point_source_ID()),
		                    static_cast<unsigned int>(lasreader->point.get_R()),
		                    static_cast<unsigned int>(lasreader->point.get_G()),
		                    static_cast<unsigned int>(lasreader->point.get_B()));
	}

	delete lasreader;
	return points;
}

std::vector<Lpoint> LasFileReader::readOverlap(const Box& box, const Box& overlap)
{
	std::vector<Lpoint> points;

	// LAS File reading
	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(path.c_str());
	LASreader* lasreader = lasreadopener.open();

	// TODO: Use extended_point_records para LAS v1.4
	// https://gitlab.citius.usc.es/oscar.garcia/las-shapefile-classifier/-/blob/master/lasreader_wrapper.cc

	// Scale factors for each coordinate
	double xScale = lasreader->header.x_scale_factor;
	double yScale = lasreader->header.y_scale_factor;
	double zScale = lasreader->header.z_scale_factor;

	double xOffset = lasreader->header.x_offset;
	double yOffset = lasreader->header.y_offset;
	double zOffset = lasreader->header.z_offset;

	// Index of the read point
	unsigned int idx = 0;

	// Main bucle
	while (lasreader->read_point())
	{
        Point p {static_cast<double>(lasreader->point.get_X() * xScale + xOffset),
                 static_cast<double>(lasreader->point.get_Y() * yScale + yOffset),
                 static_cast<double>(lasreader->point.get_Z() * zScale + zOffset)};
        if (overlap.isInside(p))
        {
            points.emplace_back(idx, static_cast<double>(lasreader->point.get_X() * xScale + xOffset),
                                static_cast<double>(lasreader->point.get_Y() * yScale + yOffset),
                                static_cast<double>(lasreader->point.get_Z() * zScale + zOffset),
                                static_cast<double>(lasreader->point.get_intensity()),
                                static_cast<unsigned short>(lasreader->point.get_return_number()),
                                static_cast<unsigned short>(lasreader->point.get_number_of_returns()),
                                static_cast<unsigned short>(lasreader->point.get_scan_direction_flag()),
                                static_cast<unsigned short>(lasreader->point.get_edge_of_flight_line()),
                                static_cast<unsigned short>(lasreader->point.get_classification()),
                                static_cast<char>(lasreader->point.get_scan_angle_rank()),
                                static_cast<unsigned short>(lasreader->point.get_user_data()),
                                static_cast<unsigned short>(lasreader->point.get_point_source_ID()),
                                static_cast<unsigned int>(lasreader->point.get_R()),
                                static_cast<unsigned int>(lasreader->point.get_G()),
                                static_cast<unsigned int>(lasreader->point.get_B()));
            points.back().setOverlap(!box.isInside(p));
        }
        idx++;

	}

	delete lasreader;
	return points;
}

std::pair<Point, Point> LasFileReader::getMinMax()
{
    // LAS File reading
	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(path.c_str());
	LASreader* lasreader = lasreadopener.open();

    Point min {lasreader->get_min_x(), lasreader->get_min_y(), lasreader->get_min_z()};
    Point max {lasreader->get_max_x(), lasreader->get_max_y(), lasreader->get_max_z()};

    delete lasreader;
    return std::make_pair(min, max);
}
