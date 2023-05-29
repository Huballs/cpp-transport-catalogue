#include "json_builder.h"

namespace json {

    Builder::Builder(){
        BuildNodes.push(BuildNode(root_, j_t::empty));
    }

    Builder& Builder::StartDictOrArray(j_t type){

        ThrowIfFinished();

        if(BuildNodes.top().type_ == j_t::empty){
            BuildNodes.top().type_ = type;
            return *this;
        }

        Node* new_node;

        switch(BuildNodes.top().type_){
            case j_t::dict:
                new_node = &PushToDict(Node{});
                break;
            case j_t::array:
                new_node = &PushToArray(Node{});
                break;
            default:
                throw std::logic_error("json wrong start of container");
        }

        BuildNodes.push(BuildNode(*new_node, type));
        return *this;
    }
    
    detail::DictItemContext Builder::StartDict(){

        StartDictOrArray(j_t::dict);

        return detail::Context(*this);
    }
    
    detail::ArrayItemContext Builder::StartArray(){
        
        StartDictOrArray(j_t::array);

        return detail::Context(*this);
    }

    Builder& Builder::EndDictOrArray(j_t type){
        
        ThrowIfFinished();

        if(BuildNodes.top().type_ == type)
            BuildNodes.pop();
        else
            throw std::logic_error("json wrong end of container");
        return *this;
    }

    Builder& Builder::EndDict(){
        return EndDictOrArray(j_t::dict);
    }

    Builder& Builder::EndArray(){
        return EndDictOrArray(j_t::array);
    }

    detail::DictKeyValueContext Builder::Key(const std::string& value){
        
        ThrowIfFinished();

        if(BuildNodes.top().type_ != j_t::dict)
            throw std::logic_error("json key on the wrong type");

        if(current_key_)
            throw std::logic_error("json key unused");

        current_key_ = value;

        return detail::Context(*this);
    }

    Builder& Builder::Value(const Node& value){

        ThrowIfFinished();

        switch(BuildNodes.top().type_){
            case j_t::dict:
                PushToDict(std::move(value));
                break;
            case j_t::array:
                PushToArray(std::move(value));
                break;
            case j_t::empty:
                BuildNodes.top().value_ = std::move(value);
                BuildNodes.top().type_ = j_t::node;
                BuildNodes.pop();
                break;
            default:
                throw std::logic_error("json value place");
        }
        return *this;
    }

    const Node& Builder::Build(){

        if(BuildNodes.size() != 0){
            if((BuildNodes.top().type_ != j_t::node)
            || (BuildNodes.size() > 1)){
                throw std::logic_error("json not finished");
            }
        }

        if(!BuildNodes.empty())
            BuildNodes.pop();

        return root_;
    }

    void Builder::Print(std::ostream& output){
        PrintNode(root_, output);
    }

    Node& Builder::PushToArray(const Node& value){

        BuildNodes.top().array_.push_back(std::move(value));

        return (BuildNodes.top().array_.back());
    }

    Node& Builder::PushToDict(const Node& value){

        if(!current_key_)
            throw std::logic_error("json no key on value");

        BuildNodes.top().dict_[*current_key_] = std::move(value);

        Node& new_node = (BuildNodes.top().dict_[*current_key_]);

        current_key_ = std::nullopt;

        return new_node;
    }

    void Builder::ThrowIfFinished(){
        if(BuildNodes.size() == 0){
                throw std::logic_error("json is finished");
        }
    }

} // namespace json