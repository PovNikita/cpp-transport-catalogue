#pragma once
#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cassert>
#include <string_view>
#include <optional>
#include <memory>

#include "domain.h"
#include "graph.h"
#include "router.h"


namespace transport_catalogue{
	
	class TransportCatalogue;
	class EdgeType {
		public:
			explicit EdgeType(std::string type) : type_(type) {}
			virtual ~EdgeType() = default;
			std::string type_;
	};

	class WaitEdgeType : public EdgeType {
		public:
			WaitEdgeType(std::string_view stop_name) : EdgeType("wait"), stop_name_(stop_name) {}
			std::string_view stop_name_;
	};

	class BusEdgeType : public EdgeType {
		public:
			BusEdgeType(std::string_view bus_name) : EdgeType("bus"), bus_name_(bus_name) {}
			BusEdgeType(std::string_view bus_name, size_t span_count, double time) : 
								EdgeType("bus"), bus_name_(bus_name), span_count_(span_count), time_(time) {}
			std::string_view bus_name_;
			size_t span_count_ = 0;
			double time_ = 0.0;
	};

	class GraphMaker {
		public:
		void FormGraph(const TransportCatalogue &transp_catalogue);
		const graph::DirectedWeightedGraph<double>& GetGraph() const;
		std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view start_stop, std::string_view end_stop) const;
		const EdgeType* GetEdgeType(graph::EdgeId id) const;
		const std::string_view GetStopNameByVertexId(graph::VertexId id) const;
		private:
		void SetAllStops();
		void SetAllBuses();
		const TransportCatalogue *transp_catalogue_;
		std::unordered_map<graph::VertexId, const Stop&> vertex_id_stop_;
		graph::VertexId vertex_id_ = 0;

		std::unordered_map<graph::EdgeId, std::unique_ptr<EdgeType>> edge_id_type_;
		std::unordered_map<std::string_view, std::pair<graph::VertexId, graph::VertexId>> stopname_to_vertex_id_;

		std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
	};

	class TransportCatalogueMap	{
		public:
		struct TypeOfConnection
		{
			double distance = 0;
		};

		void AddNode(std::string_view start_stop, std::string_view end_stop, TypeOfConnection node);
		std::optional<double> GetDistance(std::string_view start_stop, std::string_view end_stop) const;
		private:
		std::unordered_map<std::string_view, std::unordered_map<std::string_view, TypeOfConnection>> adjacency_matrix_;
	};

	class TransportCatalogue {
		friend GraphMaker;
		public:
		const Stop& AddStop(Stop& bus_stop_);
		const Stop* SearchStop(std::string_view stop_name) const;
		const Bus& AddRoute(Bus& bus_route);
		template <typename ForwardIt1, typename ForwardIt2>
		const Bus& AddRoute(std::string_view route_name, ForwardIt1 first_stop_name, ForwardIt2 last_stop_name, bool is_roundtrip);
		const Bus* SearchRoute(std::string_view route_name) const;
		void SetDistance(std::string_view start_stop, std::string_view end_stop, double distance);
		double GetDistance(std::string_view start_stop, std::string_view end_stop) const;
		std::optional<RouteInfo> GetInfoAboutRoute(std::string_view route_name) const;
		std::optional<StopInfo> GetInfoAboutBusesViaStop(std::string_view stop_name) const;
		const std::deque<Bus>& GetAllRoutes() const;
		const std::deque<Stop>& GetAllStops() const;
		
		void SetRouteSettings(int wait_time, double bus_velocity);
		const RouteSettings& GetRouteSettings() const;
		void FormGraph();
		const graph::DirectedWeightedGraph<double>& GetGraph() const;
		std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view start_stop, std::string_view end_stop) const;
		const EdgeType* GetEdgeType(graph::EdgeId id) const;
		const std::string_view GetStopNameByVertexId(graph::VertexId id) const;

		private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop&> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus&> busname_to_bus_;
		TransportCatalogueMap map_;

		GraphMaker graph_;
		RouteSettings route_settings_;
	};

	template <typename ForwardIt1, typename ForwardIt2>
	const Bus& TransportCatalogue::AddRoute(std::string_view route_name, ForwardIt1 first_stop_name, ForwardIt2 last_stop_name, bool is_roundtrip)
	{
		Bus bus;
		bus.bus_name_ = route_name;
		while(first_stop_name != last_stop_name)
		{
			Stop *bus_stop = nullptr;
			auto it = stopname_to_stop_.find(*first_stop_name);
			if(it != stopname_to_stop_.end())
			{
				bus_stop = &(it->second);
			}
			assert(bus_stop);
			bus.route_.push_back(bus_stop);
			++first_stop_name;
		}
		bus.is_roundtrip_ = is_roundtrip;
		return AddRoute(bus);
	}



};