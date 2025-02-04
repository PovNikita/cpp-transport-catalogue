#include "transport_catalogue.h"
#include <algorithm>

namespace tr_cgue{
    Stop& TransportCatalogue::AddStop(Stop& bus_stop)
    {
        auto stop = SearchStop(bus_stop.stop_name_);
        if(stop)
        {
            return *stop;
        }
        stops_.push_back(std::move(bus_stop));
        stopname_to_stop_.insert({stops_.back().stop_name_, stops_.back()});
        return stops_.back();
    }

    Stop* TransportCatalogue::SearchStop(const std::string &stop_name)
    {
        auto it = stopname_to_stop_.find(stop_name);
        if(it == stopname_to_stop_.end())
        {
            return nullptr;
        }
        return &(it->second);
    }

    Stop* TransportCatalogue::SearchStop(const std::string_view &stop_name)
    {
        auto it = stopname_to_stop_.find(stop_name);
        if(it == stopname_to_stop_.end())
        {
            return nullptr;
        }
        return &(it->second);
    }

    const Stop* TransportCatalogue::SearchStop(const std::string_view &stop_name) const
    {
        auto it = stopname_to_stop_.find(stop_name);
        if(it == stopname_to_stop_.end())
        {
            return nullptr;
        }
        return &(it->second);
    }

    Bus& TransportCatalogue::AddRoute(Bus& bus_route)
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

    Bus* TransportCatalogue::SearchRoute(const std::string &route_name)
    {
        auto it = busname_to_bus_.find(route_name);
        if(it == busname_to_bus_.end())
        {
            return nullptr;
        }
        return &(it->second);
    }

    const Bus* TransportCatalogue::SearchRoute(const std::string &route_name) const
    {
        auto it = busname_to_bus_.find(route_name);
        if(it == busname_to_bus_.end())
        {
            return nullptr;
        }
        return &(it->second);
    }

    RouteInfo TransportCatalogue::GetInfoAboutRoute(const std::string &route_name) const
    {
        RouteInfo info;
        auto route = SearchRoute(route_name);
        if(!route)
        {
            return info;
        }
        else
        {
            if(route->route_.size() == 0)
            {
                return info;
            }
            if(route->route_.size() == 1)
            {
                info.number_of_stops_ = 1;
                info.number_of_uniq_stops_ = 1;
                return info;
            }
            info.number_of_stops_ = route->route_.size();
            std::unordered_map<Stop*, size_t, StopHasher, StopEqual> unique_elements;
            for(const auto el : route->route_)
            {
                unique_elements[el] += 1;
            }
            info.number_of_uniq_stops_ = unique_elements.size();
            for(size_t i = 1; i < route->route_.size(); ++i)
            {
                info.route_length_ += ComputeDistance(route->route_[i-1]->stop_coordinates_, route->route_[i]->stop_coordinates_);
            }
            return info;
        }
    }

    StopInfo TransportCatalogue::GetInfoAboutBusesViaStop(const std::string &stop_name) const
    {
        StopInfo stop_info;
        const Stop* stop = SearchStop(stop_name);
        if(!stop)
        {
            return stop_info;
        }
        else
        {
            stop_info.is_stop_found = true;
            stop_info.route_names_.reserve(stop->routes_.size());
            stop_info.route_names_.insert(stop_info.route_names_.begin(), stop->routes_.begin(), stop->routes_.end());
            std::sort(stop_info.route_names_.begin(), stop_info.route_names_.end(), [](auto &l, auto &r){
                                                return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end()); });
        }
        return stop_info;

    }
};
