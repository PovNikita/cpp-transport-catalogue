#pragma once
#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cassert>
#include <string_view>
#include <optional>

#include "geo.h"

namespace transport_catalogue{

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
	};
	

	class TransportCatalogue {
		public:
		const Stop& AddStop(Stop& bus_stop_);
		const Stop* SearchStop(std::string_view stop_name) const;
		const Bus& AddRoute(Bus& bus_route);
		template <typename ForwardIt1, typename ForwardIt2>
		const Bus& AddRoute(std::string_view route_name, ForwardIt1 first_stop_name, ForwardIt2 last_stop_name);
		const Bus* SearchRoute(std::string_view route_name) const;
		std::optional<RouteInfo> GetInfoAboutRoute(std::string_view route_name) const;
		std::optional<StopInfo> GetInfoAboutBusesViaStop(std::string_view stop_name) const;
		private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop&> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus&> busname_to_bus_;
		

	};

	template <typename ForwardIt1, typename ForwardIt2>
	const Bus& TransportCatalogue::AddRoute(std::string_view route_name, ForwardIt1 first_stop_name, ForwardIt2 last_stop_name)
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
		return AddRoute(bus);
	}

};