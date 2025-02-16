#include "stat_reader.h"
#include <string>
#include <ostream>
#include <optional>

using namespace transport_catalogue;

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output)
{
    using namespace std;
    string command(request.begin(), request.begin() + request.find_first_of(" "));
    if(command == "Bus"s)
    {
        string route_name(request.begin() + request.find_first_of(" ") + 1, request.end());
        std::optional<RouteInfo> info = tansport_catalogue.GetInfoAboutRoute(route_name);
        if(!info)
        {
            output << "Bus "s << route_name << ": not found" << endl;
            return;
        }
        else
        {
            output << "Bus "s << route_name << ": "s << info.value().number_of_stops_ << " stops on route, "s
                    << info.value().number_of_uniq_stops_ <<" unique stops, "s << info.value().route_length_ <<" route length, "s
                    << info.value().curvature << " curvature"s << endl;
            return;
        }
    }
    else if(command == "Stop"s)
    {
        string stop_name(request.begin() + request.find_first_of(" ") + 1, request.end());
        std::optional<StopInfo> info = tansport_catalogue.GetInfoAboutBusesViaStop(stop_name);
        if(!info)
        {
            output << "Stop "s << stop_name << ": not found" << endl;
            return;
        }
        else
        {
            if(info.value().route_names_.empty())
            {
                output << "Stop "s << stop_name << ": no buses"s << endl;
            }
            else
            {
                output << "Stop "s << stop_name << ": buses"s;
                for(auto el : info.value().route_names_)
                {
                    output << " " << el;
                }
                output << endl;
            }
            return;
        }
    }
}