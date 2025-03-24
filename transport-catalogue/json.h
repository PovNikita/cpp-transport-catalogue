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

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

using NodeVar = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

class Node {
public:
    using Value = NodeVar;

    template<typename T>
    Node(T node) : node_(std::move(node)) {}
    Node() = default;
    
    
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    const std::string& AsString() const;

    const NodeVar& GetNode() const;

    NodeVar& GetValue();

private:
    NodeVar node_;
};

bool operator==(const Node& lhs, const Node& rhs);

bool operator!=(const Node& lhs, const Node& rhs);

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(const Document& lhs, const Document& rhs);

bool operator!=(const Document& lhs, const Document& rhs);

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json