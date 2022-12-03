#pragma once
#include "input_reader.h"

namespace StatReader {

	std::string BusInfo(std::map<std::string, std::string> info);

	std::string GetBusesForStop(std::vector<std::string> buses);
}

namespace OutPut {
	void print(std::string info);
}