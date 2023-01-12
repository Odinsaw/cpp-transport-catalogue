#include "json_reader.h"
#include <sstream>
#include <algorithm>

namespace DataReader {

	using namespace std::literals;

	JsonReader::JsonReader(std::istream& input, Catalogue::TransportCatalogue& catalogue)
		:input_document_(json::Load(input))
		{
		handler_ = std::make_unique<DataBaseInterface::RequestsHandler>(catalogue);
			bool correct_root = std::holds_alternative<json::Dict>(input_document_.GetRoot().GetValue());
			assert(correct_root); //в корневом узле должен быть словарь {base_requests : ... , stat_requests : ...}
		}

	JsonReader::JsonReader(std::istream& input, std::unique_ptr<DataBaseInterface::RequestsHandler>&& handler)
		:input_document_(json::Load(input)), handler_(std::move(handler))
	{
		bool correct_root = std::holds_alternative<json::Dict>(input_document_.GetRoot().GetValue());
		assert(correct_root); //в корневом узле должен быть словарь {base_requests : ... , stat_requests : ...}
	}

	JsonReader& JsonReader::SetNewRequestHandler(Catalogue::TransportCatalogue& catalogue) {
		std::unique_ptr<Modules::Module> new_handler = std::make_unique<DataBaseInterface::RequestsHandler>(catalogue);
		modules_[Modules::ModuleType::TransportRouter] = std::move(new_handler);
		return *this;
	}
	JsonReader& JsonReader::SetNewRequestHandler(std::unique_ptr<DataBaseInterface::RequestsHandler> new_handler) {
		modules_[Modules::ModuleType::TransportRouter] = std::move(new_handler);
		return *this;
	}

	JsonReader& JsonReader::SetNewMap() {
		std::unique_ptr<Modules::Module> new_map = std::make_unique<Visual::MapRenderer>(ReadMapSettings(), handler_->GetAllBuses());
		modules_[Modules::ModuleType::MapRenderer] = std::move(new_map);
		return *this;
	}
	JsonReader& JsonReader::SetNewMap(Visual::MapSettings settings) {
		std::unique_ptr<Modules::Module> new_map = std::make_unique<Visual::MapRenderer>(settings, handler_->GetAllBuses());
		modules_[Modules::ModuleType::MapRenderer] = std::move(new_map);
		return *this;
	}
	JsonReader& JsonReader::SetNewMap(std::unique_ptr<Visual::MapRenderer> new_renderer) {
		modules_[Modules::ModuleType::MapRenderer] = std::move(new_renderer);
		return *this;
	}

	JsonReader& JsonReader::SetNewTransportRouter() {
		std::unique_ptr<Modules::Module> new_router = std::make_unique<Router::TransportRouter>(ReadRouterSettings(), handler_->GetAllBuses(), handler_->GetDistances());
		modules_[Modules::ModuleType::TransportRouter] = std::move(new_router);
		return *this;
	}
	JsonReader& JsonReader::SetNewTransportRouter(Router::RouterSettings settings) {
		std::unique_ptr<Modules::Module> new_router = std::make_unique<Router::TransportRouter>(settings, handler_->GetAllBuses(), handler_->GetDistances());
		modules_[Modules::ModuleType::TransportRouter] = std::move(new_router);
		return *this;
	}
	JsonReader& JsonReader::SetNewTransportRouter(std::unique_ptr<Router::TransportRouter> new_router) {
		modules_[Modules::ModuleType::TransportRouter] = std::move(new_router);
		return *this;
	}

