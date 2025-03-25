#pragma once

#include "json.h"
#include <string>
#include <vector>

namespace json
{

class Builder {
public:

    class BaseContext;
    class KeyItemContext;
    class ValueItemContext;
    class DictItemContext;
    class ArrayItemContext;

    KeyItemContext Key(std::string str);
    Builder& Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();

private:
    Node root_;
    std::vector<Node*> nodes_stack_ {&root_};
    Node* key_value_ = nullptr;
    Builder& ValueBase(Node::Value& value, bool is_start_map_array = false);
};

class Builder::BaseContext
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

class Builder::KeyItemContext : public BaseContext
{
public:
    KeyItemContext(Builder& builder) : BaseContext(builder) {} 
    Builder& EndDict() = delete;
    Builder& EndArray()  = delete;
    KeyItemContext Key(std::string str)  = delete;
    DictItemContext Value(Node::Value value);
    Node Build() = delete;
};

class Builder::ValueItemContext : public BaseContext
{
public:
    ValueItemContext(Builder& builder) : BaseContext(builder) {} 
    Builder& Value(Node::Value value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;

};

class Builder::DictItemContext : public BaseContext
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

class Builder::ArrayItemContext : public BaseContext
{
public:
    ArrayItemContext(Builder& builder) : BaseContext(builder) {}
    KeyItemContext Key(std::string str) = delete;
    Builder& EndDict() = delete;
    ArrayItemContext Value(Node::Value value);
    Node Build() = delete;
};
} // namespace json
