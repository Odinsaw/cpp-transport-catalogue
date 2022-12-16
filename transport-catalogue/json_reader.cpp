#include "json_reader.h"
#include <sstream>
#include <algorithm>

namespace DataReader {

	JsonReader::JsonReader(std::istream& input, std::unique_ptr<DataBaseInterface::RequestsHandler>&& handler)
		:input_document_(json::Load(input)), handler_(std::move(handler))
	{
		bool correct_root = std::holds_alternative<json::Dict>(input_document_.GetRoot().GetValue());
		assert(correct_root); //в корневом узле должен быть словарь {base_requests : ... , stat_requests : ...}
	}

	void JsonReader::ReadBaseRequests() {

		json::Dict root_dict = std::get<json::Dict>(input_document_.GetRoot().GetValue());

		if (root_dict.count("base_requests")) {

			bool correct_base_requests_queue = std::holds_alternative<json::Array>(root_dict.at("base_requests").GetValue());
			assert(correct_base_requests_queue); //в узле base_requests должен быть вектор запросов
			json::Array base_requests_queue = std::get<json::Array>(root_dict.at("base_requests").GetValue());

			for (const json::Node& node : base_requests_queue) {

				bool correct_base_request = std::holds_alternative<json::Dict>(node.GetValue());
				assert(correct_base_request); //запрос должен быть в виде словаря
				json::Dict base_request = std::get<json::Dict>(node.GetValue());

				assert(base_request.count("type")); //запрос должен содержать тип
				assert(std::holds_alternative<std::string>(base_request.at("type").GetValue())); //тип в виде строки
				std::string base_request_type = std::get<std::string>(base_request.at("type").GetValue());

				if (base_request_type == "Stop") {

					auto request_parsed = ReadStopBaseRequest(base_request);
					handler_->DoStopBaseRequest(std::move(request_parsed.first), std::move(request_parsed.second));
				} //первая проходка только по остановкам
			}

			for (const json::Node& node : base_requests_queue) {

				bool correct_base_request = std::holds_alternative<json::Dict>(node.GetValue());
				assert(correct_base_request); //запрос должен быть в виде словаря
				json::Dict base_request = std::get<json::Dict>(node.GetValue());

				assert(base_request.count("type")); //запрос должен содержать тип
				assert(std::holds_alternative<std::string>(base_request.at("type").GetValue())); //тип в виде строки
				std::string base_request_type = std::get<std::string>(base_request.at("type").GetValue());

				if (base_request_type == "Bus") {
					auto request_parsed = ReadBusBaseRequest(base_request);
					handler_->DoBusBaseRequest(std::move(std::get<0>(request_parsed)), std::move(std::get<1>(request_parsed)), std::move(std::get<2>(request_parsed)));
				}
				else if (base_request_type == "Stop") {
					auto request_parsed = ReadStopBaseRequest(base_request);
					handler_->DoStopBaseRequest(std::move(request_parsed.first), std::move(request_parsed.second));
				}
			} //вторая проходка по остановкам и маршрутам

		}

	}

	std::tuple<std::string, std::vector<std::string>, std::string> JsonReader::ReadBusBaseRequest(const json::Dict base_request) {

		assert(base_request.count("name") && base_request.count("stops") && base_request.count("is_roundtrip")); //запрос содержи все нужные поля
		assert(std::holds_alternative<std::string>(base_request.at("name").GetValue())); //имя в виде строки
		assert(std::holds_alternative<json::Array>(base_request.at("stops").GetValue())); //вектор остановок
		assert(std::holds_alternative<bool>(base_request.at("is_roundtrip").GetValue())); //круговой

		std::string bus_name = std::get<std::string>(base_request.at("name").GetValue());
		json::Array stops = std::get<json::Array>(base_request.at("stops").GetValue());
		bool is_round = std::get<bool>(base_request.at("is_roundtrip").GetValue());

		std::vector<std::string> stop_list;

		std::string end_stop = ""s;
		if (!stops.empty()) {
			end_stop = stops.back().AsString();
		}

		for (const json::Node& stop : stops) {

			assert(std::holds_alternative<std::string>(stop.GetValue())); //остановка в виде строки
			stop_list.push_back(std::get<std::string>(stop.GetValue()));
		}

		if (!is_round && stop_list.size() > 1) {
			std::vector<std::string> r(stop_list.begin(), stop_list.end());
			r.pop_back();
			std::reverse(r.begin(), r.end());
			stop_list.insert(stop_list.end(), r.begin(), r.end());
		}

		return { bus_name, stop_list, end_stop };

	}

