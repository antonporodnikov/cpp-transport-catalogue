// REWORK MODULE
#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double,
        std::string>;

    Node();
    Node(int value);
    Node(double value);
    Node(bool value);
    Node(std::string value);
    Node(std::nullptr_t null);
    Node(Array array);
    Node(Dict map);

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

private:
    Value value_;
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

template <typename Value>
void PrintValue(const Value& value, std::ostream& out)
{
    out << value;
}

void PrintValue(bool value, std::ostream& out);

void PrintValue(const std::string& value, std::ostream& out);

void PrintValue(std::nullptr_t, std::ostream& out);

void PrintValue(const Array& value, std::ostream& out);

void PrintValue(const Dict& value, std::ostream& out);

void PrintNode(const Node& node, std::ostream& out);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
