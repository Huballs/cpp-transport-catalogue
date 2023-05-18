#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Number = std::variant<int, double>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, uint32_t, double, std::string>;

    Node() = default;

    Node(std::nullptr_t value);
    Node(bool value);
    Node(int value);
    Node(uint32_t value);
    Node(double value);
    Node(const std::string &value);
    Node(const Dict &value);
    Node(const Array &value);

    bool operator==(const Node& other) const{
        return value_ == other.value_;
    }

    bool operator!=(const Node& other) const{
        return value_ != other.value_;
    }

    const Value& GetValue() const { return value_; }

    bool IsInt() const;
    bool IsUInt() const;
    bool IsDouble() const;      // Возвращает true, если в Node хранится int либо double.
    bool IsPureDouble() const;  // Возвращает true, если в Node хранится double.
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const; /* Возвращает значение типа double, если внутри хранится double либо int. 
                                В последнем случае возвращается приведённое в double значение. */
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

private:
    Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) const{
        return GetRoot() == other.GetRoot();
    }
    bool operator!=(const Document& other) const{
        return GetRoot() != other.GetRoot();
    }

private:
    Node root_;
};

Document Load(std::istream& input);

void PrintNode(const Node& node, std::ostream& out);
void PrintValue( const Dict &dict, std::ostream& out);
void PrintValue(std::nullptr_t, std::ostream& out);
void PrintValue(double value, std::ostream& out);
void PrintValue(bool value, std::ostream& out);
void PrintValue(const std::string &value, std::ostream& out);
void PrintValue(int value, std::ostream &out);
void PrintValue(uint32_t value, std::ostream &out);
void PrintValue(const Array &array, std::ostream &out);
void Print(const Document& doc, std::ostream& output);

}  // namespace json