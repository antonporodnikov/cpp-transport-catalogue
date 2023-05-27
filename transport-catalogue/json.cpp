#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input)
{
    Array result;

    char c;
    for (; input >> c && c != ']';)
    {
        if (c != ',')
        {
            input.putback(c);
        }

        result.push_back(LoadNode(input));
    }

    if (c != ']')
    {
        throw ParsingError("Unexpected end of array"s);
    }
    else
    {
        return Node(move(result));
    }
}

using Number = std::variant<int, double>;

Number ProcessNumber(std::istream& input)
{
    using namespace std::literals;

    std::string parsed_num;

    auto read_char = [&parsed_num, &input]
    {
        parsed_num += static_cast<char>(input.get());
        if (!input)
        {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    auto read_digits = [&input, read_char]
    {
        if (!std::isdigit(input.peek()))
        {
            throw ParsingError("A digit is expected"s);
        }

        while (std::isdigit(input.peek()))
        {
            read_char();
        }
    };

    if (input.peek() == '-')
    {
        read_char();
    }

    if (input.peek() == '0')
    {
        read_char();
    }
    else
    {
        read_digits();
    }

    bool is_int = true;
    if (input.peek() == '.')
    {
        read_char();
        read_digits();
        is_int = false;
    }

    if (int ch = input.peek(); ch == 'e' || ch == 'E')
    {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-')
        {
            read_char();
        }

        read_digits();
        is_int = false;
    }

    try
    {
        if (is_int)
        {
            try
            {
                return std::stoi(parsed_num);
            }
            catch (...)
            {
            }
        }

        return std::stod(parsed_num);
    }
    catch (...)
    {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNumber(istream& input)
{
    Number num = ProcessNumber(input);

    if (holds_alternative<int>(num))
    {
        return Node(get<int>(num));
    }

    return Node(get<double>(num));
}

std::string ProcessString(std::istream& input)
{
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true)
    {
        if (it == end)
        {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"')
        {
            ++it;
            break;
        }
        else if (ch == '\\')
        {
            ++it;
            if (it == end)
            {
                throw ParsingError("String parsing error");
            }

            const char escaped_char = *(it);
            switch (escaped_char)
            {
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
                    throw ParsingError(
                        "Unrecognized escape sequence \\"s + escaped_char);
            }
        }
        else if (ch == '\n' || ch == '\r')
        {
            throw ParsingError("Unexpected end of line"s);
        }
        else
        {
            s.push_back(ch);
        }

        ++it;
    }

    return s;
}

Node LoadString(istream& input)
{
    return Node(move(ProcessString(input)));
}

Node LoadDict(istream& input)
{
    Dict result;

    char c;
    for (; input >> c && c != '}';)
    {
        if (c == ',')
        {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    if (c != '}')
    {
        throw ParsingError("Unexpected end of array"s);
    }
    else
    {
        return Node(move(result));
    }
}

Node LoadNull(istream& input)
{
    std::string check;

    char c;
    for (int i = 0; i < 2; ++i)
    {
        input >> c;
        check.push_back(c);
        continue;
    }

    c = 'e';
    input >> c;
    check.push_back(c);

    if (check == "ull")
    {
        return Node(nullptr);
    }
    else
    {
        throw ParsingError("Wrong null value"s);
    }
}

Node LoadBool(istream& input)
{
    std::string check;

    int i = 0;
    char c;
    for (; input >> c && i < 2; ++i)
    {
        check.push_back(c);
        continue;
    }
    check.push_back(c);

    if (c == 'e' && check == "rue")
    {
        return Node(true);
    }

    input >> c;
    check.push_back(c);
    if (check == "alse")
    {
        return Node(false);
    }
    else
    {
        throw ParsingError("Wrong bool value"s);
    }
}

Node LoadNode(istream& input)
{
    char c;
    input >> c;

    string s;
    while (c == '\\')
    {
        s = input.get();
        input >> c;
    }

    if (c == '[')
    {
        return LoadArray(input);
    }
    else if (c == '{')
    {
        return LoadDict(input);
    }
    else if (c == 'n')
    {
        return LoadNull(input);
    }
    else if (c == '"')
    {
        return LoadString(input);
    } 
    else if (c == 't' || c == 'f')
    {
        return LoadBool(input);
    }
    else if (std::isdigit(c) || c == '-')
    {
        input.putback(c);
        return LoadNumber(input);
    }
    else
    {
        throw ParsingError("Not valid json"s);
    }
}

}

Node::Node() : value_(nullptr) {}

Node::Node(int value) : value_(value) {}

Node::Node(double value) : value_(value) {}

Node::Node(bool value) : value_(value) {}

Node::Node(string value) : value_(move(value)) {}

Node::Node(std::nullptr_t) : value_(nullptr) {}
    
Node::Node(Array array) : value_(move(array)) {}

Node::Node(Dict map) : value_(move(map)) {}

int Node::AsInt() const
{
    try
    {
        return get<int>(value_);
    }
    catch (const bad_variant_access& e)
    {
        throw logic_error(e.what());
    }
}

bool Node::AsBool() const
{
    try
    {
        return get<bool>(value_);
    }
    catch (const bad_variant_access& e)
    {
        throw logic_error(e.what());
    }
}

double Node::AsDouble() const
{
    try
    {
        return holds_alternative<double>(value_) ? get<double>(value_) :
            static_cast<double>(get<int>(value_));
    }
    catch (const bad_variant_access& e)
    {
        throw logic_error(e.what());
    }
}

const string& Node::AsString() const
{
    try
    {
        return get<string>(value_);
    }
    catch (const bad_variant_access& e)
    {
        throw logic_error(e.what());
    }
}

const Array& Node::AsArray() const
{
    try
    {
        return get<Array>(value_);
    }
    catch (const bad_variant_access& e)
    {
        throw logic_error(e.what());
    }
}

const Dict& Node::AsMap() const
{
    try
    {
        return get<Dict>(value_);
    }
    catch (const bad_variant_access& e)
    {
        throw logic_error(e.what());
    }
}

bool Node::IsInt() const
{
    if (holds_alternative<int>(value_))
    {
        return true;
    }

    return false;
}

bool Node::IsDouble() const
{
    if (holds_alternative<int>(value_) || holds_alternative<double>(value_))
    {
        return true;
    }

    return false;
}

bool Node::IsPureDouble() const
{
    if (holds_alternative<double>(value_))
    {
        return true;
    }

    return false;
}

bool Node::IsBool() const
{
    if (holds_alternative<bool>(value_))
    {
        return true;
    }

    return false;
}

bool Node::IsString() const
{
    if (holds_alternative<std::string>(value_))
    {
        return true;
    }

    return false;
}

bool Node::IsNull() const
{
    if (holds_alternative<nullptr_t>(value_))
    {
        return true;
    }

    return false;
}

bool Node::IsArray() const
{
    if (holds_alternative<Array>(value_))
    {
        return true;
    }

    return false;
}

bool Node::IsMap() const
{
    if (holds_alternative<Dict>(value_))
    {
        return true;
    }

    return false;
}

const Node::Value& Node::GetValue() const
{
    return value_;
}

bool Node::operator==(const Node& rhs) const
{
    return this->value_ == rhs.value_;
}

bool Node::operator!=(const Node& rhs) const
{
    return !(*this == rhs);
}

Document::Document(Node root) : root_(move(root)) {}

const Node& Document::GetRoot() const
{
    return root_;
}

bool Document::operator==(const Document& rhs) const
{
    return this->GetRoot() == rhs.GetRoot();
}

bool Document::operator!=(const Document& rhs) const
{
    return !(*this == rhs);
}

Document Load(istream& input)
{
    return Document{LoadNode(input)};
}

void PrintValue(bool value, std::ostream& out)
{
    if (value)
    {
        out << "true"sv;
        return;
    }

    out << "false"sv;
}

void PrintValue(const string& value, std::ostream& out)
{
    string result;
    result.push_back('\"');

    for (char c : value)
    {
        if (c == '\\' || c == '\"' || c == '\'')
        {
            result.push_back('\\');
            result.push_back(c);

            continue;
        }

        if (c == '\r')
        {
            result.push_back('\\');
            result.push_back('r');

            continue;
        }

        if (c == '\n')
        {
            result.push_back('\\');
            result.push_back('n');

            continue;
        }

        result.push_back(c);
    }

    result.push_back('\"');

    out << result;
}

void PrintValue(std::nullptr_t, std::ostream& out)
{
    out << "null"sv;
}

void PrintValue(const Array& value, std::ostream& out)
{
    out << '[';

    for (auto it = value.begin(); it != value.end(); ++it)
    {
        if (it == value.begin())
        {
            PrintNode(*it, out);
            continue;
        }

        out << ", "sv;
        PrintNode(*it, out);
    }

    out << ']';
}

void PrintValue(const Dict& value, std::ostream& out)
{
    out << '{';

    for (auto it = value.begin(); it != value.end(); ++it)
    {
        if (it == value.begin())
        {
            out << '\"' << (*it).first << "\": "sv;
            PrintNode((*it).second, out);
            continue;
        }

        out << ", "sv;
        out << '\"' << (*it).first << "\": "sv;
        PrintNode((*it).second, out);
    }

    out << '}';
}

void PrintNode(const Node& node, std::ostream& out)
{
    std::visit([&out](const auto& value) { PrintValue(value, out); },
        node.GetValue());
} 

void Print(const Document& doc, std::ostream& output)
{
    PrintNode(doc.GetRoot(), output);
}

}
