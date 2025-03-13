#include "json.h"

#include <cassert>
#include <exception>
#include <cmath>
#include <iterator>
#include <sstream>

using namespace std;

namespace json {

namespace {

void ReplaceAllSubString(const std::string& sub_string_to_replacement, const std::string& sub_string, std::string& input_string)
{
    if (sub_string_to_replacement.empty()) return;

    auto pos = input_string.find(sub_string_to_replacement);
    while(pos!= input_string.npos) {
        input_string.replace(pos, sub_string_to_replacement.size(), sub_string);
        pos += sub_string.size();
        pos = input_string.find(sub_string_to_replacement, pos);
    }
}

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    char c;
    if(input >> c && c == ']')
    {
        return Node(move(result));
    }
    else
    {
        input.putback(c);
    }
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if(c == ']') {
        return Node(move(result));
    }
    else
    {
        throw ParsingError("There is not ']'");
        return Node(move(std::nullptr_t()));
    }
}

Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
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
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        } else {
            s.push_back(ch);
        }
        ++it;
    }

    return Node(s);
}

Node LoadDict(istream& input) {
    Dict result;
    char c;
    if(input >> c && c == '}')
    {
        return Node(move(result));
    }
    else
    {
        input.putback(c);
    }
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }
        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if(c == '}') {
        return Node(move(result));
    }
    else
    {
        throw ParsingError("There is not '}'");
        return Node(move(std::nullptr_t()));
    }
}

Node LoadBoolean(istream& input) {
    char c;
    input >> c;
    string str;
    if(c == 't')
    {
        str.push_back(c);
        for(int i = 0; input >> c && (c != ',' && c != ']' && c != '}'); ++i) {
            str.push_back(c);
        }
        if(str != "true"s)
        {
            throw ParsingError(str + "isn't equal: true"s);
        }
        if(c == '}' || c == ']')
        {
            input.putback(c);
        }
        return Node(move(true));
    }
    else
    {
        str.push_back(c);
        for(int i = 0; input >> c && (c != ',' && c != ']' && c != '}'); ++i) {
            str.push_back(c);
        }
        if(str != "false"s)
        {
            throw ParsingError(str + "isn't equal: false"s);
        }
        if(c == '}' || c == ']')
        {
            input.putback(c);
        }
        return Node(move(false));
    }
}

Node LoadNull(istream& input) {
    char c;
    input >> c;
    string str;
    str.push_back(c);
    for(int i = 0; input >> c && (c != ',' && c != ']' && c != '}'); ++i) {
        str.push_back(c);
    }
    if(str != "null"s)
    {
        throw ParsingError(str + "isn't equal: null"s);
    }
    if(c == '}' || c == ']')
    {
        input.putback(c);
    }
    return Node(move(std::nullptr_t()));
}


Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

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
    if (input.peek() == '0') {
        read_char();
    } else {
        read_digits();
    }

    bool is_int = true;
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

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
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNode(istream& input) {
    char c;
    input >> std::boolalpha;
    input >> c;

    if (c == '[') {
        input >> std::noboolalpha;
        return LoadArray(input);
    } else if (c == '{') {
        input >> std::noboolalpha;
        return LoadDict(input);
    } else if (c == '"') {
        input >> std::noboolalpha;
        return LoadString(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        input >> std::noboolalpha;
        return LoadBoolean(input);
    } else if (c == 'n') {
        input.putback(c);
        input >> std::noboolalpha;
        return LoadNull(input);
    } else if (c == '}') {
        throw ParsingError("There is not '{'");
    }
    else if (c == ']') {
        throw ParsingError("There is not '['");
    }else {
        input.putback(c);
        input >> std::noboolalpha;
        return LoadNumber(input);
    }
    
}

}  // namespace

Node::Node(Array array)
    : node_(move(array)) {
}

Node::Node(Dict map)
    : node_(move(map)) {
}

Node::Node(int value)
    : node_(value) {
}

Node::Node(string value)
    : node_(move(value)) {
}

Node::Node(double value)
    : node_(value) {
}

Node::Node(bool value)
    : node_(value) {
}

Node::Node(std::nullptr_t value)
    : node_(value) {

}

bool Node::IsInt() const {
    return std::holds_alternative<int>(node_);
}

