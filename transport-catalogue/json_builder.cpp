#include "json_builder.h"
#include <variant>
#include <exception>

using namespace std::literals;

namespace json
{

KeyItemContext Builder::Key(std::string str) {
    if(nodes_stack_.empty())
    {
        throw std::logic_error("Incorrect usage of Key method"s);
    }
    auto& curr_value = nodes_stack_.back()->GetValue();
    if(std::holds_alternative<Dict>(curr_value) && key_value_ == nullptr)
    {
        auto [iterator, is_success] = std::get<Dict>(curr_value).emplace(str, Node());
        key_value_ = &(iterator->second);
    }
    else
    {
        throw std::logic_error("Incorrect usage of Key method"s);
    }
    return KeyItemContext(*this);
}

Builder& Builder::Value(Node::Value value, bool is_start_map_array) {
    if(nodes_stack_.empty())
    {
        throw std::logic_error("Incorrect usage of Value method"s);
    }
    auto& curr_value = nodes_stack_.back()->GetValue();
    if(std::holds_alternative<Dict>(curr_value) && key_value_ != nullptr)
    {
        key_value_->GetValue() = value;
        if(std::holds_alternative<Dict>(value) || std::holds_alternative<Array>(value))
        {
            nodes_stack_.push_back(key_value_);
        }
        key_value_ = nullptr;
    }
    else if(std::holds_alternative<Array>(curr_value))
    {
        auto& node = std::get<Array>(curr_value);
        node.emplace_back(std::move(value));
        if(std::holds_alternative<Dict>(value) || std::holds_alternative<Array>(value))
        {
            nodes_stack_.push_back(&node.back());
        }
    }
    else if(std::holds_alternative<std::nullptr_t>(curr_value))
    {
        curr_value = value;
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

DictItemContext Builder::StartDict() {
    return DictItemContext(Value(Dict{}, true));
}

ArrayItemContext Builder::StartArray() {
    return ArrayItemContext(Value(Array{}, true));
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
    /*if(root_.GetValue().index() == 0)
    {
        throw std::logic_error("Incorrect usage of Build method"s);
    }*/
    else
    {
        return root_;
    }
}

    Node BaseContext::Build()
    {
        return builder_.Build();
    }
    KeyItemContext BaseContext::Key(std::string str)
    {
        return builder_.Key(str);
    }
    Builder& BaseContext::Value(Node::Value value)
    {
        return (builder_.Value(value));
    }
    DictItemContext BaseContext::StartDict()
    {
        return builder_.StartDict();
    }
    ArrayItemContext BaseContext::StartArray()
    {
        return builder_.StartArray();
    }
    Builder& BaseContext::EndDict()
    {
        return builder_.EndDict();
    }
    Builder& BaseContext::EndArray()
    {
        return builder_.EndArray();
    }

    Builder& BaseContext::GetBuilderRef()
    {
        return builder_;
    }

    DictItemContext KeyItemContext::Value(Node::Value value)
    {
        return DictItemContext(GetBuilderRef().Value(value));
    }

    ArrayItemContext ArrayItemContext::Value(Node::Value value)
    {
        return ArrayItemContext(GetBuilderRef().Value(value));
    }

} // namespace json