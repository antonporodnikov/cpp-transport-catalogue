#pragma once

#include "json.h"

namespace json {

class BaseContext;
class DictValueContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    DictValueContext Key(const std::string& key);
    BaseContext Value(const Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build() const;

private:
    Node root_;
    Node* current_ = &root_;
    std::vector<Node*> nodes_stack_;
    std::string dict_key_;
    bool key_flag_ = false;
};

class BaseContext {
public:
    BaseContext(Builder& builder);

    virtual ~BaseContext() = default;

    DictValueContext Key(const std::string& key);
    BaseContext Value(const Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build() const;

protected:
    Builder& builder_;
};

class DictValueContext : public BaseContext {
public:
    using BaseContext::BaseContext;
    using BaseContext::StartDict;
    using BaseContext::StartArray;

    DictItemContext Value(const Node::Value value);

private:
    using BaseContext::Key;
    using BaseContext::EndDict;
    using BaseContext::EndArray;
    using BaseContext::Build;
};

class DictItemContext : public BaseContext {
public:
    using BaseContext::BaseContext;
    using BaseContext::Key;
    using BaseContext::EndDict;

private:
    using BaseContext::Value;
    using BaseContext::StartDict;
    using BaseContext::StartArray;
    using BaseContext::EndArray;
    using BaseContext::Build;
};

class ArrayItemContext : private BaseContext {
public:
    using BaseContext::BaseContext;
    using BaseContext::StartDict;
    using BaseContext::StartArray;
    using BaseContext::EndArray;

    ArrayItemContext Value(const Node::Value value);

private:
    using BaseContext::Key;
    using BaseContext::EndDict;
    using BaseContext::Build;
};

}
