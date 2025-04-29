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



namespace transport_catalogue{
	
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
		
		private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop&> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus&> busname_to_bus_;
		TransportCatalogueMap map_;
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