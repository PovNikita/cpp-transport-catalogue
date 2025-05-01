#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "geo.h"

struct Stop{
    std::string stop_name_ = "";
    geo::Coordinates stop_coordinates_;
    std::unordered_set<std::string_view> routes_ = {};
    bool operator==(const Stop &rhs)
    {
        return (this->stop_name_ == rhs.stop_name_) &&
                (this->stop_coordinates_ == rhs.stop_coordinates_);
    }
};

class StopHasher{
public:
    size_t operator()(const Stop* bus_stop) const
    {
        return std::hash<std::string>{}(bus_stop->stop_name_);
        
    }
};

struct StopEqual {
    bool operator()(const Stop* lhs, const Stop* rhs) const {
        return lhs->stop_name_ == rhs->stop_name_;
    }
};

struct Bus{
    std::string bus_name_ = "";
    std::vector<Stop*> route_;
    bool is_roundtrip_ = false;

    bool operator==(const Bus &rhs)
    {
        return this->bus_name_ == rhs.bus_name_ &&
                this->route_ == rhs.route_;
    }
};

struct RouteInfo{
    size_t number_of_stops_ = 0;
    size_t number_of_uniq_stops_ = 0;
    double route_length_ = 0.0;
    double curvature = 1.0;

    operator bool()
    {
        return number_of_stops_ != 0;
    }

};

struct StopInfo
{
    std::vector<std::string_view> route_names_;
};

struct RouteSettings
{
    int bus_wait_time_ = 0;
    double bus_velocity_ = 0.0;
};

class RouteItem {
    public:
        explicit RouteItem(std::string type) : type_(type) {}
    
        std::string type_;
        virtual ~RouteItem() = default;
};
    
class WaitRouteItem : public RouteItem {
    public:
        WaitRouteItem() : RouteItem("Wait") {}
        std::string stop_name_;
        int time_;
};

class BusRouteItem : public RouteItem {
    public:
        BusRouteItem() : RouteItem("Bus") {}
        std::string bus_;
        int span_count_ = 0;
        double time_ = 0;
};