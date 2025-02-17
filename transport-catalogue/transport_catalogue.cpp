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

};