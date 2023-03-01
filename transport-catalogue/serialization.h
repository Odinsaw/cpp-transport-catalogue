#pragma once

#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
#include "map_renderer.pb.h"
#include "transport_router.h"

#include <filesystem>
#include <fstream>

namespace Serialization {
	void DoSerialization(std::filesystem::path file,
		const Catalogue::TransportCatalogue& catalogue,
		const Visual::MapSettings& map_settings,
		const Router::TransportRouter& router);
	void SerializeCatalogueDB(transport_catalogue::Catalogue& catalogue_proto, const Catalogue::TransportCatalogue& catalogue);
	void SerializeMapSettings(transport_catalogue::Catalogue& catalogue_proto, const Visual::MapSettings& map_settings);
	void SerializeRouter(transport_catalogue::Catalogue& catalogue_proto, const Router::TransportRouter& router);

	void SerializeRouterSettings(transport_catalogue::Catalogue& catalogue_proto, const Router::RouterSettings& router_settings);
	void SerializeGraph(transport_catalogue::Catalogue& catalogue_proto, const graph::DirectedWeightedGraph<Router::TransportRouter::Time>& graph);
	void SerializeStopToIdsMap(transport_catalogue::Catalogue& catalogue_proto,
		const std::unordered_map<const Catalogue::Stop*, Router::TransportRouter::StopAsEdge, Router::TransportRouter::PointerHasher>& stop_to_id);
	void SerializeRouteItems(transport_catalogue::Catalogue& catalogue_proto,
		const std::vector<Router::TransportRouter::RouteItemPtr>& route_items);

	void DoDeSerialization(std::filesystem::path file, Catalogue::TransportCatalogue& catalogue, Visual::MapSettings& map_settings, Router::TransportRouter& router);
	void DeSerializeCatalogueDB(const transport_catalogue::Catalogue& catalogue_proto, Catalogue::TransportCatalogue& catalogue);
	void DeSerializeMapSettings(const transport_catalogue::Catalogue& catalogue_proto, Visual::MapSettings& map_settings);
	void DeSerializeRouter(const transport_catalogue::Catalogue& catalogue_proto, Router::TransportRouter& router, const Catalogue::TransportCatalogue& restored_catalogue);

	void DeSerializeRouterSettings(const transport_catalogue::Catalogue& catalogue_proto, Router::RouterSettings& router_settings);
	void DeSerializeGraph(const transport_catalogue::Catalogue& catalogue_proto, graph::DirectedWeightedGraph<Router::TransportRouter::Time>& graph);
	void DeSerializeStopToIdsMap(const transport_catalogue::Catalogue& catalogue_proto,
		std::unordered_map<const Catalogue::Stop*, Router::TransportRouter::StopAsEdge, Router::TransportRouter::PointerHasher>& stop_to_id_map,
		const Catalogue::TransportCatalogue& catalogue);
	void DeSerializeRouteItems(const transport_catalogue::Catalogue& catalogue_proto,
		std::vector<Router::TransportRouter::RouteItemPtr>& route_items, const Catalogue::TransportCatalogue& catalogue);
} //end Serialization