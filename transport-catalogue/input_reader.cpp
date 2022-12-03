#include "input_reader.h"
#include <utility>
#include <algorithm>

using namespace std;
namespace InputReader {
	//удаляет лишние знаки, приводит запрос к вектору {имя, координата x, координата y, (расстояния)...}
	vector<string> Query::AddStop(QueryVector command) {
		vector<string> out;
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

		out.push_back(name);
		out.push_back(x);
		out.push_back(y);

		if (index + 3 == command.size()) {
			return out;
		}

		out.back().pop_back();

		unsigned int cur_pos = 3;

		do {
			auto next_comma = find_if(command.begin() + cur_pos, command.end(), [](string_view s) {return s.back() == ','; });

			string dist = string(command[cur_pos]);
			dist.pop_back();
			out.push_back(dist);
			string n;
			unsigned int d = distance(command.begin(), next_comma);
			for (auto i = cur_pos + 2; i <= min(d, static_cast<unsigned int>(command.size() - 1)); ++i) {
				n += string(command[i]) + " "s;
			}
			if (d < command.size() - 1) {
				n = n.substr(0, n.size() - 2);

			}
			else {
				n.pop_back();
			}
			out.push_back(n);
			cur_pos = d + 1;
		} while (cur_pos < command.size());

		return out;
	}

	//удаляет лишние знаки, приводит запрос к вектору {имя, остановка 1, остановка 2, ...}
	vector<string> Query::AddBus(QueryVector command) {
		vector<string> out;
		out.reserve(command.size());

		auto it = find_if(command.begin(), command.end(), [](string_view s) {return s.back() == ':'; });
		int index = distance(command.begin(), it);
		string name;
		for (auto i = 0; i <= index; ++i) {
			name += string(command[i]) + " "s;
		}
		name = name.substr(0, name.size() - 2);
		out.push_back(name);
		bool rev = false;
		string temp;
		for (int i = index + 1; i < command.size(); ++i) {
			if (command[i] == ">"sv) {
				temp.pop_back();
				out.push_back(temp);
				temp.clear();
				continue;
			}
			else if (command[i] == "-"sv) {
				rev = true;
				temp.pop_back();
				out.push_back(temp);
				temp.clear();
				continue;
			}
			temp += string(command[i]) + " "s;
		}
		temp.pop_back();
		out.push_back(temp);

		if (rev && out.size() > 2) {
			vector<string> r(out.begin() + 1, out.end());
			r.pop_back();
			reverse(r.begin(), r.end());
			out.insert(out.end(), r.begin(), r.end());
		}
		return out;
	}

	//оставляет только имя
	string Query::GetBusInfo(QueryVector command) {
		string name;
		for (auto i = 0; i < command.size(); ++i) {
			name += string(command[i]) + " "s;
		}
		name.pop_back();
		return name;
	}

	//оставляет только имя
	string Query::GetBusesForStop(QueryVector command) {
		string name;
		for (auto i = 0; i < command.size(); ++i) {
			name += string(command[i]) + " "s;
		}
		name.pop_back();
		return name;
	}

	int detail::ReadNumber(istream& input) {
		string s;
		getline(input, s);
		return stoi(s);
	}

	//заполнение очереди
	void Query::ReadCIn(istream& input) {
		int n = detail::ReadNumber(input);
		for (int i = 0; i < n; ++i) {
			string line;
			getline(input, line);
			query_data_.push_back(move(line));
			vector<string_view> in = detail::SplitIntoWordsByONESPACE(query_data_.back());
			vector<string_view> command(in.begin() + 1, in.end());
			if (in[0] == "Stop"sv) {
				query_[QueryType::AddStop].push_back(command);
			}
			else if (in[0] == "Bus"sv) {
				query_[QueryType::AddBus].push_back(command);
			}
		}
		n = detail::ReadNumber(input);
		for (int i = 0; i < n; ++i) {
			string line;
			getline(input, line);
			query_data_.push_back(move(line));
			vector<string_view> in = detail::SplitIntoWordsByONESPACE(query_data_.back());
			vector<string_view> command(in.begin() + 1, in.end());
			if (in[0] == "Stop"sv) {
				query2_.push_back({ QueryType::GetBusesForStop , command });
			}
			else if (in[0] == "Bus"sv) {
				query2_.push_back({ QueryType::GetBusInfo , command });
			}
		}
	}

	//обработка очереди
	void Query::Compute(Catalogue::TransportCatalogue& t) {
		for (pair<QueryType, vector<QueryVector>> q_type : query_) {
			for (QueryVector command : q_type.second) {
				if (q_type.first == QueryType::AddStop) {
					t.AddStop(AddStop(command));
				}
				else if (q_type.first == QueryType::AddBus) {
					t.AddBus(AddBus(command));
				}
			}
		}

		if (query_.count(QueryType::AddStop)) {
			for (QueryVector command : query_.at(QueryType::AddStop)) {
				t.AddStop(AddStop(command));
			}
		}

		for (pair<QueryType, QueryVector> q : query2_) {
			if (q.first == QueryType::GetBusInfo) {
				OutPut::print(StatReader::BusInfo(t.GetBusInfo(GetBusInfo(q.second))));
			}
			else if (q.first == QueryType::GetBusesForStop) {
				OutPut::print(StatReader::GetBusesForStop(t.GetBusesForStop(GetBusesForStop(q.second))));
			}
		}
	}

	void Query::ClearQuery() {
		query_.clear();
		query2_.clear();
		query_data_.clear();
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
