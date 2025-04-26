#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <memory>

#include "json.h"
#include "request_handler.h"
#include "geo.h"
#include "transport_catalogue.h"
#include "svg.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "domain.h"


class ReadCommandDescription {
public:
    explicit ReadCommandDescription(std::string type) : type_(type) {}

    std::string type_ = "";
    std::string name_ = "";
    virtual ~ReadCommandDescription() = default;
};

class BusReadCommand : public ReadCommandDescription {
public:
    BusReadCommand() : ReadCommandDescription("Bus") {}
    std::string name_ = "";
    std::vector<std::string> stops_;
    bool is_round_trip = false;
};

class StopReadCommand : public ReadCommandDescription {
public:
    StopReadCommand() : ReadCommandDescription("Stop") {}
    std::string name_ = "";
    geo::Coordinates cor_;
    std::unordered_map<std::string, double> distance_to_stop_;
};

class BusRequestDescription : public RequestDescription {
public:
    std::string name_ = "";
};

class StopRequestDescription : public RequestDescription {
public:
    std::string name_ = "";
};

class RouteRequestDescription : public RequestDescription {
public:
    std::string from = "";
    std::string to = "";
};

class InputReader : public InputInterface {
public:
    void FormCatalogue(std::istream& input, transport_catalogue::TransportCatalogue& catalogue, map_render::RenderSettings* settings) override;
    void FormRequsts(std::istream& input, std::queue<std::unique_ptr<RequestDescription>>& requests) override;
private:
    void ReadJson(std::istream& input);
    void ParseJsonInputRequests();
    void ParseJsonStatRequests(std::queue<std::unique_ptr<RequestDescription>>& requests);
    void ParseJsonRenderSettings(map_render::RenderSettings* settings);
    void ParseJsonRouterSettings(transport_catalogue::TransportCatalogue& catalogue);
    void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

    std::vector<std::unique_ptr<ReadCommandDescription>> stop_comands_;
    std::vector<std::unique_ptr<ReadCommandDescription>> bus_comands_;
    std::vector<std::unique_ptr<ReadCommandDescription>> route_commands_;
    std::unique_ptr<json::Document> doc_;
};

class AnswerDescription {
public:
    explicit AnswerDescription(std::string type) : type_(type) {}

    std::string type_ = "";
    int request_id_ = 0;
    virtual ~AnswerDescription() = default;
};

class AnswerBus : public AnswerDescription {
public:
    AnswerBus() : AnswerDescription("Bus") {}

    RouteInfo rourte_info_;
};

class AnswerStop : public AnswerDescription {
public:
    AnswerStop() : AnswerDescription("Stop") {}

    StopInfo stop_info_;
};

class AnswerMap : public AnswerDescription {
public: 
    AnswerMap() : AnswerDescription("Map") {}

    std::string svg_;
};

class AnswerError : public AnswerDescription {
public:
    AnswerError() : AnswerDescription("Error") {}
    AnswerError(std::string message) : AnswerDescription("Error"), error_message_(message) {}

    std::string error_message_;
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

class AnswerRoute : public AnswerDescription {
public:
    AnswerRoute() : AnswerDescription("Route") {}
    double total_time_ = 0.0;
    std::vector<std::unique_ptr<RouteItem>> items_;
};

class StatAnswer : public OutputInterface {
public:
    void HandleRequests(std::ostream& output, std::queue<std::unique_ptr<RequestDescription>>& requests, 
                        const transport_catalogue::TransportCatalogue& catalogue, map_render::RenderSettings& render_settings) override;
private:
    void AddAnswerToArr(AnswerDescription* answer);
    json::Builder builder_{};
    json::Array arr_;
};