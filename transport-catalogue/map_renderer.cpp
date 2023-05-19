#include "map_renderer.h"

namespace TC::Renderer {

    svg::Document MapRenderer::Render(const RequestHandler& request_handler){

        const auto& buses = request_handler.GetBuses();

        SphereProjector projector(request_handler.GetMinCoordinates(), 
                                  request_handler.GetMaxCoordinates(), 
                                  settings_.width, 
                                  settings_.height, 
                                  settings_.padding);

        svg::Document document;

        for(const auto& bus : buses){
            DrawRoute(bus, document, projector);
        }

        return document;
    }

    void MapRenderer::DrawRoute(const Bus& bus, svg::Document& document, SphereProjector& projector){
        
        //lines_.push_back(svg::Polyline{});
        //auto& route = lines_.back();
        //std::unique_ptr<svg::Drawable> picture;
        svg::Polyline route;

        route.SetStrokeWidth(settings_.line_width);
        route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for(const auto& stop : bus.GetStops()){
            route.AddPoint(projector(stop->getCoordinates()));
            
        }
        //picture = (std::make_unique<svg::Polyline>(route));
        //document.Add(route);
    }


    namespace detail {
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }
    } // namespace detail
} // namespace TC