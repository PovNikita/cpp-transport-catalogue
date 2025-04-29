#include "transport_router.h"

namespace transport_router
{
    using namespace transport_catalogue;

    void Router::SetRouteSettings(int wait_time, double bus_velocity) {
        route_settings_.bus_velocity_ = bus_velocity;
        route_settings_.bus_wait_time_ = wait_time;
    }

    const RouteSettings& Router::GetRouteSettings() const {
        return route_settings_;
    }

    void Router::FormGraph(const TransportCatalogue &transp_catalogue) {
        transp_catalogue_ = &transp_catalogue;
        graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(graph::DirectedWeightedGraph<double>(transp_catalogue.GetAllStops().size() * 2));
        SetAllStops();
        SetAllBuses();
        router_ = std::make_unique<graph::Router<double>>(*graph_.get());
    }

    const graph::DirectedWeightedGraph<double>& Router::GetGraph() const {
        return *graph_.get();
    }

    std::optional<graph::Router<double>::RouteInfo> Router::BuildRoute(std::string_view start_stop, std::string_view end_stop) const {
        using namespace graph;
        if(start_stop == end_stop)
        {
            graph::Router<double>::RouteInfo info;
            info.weight = 0;
            return info;
        }
        return (*router_.get()).BuildRoute(stopname_to_vertex_id_.at(start_stop).first, stopname_to_vertex_id_.at(end_stop).first);
    }

    const EdgeType* Router::GetEdgeType(graph::EdgeId id) const
    {
        using namespace graph;
        return edge_id_type_.at(id).get();
    }

    const std::string_view Router::GetStopNameByVertexId(graph::VertexId id) const
    {
        if(id % 2 == 0)
        {
            return vertex_id_stop_.at(id).stop_name_;
        }
        else
        {
            return vertex_id_stop_.at(id-1).stop_name_;
        }
    }

    void Router::SetAllStops() {
        using namespace graph;
        for(const auto& stop : transp_catalogue_->GetAllStops())
        {
            Edge edge{vertex_id_, vertex_id_ + 1, static_cast<double>(route_settings_.bus_wait_time_)};
            edge_id_type_.insert({(*graph_.get()).AddEdge(edge), std::make_unique<WaitEdgeType>(stop.stop_name_)});
            stopname_to_vertex_id_.insert({stop.stop_name_, std::make_pair(vertex_id_, vertex_id_ + 1)});
            vertex_id_stop_.insert({vertex_id_, stop});
            vertex_id_ += 2;
            
        }
    }

    void Router::SetAllBuses() {
        using namespace graph;
        for(const auto& bus : transp_catalogue_->GetAllRoutes())
        {
            for(size_t i = 0; i < bus.route_.size() - 1; ++i)
            {
                const auto& stop_from = *(bus.route_.at(i));
                double distance = 0.0;
                for(size_t j = i + 1; j < bus.route_.size(); ++j)
                {
                    const auto& stop_to = *(bus.route_.at(j));
                    distance += transp_catalogue_->GetDistance(bus.route_.at(j-1)->stop_name_, stop_to.stop_name_);
                    double time = (distance / (1000 * route_settings_.bus_velocity_)) * 60; // 1000 - meters to km, 60 hours to minutes
                    Edge edge{stopname_to_vertex_id_.at(stop_from.stop_name_).second, 
                                stopname_to_vertex_id_.at(stop_to.stop_name_).first, 
                                time};
                    edge_id_type_.insert({(*graph_.get()).AddEdge(edge), std::make_unique<BusEdgeType>(bus.bus_name_, j - i, time)});
                }
            }
        }
    }
}