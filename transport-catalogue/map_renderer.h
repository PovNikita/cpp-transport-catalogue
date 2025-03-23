#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include <optional>
#include <string>

#include "svg.h"
#include "geo.h"
#include "domain.h"
#include "request_handler.h"

namespace map_render {

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    const double EPSILON = 1e-6;

    bool IsZero(double value);
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class Render {
public:
    void DrawMap(std::ostream& output, const std::deque<Bus>& busses, const RenderSettings* settings);
    void DrawRoute(const Bus& bus, svg::Color color, const SphereProjector& proj);
    void DrawRouteLabel(const Bus& bus, svg::Color color, const SphereProjector& proj);
    void DrawStops(const Bus& bus, const SphereProjector& proj);
    void DrawStop(const Stop& stop, const SphereProjector& proj);
    void DrawStopLabel(const Stop& stop, const SphereProjector& proj);
    void DrawStopsLabel(const Bus& bus, const SphereProjector& proj);

    void AddRouteLabel(const std::string& data, svg::Point position, svg::Color color);
    void AddStopLabel(const std::string& data, svg::Point position);
private:
    void SetUpSettings(const RenderSettings& settings);
    RenderSettings const* settings_;
    svg::Document doc_;
};

};