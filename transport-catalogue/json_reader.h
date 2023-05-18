#pragma once

#include <istream>
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

namespace TC {

    namespace Input{

        namespace Json {

            /* reads a stream and fills tc with buses and stops if any
                returns stream converted to json */
            json::Document Reader(RequestHandler& request_handler, std::istream& input);

            /* reads stat requests from json and puts results 
                from tc in to a stream in json format */
            void ReadStatRequests(RequestHandler& request_handler, const json::Document& document, std::ostream& output);

            Renderer::map_settings_t ReadMapRenderSettings(const json::Document& document);
        }

    } // namespace JsonReader
} // namespace TC