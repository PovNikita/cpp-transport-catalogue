#include "transport_catalogue.h"
#include <algorithm>

namespace transport_catalogue{
    const Stop& TransportCatalogue::AddStop(Stop& bus_stop)
    {
        const auto &stop = SearchStop(bus_stop.stop_name_);
        if(stop)
        {
            return *stop;
        }
        stops_.push_back(std::move(bus_stop));
        stopname_to_stop_.insert({stops_.back().stop_name_, stops_.back()});
        return stops_.back();
    }

    const Stop* TransportCatalogue::SearchStop(std::string_view stop_name) const
    {
        auto it = stopname_to_stop_.find(stop_name);
        if(it == stopname_to_stop_.end())
        {
            return nullptr;
        }
        return &(it->second);
    }

    const Bus& TransportCatalogue::AddRoute(Bus& bus_route)
    {
        auto bus = SearchRoute(bus_route.bus_name_);
        if(bus)
        {
            return *bus;
        }
        buses_.push_back(std::move(bus_route));
        busname_to_bus_.insert({buses_.back().bus_name_, buses_.back()});
        for(auto &stop : buses_.back().route_)
        {
            stop->routes_.insert(buses_.back().bus_name_);
        }
        return buses_.back();
    }

    const Bus* TransportCatalogue::SearchRoute(std::string_view route_name) const
    {
        auto it = busname_to_bus_.find(route_name);
        if(it == busname_to_bus_.end())
        {
            return nullptr;
        }
        return &(it->second);
    }

    void TransportCatalogue::SetDistance(std::string_view start_stop, std::string_view end_stop, double distance)
    {
        auto it_start_stop = stopname_to_stop_.find(start_stop);
        if(it_start_stop != stopname_to_stop_.end())
        {
            auto it_end_stop = stopname_to_stop_.find(end_stop);
            if(it_end_stop != stopname_to_stop_.end())
            {
                map_.AddNode(it_start_stop->first, it_end_stop->first, {distance});
            }
        }
    }

    double TransportCatalogue::GetDistance(std::string_view start_stop, std::string_view end_stop) const
    {
        std::optional<double> dis = map_.GetDistance(start_stop, end_stop);
        if(!dis.has_value())
        {
            return ComputeDistance(stopname_to_stop_.at(start_stop).stop_coordinates_, stopname_to_stop_.at(end_stop).stop_coordinates_);
        }
        return dis.value();
    }

    std::optional<RouteInfo> TransportCatalogue::GetInfoAboutRoute(std::string_view route_name) const
    {
        RouteInfo info;
        auto route = SearchRoute(route_name);
        if(!route)
        {
            return std::nullopt;
        }
        else
        {
            if(route->route_.size() == 0)
            {
                return std::optional{info};
            }
            if(route->route_.size() == 1)
            {
                info.number_of_stops_ = 1;
                info.number_of_uniq_stops_ = 1;
                return std::optional{info};
            }
            info.number_of_stops_ = route->route_.size();
            std::unordered_map<Stop*, size_t, StopHasher, StopEqual> unique_elements;
            for(const auto el : route->route_)
            {
                unique_elements[el] += 1;
            }
            info.number_of_uniq_stops_ = unique_elements.size();
            double geo_distance = 0.0;
            for(size_t i = 1; i < route->route_.size(); ++i)
            {
                info.route_length_ += GetDistance(route->route_[i-1]->stop_name_, route->route_[i]->stop_name_);
                geo_distance += ComputeDistance(route->route_[i-1]->stop_coordinates_, route->route_[i]->stop_coordinates_);
            }
            info.curvature = info.route_length_ / geo_distance;
            return std::optional{info};
        }
    }

