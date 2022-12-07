#include "input_reader.h"
#include <utility>
#include <algorithm>

using namespace std;
namespace InputReader {

	pair<Catalogue::Stop, BusToDistance> DataBaseUpdater::StopUpdate(UpdateVector command) {
		
		auto it = find_if(command.begin(), command.end(), [](string_view s) {return s.back() == ':'; });
		int index = distance(command.begin(), it);
		string name;
		for (auto i = 0; i <= index; ++i) {
			name += string(command[i]) + " "s;
		}
		name = name.substr(0, name.size() - 2);
		string x(command[index + 1]);
		x.pop_back();
		string y(command[index + 2]);

		if (index + 3 == command.size()) {
			return { {name, stod(x), stod(y)}, {} }; //возвращаем остановку для добавления без информации по расстояниям
		}

		y.pop_back(); //удаляем лишнюю запятую

		Catalogue::Stop new_stop{ name, stod(x), stod(y) }; //остановка

		BusToDistance distances;

		unsigned int cur_pos = index + 3;

		do {
			auto next_comma = find_if(command.begin() + cur_pos, command.end(), [](string_view s) {return s.back() == ','; });

			string distance_info = string(command[cur_pos]); //значение расстояния
			distance_info.pop_back(); //удаляем 'm'
			string bus_name;
			unsigned int d = distance(command.begin(), next_comma);
			for (auto i = cur_pos + 2; i <= min(d, static_cast<unsigned int>(command.size() - 1)); ++i) {
				bus_name += string(command[i]) + " "s;
			}
			if (d < command.size() - 1) {
				bus_name = bus_name.substr(0, bus_name.size() - 2);

			}
			else {
				bus_name.pop_back();
			}
			distances[bus_name] = stod(distance_info);
			cur_pos = d + 1;
		} while (cur_pos < command.size());

		return { new_stop, distances };
	}

	//возвращает имя маршрута + список остановок
	pair<string, vector<string>> DataBaseUpdater::BusUpdate(UpdateVector command) {
		vector<string> bus_list;
		bus_list.reserve(command.size());

		auto it = find_if(command.begin(), command.end(), [](string_view s) {return s.back() == ':'; });
		int index = distance(command.begin(), it);
		string bus_name;
		for (auto i = 0; i <= index; ++i) {
			bus_name += string(command[i]) + " "s;
		}
		bus_name = bus_name.substr(0, bus_name.size() - 2);
		
		bool rev = false;
		string stop_of_bus;
		for (int i = index + 1; i < command.size(); ++i) {
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
		string s;
		getline(input, s);
		return stoi(s);
	}

	//добавления остановок и маршрутов
	void DataBaseUpdater::ReadUpdates(istream& input, Catalogue::TransportCatalogue& catalogue) {
		int number_lines = detail::ReadNumber(input);
		for (int i = 0; i < number_lines; ++i) {
			string line;
			getline(input, line);
			vector<string_view> in = detail::SplitIntoWordsByONESPACE(line);
			vector<string> command(in.begin() + 1, in.end());
			if (in[0] == "Stop"sv) {

				stop_updates_.push_back(command);
				auto [new_stop, distances] = StopUpdate(command);
				catalogue.AddStop(new_stop, distances);

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

			auto [new_stop, distances] = StopUpdate(command);
			catalogue.AddStop(new_stop, distances);
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
