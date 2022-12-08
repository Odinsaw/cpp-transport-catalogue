#include "stat_reader.h"
#include  <iomanip>

using namespace std;

namespace StatReader {

	void TransportInfoReader::ReadRequests(istream& input, Catalogue::TransportCatalogue& catalogue) {

		size_t number_lines = InputReader::detail::ReadNumber(input);

		for (size_t i = 0; i < number_lines; ++i) {

			string line = "";
			getline(input, line);

			vector<string_view> in = InputReader::detail::SplitIntoWordsByONESPACE(line);
			vector<string> command(in.begin() + 1, in.end());

			if (in[0] == "Stop"sv) {
				Catalogue::StopInfo info = move(catalogue.GetBusesForStop(detail::GetName(command)));

				cout << "Stop "s + detail::GetName(command) + ":"s;
				if (info.is_not_found == false && info.bus_list.size() == 0) {
					cout << " no buses"s << endl;
				}
				else if (info.is_not_found == true) {
					cout << " not found"s << endl;
				}
				else {
					cout << " buses"s;

					for (string& bus : info.bus_list) {
						cout << " "s + bus;
					}
					cout << endl;
				}
			}
			else if (in[0] == "Bus"sv) {
				Catalogue::BusInfo info = catalogue.GetBusInfo(detail::GetName(command));

				cout << "Bus " << info.name << ": "s;
				if (info.is_not_found) {
					cout << "not found"s << endl;
					continue;
				}
				cout << info.stops_number << " stops on route, "s
					<< info.unique_stops_number << " unique stops, "s 
					<< info.length << " route length, "s 
					<< setprecision(6) << info.curvature << " curvature"s << endl;

			}
		}

	}

	//оставляет только имя
	string detail::GetName(QueryVector command) {
		string name = "";
		for (auto i = 0; i < command.size(); ++i) {
			name += string(command[i]) + " "s;
		}
		name.pop_back();
		return name;
	}

}

void OutPut::print(string info) {
	cout << info << endl;
}