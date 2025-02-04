#pragma once
#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cassert>
#include <string_view>

#include "geo.h"

namespace tr_cgue{
	
	#define EPSILON 1E-6

	struct Stop{
		std::string stop_name_ = "";
		Coordinates stop_coordinates_;
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

		operator bool()
		{
			return number_of_stops_ != 0;
		}
	};

	struct StopInfo
	{
		std::vector<std::string_view> route_names_;
		bool is_stop_found = false;

		operator bool()
		{
			return is_stop_found;
		}
	};
	

	class TransportCatalogue {
		public:
		Stop& AddStop(Stop& bus_stop_);
		Stop* SearchStop(const std::string &stop_name);
		Stop* SearchStop(const std::string_view &stop_name);
		const Stop* SearchStop(const std::string_view &stop_name) const;
		Bus& AddRoute(Bus& bus_route);
		template <typename ForwardIt1, typename ForwardIt2>
		Bus& AddRoute(std::string &route_name, ForwardIt1 first_stop_name, ForwardIt2 last_stop_name);
		Bus* SearchRoute(const std::string &route_name);
		const Bus* SearchRoute(const std::string &route_name) const;
		RouteInfo GetInfoAboutRoute(const std::string &route_name) const;
		StopInfo GetInfoAboutBusesViaStop(const std::string &stop_name) const;
		private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop&> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus&> busname_to_bus_;
		

	};

	template <typename ForwardIt1, typename ForwardIt2>
	Bus& TransportCatalogue::AddRoute(std::string &route_name, ForwardIt1 first_stop_name, ForwardIt2 last_stop_name)
	{
		Bus bus;
		bus.bus_name_ = route_name;
		while(first_stop_name != last_stop_name)
		{
			Stop *bus_stop = SearchStop(*first_stop_name);
			assert(bus_stop);
			bus.route_.push_back(bus_stop);
			++first_stop_name;
		}
		return AddRoute(bus);
	}

};