	std::pair<Catalogue::Stop, Catalogue::BusToDistance> JsonReader::ReadStopBaseRequest(const json::Dict base_request) {
		assert(base_request.count("name") && base_request.count("latitude") && base_request.count("longitude")); //запрос содержи все обязательные поля
		assert(std::holds_alternative<std::string>(base_request.at("name").GetValue())); //имя в виде строки
		assert(base_request.at("latitude").IsDouble()); //первая координата числом
		assert(base_request.at("longitude").IsDouble()); //вторая координата числом

		std::string stop_name = std::get<std::string>(base_request.at("name").GetValue());
		double x_coord = base_request.at("latitude").AsDouble();
		double y_coord = base_request.at("longitude").AsDouble();

		Catalogue::Stop stop{ std::move(stop_name), Geo::Coordinates{ x_coord, y_coord} };

		if (!base_request.count("road_distances")) {
			return { std::move(stop), {} };
		}
		Catalogue::BusToDistance bus_to_distance_map;

		assert(std::holds_alternative<json::Dict>(base_request.at("road_distances").GetValue())); //расстояния в виде словаря
		json::Dict distances_json = std::get<json::Dict>(base_request.at("road_distances").GetValue());

		for (const std::pair<std::string, json::Node>& val : distances_json) {
			assert(val.second.IsDouble()); //расстояние хранится числом
			bus_to_distance_map[val.first] = val.second.AsDouble();
		}
		return { std::move(stop), std::move(bus_to_distance_map) };

	}

	void JsonReader::ReadStatRequests(std::ostream& output) {

		json::Dict root_dict = std::get<json::Dict>(input_document_.GetRoot().GetValue());

		if (root_dict.count("stat_requests")) {

			bool correct_stat_requests_queue = std::holds_alternative<json::Array>(root_dict.at("stat_requests").GetValue());
			assert(correct_stat_requests_queue); //в узле stat_requests должен быть вектор запросов
			json::Array stat_requests_queue = std::get<json::Array>(root_dict.at("stat_requests").GetValue());

			std::vector<json::Document> answered_requests;
			answered_requests.reserve(stat_requests_queue.size());

			for (const json::Node& node : stat_requests_queue) {

				bool correct_stat_request = std::holds_alternative<json::Dict>(node.GetValue());
				assert(correct_stat_request); //запрос должен быть в виде словаря
				json::Dict stat_request = std::get<json::Dict>(node.GetValue());

				assert(stat_request.count("id")); //запрос должен содержать айди
				assert(stat_request.count("type")); //запрос должен содержать тип
				assert(std::holds_alternative<std::string>(stat_request.at("type").GetValue())); //тип в виде строки
				std::string stat_request_type = std::get<std::string>(stat_request.at("type").GetValue());

				if (stat_request_type == "Bus") {
					auto request_parsed = ReadStatRequest(stat_request);
					Catalogue::BusInfo bus_info = handler_->DoBusStatRequest(request_parsed.second);
					json::Document result = MakeBusInfoJSON(request_parsed.first, bus_info);
					answered_requests.push_back(std::move(result));
				}
				else if (stat_request_type == "Stop") {
					auto request_parsed = ReadStatRequest(stat_request);
					Catalogue::StopInfo stop_info = handler_->DoStopStatRequest(request_parsed.second);
					json::Document result = MakeStopInfoJSON(request_parsed.first, stop_info);
					answered_requests.push_back(std::move(result));
				}
				else if (stat_request_type == "Map") {
					assert(std::holds_alternative<int>(stat_request.at("id").GetValue())); //id только целое число
					int id = std::get<int>(stat_request.at("id").GetValue());

					json::Document result = MakeMapJSON(id);
					answered_requests.push_back(std::move(result));
				}
			}

			output << "[\n";
			bool is_first = true;
			for (const auto& doc : answered_requests) {
				if (is_first) {
					json::Print(doc, output);
					is_first = false;
					continue;
				}
				output << ",\n";
				json::Print(doc, output);
			}
			output << "\n]";
		}
	}

	std::pair<int, std::string> JsonReader::ReadStatRequest(const json::Dict stat_request) {
		assert(stat_request.count("name")); //запрос должен содержать имя
		assert(std::holds_alternative<std::string>(stat_request.at("name").GetValue())); //имя в виде строки
		std::string name = std::get<std::string>(stat_request.at("name").GetValue());

		assert(std::holds_alternative<int>(stat_request.at("id").GetValue())); //id только целое число
		int id = std::get<int>(stat_request.at("id").GetValue());
		return { id, name };
	}

	json::Document JsonReader::MakeBusInfoJSON(int id, Catalogue::BusInfo& bus_info) {

		std::stringstream strm;

		if (bus_info.is_not_found) {
			strm << "{ \"request_id\": " << id << ", \"error_message\": \"not found\"}";
			return json::Load(strm);
		}

		strm << "{";
		strm << "\"curvature\":" << bus_info.curvature << ", ";
		strm << "\"request_id\":" << id << ", ";
		strm << "\"route_length\":" << bus_info.length << ", ";
		strm << "\"stop_count\":" << bus_info.stops_number << ", ";
		strm << "\"unique_stop_count\":" << bus_info.unique_stops_number;
		strm << "}";
		return json::Load(strm);
	}

