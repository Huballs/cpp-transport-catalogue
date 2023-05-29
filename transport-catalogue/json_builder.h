#pragma once

#include "json.h"
#include <stack>
#include <optional>

namespace json {
    
    class Builder;

    namespace detail {

        class Context {
        public:
            Context(Builder& builder) : builder_(builder){};

        protected:
            Builder& builder_;
            
        };

        class DictItemContext : private Context {
            public:
                DictItemContext(const Context& context) : Context(context) {}

                inline auto& EndDict();
                inline auto Key(const std::string& value);
        };

        class DictKeyValueContext : private Context {
            public:
                DictKeyValueContext(const Context& context) : Context(context) {}

                inline auto Value(const Node& value);
                inline auto StartDict();
                inline auto StartArray();
        };

        class ValueContext : private Context  {
            public:
                ValueContext(const Context& context) : Context(context) {}

                inline auto& Value(const Node& value);
                inline auto StartDict();
                inline auto StartArray();
        };

        class ArrayItemContext : private Context {
            public:
                ArrayItemContext(const Context& context) : Context(context) {}

                inline auto Value(const Node& value);
                inline auto StartDict();
                inline auto StartArray();
                inline auto& EndArray();
        };
    } // namespace detail

    class Builder {
    public:

        Builder();

        Builder& Value(const Node& value);

        detail::DictItemContext StartDict();
        Builder& EndDict();
        detail::DictKeyValueContext Key(const std::string& value);
        detail::ArrayItemContext StartArray();
        Builder& EndArray();

        const Node& Build();

        void Print(std::ostream& output);

    private:

        enum class j_t{
            empty,
            dict,
            array,
            node,
        };

        Builder& StartDictOrArray(j_t type);
        Builder& EndDictOrArray(j_t type);

        Node& PushToArray(const Node& value);
        Node& PushToDict(const Node& value);

        void ThrowIfFinished(); 

        struct BuildNode{
            
            BuildNode(Node& value, j_t type) : value_(value), type_(type){};

            ~BuildNode() {
                switch(type_){
                    case j_t::array:
                        value_ = std::move(array_);
                        break;
                    case j_t::dict:
                        value_ = std::move(dict_);
                        break;
                    case j_t::node: // should already be in &value_
                        break;
                    default:
                        value_ = nullptr;
                }
            }

            Node& value_;
            j_t type_ = j_t::empty;
            Dict dict_;
            Array array_;

        };

        Node root_;
        std::optional<std::string> current_key_; // if key was used == nullopt
        
        std::stack<BuildNode> BuildNodes;
    };

    namespace detail {

        inline auto& DictItemContext::EndDict(){
            return builder_.EndDict();
        }
        inline auto DictItemContext::Key(const std::string& value){
            return builder_.Key(value);
        }


        inline auto DictKeyValueContext::Value(const Node& value){
            builder_.Value(value);
            return DictItemContext(builder_);
        }
        inline auto DictKeyValueContext::StartDict(){
            return builder_.StartDict();
        }
        inline auto DictKeyValueContext::StartArray(){
            return builder_.StartArray();
        }


        inline auto& ValueContext::Value(const Node& value){
            return builder_.Value(value);
        }
        inline auto ValueContext::StartDict(){
            return builder_.StartDict();
        }
        inline auto ValueContext::StartArray(){
            return builder_.StartArray();
        }

        inline auto ArrayItemContext::Value(const Node& value){
            builder_.Value(value);
            return ArrayItemContext(builder_);
        }
        inline auto ArrayItemContext::StartDict(){
            return builder_.StartDict();
        }
        inline auto ArrayItemContext::StartArray(){
            return builder_.StartArray();
        }
        inline auto& ArrayItemContext::EndArray(){
            return builder_.EndArray();
        }
    } // namespace detail

} //namespace json