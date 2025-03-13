#pragma once

#include <sstream>
#include <queue>
#include <string>

#include "transport_catalogue.h"
#include "svg.h"

class RequestDescription {
public:
    int id_ = 0;
    std::string type_ = "";
    std::string name_ = "";

    virtual ~RequestDescription() = default;
};

class RenderSettings
{
public:
    //canvas settings
    double width_ = 0.0;
    double height_ = 0.0;
    double padding_ = 0.0;

    //stop srttings
    double stop_radius_ = 0.0;
    size_t stop_label_font_size_ = 0;
    std::vector<double> stop_label_offset_;

    // routes settings
    size_t bus_label_font_size_ = 0;
    std::vector<double> bus_label_offset_;
    double line_width_ = 0.0;

    //underlayer settings
    svg::Color underlayer_color_;
    double underlayer_width_ = 0.0;

    //other settings
    std::vector<svg::Color> color_palette_;
};

class DrawMapInterface {
public:
    virtual void DrawMap(std::ostream& output, const std::deque<Bus>& busses, RenderSettings *settings) = 0;
    virtual ~DrawMapInterface() = default;
};

class InputInterface {
public:
    virtual void FormCatalogue(std::istream& input, transport_catalogue::TransportCatalogue& catalogue, RenderSettings *settings) = 0;
    virtual void FormRequsts(std::istream& input, std::queue<RequestDescription>& requsts) = 0;
    virtual ~InputInterface() = default;
};
    
class OutputInterface {
public:
    virtual void HandleRequests(std::ostream& output, std::queue<RequestDescription>& requsts, const transport_catalogue::TransportCatalogue& catalogue, RenderSettings& render_settings) = 0;
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
    RenderSettings render_settings_;
};