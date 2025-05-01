#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <memory>
#include <variant>

#include "transport_catalogue.h"
#include "domain.h"
#include "graph.h"
#include "router.h"

namespace transport_router
{
    struct WaitEdgeType {
        WaitEdgeType(std::string_view stop_name) : stop_name_(stop_name) {}
        std::string_view stop_name_;
    };

    struct BusEdgeType {
        BusEdgeType(std::string_view bus_name) : bus_name_(bus_name) {}
        BusEdgeType(std::string_view bus_name, size_t span_count, double time) : 
                                bus_name_(bus_name), span_count_(span_count), time_(time) {}
        std::string_view bus_name_;
        size_t span_count_ = 0;
        double time_ = 0.0;
    };

    struct BuildedRoute
    {
        std::vector<std::unique_ptr<RouteItem>> items_;
        double total_weight_ = 0.0;
    };

    using EdgeType = std::variant<WaitEdgeType, BusEdgeType>;

    class Router {
        public:
        void SetRouteSettings(int wait_time, double bus_velocity);
		const RouteSettings& GetRouteSettings() const;
        void FormGraph(const transport_catalogue::TransportCatalogue &transp_catalogue);
        const graph::DirectedWeightedGraph<double>& GetGraph() const;
        std::optional<BuildedRoute> BuildRoute(std::string_view start_stop, std::string_view end_stop) const;
        const EdgeType& GetEdgeType(graph::EdgeId id) const;
        const std::string_view GetStopNameByVertexId(graph::VertexId id) const;
        private:
        void SetAllStops();
        void SetAllBuses();
        std::optional<graph::Router<double>::RouteInfo> BuildRouteImpl(std::string_view start_stop, std::string_view end_stop) const;
        const transport_catalogue::TransportCatalogue *transp_catalogue_;
        std::unordered_map<graph::VertexId, const Stop&> vertex_id_stop_;
        graph::VertexId vertex_id_ = 0;

        std::unordered_map<graph::EdgeId, EdgeType> edge_id_type_;
        std::unordered_map<std::string_view, std::pair<graph::VertexId, graph::VertexId>> stopname_to_vertex_id_;

        std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
        std::unique_ptr<graph::Router<double>> router_;

        RouteSettings route_settings_;
    };
}