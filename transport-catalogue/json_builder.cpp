#include "json_builder.h"
#include <variant>
#include <exception>

using namespace std::literals;

namespace json
{

Builder::KeyItemContext Builder::Key(std::string str) {
    if(nodes_stack_.empty())
    {
        throw std::logic_error("Incorrect usage of Key method"s);
    }
    auto& curr_value = nodes_stack_.back()->GetValue();
    if(std::holds_alternative<Dict>(curr_value) && key_value_ == nullptr)
    {
        auto [iterator, is_success] = std::get<Dict>(curr_value).emplace(std::move(str), Node());
        key_value_ = &(iterator->second);
    }
    else
    {
        throw std::logic_error("Incorrect usage of Key method"s);
    }
    return KeyItemContext(*this);
}

Builder& Builder::Value(Node::Value value) {
    return ValueBase(value, false);
}

Builder& Builder::ValueBase(Node::Value& value, bool is_start_map_array) {
    if(nodes_stack_.empty())
    {
        throw std::logic_error("Incorrect usage of Value method"s);
    }
    auto& curr_value = nodes_stack_.back()->GetValue();
    if(std::holds_alternative<Dict>(curr_value) && key_value_ != nullptr)
    {
        key_value_->GetValue() = std::move(value);
        if(std::holds_alternative<Dict>(key_value_->GetValue()) || std::holds_alternative<Array>(key_value_->GetValue()))
        {
            nodes_stack_.push_back(key_value_);
        }
        key_value_ = nullptr;
    }
    else if(std::holds_alternative<Array>(curr_value))
    {
        auto& node = std::get<Array>(curr_value);
        node.emplace_back(std::move(value));
        if(std::holds_alternative<Dict>(node.back().GetValue()) || std::holds_alternative<Array>(node.back().GetValue()))
        {
            nodes_stack_.push_back(&node.back());
        }
    }
    else if(std::holds_alternative<std::nullptr_t>(curr_value))
    {
        curr_value = std::move(value);
        if(is_start_map_array == false)
        {
            nodes_stack_.pop_back();
        }
    }
    else
    {
        throw std::logic_error("Incorrect usage of Value method"s);
    }
    return *this;
}

Builder::DictItemContext Builder::StartDict() {
    NodeVar node(Dict{});
    return DictItemContext(ValueBase(node, true));
}

Builder::ArrayItemContext Builder::StartArray() {
    NodeVar node(Array{});
    return ArrayItemContext(ValueBase(node, true));
}

Builder& Builder::EndDict() {
    if(nodes_stack_.empty())
    {
        throw std::logic_error("Incorrect usage of EndDict method"s);
    }
    auto& curr_value = nodes_stack_.back()->GetValue();
    if(std::holds_alternative<Dict>(curr_value))
    {
        nodes_stack_.pop_back();
    }
    else
    {
        throw std::logic_error("Dictionary is incompleted"s);
    }
    return *this;
}

Builder& Builder::EndArray() {
    if(nodes_stack_.empty())
    {
        throw std::logic_error("Incorrect usage of EndArray method"s);
    }
    auto& curr_value = nodes_stack_.back()->GetValue();
    if(std::holds_alternative<Array>(curr_value))
    {
        nodes_stack_.pop_back();
    }
    else
    {
        throw std::logic_error("Array is incompleted"s);
    }
    return *this;
}

Node Builder::Build() {
    if(!nodes_stack_.empty() || key_value_ != nullptr)
    {
        throw std::logic_error("Incorrect usage of Build method"s);
    }
    else
    {
        return root_;
    }
}

Node Builder::BaseContext::Build()
{
    return builder_.Build();
}
Builder::KeyItemContext Builder::BaseContext::Key(std::string str)
{
    return builder_.Key(std::move(str));
}
Builder& Builder::BaseContext::Value(Node::Value value)
{
    return (builder_.Value(std::move(value)));
}
Builder::DictItemContext Builder::BaseContext::StartDict()
{
    return builder_.StartDict();
}
Builder::ArrayItemContext Builder::BaseContext::StartArray()
{
    return builder_.StartArray();
}
Builder& Builder::BaseContext::EndDict()
{
    return builder_.EndDict();
}
Builder& Builder::BaseContext::EndArray()
{
    return builder_.EndArray();
}

Builder& Builder::BaseContext::GetBuilderRef()
{
    return builder_;
}

Builder::DictItemContext Builder::KeyItemContext::Value(Node::Value value)
{
    return DictItemContext(GetBuilderRef().Value(std::move(value)));
}

Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value)
{
    return ArrayItemContext(GetBuilderRef().Value(std::move(value)));
}

} // namespace json