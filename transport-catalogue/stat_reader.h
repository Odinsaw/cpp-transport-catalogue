#pragma once
#include "input_reader.h"

namespace StatReader {

	using QueryVector = std::vector<std::string>;

	enum class RequestType {
		StopRequest,
		BusRequest
	};

	class TransportInfoReader {

	public:

		void ReadRequests(std::istream& input, Catalogue::TransportCatalogue& catalogue);

	};

	namespace detail {

		std::string GetName(QueryVector command);

	}
}

namespace OutPut {
	void print(std::string info);
}