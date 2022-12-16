#pragma once
#include "geo.h"
#include "svg.h"
#include <string>
#include <vector>

namespace Catalogue {
	struct Stop {
		std::string name;
		Geo::Coordinates coords{ 0,0 };
	};

	struct Bus {
		std::string name;
		std::vector<Stop*> stops;
		Stop* end_stop;
	};

	struct BusInfo {
		std::string name = "";
		bool is_not_found = false;
		std::size_t stops_number = 0;
		std::size_t unique_stops_number = 0;
		double length = 0.0;
		double curvature = 0.0;
	};

	struct StopInfo {
		std::string name = "";
		std::vector<std::string> bus_list;
		bool is_not_found = false;
	};
}
namespace Visual {
	struct MapSettings {
		double width = 0;
		double height = 0;
		double padding = 0;
		double line_width = 0;
		double stop_radius = 0;
		int bus_label_font_size = 0;
		std::pair<double, double> bus_label_offset = { 0,0 };
		int stop_label_font_size = 0;
		std::pair<double, double> stop_label_offset = { 0,0 };
		svg::Color underlayer_color{ std::string("white") };
		double underlayer_width = 0;
		std::string stop_label_font_family{ std::string("Verdana") };
		std::string bus_label_font_family{ std::string("Verdana") };
		std::vector < svg::Color> color_palette{ std::string("black") };
	};
}

namespace DataReader {
	//интерфейс для взаимодействия с данными
	class Reader {

	public:

		virtual void ReadBaseRequests() = 0;
		virtual void ReadStatRequests(std::ostream& output) = 0;
		virtual Visual::MapSettings ReadMapSettings() = 0;

	};
}