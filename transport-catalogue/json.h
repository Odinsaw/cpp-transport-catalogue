#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    // ��������� ���������� Dict � Array ��� ���������
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // ��� ������ ������ ������������� ��� ������� �������� JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    using Number = std::variant<int, double>;

    Number GetNumber(std::istream& input);

    // ��������� ���������� ���������� �������� JSON-���������
    // ������� ������� ������������ ����� ���������� ������������ ������� ":
    std::string GetString(std::istream& input);

    class Node final
        : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        using variant::variant;

        bool IsInt() const;
        bool IsDouble() const; //���������� true, ���� � Node �������� int ���� double.
        bool IsPureDouble() const; //���������� true, ���� � Node �������� double.
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        const Array& AsArray() const;
        bool AsBool() const;
        double AsDouble() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;

        const Value& GetValue() const;

        bool operator==(const Node& rhs) const{
            if (&rhs == this) {
                return true;
            }
            //return rhs.GetValue() == value_;
            return rhs.GetValue() == *this;
        }

        bool operator!=(const Node& rhs) const{
            return !(rhs == *this);
        }
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& rhs) const {
            if (&rhs == this) {
                return true;
            }
            return rhs.root_.GetValue() == root_.GetValue();
        }

        bool operator!=(const Document& rhs) const {
            return !(rhs == *this);
        }

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const;

        // ���������� ����� �������� ������ � ����������� ���������
        PrintContext Indented() const;
    };

    // ���������� ������� PrintValue ��� ������ �������� null
    void PrintValue(std::nullptr_t, const PrintContext& ctx);

    // ���������� ������� PrintValue ��� ������ �������� string
    void PrintValue(std::string, const PrintContext& ctx);

    // ���������� ������� PrintValue ��� ������ �������� bool
    void PrintValue(bool value, const PrintContext& ctx);

    // ���������� ������� PrintValue ��� ������ �������� Array
    void PrintValue(json::Array array, const PrintContext& ctx);

    // ���������� ������� PrintValue ��� ������ �������� Dict
    void PrintValue(json::Dict map, const PrintContext& ctx);

    // ������, ���������� ��� ������ double � int
    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << value;
    }

    void PrintNode(const Node& node, std::ostream& out);

}  // namespace json