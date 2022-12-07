#include "stat_reader.h"
#include  <iomanip>

using namespace std;

namespace StatReader {

	void TransportInfoReader::ReadRequests(istream& input, Catalogue::TransportCatalogue& catalogue) {

		FillQueue(input);

		ProcessQueue(catalogue);

	}

	void TransportInfoReader::FillQueue(std::istream& input) {

		int number_lines = InputReader::detail::ReadNumber(input);

		for (int i = 0; i < number_lines; ++i) {

			string line;
			getline(input, line);

			vector<string_view> in = InputReader::detail::SplitIntoWordsByONESPACE(line);
			vector<string> command(in.begin() + 1, in.end());

			if (in[0] == "Stop"sv) {
				requests_.push_back({ RequestType::StopRequest, move(command) });
			}
			else if (in[0] == "Bus"sv) {
				requests_.push_back({ RequestType::BusRequest, move(command) });
			}
		}
	}

		void TransportInfoReader::ProcessQueue(Catalogue::TransportCatalogue & catalogue) {

			for (auto request : requests_) {

				if (request.first == RequestType::StopRequest) {

					vector<string> bus_list = catalogue.GetBusesForStop(detail::GetName(request.second));

					cout << "Stop "s + detail::GetName(request.second) + ":"s;
					if (bus_list.size() == 0) {
						cout << " no buses"s << endl;
					}
					else if (bus_list.size() == 1 && bus_list[0] == ""s) {
						cout << " not found"s << endl;
					}
					else {
						cout << " buses"s;

						for (string& bus : bus_list) {
							cout << " "s + bus;
						}
						cout << endl;
					}
				}
				else if (request.first == RequestType::BusRequest) {
					vector<string> info = catalogue.GetBusInfo(detail::GetName(request.second));

					cout << "Bus " << info[0] << ": "s;
					if (info.size() == 2 && info[1] == "not found"s) {
						cout << "not found"s << endl;
						continue;
					}
					cout << info[1] << " stops on route, "s << info[2] << " unique stops, "s << info[3] << " route length, "s << setprecision(6) << info[4] << " curvature"s << endl;
				}
			}
		}

	//оставляет только имя
	string detail::GetName(QueryVector command) {
		string name;
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