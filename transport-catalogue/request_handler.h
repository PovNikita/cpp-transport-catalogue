#pragma once

#include <sstream>
#include <queue>
#include <string>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "svg.h"

class RequestDescription {
public:
    int id_ = 0;
    std::string type_ = "";

    virtual ~RequestDescription() = default;
};

class InputInterface {
public:
    virtual void FormCatalogue(std::istream& input, transport_catalogue::TransportCatalogue& catalogue, 
                                map_render::RenderSettings *settings, transport_router::Router& router) = 0;
    virtual void FormRequsts(std::istream& input, std::queue<std::unique_ptr<RequestDescription>>& requests) = 0;
    virtual ~InputInterface() = default;
};
    
class OutputInterface {
public:
    virtual void HandleRequests(std::ostream& output, std::queue<std::unique_ptr<RequestDescription>>& requests, 
                                const transport_catalogue::TransportCatalogue& catalogue, map_render::RenderSettings& render_settings,
                                const transport_router::Router& router) = 0;
    virtual ~OutputInterface() = default;
};

class Handler {
public:
    void FormCatalogueFromJson(std::istream& input, InputInterface* interface);
    void FormRequestsFromJson(std::istream& input, InputInterface* interface);
    void HandleRequestsJson(std::ostream& output, OutputInterface* interface);
    void DrawMap(std::ostream& output);
private:
    transport_catalogue::TransportCatalogue catalogue_;
    transport_router::Router router_;
    std::queue<std::unique_ptr<RequestDescription>> requests_;
    map_render::RenderSettings render_settings_;
};