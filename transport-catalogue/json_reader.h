#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "domain.h"

namespace TC {

    namespace Input{

        class JSONReader : public Reader<json::Array, json::Dict, json::Node>{
        public:
            JSONReader(std::istream& stream);

            const json::Array& GetRequestNodesAsArray(std::string_view name) override;
            const json::Dict& GetRequestNodesAsMap(std::string_view name) override;

            std::string_view GetFieldAsString(const json::Node& node, std::string_view name) override;
            bool GetFieldAsBool(const json::Node& node, std::string_view name) override;
            double GetFieldAsDouble(const json::Node& node, std::string_view name) override;
            int GetFieldAsInt(const json::Node& node, std::string_view name) override;

            const json::Node& GetFieldAsNode(const json::Node& node, std::string_view name) override;

            const json::Array& GetFieldAsArrayNodes(const json::Node& node, std::string_view name) override;
            const json::Dict& GetFieldAsMapNodes(const json::Node& node, std::string_view name) override;

            void PrintOut(json::Node node, std::ostream& out) override{
                PrintNode(node, out);
            }

        private:
            json::Document document_;
        };

    } // namespace Input
} // namespace TC