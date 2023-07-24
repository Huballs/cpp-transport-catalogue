#include "serialization.h"

namespace TC {

        namespace detail {

            inline TC_PROTO::Stop* StopToProto(const Stop& stop){
                TC_PROTO::Stop* proto_stop = new TC_PROTO::Stop();
                proto_stop->set_id(stop.GetIndex());
                proto_stop->set_name(stop.GetNameStr());
                proto_stop->set_latitude(stop.getCoordinates().lat);
                proto_stop->set_longitude(stop.getCoordinates().lng);

                return proto_stop;
            }

            inline TC_PROTO::Bus* BusToProto(const Bus& bus){
                TC_PROTO::Bus* proto_bus = new TC_PROTO::Bus();

                proto_bus->set_name(bus.GetNameStr());
                proto_bus->set_is_roundtrip(bus.IsCircular());

                for(const Stop* stop : bus.GetStops()){
                    proto_bus->mutable_stops()->Add(stop->GetIndex());
                }

                return proto_bus;

            }

            inline TC_PROTO::Distance* StopsDistanceToProto(const std::pair<TC::Stop *, TC::Stop *>& stops, uint32_t distance){
                TC_PROTO::Distance* proto_dist = new TC_PROTO::Distance();

                proto_dist->set_start(stops.first->GetIndex());
                proto_dist->set_end(stops.second->GetIndex());
                proto_dist->set_distance(distance);

                return proto_dist;
            }

            inline TC_PROTO::Point* PointToProto(const svg::Point& point){
                TC_PROTO::Point* result = new TC_PROTO::Point();
                result->set_x(point.x);
                result->set_y(point.y);
                return result;
            }

            inline TC_PROTO::Color* ColorToProto(const svg::Color& color){
                TC_PROTO::Color* result = new TC_PROTO::Color();

                if(std::holds_alternative<std::monostate>(color)){
                    result->set_none(true);
                } else
                if(std::holds_alternative<svg::Rgb>(color)){
                    const auto& rgb = std::get<svg::Rgb>(color);
                    TC_PROTO::Rgb* rgb_proto = new TC_PROTO::Rgb();
                    rgb_proto->set_red(rgb.red);
                    rgb_proto->set_green(rgb.green);
                    rgb_proto->set_blue(rgb.blue);

                    result->set_allocated_rgb(rgb_proto);
                } else
                if (std::holds_alternative<svg::Rgba>(color)){
                    const auto& rgba = std::get<svg::Rgba>(color);
                    TC_PROTO::Rgba* rgba_proto = new TC_PROTO::Rgba();
                    rgba_proto->set_red(rgba.red);
                    rgba_proto->set_green(rgba.green);
                    rgba_proto->set_blue(rgba.blue);
                    rgba_proto->set_opacity(rgba.opacity);

                    result->set_allocated_rgba(rgba_proto);
                } else
                if(std::holds_alternative<std::string>(color)){
                    const auto& string_color = std::get<std::string>(color);
                    result->set_string_color(string_color);
                } 
                else{
                    result->set_none(true);
                }

                return result;
            }

            inline svg::Point ProtoToPoint(const TC_PROTO::Point& proto_point){

                svg::Point result;

                result.x = proto_point.x();
                result.y = proto_point.y();

                return result;
            }

            inline svg::Color ProtoToColor(const TC_PROTO::Color& proto_color){

                svg::Color result;

                if(proto_color.none()){
                    result = std::monostate{};
                } else 
                if(proto_color.has_rgb()){
                    result = svg::Rgb{static_cast<uint8_t>(proto_color.rgb().red())
                                    , static_cast<uint8_t>(proto_color.rgb().green())
                                    , static_cast<uint8_t>(proto_color.rgb().blue())};
                } else 
                if(proto_color.has_rgba()){
                    result = svg::Rgba{static_cast<uint8_t>(proto_color.rgba().red())
                                    , static_cast<uint8_t>(proto_color.rgba().green())
                                    , static_cast<uint8_t>(proto_color.rgba().blue())
                                    , proto_color.rgba().opacity()};
                } else 
                if(!proto_color.string_color().empty()){
                    result = proto_color.string_color();
                }

                return result;
            }
    }

