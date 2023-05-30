#pragma once

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <variant>
#include <vector>

using namespace std::literals;

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, 
        std::string>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : private Value {
public:
    using variant::variant;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    const Value& GetValue() const;

    bool operator==(const Node& rhs) const;
    bool operator!=(const Node& rhs) const;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& rhs) const;
    bool operator!=(const Document& rhs) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void PrintNode(const Node& node, std::ostream& out);

struct PrintValue {
    std::ostream& out;

    template <typename T>
    void operator()(const T& value) const;

    void operator()(std::nullptr_t) const;

    void operator()(const Array& value) const;

    void operator()(const Dict& value) const;

    void operator()(const bool value) const;

    void operator()(const std::string& value) const;
};

template <typename T>
void PrintValue::operator()(const T& value) const
{
    out << value;
}

void Print(const Document& doc, std::ostream& output);

}
