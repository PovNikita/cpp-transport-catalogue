#pragma once

#include "json.h"

#include <vector>
#include <string>

namespace json
{

class KeyItemContext;
class ValueItemContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    KeyItemContext Key(std::string str);
    Builder& Value(NodeVar value, bool is_start_map_array = false);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();

private:
    Node root_;
    std::vector<Node*> nodes_stack_ {&root_};
    Node* key_value_ = nullptr;
};

class BaseContext
{
public:
    BaseContext(Builder& builder) : builder_(builder) {}
    Node Build();
    KeyItemContext Key(std::string str);
    Builder& Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();

    Builder& GetBuilderRef();
private:
    Builder& builder_;
};

class KeyItemContext : public BaseContext
{
public:
    KeyItemContext(Builder& builder) : BaseContext(builder) {} 
    Builder& EndDict() = delete;
    Builder& EndArray()  = delete;
    KeyItemContext Key(std::string str)  = delete;
    DictItemContext Value(Node::Value value);
    Node Build() = delete;
};

class ValueItemContext : public BaseContext
{
public:
    ValueItemContext(Builder& builder) : BaseContext(builder) {} 
    Builder& Value(Node::Value value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;

};

class DictItemContext : public BaseContext
{
public:
    DictItemContext(Builder& builder) : BaseContext(builder) {}
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
    DictItemContext Value(Node::Value value) = delete;
    Node Build() = delete;
private:
};

class ArrayItemContext : public BaseContext
{
public:
    ArrayItemContext(Builder& builder) : BaseContext(builder) {}
    KeyItemContext Key(std::string str) = delete;
    Builder& EndDict() = delete;
    ArrayItemContext Value(Node::Value value);
    Node Build() = delete;
};
} // namespace json
