#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;

    char c = 0;
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (c != ']')
        throw ParsingError("Array error"s);


    return Node(move(result));
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNull(istream& input) {
    const string nameNull = "null";
    for (size_t i = 0; i < nameNull.size(); i++)
    {
        if (nameNull.at(i) == input.get())
            continue;
        else
        {
            throw ParsingError("Null parsing error");
        }
    }
    return {};
}

Node LoadBool(istream& input) {
    const string nameFalse = "false";
    const string nameTrue = "true";
    char c = input.get();
    bool value = (c == 't');
    string const * name = value ? &nameTrue : &nameFalse;
    for (size_t i = 1; i < name->size(); i++)
    {
        if (name->at(i) == input.get())
            continue;
        else
        {
            throw ParsingError("Bool parsing error");
        }
    }
    return Node(value);

}

Node LoadString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }
    return Node(s);
}

Node LoadDict(istream& input) {
    Dict result;

    char c = 0;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    if (c != '}')
        throw ParsingError("Dict error"s);

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 'f' || c == 't') {
        input.putback(c);
        return LoadBool(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull (input);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

// ---------------------- Node ---------------------
Node::Node(std::nullptr_t value) : value_(value) {
}

Node::Node(bool value) : value_(value) {
}

Node::Node(int value) : value_(value) {
}

Node::Node(double value) : value_(value) {
}

Node::Node(const std::string &value) : value_(move(value)) {
}

Node::Node(const Array &value) : value_(move(value)) {
}

Node::Node(const Dict &value) : value_(move(value)){
}



bool Node::IsInt() const {
    return (std::get_if<int>(&value_) != nullptr);
}
bool Node::IsDouble() const{      // Возвращает true, если в Node хранится int либо double.
    return IsInt() || IsPureDouble();
}
bool Node::IsPureDouble() const{  // Возвращает true, если в Node хранится double.
    return (std::get_if<double>(&value_) != nullptr);
}
bool Node::IsBool() const{
    return (std::get_if<bool>(&value_) != nullptr);
}
bool Node::IsString() const{
    return (std::get_if<std::string>(&value_) != nullptr);
}
bool Node::IsNull() const{
    return (std::get_if<nullptr_t>(&value_) != nullptr);
}
bool Node::IsArray() const{
    return (std::get_if<Array>(&value_) != nullptr);
}
bool Node::IsMap() const{
    return (std::get_if<Dict>(&value_) != nullptr);
}

int Node::AsInt() const {
    if(IsInt()){
        return std::get<int>(value_);
    }
    throw std::logic_error("Not an Int");
}

bool Node::AsBool() const {
    if(IsBool()){
        return std::get<bool>(value_);
    }
    throw std::logic_error("Not a bool");
}

double Node::AsDouble() const {
    if(IsInt()){
        return static_cast<double>(std::get<int>(value_));
    } else 
    if (IsPureDouble()){
        return std::get<double>(value_);
    }
    throw std::logic_error("Not a number");
}
                            
const std::string& Node::AsString() const {
    if(IsString()){
        return std::get<std::string>(value_);
    }
    throw std::logic_error("Not a string");
}

const Array& Node::AsArray() const {
    if(IsArray()){
        return std::get<Array>(value_);
    }
    throw std::logic_error("Not an array");
}

const Dict& Node::AsMap() const {
    if(IsMap()){
        return std::get<Dict>(value_);
    }
    throw std::logic_error("Not a map");
}

// ---------------------- Document ---------------------

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    auto root = doc.GetRoot();
    PrintNode(root, output);
}

void PrintNode(const Node& node, std::ostream& out) {
    std::visit( [&out](const auto& value){ PrintValue(value, out); },
    node.GetValue());
}

void PrintValue( const Dict &dict, std::ostream& out){
    bool next = false;
    out << "\n{ \n";
    for (const auto & value:dict )
    {
        if (next)
            out << ",\n";
        next = true;
        out << '\"';
        out << value.first << "\": ";
        PrintNode(value.second, out);
    }
    out << "\n}";
}

void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null";
}

void PrintValue(double value, std::ostream& out) {
    size_t t= cout.precision();
    out.precision(11);
    out << value;
    out.precision(t);
}

void PrintValue(bool value, std::ostream& out) {
    out << std::boolalpha << value << std::noboolalpha;
}

void PrintValue(const string &value, std::ostream& out) {
    out << "\"";
    for (auto i = value.begin(); i < value.end(); i++)
    {
        if (*i == '\"')
        {
            out << '\\' << '\"';
        }
        else if (*i == '\n')
        {
            out << '\\' << 'n';
        }
        else if (*i == '\r')
        {
            out << '\\' << 'r';
        }
        else if (*i == '\\')
        {
            out << '\\' << '\\';
        }
        else
            out << *i;
    }
    out << "\"";
}

void PrintValue(int value, std::ostream &out){
    out << value;
}

void PrintValue(const Array &array, std::ostream &out){
    bool next = false;
    out << "[";
    for (const auto & value:array )
    {
        if (next)
            out << ',';
        next = true;
        PrintNode(value, out);
    }
    out << "]";
}

}  // namespace json