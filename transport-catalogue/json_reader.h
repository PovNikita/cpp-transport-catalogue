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

    std::vector<std::string> stops_;
    bool is_round_trip = false;
};

class StopReadCommand : public ReadCommandDescription {
public:
    StopReadCommand() : ReadCommandDescription("Stop") {}

    geo::Coordinates cor_;
    std::unordered_map<std::string, double> distance_to_stop_;
};

class InputReader : public InputInterface {
public:
    void FormCatalogue(std::istream& input, transport_catalogue::TransportCatalogue& catalogue, map_render::RenderSettings* settings) override;
    void FormRequsts(std::istream& input, std::queue<RequestDescription>& requests) override;
private:
    void ReadJson(std::istream& input);
    void ParseJsonInputRequests();
    void ParseJsonStatRequests(std::queue<RequestDescription>& requests);
    void ParseJsonRenderSettings(map_render::RenderSettings* settings);
    void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

    std::vector<std::unique_ptr<ReadCommandDescription>> stop_comands_;
    std::vector<std::unique_ptr<ReadCommandDescription>> bus_comands_;
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

class StatAnswer : public OutputInterface {
public:
    void HandleRequests(std::ostream& output, std::queue<RequestDescription>& requsts, 
                        const transport_catalogue::TransportCatalogue& catalogue, map_render::RenderSettings& render_settings) override;
private:
    void AddAnswerToArr(AnswerDescription* answer);
    json::Builder builder_{};
    json::Array arr_;
};