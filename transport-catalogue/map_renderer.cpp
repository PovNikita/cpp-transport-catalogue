#include "map_renderer.h"

#include <vector>
#include <string>
#include <algorithm>

namespace map_render {

    svg::Point operator+(const svg::Point lhs, const svg::Point rhs)
    {
        return {lhs.x + rhs.x, lhs.y + rhs.y};
    }

    bool SphereProjector::IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

void Render::DrawMap(std::ostream& output, const std::deque<Bus>& busses, RenderSettings* settings) {
    using namespace std::literals;
    SetUpSettings(*settings);
    std::vector<std::pair<std::string, const Bus*>> sorted_routes;
    sorted_routes.reserve(busses.size());
    for(const auto& bus : busses)
    {
        sorted_routes.push_back(std::make_pair(bus.bus_name_, &bus));
    }
    std::sort(sorted_routes.begin(), sorted_routes.end(), [](const auto& lhs, const auto& rhs) {
                                                            return lhs.first < rhs.first;});
    size_t color_iterator = 0;
    std::deque<geo::Coordinates> all_coordinates_;
    for(const auto& route : sorted_routes)
    {
        for(const auto& stop : route.second->route_)
        {
            all_coordinates_.push_back(stop->stop_coordinates_);
        }
    }
    const SphereProjector proj{
        all_coordinates_.begin(), all_coordinates_.end(), settings_->width_, settings_->height_, settings_->padding_
    };
    for(const auto& route : sorted_routes)
    {
        if(!route.second->route_.empty()) {
            DrawRoute(*route.second, settings_->color_palette_.at(color_iterator % settings_->color_palette_.size()), proj);
            ++color_iterator;
        }
    }
    color_iterator = 0;
    for(const auto& route : sorted_routes)
    {

        if(!route.second->route_.empty()) {
            DrawRouteLabel(*route.second, settings_->color_palette_.at(color_iterator % settings_->color_palette_.size()), proj);
            ++color_iterator;
        }


    }
    std::vector<std::pair<std::string, const Stop*>> sorted_stops;
    sorted_stops.reserve(busses.size());
    for(const auto& bus : busses)
    {
        if(bus.route_.size() > 0)
        {
            for(const auto& stop : bus.route_) 
            {
                sorted_stops.push_back(std::make_pair(stop->stop_name_, stop));
            }
        }
    }
    std::sort(sorted_stops.begin(), sorted_stops.end(), [](const auto& lhs, const auto& rhs) {
                                                            return lhs.first < rhs.first;});
    auto last = std::unique(sorted_stops.begin(), sorted_stops.end(), [](const auto& lhs, const auto& rhs)
                                                                        {
                                                                            return lhs == rhs;
                                                                        });
    sorted_stops.erase(last, sorted_stops.end());
    for(const auto& stop : sorted_stops)
    {
        DrawStop(*stop.second, proj);
    }
    for(const auto& stop : sorted_stops)
    {
        DrawStopLabel(*stop.second, proj);
    }

    doc_.Render(output);
}

void Render::DrawRoute(const Bus& bus, svg::Color color, const SphereProjector& proj) {
    using namespace std::literals;
    std::vector<geo::Coordinates> vector_coordinates_;
    vector_coordinates_.reserve(bus.route_.size());
    for(const auto stop : bus.route_)
    {
        vector_coordinates_.push_back(stop->stop_coordinates_);
    }

    svg::Polyline route;
    for(const auto& coor : vector_coordinates_)
    {
        route.AddPoint(proj(coor));
    }
    route.SetFillColor(svg::NoneColor).SetStrokeColor(color).SetStrokeWidth(settings_->line_width_).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    doc_.Add(route);
    
}

void Render::DrawRouteLabel(const Bus& bus, svg::Color color, const SphereProjector& proj)
{
    using namespace std::literals;
    if(bus.is_roundtrip_)
    {
        AddRouteLabel(bus.bus_name_, 
            proj(bus.route_.at(0)->stop_coordinates_),
            color);
    }
    else
    {
        if(bus.route_.size() > 2)   {
            AddRouteLabel(bus.bus_name_, 
                        proj(bus.route_.front()->stop_coordinates_),
                        color);
            if(bus.route_.at(bus.route_.size() / 2)->stop_name_ != bus.route_.front()->stop_name_) {
                AddRouteLabel(bus.bus_name_, 
                    proj(bus.route_.at(bus.route_.size() / 2)->stop_coordinates_),
                    color);
            }
        }
        else
        {
            AddRouteLabel(bus.bus_name_, 
                proj(bus.route_.front()->stop_coordinates_),
                color);
        }
    }
}

void Render::DrawStop(const Stop& stop, const SphereProjector& proj)
{
    using namespace std::literals;
    svg::Circle stop_mark;
    stop_mark.SetCenter(proj(stop.stop_coordinates_));
    stop_mark.SetRadius(settings_->stop_radius_);
    stop_mark.SetFillColor("white"s);

    doc_.Add(stop_mark);
}

void Render::DrawStops(const Bus& bus, const SphereProjector& proj) {
    using namespace std::literals;
    if(!bus.is_roundtrip_)
    {
        std::unordered_map<std::string_view, int> unique_stops_;
        for(auto stop : bus.route_)
        {
            unique_stops_[stop->stop_name_] += 1;
        }
        svg::Circle stop_mark;
        stop_mark.SetCenter(proj(bus.route_.front()->stop_coordinates_));
        stop_mark.SetRadius(settings_->stop_radius_);
        stop_mark.SetFillColor("white"s);

        doc_.Add(stop_mark);
        for(const auto stop : bus.route_)
        {
            if(unique_stops_.at(stop->stop_name_) == 1)
            {   
                svg::Circle stop_mark;
                stop_mark.SetCenter(proj(stop->stop_coordinates_));
                stop_mark.SetRadius(settings_->stop_radius_);
                stop_mark.SetFillColor("white"s);

                doc_.Add(stop_mark);
            }
        }
    }
    else
    {
        for(const auto stop : bus.route_)
        {
            svg::Circle stop_mark;
            stop_mark.SetCenter(proj(stop->stop_coordinates_));
            stop_mark.SetRadius(settings_->stop_radius_);
            stop_mark.SetFillColor("white"s);

            doc_.Add(stop_mark);
        }
    }
}

void Render::DrawStopLabel(const Stop& stop, const SphereProjector& proj) {
    AddStopLabel(stop.stop_name_, proj(stop.stop_coordinates_));
}

void Render::DrawStopsLabel(const Bus& bus, const SphereProjector& proj) {
    using namespace std::literals;
    if(!bus.is_roundtrip_)
    {
        std::unordered_map<std::string_view, int> unique_stops_;
        for(auto stop : bus.route_)
        {
            unique_stops_[stop->stop_name_] += 1;
        }
        svg::Text name, substrate;
        AddStopLabel(bus.route_.front()->stop_name_, 
                    proj(bus.route_.front()->stop_coordinates_));

        for(const auto stop : bus.route_)
        {
            if(unique_stops_.at(stop->stop_name_) == 1)
            {
                AddStopLabel(stop->stop_name_, proj(stop->stop_coordinates_));
            }
        }
    }
    else
    {
        for(const auto stop : bus.route_)
        {
            AddStopLabel(stop->stop_name_, proj(stop->stop_coordinates_));
        }
    }
}

void Render::AddRouteLabel(const std::string& data, svg::Point position, svg::Color color) {
    using namespace std::literals;
    svg::Text name, substrate;
    substrate.SetPosition(position);
    substrate.SetOffset(svg::Point(settings_->bus_label_offset_.at(0), settings_->bus_label_offset_.at(1)));
    substrate.SetData(data);
    substrate.SetFontSize(settings_->bus_label_font_size_).SetFontFamily("Verdana"s).SetFontWeight("bold"s);
    substrate.SetFillColor(settings_->underlayer_color_).SetStrokeColor(settings_->underlayer_color_);
    substrate.SetStrokeWidth(settings_->underlayer_width_).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    doc_.Add(substrate);

    name.SetPosition(position);
    name.SetData(data);
    name.SetOffset(svg::Point(settings_->bus_label_offset_.at(0), settings_->bus_label_offset_.at(1)));
    name.SetFontSize(settings_->bus_label_font_size_).SetFontFamily("Verdana"s).SetFontWeight("bold"s);
    name.SetFillColor(color);
    doc_.Add(name);
}

void Render::AddStopLabel(const std::string& data, svg::Point position) {
    using namespace std::literals;
    svg::Text name, substrate;
    substrate.SetPosition(position);
    substrate.SetData(data).SetOffset(svg::Point(settings_->stop_label_offset_.at(0), settings_->stop_label_offset_.at(1)));
    substrate.SetFontSize(settings_->stop_label_font_size_).SetFontFamily("Verdana"s);
    substrate.SetFillColor(settings_->underlayer_color_).SetStrokeColor(settings_->underlayer_color_);
    substrate.SetStrokeWidth(settings_->underlayer_width_).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    name.SetPosition(position);
    name.SetData(data);
    name.SetOffset(svg::Point(settings_->stop_label_offset_.at(0), settings_->stop_label_offset_.at(1)));
    name.SetFontSize(settings_->stop_label_font_size_).SetFontFamily("Verdana"s);
    name.SetFillColor("black"s);

    doc_.Add(substrate);
    doc_.Add(name);
}

void Render::SetUpSettings(RenderSettings& settings) {
    settings_ = &settings;
}

};