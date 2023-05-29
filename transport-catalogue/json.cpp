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

using Number = variant<int, double>;

Number ProcessNumber(istream& input)
{
    using namespace literals;

    string parsed_num;

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
        if (!isdigit(input.peek()))
        {
            throw ParsingError("A digit is expected"s);
        }

        while (isdigit(input.peek()))
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
                return stoi(parsed_num);
            }
            catch (...)
            {
            }
        }

        return stod(parsed_num);
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

string ProcessString(istream& input)
{
    using namespace literals;
    
    auto it = istreambuf_iterator<char>(input);
    auto end = istreambuf_iterator<char>();
    string s;
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
    string check;

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
    string check;

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
    else if (isdigit(c) || c == '-')
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

int Node::AsInt() const
{
    try
    {
        return get<int>(*this);
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
        return get<bool>(*this);
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
        return holds_alternative<double>(*this) ? get<double>(*this) :
            static_cast<double>(get<int>(*this));
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
        return get<string>(*this);
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
        return get<Array>(*this);
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
        return get<Dict>(*this);
    }
    catch (const bad_variant_access& e)
    {
        throw logic_error(e.what());
    }
}

bool Node::IsInt() const
{
    if (holds_alternative<int>(*this))
    {
        return true;
    }

    return false;
}

bool Node::IsDouble() const
{
    if (holds_alternative<int>(*this) || holds_alternative<double>(*this))
    {
        return true;
    }

    return false;
}

bool Node::IsPureDouble() const
{
    if (holds_alternative<double>(*this))
    {
        return true;
    }

    return false;
}

bool Node::IsBool() const
{
    if (holds_alternative<bool>(*this))
    {
        return true;
    }

    return false;
}

bool Node::IsString() const
{
    if (holds_alternative<string>(*this))
    {
        return true;
    }

    return false;
}

bool Node::IsNull() const
{
    if (holds_alternative<nullptr_t>(*this))
    {
        return true;
    }

    return false;
}

bool Node::IsArray() const
{
    if (holds_alternative<Array>(*this))
    {
        return true;
    }

    return false;
}

bool Node::IsMap() const
{
    if (holds_alternative<Dict>(*this))
    {
        return true;
    }

    return false;
}

const Value& Node::GetValue() const
{
    return *this;
}

bool Node::operator==(const Node& rhs) const
{
    return this->GetValue() == rhs.GetValue();
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

void PrintNode(const Node& node, ostream& out)
{
    ostringstream strm;

    visit(PrintValue{strm}, node.GetValue());
    out << strm.str();
} 

void Print(const Document& doc, ostream& output)
{
    PrintNode(doc.GetRoot(), output);
}

}