    TC_PROTO::RenderSettings* RenderSettingsToProto(const map_settings_t& settings){
        using namespace detail; 

        TC_PROTO::RenderSettings* settings_proto = new TC_PROTO::RenderSettings();

        settings_proto->set_width(settings.width);
        settings_proto->set_height(settings.height);
        settings_proto->set_padding(settings.padding);
        settings_proto->set_line_width(settings.line_width);
        settings_proto->set_stop_radius(settings.stop_radius);
        settings_proto->set_bus_label_font_size(settings.bus_label_font_size);
        settings_proto->set_allocated_bus_label_offset(PointToProto(settings.bus_label_offset));
        settings_proto->set_stop_label_font_size(settings.stop_label_font_size);
        settings_proto->set_allocated_stop_label_offset(PointToProto(settings.stop_label_offset));
        settings_proto->set_allocated_underlayer_color(ColorToProto(settings.underlayer_color));
        settings_proto->set_underlayer_width(settings.underlayer_width);
        
        for(const auto& color : settings.color_palette){
            settings_proto->mutable_color_palette()->AddAllocated(ColorToProto(color));
        }

        return settings_proto;

    }

    TC_PROTO::TransportCatalogue* TransportCatalogueToProto(const TransportCatalogue& catalogue){
        TC_PROTO::TransportCatalogue* proto_catalogue = new TC_PROTO::TransportCatalogue();

        using namespace detail;

        for(const auto& stop : catalogue.GetStops()){
            proto_catalogue->mutable_stops()->AddAllocated(StopToProto(stop));
        }

        for(const auto& bus : catalogue.GetBuses()){
            proto_catalogue->mutable_buses()->AddAllocated(BusToProto(bus));
        }

        for(const auto& [stops, dist] : catalogue.GetStopsDistances()){
            proto_catalogue->mutable_distances()->AddAllocated(StopsDistanceToProto(stops, dist));
        }

       return proto_catalogue;

    }

    TC_PROTO::TransportRouter* TransportRouterToProto(const TransportRouter& transport_router){
        using namespace detail;

        TC_PROTO::TransportRouter* proto_router = new TC_PROTO::TransportRouter();

        proto_router->mutable_settings()->set_bus_velocity_kmh(transport_router.GetSettings().bus_velocity_kmh);
        proto_router->mutable_settings()->set_bus_wait_time_min(transport_router.GetSettings().bus_wait_time_min);

        for(const auto& edge : transport_router.GetGraph().GetEdges()){

            TC_PROTO::Edge proto_edge;

            proto_edge.set_from(edge.from);
            proto_edge.set_to(edge.to);
            proto_edge.set_weight(edge.weight);

            proto_router->mutable_graph()->mutable_edges()->Add(std::move(proto_edge));
        }

        for(const auto& routes_internal_data : transport_router.GetRouter().GetRoutesInternalData()){
            TC_PROTO::RouteInternalDataList proto_list;
            for(const auto& route_data : routes_internal_data){
                TC_PROTO::RouteInternalData proto_data;
                if(route_data){
                    proto_data.set_empty_data(false);
                    proto_data.set_weight(route_data->weight);
                    if(route_data->prev_edge){
                        proto_data.set_empty_edge(false);
                        proto_data.set_prev_edge(route_data->prev_edge.value());
                    } else{
                        proto_data.set_empty_edge(true);
                    }
                    proto_list.mutable_routes_internal_data_list();
                } else {
                    proto_data.set_empty_data(true);
                }
                proto_list.mutable_routes_internal_data_list()->Add(std::move(proto_data));
            }
            proto_router->mutable_routes_internal_data()->Add(std::move(proto_list));
        }

        for(const auto& edge_info : transport_router.GetEdgeInfos()){
            TC_PROTO::EdgeInfo proto_edge_info;
            proto_edge_info.set_bus_id(edge_info.bus_id);
            proto_edge_info.set_distance_m(edge_info.distance_m);
            proto_edge_info.set_from(edge_info.from);
            proto_edge_info.set_to(edge_info.to);
            proto_edge_info.set_span_count(edge_info.span_count);

            proto_router->mutable_edge_infos()->Add(std::move(proto_edge_info));
        }

        return proto_router;
    }

    void ProtoToTransportCatalogue(TransportCatalogue& catalogue, const TC_PROTO::TransportCatalogue& proto_catalogue){
        for(const auto& stop : proto_catalogue.stops()){
            catalogue.AddStop(stop.name(), {stop.latitude(), stop.longitude()});
        }

        for(const auto& dist : proto_catalogue.distances()){
            catalogue.AddDistance(dist.start(), dist.end(), dist.distance());
        }

        for(const auto& bus : proto_catalogue.buses()){
            std::vector<size_t> indexed_stops;
            indexed_stops.reserve(bus.stops_size());

            for(size_t index : bus.stops()){
                indexed_stops.push_back(index);
            }

            catalogue.AddRoute(bus.name(), indexed_stops, bus.is_roundtrip());
        }
    }

