#include "test_transport_catalogue.h"

#include "transport_catalogue.h"
#include <string>
#include <cassert>
#include <algorithm>
#include <optional>

#include "geo.h"

const double EPSILON = 1E-6;

using namespace std;
void TestTransportCatalogue()
{
    using namespace transport_catalogue;
    {
        Stop stop1 = {"stop_name"s, {55.611087, 37.208290}};
        Stop stop_copy1 = stop1;
        TransportCatalogue catalogue;
        catalogue.AddStop(stop_copy1);
        auto stop11 = catalogue.SearchStop("stop_name"s);
        assert(stop11->stop_name_ == stop1.stop_name_);
        assert(stop11->stop_coordinates_ == stop1.stop_coordinates_);

    }
    /*{ ТЕСТИРОВАНИЕ ДУБЛИКАТОВ
        Stop stop1 = {"stop_name"s, {55.611087, 37.208290}};
        Stop stop2 = {"stop_name"s, {55.611087, 37.208290}};
        Stop stop_copy1 = stop1;
        Stop stop_copy2 = stop2;
        TransportCatalogue catalogue;
        catalogue.AddStop(stop_copy1);
        catalogue.AddStop(stop_copy2);
        auto stop11 = catalogue.SearchStop("stop_name"s);
        assert(*stop11 == stop1);

    }*/

    {
        Stop stop1 = {"stop_name"s, {55.611087, 37.208290}};
        Stop stop_copy = stop1;
        TransportCatalogue catalogue;
        catalogue.AddStop(stop_copy);
        auto stop2 = catalogue.SearchStop("stop_name2"s);
        assert(!stop2);
    }

    {
        Stop stop1 = {"stop1"s, {55.611087, 37.208290}};
        Stop stop2 = {"stop2"s, {55.595884, 37.209755}};
        Stop stop3 = {"stop3"s, {55.632761, 37.333324}};
        TransportCatalogue catalogue;
        stop1 = catalogue.AddStop(stop1);
        stop2 = catalogue.AddStop(stop2);
        stop3 = catalogue.AddStop(stop3);
        Bus bus = {"Bus"s, {&stop1, &stop2, &stop3}};
        Bus bus_copy = bus;
        bus = catalogue.AddRoute(bus);
        auto bus3 = catalogue.SearchRoute("Bus"s);
        assert(bus3->bus_name_ == bus.bus_name_);
        assert(bus3->route_.size() == bus.route_.size());
        assert(equal(bus3->route_.begin(), bus3->route_.end(), bus.route_.begin()));
    }

    {
        Stop stop1 = {"stop1"s, {55.611087, 37.208290}};
        Stop stop2 = {"stop2"s, {55.595884, 37.209755}};
        Stop stop3 = {"stop3"s, {55.632761, 37.333324}};
        TransportCatalogue catalogue;
        stop1 = catalogue.AddStop(stop1);
        stop2 = catalogue.AddStop(stop2);
        stop3 = catalogue.AddStop(stop3);
        double distance = ComputeDistance({55.611087, 37.208290}, {55.595884, 37.209755});
        distance+= ComputeDistance({55.595884, 37.209755}, {55.632761, 37.333324});
        Bus bus = {"Bus"s, {&stop1, &stop2, &stop3}};
        Bus bus_copy = bus;
        bus = catalogue.AddRoute(bus);
        optional<RouteInfo> info = catalogue.GetInfoAboutRoute("Bus"s);
        assert(info.value().number_of_stops_ == 3);
        assert(info.value().number_of_uniq_stops_ == 3);
        assert(std::abs(info.value().route_length_ - distance) < EPSILON);
    }
    //Стоит проверить добавление остановок с одинаковыми именами и разными координатами
    /*
    {
        Stop stop1 = {"stop1"s, {55.611087, 37.208290}};
        Stop stop2 = {"stop1"s, {55.611087, 37.208290}};
        Stop stop3 = {"stop1"s, {55.632761, 37.333324}};
        TransportCatalogue catalogue;
        stop1 = catalogue.AddStop(stop1);
        stop2 = catalogue.AddStop(stop2);
        stop3 = catalogue.AddStop(stop3);
        double distance = ComputeDistance({55.611087, 37.208290}, {55.632761, 37.333324});
        Bus bus = {"Bus"s, {&stop1, &stop2, &stop3}};
        Bus bus_copy = bus;
        bus = catalogue.AddRoute(bus);
        RouteInfo info = catalogue.GetInfoAboutRoute("Bus"s);
        assert(info.number_of_stops_ == 3);
        assert(info.number_of_uniq_stops_ == 2);
        assert(std::abs(info.route_length_ - distance) < EPSILON);
    }*/

    {
        Stop stop1 = {"stop1"s, {55.611087, 37.208290}};
        Stop stop2 = {"stop1"s, {55.611087, 37.208290}};
        Stop stop3 = {"stop2"s, {55.632761, 37.333324}};
        TransportCatalogue catalogue;
        stop1 = catalogue.AddStop(stop1);
        stop2 = catalogue.AddStop(stop2);
        assert(stop1 == stop2);
        stop3 = catalogue.AddStop(stop3);
        double distance = ComputeDistance({55.611087, 37.208290}, {55.632761, 37.333324});
        Bus bus = {"Bus"s, {&stop1, &stop2, &stop3}};
        bus = catalogue.AddRoute(bus);
        optional<RouteInfo> info = catalogue.GetInfoAboutRoute("Bus"s);
        assert(info.value().number_of_stops_ == 3);
        assert(info.value().number_of_uniq_stops_ == 2);
        assert(std::abs(info.value().route_length_ - distance) < EPSILON);
    }
    {
        Stop stop1 = {"stop1"s, {55.611087, 37.208290}};
        Stop stop2 = {"stop1"s, {55.611087, 37.208290}};
        Stop stop3 = {"stop2"s, {55.632761, 37.333324}};
        TransportCatalogue catalogue;
        stop1 = catalogue.AddStop(stop1);
        stop2 = catalogue.AddStop(stop2);
        assert(stop1 == stop2);
        stop3 = catalogue.AddStop(stop3);
        double distance = ComputeDistance({55.611087, 37.208290}, {55.632761, 37.333324});
        std::vector<string> stop_names = {"stop1"s, "stop1"s, "stop2"s};
        std::string bus_name = "Bus";
        Bus bus = catalogue.AddRoute(bus_name, stop_names.begin(), stop_names.end());
        optional<RouteInfo> info = catalogue.GetInfoAboutRoute("Bus"s);
        assert(info.value().number_of_stops_ == 3);
        assert(info.value().number_of_uniq_stops_ == 2);
        assert(std::abs(info.value().route_length_ - distance) < EPSILON);
    }
}