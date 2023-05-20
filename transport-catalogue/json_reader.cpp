#include "json_reader.h"
#include <string>
#include <sstream>

namespace TC {

    namespace Input{

        JSONReader::JSONReader(std::istream& stream) : document_(json::Load(stream)){

            if(!document_.GetRoot().IsMap())
                throw json::ParsingError("Json::reader top level parsing error");
        }

        const json::Node& JSONReader::GetFieldAsNode(const json::Node& node, std::string_view name) {

            if(!node.IsMap())
                throw json::ParsingError("Json::reader bottom level base parsing error");
            
            return node.AsMap().at(std::string(name));
        }

        const json::Array& JSONReader::GetRequestNodesAsArray(std::string_view name){

            const auto& requests_node = document_.GetRoot().AsMap().find(std::string(name))->second;

            if(!requests_node.IsArray())
                throw json::ParsingError("Json::reader second level base parsing error");
            
             return requests_node.AsArray();
        }

        const json::Dict& JSONReader::GetRequestNodesAsMap(std::string_view name){

            const auto& requests_node = document_.GetRoot().AsMap().find(std::string(name))->second;

            if(!requests_node.IsMap())
                throw json::ParsingError("Json::reader second level base parsing error");
            
             return requests_node.AsMap();
        }

        std::string_view JSONReader::GetFieldAsString(const json::Node& node, std::string_view name){

            if(!node.IsMap())
                throw json::ParsingError("Json::reader bottom level base parsing error");
            
            return node.AsMap().at(std::string(name)).AsString();
        }

        bool JSONReader::GetFieldAsBool(const json::Node& node, std::string_view name){

            if(!node.IsMap())
                throw json::ParsingError("Json::reader bottom level base parsing error");
            
            return node.AsMap().at(std::string(name)).AsBool();
        }

        const json::Array& JSONReader::GetFieldAsArrayNodes(const json::Node& node, std::string_view name){

            if(!node.IsMap())
                throw json::ParsingError("Json::reader bottom level base parsing error");
            
            return node.AsMap().at(std::string(name)).AsArray();
        }

        double JSONReader::GetFieldAsDouble(const json::Node& node, std::string_view name){

            if(!node.IsMap())
                throw json::ParsingError("Json::reader bottom level base parsing error");
            
            return node.AsMap().at(std::string(name)).AsDouble();
        }

        const json::Dict& JSONReader::GetFieldAsMapNodes(const json::Node& node, std::string_view name){

            if(!node.IsMap())
                throw json::ParsingError("Json::reader bottom level base parsing error");
            
            return node.AsMap().at(std::string(name)).AsMap();
        }

        int JSONReader::GetFieldAsInt(const json::Node& node, std::string_view name){

            if(!node.IsMap())
                throw json::ParsingError("Json::reader bottom level base parsing error");
            
            return node.AsMap().at(std::string(name)).AsInt();
        }
    }
}