bool Node::IsDouble() const {
    return std::holds_alternative<int>(node_) | std::holds_alternative<double>(node_);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(node_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(node_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(node_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(node_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(node_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(node_);
}


int Node::AsInt() const {
    if(const int* p = std::get_if<int>(&node_))
    {
        if(p == nullptr)
        {
            throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsInt() const\""s);
        }
        else
        {
            return *p;
        }
    }
    else
    {
        throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsInt() const\""s);
    }
}

bool Node::AsBool() const
{
    if(const bool* p = std::get_if<bool>(&node_))
    {
        if(p == nullptr)
        {
            throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsBool() const\""s);
        }
        else
        {
            return *p;
        }
    }
    else
    {
        throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsBool() const\""s);
    }
}

double Node::AsDouble() const
{
    if(const double* p = std::get_if<double>(&node_))
    {
        if(p == nullptr)
        {
            throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsDouble() const\""s);
        }
        else
        {
            return *p;
        }
    }
    else
    {
        if(const int* p = std::get_if<int>(&node_))
        {
            if(p == nullptr)
            {
                throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsDouble() const\""s);
            }
            else
            {
                return static_cast<double>(*p);
            }
        }
        else {
            throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsDouble() const\""s);
        }
    }
}

const Array& Node::AsArray() const {
    if(const Array* p = std::get_if<Array>(&node_))
    {
        if(p == nullptr)
        {
            throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsArray() const\""s);
        }
        else
        {
            return *p;
        }
    }
    else
    {
        throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsArray() const\""s);
    }
}

const Dict& Node::AsMap() const {
    if(const Dict* p = std::get_if<Dict>(&node_))
    {
        if(p == nullptr)
        {
            throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsMap() const\""s);
        }
        else
        {
            return *p;
        }
    }
    else
    {
        throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsMap() const\""s);
    }
}

const string& Node::AsString() const {
    if(const string* p = std::get_if<string>(&node_))
    {
        if(p == nullptr)
        {
            throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsString() const\""s);
        }
        else
        {
            return *p;
        }
    }
    else
    {
        throw std::logic_error("an attempt to get an access to incorrect type of value \"int Node::AsString() const\""s);
    }
}

bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.GetNode() == rhs.GetNode();
}

bool operator!=(const Node& lhs, const Node& rhs) {
    return lhs.GetNode() != rhs.GetNode();
}

const Node_var& Node::GetNode() const
{
    return node_;
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot().GetNode() == rhs.GetRoot().GetNode();
}

bool operator!=(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot().GetNode() != rhs.GetRoot().GetNode();
}

void PrintNode(std::ostream& out_strm, std::nullptr_t)
{
    out_strm << "null"s;
}

void PrintNode(std::ostream& out_strm, int value)
{
    out_strm << value;
}

void PrintNode(std::ostream& out_strm, double value)
{
    out_strm << value;
}

void PrintNode(std::ostream& out_strm, std::string str)
{
    ReplaceAllSubString(R"(\)"s, R"(\\)"s, str);
    ReplaceAllSubString(R"(")"s, R"(\")"s, str);
    ReplaceAllSubString("\n"s, R"(\n)"s, str);
    ReplaceAllSubString("\t"s, R"(\t)"s, str);
    ReplaceAllSubString("\r"s, R"(\r)"s, str);
    str = "\""s + str + "\"";
    out_strm << str;
}

void PrintNode(std::ostream& out_strm, bool value)
{
    out_strm << boolalpha << value << noboolalpha;
}

void PrintNode(std::ostream& out_strm, Array nodes)
{
    out_strm << "["s;
    size_t i = 0;
    for(auto& node : nodes)
    {
        std::visit( [&out_strm] (auto value) 
        {
            PrintNode(out_strm, value);
        }, node.GetNode());
        if(i == nodes.size() - 1)
        {
            continue;
            out_strm << ","s;
        }
        out_strm << ","s;
        ++i;
    }
    out_strm << "]"s;    
}

void PrintNode(std::ostream& out_strm, Dict nodes)
{
    out_strm << "{"s;
    for(auto& [key, value] : nodes)
    {
        out_strm << "\""s << key << "\": "s;
        std::visit( [&out_strm] (auto value) 
        {
            PrintNode(out_strm, value);
        }, value.GetNode());
        if(key == (std::next(nodes.end(), -1)->first))
        {
            continue;
        }
        out_strm << ","s;
    }
    out_strm << "}"s;
}

void Print(const Document& doc, std::ostream& output) {
    std::visit( [&output] (auto value) 
                        {
                            PrintNode(output, value);
                        }, doc.GetRoot().GetNode());
}

}  // namespace json