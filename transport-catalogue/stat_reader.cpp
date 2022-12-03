#include "stat_reader.h"

using namespace std;

//обрабатывает словарь с информацией
string StatReader::BusInfo(map<string,string> info) {
	string out;
	out = "Bus "s + info.at("name"s) +": "s;
	if (info.size() == 1) {
		out += "not found"s;
		return out;
	}
	out += info.at("stops_num"s) + " stops on route, "s + info.at("unique_num"s) + " unique stops, "s + info.at("length"s) + " route length, "s + info.at("curvature"s) + " curvature"s;
	return out;
}
//обрабатывает вектор с остановками
string StatReader::GetBusesForStop(vector<string> buses) {
	string out;
	out = "Stop "s + buses[0] + ":"s;
	if (buses.size() == 1) {
		out += " no buses"s;
	}
	else if (buses.size() == 2 && buses[1] == ""s) {
		out += " not found"s;
	}
	else {
		out += " buses "s;
		for (int i = 1; i < buses.size(); ++i) {
			out += buses[i] + " "s;
		}
		out.pop_back();
	}
	return out;
}

void OutPut::print(string info) {
	cout << info << endl;
}