	void JsonReader::ReadBusesBaseRequests() {

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

				if (base_request_type == "Bus") {
					auto request_parsed = ReadBusBaseRequest(base_request);
					handler_->DoBusBaseRequest(std::move(std::get<0>(request_parsed)), std::move(std::get<1>(request_parsed)), std::move(std::get<2>(request_parsed)));
				}
			}
		}
	}

	void JsonReader::ReadStopsBaseRequests() {

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
		} //первая проходка
	}

	for (const json::Node& node : base_requests_queue) {

		json::Dict base_request = std::get<json::Dict>(node.GetValue());
		std::string base_request_type = std::get<std::string>(base_request.at("type").GetValue());

		if (base_request_type == "Stop") {
			auto request_parsed = ReadStopBaseRequest(base_request);
			handler_->DoStopBaseRequest(std::move(request_parsed.first), std::move(request_parsed.second));
		}
	} //вторая проходка

}
	}

	void JsonReader::ReadBaseRequests() {
		ReadStopsBaseRequests();
		ReadBusesBaseRequests();
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

					std::string map_string = DoMapRequest();
					json::Document result = MakeMapJSON(id, map_string);
					answered_requests.push_back(std::move(result));
				}
				else if (stat_request_type == "Route") {
					RouteRequest request_parsed = ReadRouteRequest(stat_request);
					std::optional<Router::TransportRouter::RouteInfo> route = DoRouteRequest(request_parsed);
					json::Document result = MakeRouteJSON(request_parsed.id, route);
					answered_requests.push_back(std::move(result));
				}
			}
			json::Array out_requests;
			for (const auto& doc : answered_requests) {
				out_requests.emplace_back(doc.GetRoot());
			}
			json::Document result_node{ json::Builder{}.Value(out_requests).Build() };
			json::Print(result_node, output);
		}
	}

	JsonReader::RouteRequest JsonReader::ReadRouteRequest(const json::Dict stat_request) {
		assert(stat_request.count("from") && stat_request.count("to")); //запрос должен откуда и куда
		assert(std::holds_alternative<std::string>(stat_request.at("from").GetValue())); //в виде строки
		assert(std::holds_alternative<std::string>(stat_request.at("to").GetValue())); //в виде строки
		std::string to = std::get<std::string>(stat_request.at("to").GetValue());
		std::string from = std::get<std::string>(stat_request.at("from").GetValue());

		assert(std::holds_alternative<int>(stat_request.at("id").GetValue())); //id только целое число
		int id = std::get<int>(stat_request.at("id").GetValue());
		return { from, to, id };
	}

	std::optional<Router::TransportRouter::RouteInfo> JsonReader::DoRouteRequest(RouteRequest request_parsed) {

		if (!modules_.count(Modules::ModuleType::TransportRouter)
			|| modules_.at(Modules::ModuleType::TransportRouter) == nullptr
			|| modules_.at(Modules::ModuleType::TransportRouter)->type != Modules::ModuleType::TransportRouter) {
			SetNewTransportRouter();
		}

		Modules::Module* cur_module = modules_.at(Modules::ModuleType::TransportRouter).get();
		Router::TransportRouter* router = dynamic_cast<Router::TransportRouter*>(cur_module);
		return router->FindRoute(
		handler_->GetStop(request_parsed.from), handler_->GetStop(request_parsed.to));
	}

	std::string JsonReader::DoMapRequest() {

		if (!modules_.count(Modules::ModuleType::MapRenderer)
			|| modules_.at(Modules::ModuleType::MapRenderer) == nullptr
			|| modules_.at(Modules::ModuleType::MapRenderer)->type != Modules::ModuleType::MapRenderer) {
			SetNewMap();
		}

		Modules::Module* cur_module = modules_.at(Modules::ModuleType::MapRenderer).get();
		Visual::MapRenderer* map = dynamic_cast<Visual::MapRenderer*>(cur_module);
		return map->GetMap();
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

		if (bus_info.is_not_found) {
			json::Document result_node{ json::Builder{}.StartDict().Key("request_id").Value(id).Key("error_message").Value("not found"s).EndDict().Build() };
			return result_node;
		}

		json::Document result_node{ json::Builder{}.StartDict().Key("curvature").Value(bus_info.curvature).Key("request_id").Value(id)
		.Key("route_length").Value(bus_info.length).Key("stop_count").Value(static_cast<int>(bus_info.stops_number)).Key("unique_stop_count").Value(static_cast<int>(bus_info.unique_stops_number)).EndDict()
		.Build()};

		return result_node;
	}

	json::Document JsonReader::MakeStopInfoJSON(int id, Catalogue::StopInfo& stop_info) {

		if (stop_info.is_not_found) {
			json::Document result_node{ json::Builder{}.StartDict().Key("request_id").Value(id).Key("error_message").Value("not found"s).EndDict().Build()};
			return result_node;
		}

		json::Array buses;
		for (const std::string& bus : stop_info.bus_list) {
			buses.emplace_back(bus);
			}

		json::Document result_node{ json::Builder{}.StartDict().Key("buses").Value(buses).Key("request_id").Value(id).EndDict().Build() };

		return result_node;
	}

	Visual::MapSettings JsonReader::ReadMapSettings() {

		json::Dict root_dict = std::get<json::Dict>(input_document_.GetRoot().GetValue());
		Visual::MapSettings settings;

		if (root_dict.count("render_settings")) {

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
			settings.bus_label_offset = { x1, y1 };

			settings.stop_label_font_size = settings_from_json.at("stop_label_font_size").AsInt();

			json::Array stop_offset = settings_from_json.at("stop_label_offset").AsArray();
			assert(stop_offset.size() == 2);
			double x2 = stop_offset[0].AsDouble();
			double y2 = stop_offset[1].AsDouble();
			settings.stop_label_offset = { x2, y2 };

			settings.underlayer_color = GetColorFromNode(settings_from_json.at("underlayer_color"));

			settings.underlayer_width = settings_from_json.at("underlayer_width").AsDouble();

			std::vector < svg::Color> color_palette;
			if (!settings_from_json.at("color_palette").AsArray().empty()) {
				settings.color_palette.clear();
				for (const json::Node& cur_node : settings_from_json.at("color_palette").AsArray()) {
					settings.color_palette.push_back(std::move(GetColorFromNode(cur_node)));
				}
			}
		}
		return settings;
	}

	Router::RouterSettings JsonReader::ReadRouterSettings() {

		json::Dict root_dict = std::get<json::Dict>(input_document_.GetRoot().GetValue());
		Router::RouterSettings settings;

		if (root_dict.count("routing_settings")) {

			bool correct_settings = std::holds_alternative<json::Dict>(root_dict.at("routing_settings").GetValue());
			assert(correct_settings); //в узле render_settings должен быть словарь
			json::Dict settings_from_json = std::get<json::Dict>(root_dict.at("routing_settings").GetValue());

			assert(settings_from_json.count("bus_velocity") && settings_from_json.count("bus_wait_time"));

			settings.bus_wait_time = settings_from_json.at("bus_wait_time").AsDouble();
			settings.bus_velocity = settings_from_json.at("bus_velocity").AsDouble();
		}
		return settings;
	}

	svg::Color GetColorFromNode(json::Node node) {

		assert(node.IsString() || node.IsArray());
		if (node.IsString()) {
			return node.AsString();
		}
		else if (node.IsArray()) {
			json::Array color_node = node.AsArray();
			assert(color_node.size() == 3 || color_node.size() == 4);
			if (color_node.size() == 3) {
				svg::Rgb rgb;
				rgb.red = color_node[0].AsDouble();
				rgb.green = color_node[1].AsDouble();
				rgb.blue = color_node[2].AsDouble();
				return rgb;
			}
			else if (color_node.size() == 4) {
				svg::Rgba rgba;
				rgba.red = color_node[0].AsDouble();
				rgba.green = color_node[1].AsDouble();
				rgba.blue = color_node[2].AsDouble();
				rgba.opacity = color_node[3].AsDouble();
				return rgba;
			}
		}
		return svg::Color{ svg::NoneColor };
	}

	json::Document JsonReader::MakeMapJSON(int id, std::string map_string) {

		json::Document result_node{ json::Builder{}.StartDict().Key("map").Value(std::move(map_string)).Key("request_id").Value(id).EndDict().Build() };

		return result_node;
	}

	json::Document JsonReader::MakeRouteJSON(int id, std::optional<Router::TransportRouter::RouteInfo> route) {

		using RouteItemType = Router::TransportRouter::RouteItem::ItemType;

		if (route == std::nullopt) {
			json::Document result_node{ json::Builder{}.StartDict().Key("request_id").Value(id).Key("error_message").Value("not found"s).EndDict().Build() };
			return result_node;
		}

		std::vector<json::Node> items;
		for (Router::TransportRouter::RouteItemPtr item : route.value().route_items) {

			if (item->GetType() == RouteItemType::WaitItem) {

				Router::TransportRouter::WaitItem* w_item = dynamic_cast<Router::TransportRouter::WaitItem*>(item.get());
				json::Node wait{ json::Builder{}.StartDict()
					.Key("stop_name"s).Value(w_item->stop->name)
					.Key("time"s).Value(w_item->time)
					.Key("type"s).Value("Wait"s)
					.EndDict().Build()};
				items.push_back(wait);
			}
			else if (item->GetType() == RouteItemType::BusItem) {

				Router::TransportRouter::BusItem* b_item = dynamic_cast<Router::TransportRouter::BusItem*>(item.get());
				json::Node bus{ json::Builder{}.StartDict()
					.Key("bus"s).Value(b_item->bus->name)
					.Key("span_count"s).Value(static_cast<int>(b_item->span_count))
					.Key("time"s).Value(b_item->time)
					.Key("type"s).Value("Bus"s)
					.EndDict().Build() };
				items.push_back(bus);
			}
		}

		json::Document result_node{ json::Builder{}.StartDict()
			.Key("items").Value(items)
			.Key("request_id").Value(id)
			.Key("total_time").Value(route.value().time)
			.EndDict().Build() };

		return result_node;
	}
}