	json::Document JsonReader::MakeStopInfoJSON(int id, Catalogue::StopInfo& stop_info) {

		std::stringstream strm;

		if (stop_info.is_not_found) {
			strm << "{ \"request_id\": " << id << ", \"error_message\": \"not found\"}";
			return json::Load(strm);
		}

		strm << "{\"buses\": [";

		bool is_first = true;
		for (const std::string& bus : stop_info.bus_list) {
			if (is_first) {
				strm << "\"" << bus << "\"";
				is_first = false;
				continue;
			}
			strm << ", \"" << bus << "\"";
			is_first = false;
		}

		strm << "], \"request_id\": " << id << "}";
		return json::Load(strm);
	}

	Visual::MapSettings JsonReader::ReadMapSettings() {

		json::Dict root_dict = std::get<json::Dict>(input_document_.GetRoot().GetValue());
		Visual::MapSettings settings;

		if (root_dict.count("stat_requests")) {

			bool correct_settings = std::holds_alternative<json::Dict>(root_dict.at("render_settings").GetValue());
			assert(correct_settings); //в узле render_settings должен быть словарь
			json::Dict settings_from_json = std::get<json::Dict>(root_dict.at("render_settings").GetValue());

			assert(settings_from_json.count("width") && settings_from_json.count("height") && settings_from_json.count("padding")
				&& settings_from_json.count("line_width") && settings_from_json.count("stop_radius") && settings_from_json.count("bus_label_font_size")
				&& settings_from_json.count("bus_label_offset") && settings_from_json.count("stop_label_font_size") && settings_from_json.count("stop_label_offset")
				&& settings_from_json.count("underlayer_color") && settings_from_json.count("underlayer_width") && settings_from_json.count("color_palette")
			);

			settings.width = settings_from_json.at("width").AsDouble();
			settings.height = settings_from_json.at("height").AsDouble();
			settings.padding = settings_from_json.at("padding").AsDouble();
			settings.line_width = settings_from_json.at("line_width").AsDouble();
			settings.stop_radius = settings_from_json.at("stop_radius").AsDouble();
			settings.bus_label_font_size = settings_from_json.at("bus_label_font_size").AsInt();

			json::Array bus_offset = settings_from_json.at("bus_label_offset").AsArray();
			assert(bus_offset.size() == 2);
			double x1 = bus_offset[0].AsDouble();
			double y1 = bus_offset[1].AsDouble();
			settings.bus_label_offset = { x1,y1 };

			settings.stop_label_font_size = settings_from_json.at("stop_label_font_size").AsInt();

			json::Array stop_offset = settings_from_json.at("stop_label_offset").AsArray();
			assert(stop_offset.size() == 2);
			double x2 = stop_offset[0].AsDouble();
			double y2 = stop_offset[1].AsDouble();
			settings.stop_label_offset = { x2,y2 };

			settings.underlayer_color = get_color_from_node(settings_from_json.at("underlayer_color"));

			settings.underlayer_width = settings_from_json.at("underlayer_width").AsDouble();

			std::vector < svg::Color> color_palette;
			if (!settings_from_json.at("color_palette").AsArray().empty()) {
				settings.color_palette.clear();
				for (const json::Node& cur_node : settings_from_json.at("color_palette").AsArray()) {
					settings.color_palette.push_back(std::move(get_color_from_node(cur_node)));
				}
			}
		}
		return settings;
	}

	svg::Color get_color_from_node(json::Node node) {

		assert(node.IsString() || node.IsArray());
		if (node.IsString()) {
			return node.AsString();
		}
		else if (node.IsArray()) {
			json::Array temp = node.AsArray();
			assert(temp.size() == 3 || temp.size() == 4);
			if (temp.size() == 3) {
				svg::Rgb rgb;
				/*rgb.red = temp[0].AsInt();
				rgb.green = temp[1].AsInt();
				rgb.blue = temp[2].AsInt();*/
				rgb.red = temp[0].AsDouble();
				rgb.green = temp[1].AsDouble();
				rgb.blue = temp[2].AsDouble();
				return rgb;
			}
			else if (temp.size() == 4) {
				svg::Rgba rgba;
				/*rgba.red = temp[0].AsInt();
				rgba.green = temp[1].AsInt();
				rgba.blue = temp[2].AsInt();*/
				rgba.red = temp[0].AsDouble();
				rgba.green = temp[1].AsDouble();
				rgba.blue = temp[2].AsDouble();
				rgba.opacity = temp[3].AsDouble();
				return rgba;
			}
		}
		return svg::Color{ svg::NoneColor };
	}

	json::Document JsonReader::MakeMapJSON(int id) {

		Visual::Map map(ReadMapSettings(), handler_->GetAllBuses());

		std::stringstream strm;

		svg::Document doc;
		map.Draw(doc);
		doc.Render(strm);

		std::string map_string = strm.str();
		strm.str(std::string());

		json::Node map_node(map_string);
		json::Node id_node(id);
		json::Node result_node(json::Dict{ { "map", map_node }, {"request_id", id_node} });

		return json::Document(result_node);

	}

}