    std::optional<StopInfo> TransportCatalogue::GetInfoAboutBusesViaStop(std::string_view stop_name) const
    {
        StopInfo stop_info;
        const Stop* stop = SearchStop(stop_name);
        if(!stop)
        {
            return std::nullopt;
        }
        else
        {
            stop_info.route_names_.reserve(stop->routes_.size());
            stop_info.route_names_.insert(stop_info.route_names_.begin(), stop->routes_.begin(), stop->routes_.end());
            std::sort(stop_info.route_names_.begin(), stop_info.route_names_.end(), [](auto &l, auto &r){
                                                return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end()); });
        }
        return std::optional{stop_info};

    }

    void TransportCatalogueMap::AddNode(std::string_view start_stop, std::string_view end_stop, TypeOfConnection node)
    {
        adjacency_matrix_[start_stop][end_stop] = node;
        auto it = adjacency_matrix_.find(end_stop);
        if(it == adjacency_matrix_.end() || it->second.find(start_stop) == it->second.end())
        {
            adjacency_matrix_[end_stop][start_stop] = node;
        }
    }

    std::optional<double> TransportCatalogueMap::GetDistance(std::string_view start_stop, std::string_view end_stop) const
    {
        auto it_start = adjacency_matrix_.find(start_stop);
        if(it_start != adjacency_matrix_.end())
        {
            auto it_stop = it_start->second.find(end_stop);
            if(it_stop != it_start->second.end())
            {
                return std::optional{it_stop->second.distance};
            }
        }
        return std::nullopt;
    }

    const std::deque<Bus>& TransportCatalogue::GetAllRoutes() const {
        return buses_;
    }

    const std::deque<Stop>& TransportCatalogue::GetAllStops() const {
        return stops_;
    }

    void TransportCatalogue::SetRouteSettings(int wait_time, double bus_velocity) {
        route_settings_.bus_velocity_ = bus_velocity;
        route_settings_.bus_wait_time_ = wait_time;
    }

    const RouteSettings& TransportCatalogue::GetRouteSettings() const {
        return route_settings_;
    }

    void TransportCatalogue::FormGraph() {
        graph_.FormGraph(*this);
    }

    const graph::DirectedWeightedGraph<double>& TransportCatalogue::GetGraph() const {
        return graph_.GetGraph();
    }

    std::optional<graph::Router<double>::RouteInfo> TransportCatalogue::BuildRoute(std::string_view start_stop, std::string_view end_stop) const
    {
        return graph_.BuildRoute(start_stop, end_stop);
    }

    const EdgeType* TransportCatalogue::GetEdgeType(graph::EdgeId id) const
    {
        return graph_.GetEdgeType(id);
    }

    const std::string_view TransportCatalogue::GetStopNameByVertexId(graph::VertexId id) const
    {
        return graph_.GetStopNameByVertexId(id);
    }

    void GraphMaker::FormGraph(const TransportCatalogue &transp_catalogue) {
        transp_catalogue_ = &transp_catalogue;
        graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(graph::DirectedWeightedGraph<double>(transp_catalogue.stops_.size() * 2));
        SetAllStops();
        SetAllBuses();
    }

    const graph::DirectedWeightedGraph<double>& GraphMaker::GetGraph() const {
        return *graph_.get();
    }

    std::optional<graph::Router<double>::RouteInfo> GraphMaker::BuildRoute(std::string_view start_stop, std::string_view end_stop) const {
        using namespace graph;
        if(start_stop == end_stop)
        {
            graph::Router<double>::RouteInfo info;
            info.weight = 0;
            return info;
        }
        Router router(*graph_.get());
        return router.BuildRoute(stopname_to_vertex_id_.at(start_stop).first, stopname_to_vertex_id_.at(end_stop).first);
    }

    const EdgeType* GraphMaker::GetEdgeType(graph::EdgeId id) const
    {
        using namespace graph;
        return edge_id_type_.at(id).get();
    }

    const std::string_view GraphMaker::GetStopNameByVertexId(graph::VertexId id) const
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

    void GraphMaker::SetAllStops() {
        using namespace graph;
        for(const auto& stop : transp_catalogue_->stops_)
        {
            Edge edge{vertex_id_, vertex_id_ + 1, static_cast<double>(transp_catalogue_->route_settings_.bus_wait_time_)};
            edge_id_type_.insert({(*graph_.get()).AddEdge(edge), std::make_unique<WaitEdgeType>(stop.stop_name_)});
            stopname_to_vertex_id_.insert({stop.stop_name_, std::make_pair(vertex_id_, vertex_id_ + 1)});
            vertex_id_stop_.insert({vertex_id_, stop});
            vertex_id_ += 2;
            
        }
    }

    void GraphMaker::SetAllBuses() {
        using namespace graph;
        for(const auto& bus : transp_catalogue_->buses_)
        {
            for(size_t i = 0; i < bus.route_.size() - 1; ++i)
            {
                const auto& stop_from = *(bus.route_.at(i));
                double distance = 0.0;
                for(size_t j = i + 1; j < bus.route_.size(); ++j)
                {
                    const auto& stop_to = *(bus.route_.at(j));
                    distance += transp_catalogue_->GetDistance(bus.route_.at(j-1)->stop_name_, stop_to.stop_name_);
                    double time = (distance / (1000 * transp_catalogue_->route_settings_.bus_velocity_)) * 60; // 1000 - meters to km, 60 hours to minutes
                    Edge edge{stopname_to_vertex_id_.at(stop_from.stop_name_).second, 
                                stopname_to_vertex_id_.at(stop_to.stop_name_).first, 
                                time};
                    edge_id_type_.insert({(*graph_.get()).AddEdge(edge), std::make_unique<BusEdgeType>(bus.bus_name_, j - i, time)});
                }
            }
        }
    }

    /*void GraphMaker::SetAllBuses() {
        using namespace graph;
        for(const auto& bus : transp_catalogue_->buses_)
        {
            for(size_t i = 1; i < bus.route_.size(); ++i)
            {
                const auto& stop_from = *(bus.route_.at(i - 1));
                const auto& stop_to = *(bus.route_.at(i));
                auto it = vertex_id_to_number_of_income_edges_.find(stopname_to_vertex_id_.at(stop_from.stop_name_).second);
                if(it == vertex_id_to_number_of_income_edges_.end() || it->second == 0)
                {
                    Edge edge{stopname_to_vertex_id_.at(stop_from.stop_name_).second, 
                                stopname_to_vertex_id_.at(stop_to.stop_name_).second, 
                                (transp_catalogue_->GetDistance(stop_from.stop_name_, stop_to.stop_name_) / (1000 * transp_catalogue_->route_settings_.bus_velocity_)) * 60}; // 1000 - meters to km, 60 hours to minutes
                    edge_id_type_.insert({(*graph_.get()).AddEdge(edge), std::make_unique<BusEdgeType>(bus.bus_name_)});
                    vertex_id_to_number_of_income_edges_[stopname_to_vertex_id_.at(stop_to.stop_name_).second] = 1;
                }
                else
                {
                    double time_in_the_road = (transp_catalogue_->GetDistance(stop_from.stop_name_, stop_to.stop_name_) / (1000 * transp_catalogue_->route_settings_.bus_velocity_)) * 60; // 1000 - meters to km, 60 hours to minutes
                    Edge edge{stopname_to_vertex_id_.at(stop_from.stop_name_).second, 
                            stopname_to_vertex_id_.at(stop_to.stop_name_).second, 
                            time_in_the_road};
                    edge_id_type_.insert({(*graph_.get()).AddEdge(edge), std::make_unique<BusEdgeType>(bus.bus_name_)});
                    edge.from = stopname_to_vertex_id_.at(stop_from.stop_name_).first;
                    edge.to = stopname_to_vertex_id_.at(stop_to.stop_name_).second;
                    edge.weight = time_in_the_road + static_cast<double>(transp_catalogue_->route_settings_.bus_wait_time_);
                    edge_id_type_.insert({(*graph_.get()).AddEdge(edge), std::make_unique<BusEdgeType>(bus.bus_name_)});
                }
            }
        }
    }*/

};