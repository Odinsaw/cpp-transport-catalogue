#pragma once

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <optional>

namespace Router {

	using Distances = const std::unordered_map<std::pair<const Catalogue::Stop*, const Catalogue::Stop*>, std::size_t, Catalogue::PointerPairHasher>;

	class TransportRouter : public Modules::Module{

	public:

		class PointerHasher {
		public:
			std::size_t operator()(const Catalogue::Stop* input) const {
				return std::hash<const void*>()(input);
			}
		};

		using Time = double;
		using VertexId = graph::VertexId;

		struct StopAsEdge {
			VertexId in; //считаем, что чтобы отправится с остановки нужно сначала пройти(подождать) по ребру in -> out
			VertexId out; 
			Time wait;
		};

		struct RouteItem {
			
			virtual ~RouteItem() = default;

			enum class ItemType {
				WaitItem,
				BusItem
			};

			RouteItem(ItemType t) 
				:type(t)
			{
			}

			ItemType GetType() {
				return type;
			}

			ItemType type;
		};

		using RouteItemPtr = std::shared_ptr<RouteItem>;
		using RouteItems = std::vector<RouteItemPtr>;

		struct RouteInfo {
			Time time;
			RouteItems route_items;
		};

		struct WaitItem : RouteItem{

			WaitItem(const Catalogue::Stop* s, Time t)
				:RouteItem(ItemType::WaitItem), stop(s), time(t)
			{
			}

			const Catalogue::Stop* stop;
			Time time;
		};

		struct BusItem : RouteItem {

			BusItem(const Catalogue::Bus* b, size_t s_c, Time t)
				:RouteItem(ItemType::BusItem), bus(b), span_count(s_c), time(t)
			{
			}

			const Catalogue::Bus* bus;
			size_t span_count;
			Time time;
		};

		TransportRouter(RouterSettings settings, Catalogue::TransportCatalogue& catalogue);
		TransportRouter(RouterSettings settings, std::vector<Catalogue::Bus*> buses, Distances& distances);

		std::optional<RouteInfo> FindRoute(const Catalogue::Stop* stop_from, const Catalogue::Stop* stop_to) const;

	private:

		void LoadGraph();
		void LoadStopsToGraph();
		void LoadBusesToGraph();
		double GetDistance(const Catalogue::Stop* stop1, const Catalogue::Stop* stop2) const;

		std::vector<Catalogue::Bus*> buses_;
		std::unordered_set<Catalogue::Stop*> stops_;
		std::unique_ptr<graph::DirectedWeightedGraph<Time>> graph_;
		std::unique_ptr<graph::Router<Time>> router_;
		RouterSettings settings_;
		std::unordered_map<const Catalogue::Stop*, StopAsEdge, PointerHasher> stop_to_id_;
		std::vector<RouteItemPtr> route_items_; //id ребра - индекс вектора, где лежит соотвутствующий route item 
		Distances& distances_;
	};

}