    void ProtoToRenderSettings(map_settings_t& render_settings, const TC_PROTO::RenderSettings& proto_render_settings){
        using namespace detail; 

        render_settings.width = proto_render_settings.width();
        render_settings.height = proto_render_settings.height();
        render_settings.padding = proto_render_settings.padding();
        render_settings.line_width = proto_render_settings.line_width();
        render_settings.stop_radius = proto_render_settings.stop_radius();
        render_settings.bus_label_font_size = proto_render_settings.bus_label_font_size();
        render_settings.stop_label_font_size = proto_render_settings.stop_label_font_size();
        render_settings.underlayer_width = proto_render_settings.underlayer_width();

        render_settings.bus_label_offset = ProtoToPoint(proto_render_settings.bus_label_offset());
        
        render_settings.stop_label_offset = ProtoToPoint(proto_render_settings.stop_label_offset());
        
        render_settings.underlayer_color = ProtoToColor(proto_render_settings.underlayer_color());

        for(const auto& color_node : proto_render_settings.color_palette()){
            render_settings.color_palette.push_back(ProtoToColor(color_node));
        }
    }

    void ProtoToTransportRouter(TransportRouter& transport_router, const TC_PROTO::TransportRouter& proto_router){

        auto graph = std::make_unique<graph::DirectedWeightedGraph<TransportRouter::Weight>>();

        for(const auto& proto_edge : proto_router.graph().edges()){
            graph::Edge<TransportRouter::Weight> edge;

            edge.from = proto_edge.from();
            edge.to = proto_edge.to();
            edge.weight = proto_edge.weight();

            graph->AddEdge(edge);
        }

        //auto router = std::make_unique<graph::Router<TransportRouter::Weight>>();

        transport_router.SetGraph(std::move(graph));

        graph::Router<TransportRouter::Weight>::RoutesInternalData router_data;

        for(const auto& proto_data_list : proto_router.routes_internal_data()){
            router_data.emplace_back();
            for(const auto& proto_data : proto_data_list.routes_internal_data_list()){
                graph::Router<TransportRouter::Weight>::RouteInternalData data;
                if(proto_data.empty_data()){
                    router_data.back().push_back(std::nullopt);  
                } else{
                    data.weight = proto_data.weight();
                    if(proto_data.empty_edge()){
                        data.prev_edge = std::nullopt;
                    } else {
                        data.prev_edge = proto_data.prev_edge();
                    }
                    router_data.back().push_back(std::move(data));
                }
                
            }
        }

        auto router = std::make_unique<graph::Router<TransportRouter::Weight>>(transport_router.GetGraph(), router_data);
        
        transport_router.SetRouter(std::move(router));

        routing_settings_t settings;
        settings.bus_velocity_kmh = proto_router.settings().bus_velocity_kmh();
        settings.bus_wait_time_min = proto_router.settings().bus_wait_time_min();

        transport_router.SetSettings(settings);

        std::vector<TransportRouter::EdgeInfo> edge_infos;
        edge_infos.reserve(proto_router.edge_infos_size());

        for(const auto& proto_edge_info : proto_router.edge_infos()){
            TransportRouter::EdgeInfo edge_info;
            edge_info.bus_id = proto_edge_info.bus_id();
            edge_info.distance_m = proto_edge_info.distance_m();
            edge_info.from = proto_edge_info.from();
            edge_info.to = proto_edge_info.to();
            edge_info.span_count = proto_edge_info.span_count();
            edge_infos.push_back(std::move(edge_info));
        }

        transport_router.SetEdgeInfos(edge_infos);
    }

    void DeseriallizeBusManager(TransportCatalogue& catalogue, map_settings_t& render_settings, TransportRouter& transport_router, std::istream& in_stream){
        
        TC_PROTO::BusManager bus_manager;

        bus_manager.ParseFromIstream(&in_stream);

        ProtoToTransportCatalogue(catalogue,  bus_manager.transport_catalogue());
        ProtoToRenderSettings(render_settings, bus_manager.render_settings());
        ProtoToTransportRouter(transport_router, bus_manager.transport_router());
    }

}