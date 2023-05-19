#pragma once

#include "geo.h"
#include "svg.h"
#include "request_handler.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>

namespace TC::Renderer {

    using namespace svg;

    inline const double EPSILON = 1e-6;

    class SphereProjector;
    
    struct map_settings_t {
        double width;
        double height;
        double padding;

        double line_width;
        double stop_radius;

        int bus_label_font_size;
        Point bus_label_offset;

        int stop_label_font_size;
        Point stop_label_offset;

        Color underlayer_color;
        double underlayer_width;

        std::vector<Color> color_palette;
    };

    class MapRenderer{
    public:
        MapRenderer(map_settings_t& settings) 
            : settings_(std::move(settings)){};

        void  Render(const RequestHandler& request_handler, std::ostream& output);

    private:
        void DrawRoute(const Bus& bus, svg::Document& document, SphereProjector& projector, svg::Color color);

        std::vector<Geo::Coordinates> GetUsedCoordinates(const std::map<std::string_view, const TC::Bus *> &buses) const;

        map_settings_t settings_;

        //std::vector<svg::Polyline> lines_;
    };

    namespace detail {
        bool IsZero(double value);
    }

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding)
            : padding_(padding) 

        //template <typename PointInputIt>
       /* SphereProjector(Geo::Coordinates min_coord, Geo::Coordinates max_coord,
                        double max_width, double max_height, double padding)
            : padding_(padding) */
        {
            using namespace detail;
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }
/*
            min_lon_ = min_coord.lng;
            const double max_lon = max_coord.lng;

            max_lat_ = max_coord.lat;
            const double min_lat = min_coord.lat;*/
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
        svg::Point operator()(TC::Geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

} // namespace TC::MapRender 