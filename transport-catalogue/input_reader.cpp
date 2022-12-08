#include "input_reader.h"
#include <utility>
#include <algorithm>
#include <cassert>


using namespace std;
namespace InputReader {

	pair<Catalogue::Stop, BusToDistance> DataBaseUpdater::StopUpdate(UpdateVector command) {
		
		assert(command.size() > 1);

		auto it = find_if(command.begin(), command.end(), [](string_view s) {return s.back() == ':'; });
		size_t index = distance(command.begin(), it);
		string name = ""s;
		for (size_t i = 0; i <= index; ++i) {
			name += string(command[i]) + " "s;
		}
		assert(name.size() > 2);
		name = name.substr(0, name.size() - 2);
		string x_coordinate(command[index + 1]);
		x_coordinate.pop_back();
		string y_coordinate(command[index + 2]);

		if (index + 3 == command.size()) {
			return { {name, stod(x_coordinate), stod(y_coordinate)}, {} }; //возвращаем остановку для добавления без информации по расстояниям
		}

		y_coordinate.pop_back(); //удаляем лишнюю запятую

		Catalogue::Stop new_stop{ name, stod(x_coordinate), stod(y_coordinate) }; //остановка

		BusToDistance distances;

		size_t cur_pos = index + 3;

		do {
			auto next_comma = find_if(command.begin() + cur_pos, command.end(), [](string_view s) {return s.back() == ','; });

			string distance_info = string(command[cur_pos]); //значение расстояния
			distance_info.pop_back(); //удаляем 'm'
			string bus_name = ""s;
			size_t position = distance(command.begin(), next_comma);
			for (auto i = cur_pos + 2; i <= min(position, static_cast<size_t>(command.size() - 1)); ++i) {
				bus_name += string(command[i]) + " "s;
			}
			if (position < command.size() - 1) {
				assert(bus_name.size() > 2);
				bus_name = bus_name.substr(0, bus_name.size() - 2);

			}
			else {
				bus_name.pop_back();
			}
			distances[bus_name] = stod(distance_info);
			cur_pos = position + 1;
		} while (cur_pos < command.size());

		return { new_stop, distances };
	}

	//возвращает имя маршрута + список остановок
	pair<string, vector<string>> DataBaseUpdater::BusUpdate(UpdateVector command) {
		vector<string> bus_list;
		bus_list.reserve(command.size());

		auto it = find_if(command.begin(), command.end(), [](string_view strv) {return strv.back() == ':'; });
		size_t index = distance(command.begin(), it);
		string bus_name;
		for (size_t i = 0; i <= index; ++i) {
			bus_name += string(command[i]) + " "s;
		}
		assert(bus_name.size() > 2);
		bus_name = bus_name.substr(0, bus_name.size() - 2);
		
		bool rev = false;
		string stop_of_bus = "";
		for (size_t i = index + 1; i < command.size(); ++i) {
			if (command[i] == ">"sv) {
				stop_of_bus.pop_back();
				bus_list.push_back(stop_of_bus);
				stop_of_bus.clear();
				continue;
			}
			else if (command[i] == "-"sv) {
				rev = true;
				stop_of_bus.pop_back();
				bus_list.push_back(stop_of_bus);
				stop_of_bus.clear();
				continue;
			}
			stop_of_bus += string(command[i]) + " "s;
		}
		stop_of_bus.pop_back();
		bus_list.push_back(stop_of_bus);

		if (rev && bus_list.size() > 1) {
			vector<string> r(bus_list.begin(), bus_list.end());
			r.pop_back();
			reverse(r.begin(), r.end());
			bus_list.insert(bus_list.end(), r.begin(), r.end());
		}
		return { bus_name, bus_list };
	}

	
	int detail::ReadNumber(istream& input) {
		string sstr = "";
		getline(input, sstr);
		return stoi(sstr);
	}

	//добавления остановок и маршрутов
	void DataBaseUpdater::ReadCommands(istream& input, Catalogue::TransportCatalogue& catalogue) {
		int number_lines = detail::ReadNumber(input);
		for (int i = 0; i < number_lines; ++i) {
			string line = "";
			getline(input, line);
			vector<string_view> in = detail::SplitIntoWordsByONESPACE(line);
			vector<string> command(in.begin() + 1, in.end());
			if (in[0] == "Stop"sv) {

				stop_updates_.push_back(command);
				auto [stop, distances] = StopUpdate(command);
				catalogue.AddStop(stop.name, stop.coords);
				catalogue.AddDistances(stop.name, distances);

			}
			else if (in[0] == "Bus"sv) {

				bus_updates_.push_back(command);

			}
		}
		for (UpdateVector command : bus_updates_) {
			
			auto [new_bus_name, bus_list] = BusUpdate(command);

			catalogue.AddBus(new_bus_name, bus_list);
		}

		for (UpdateVector command : stop_updates_) { //вторая проходка по остановкам

			auto [stop, distances] = StopUpdate(command);
			catalogue.AddStop(stop.name, stop.coords);
			catalogue.AddDistances(stop.name, distances);
		}

	}

	void DataBaseUpdater::ClearQuery() {
		bus_updates_.clear();
	}

	//удалить все пробелы
	vector<string_view> detail::SplitIntoWords(string_view str) {
		vector<string_view> result;
		while (true) {
			const auto space = str.find(' ');
			if (space != 0 && !str.empty()) {
				result.push_back(str.substr(0, space));
			}
			if (space == str.npos) {
				break;
			}
			else {
				str.remove_prefix(space + 1);
			}
		}
		return result;
	}

	//разделять только по одному пробелу
	vector<string_view> detail::SplitIntoWordsByONESPACE(string_view str) {
		char ch = ' ';
		size_t pos = str.find(ch);
		size_t initialPos = 0;
		vector<string_view> strs;

		while (pos != string::npos) {
			strs.push_back(str.substr(initialPos, pos - initialPos));
			initialPos = pos + 1;

			pos = str.find(ch, initialPos + 1);
		}

		strs.push_back(str.substr(initialPos, std::min(pos, str.size()) - initialPos + 1));

		return strs;
	}
}
