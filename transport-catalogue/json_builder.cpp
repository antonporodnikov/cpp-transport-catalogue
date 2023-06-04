#include "json_builder.h"

namespace json {
using namespace std::literals;

DictValueContext Builder::Key(const std::string& key)
{
    if (nodes_stack_.empty() || !(nodes_stack_.back()->IsDict()))
        throw std::logic_error(
            "Key error: attempt to call Key outside the Dict"s);

    if (key_flag_)
        throw std::logic_error("Key error: two Key calls in a row"s);
    
    dict_key_ = key;
    key_flag_ = true;

    return *this;
}

BaseContext Builder::Value(const Node::Value value)
{
    if (!(root_.IsNull()) && nodes_stack_.empty())
        throw std::logic_error("Value error: document completed"s);

    if (root_.IsNull())
    {
        root_.GetValue() = std::move(value);
    }
    else if (current_->IsDict())
    {
        if (!key_flag_)
            throw std::logic_error(
                "Value error: attempt to insert in Dict wihtout Key"s);

        Dict& dict = std::get<Dict>(current_->GetValue());
        dict.emplace(dict_key_, std::move(value));

        key_flag_ = false;
    }
    else if (current_->IsArray())
    {
        Array& arr = std::get<Array>(current_->GetValue());
        arr.emplace(arr.end(), std::move(value));
    }

    return *this;
}

DictItemContext Builder::StartDict()
{
    if (!(root_.IsNull()) && nodes_stack_.empty())
        throw std::logic_error("StartDict error: document completed"s);

    if (root_.IsNull())
    {
        root_.GetValue() = std::move(Dict({}));
        nodes_stack_.push_back(current_);
    }
    else if (current_->IsDict())
    {
        if (!key_flag_)
            throw std::logic_error(
                "StartDict error: attempt to insert in Dict wihtout Key"s);

        Dict& dict = std::get<Dict>(current_->GetValue());
        dict.emplace(dict_key_, std::move(Dict({})));
        current_ = &(dict.at(dict_key_));
        nodes_stack_.push_back(current_);

        key_flag_ = false;
    }
    else if (current_->IsArray())
    {
        Array& arr = std::get<Array>(current_->GetValue());
        arr.emplace(arr.end(), std::move(Dict({})));
        current_ = &(arr.back());
        nodes_stack_.push_back(current_);
    }
    
    return *this;
}

ArrayItemContext Builder::StartArray()
{
    if (!(root_.IsNull()) && nodes_stack_.empty())
        throw std::logic_error("StartArray error: document completed"s);

    if (root_.IsNull())
    {
        root_.GetValue() = std::move(Array({}));
        nodes_stack_.push_back(current_);
    }
    else if (current_->IsDict())
    {
        if (!key_flag_)
            throw std::logic_error(
                "StartArray error: attempt to insert in Dict wihtout Key"s);

        Dict& dict = std::get<Dict>(current_->GetValue());
        dict.emplace(dict_key_, std::move(Array({})));
        current_ = &(dict.at(dict_key_));
        nodes_stack_.push_back(current_);

        key_flag_ = false;
    }
    else if (current_->IsArray())
    {
        Array& arr = std::get<Array>(current_->GetValue());
        arr.emplace(arr.end(), std::move(Array({})));
        current_ = &(arr.back());
        nodes_stack_.push_back(current_);
    }

    return *this;
}

Builder& Builder::EndDict()
{
    if (key_flag_)
        throw std::logic_error("EndDict error: Dict element has no value"s);
    
    if (nodes_stack_.empty() || !(nodes_stack_.back()->IsDict()))
        throw std::logic_error(
            "EndDict error: attempt to close not a Dict object"s);

    nodes_stack_.pop_back();
    nodes_stack_.empty() ? current_ = &root_ : current_ = nodes_stack_.back();

    return *this;
}

Builder& Builder::EndArray()
{
    if (nodes_stack_.empty() || !(nodes_stack_.back()->IsArray()))
        throw std::logic_error(
            "EndArray error: attempt to close not an Array object"s);

    nodes_stack_.pop_back();
    nodes_stack_.empty() ? current_ = &root_ : current_ = nodes_stack_.back();

    return *this;
}

Node Builder::Build() const
{
    if (root_.IsNull() || !(nodes_stack_.empty()))
        throw std::logic_error("Build error: Document not completed"s);

    return root_;
}

BaseContext::BaseContext(Builder& builder)
    : builder_(builder) {}

DictValueContext BaseContext::Key(const std::string& key)
{
    return builder_.Key(key);
}

BaseContext BaseContext::Value(const Node::Value value)
{
    return builder_.Value(value);
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

Node BaseContext::Build() const
{
    return builder_.Build();
}

DictItemContext DictValueContext::Value(const Node::Value value)
{
    BaseContext temp = builder_.Value(value);
    return static_cast<DictItemContext&>(temp);
}

ArrayItemContext ArrayItemContext::Value(const Node::Value value)
{
    BaseContext temp = builder_.Value(value);
    return static_cast<ArrayItemContext&>(temp);
}

}
