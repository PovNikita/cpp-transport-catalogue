#pragma once

#include <sstream>
#include <queue>
#include <string>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"

class RequestDescription {
public:
    int id_ = 0;
    std::string type_ = "";
    std::string name_ = "";

    virtual ~RequestDescription() = default;
};

class InputInterface {
public:
    virtual void FormCatalogue(std::istream& input, transport_catalogue::TransportCatalogue& catalogue, map_render::RenderSettings *settings) = 0;
    virtual void FormRequsts(std::istream& input, std::queue<RequestDescription>& requsts) = 0;
    virtual ~InputInterface() = default;
};
    
class OutputInterface {
public:
    virtual void HandleRequests(std::ostream& output, std::queue<RequestDescription>& requsts, const transport_catalogue::TransportCatalogue& catalogue, map_render::RenderSettings& render_settings) = 0;
    virtual ~OutputInterface() = default;
};

class Handler {
public:
    void FormCatalogueFromJson(std::istream& input, InputInterface* interface);
    void FormRequestsFromJson(std::istream& input, InputInterface* interface);
    void HandleRequestsJson(std::ostream& output, OutputInterface* interface);
private:
    transport_catalogue::TransportCatalogue catalogue_;
    std::queue<RequestDescription> requests_;
    map_render::RenderSettings render_settings_;
};