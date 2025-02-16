#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"
#include "test_transport_catalogue.h"

using namespace std;

void TestParseStopRequest();

int main() {

    TestTransportCatalogue();
    TestParseStopRequest();
    transport_catalogue::TransportCatalogue catalogue;

    int base_request_count = 0;
    cin >> base_request_count >> ws;

    {
        InputReader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(cin, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count = 0;
    cin >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(cin, line);
        ParseAndPrintStat(catalogue, line, cout);
    }
}

void TestParseStopRequest()
{
    {   
        std::string request = "55.611087, 37.20829, 3900m to Marushkino"s;
        StopRequest req = ParseStopRequest(request);
        assert(req.cor_.lat == 55.611087);
        assert(req.cor_.lng == 37.20829);
        assert(req.distance_to_stop_["Marushkino"s] == 3900.0);
    }
    {   
        std::string request = "55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"s;
        StopRequest req = ParseStopRequest(request);
        assert(req.cor_.lat == 55.595884);
        assert(req.cor_.lng == 37.209755);
        assert(req.distance_to_stop_["Marushkino"s] == 100);
        assert(req.distance_to_stop_["Rasskazovka"s] == 9900);
    }
    
}