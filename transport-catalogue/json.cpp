#include "json.h"

using namespace std;

namespace json {

    namespace {

        Number GetNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        std::string GetString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
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
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            bool last_char_was_correct = false;
            for (char c; input >> c;) {
                
                if (c == ']') {
                    last_char_was_correct = true;
                    break;
                }

                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (!last_char_was_correct) {
                throw ParsingError("Parsing error");
            }

            return Node(move(result));
        }

        Node LoadNumber(istream& input) {
            variant<int, double> result = GetNumber(input);
            if (holds_alternative<double>(result)) {
                return Node(get<double>(result));
            }
            else if (holds_alternative<int>(result)) {
                return Node(get<int>(result));
            }
            throw ParsingError("Parsing error");
        }

        Node LoadString(istream& input) {
            string line = GetString(input);
            return Node(move(line));
        }

        Node LoadDict(istream& input) {
            Dict result;

            bool last_char_was_correct = false;
            for (char c; input >> c;) {

                if (c == '}') {
                    last_char_was_correct = true;
                    break;
                }

                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            if (!last_char_was_correct) {
                throw ParsingError("Parsing error");
            }

            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == ']') {
                throw json::ParsingError("Parsing error");
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '}') {
                throw json::ParsingError("Parsing error");
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 't') {
                string check = "";
                check.push_back(c);
                for (int i = 0; i < 3 && input >> c; ++i) {
                    check.push_back(c);
                }
                if (check == "true") {
                    return Node(true);
                }
                else {
                    throw ParsingError("Parsing error");
                }
            }
            else if (c == 'f') {
                string check = "";
                check.push_back(c);
                for (int i = 0; i < 4 && input >> c; ++i) {
                    check.push_back(c);
                }
                if (check == "false") {
                    return Node(false);
                }
                else {
                    throw ParsingError("Parsing error");
                }
            }
            else if (c == 'n') {
                string check = "";
                check.push_back(c);
                for (int i = 0; i < 3 && input >> c; ++i) {
                    check.push_back(c);
                }
                if (check == "null") {
                        return Node();
                    }
                else {
                   throw ParsingError("Parsing error");
                    }
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    const Node::Value& Node::GetValue() const
    { 
        return value_; 
    }

    Node::Node(Array array)
        : value_(move(array)) {
    }

    Node::Node(Dict map)
        : value_(move(map)) {
    }

    Node::Node(int value)
        : value_(value) {
    }

    Node::Node(string value)
        : value_(move(value)) {
    }

    Node::Node(double value)
        : value_(move(value)) {
    }

    Node::Node(bool value)
        : value_(move(value)) {
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

    bool Node::IsDouble() const {
        return  IsInt() || IsPureDouble();
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<json::Array>(value_);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<json::Dict>(value_);
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<json::Array>(value_);
        }
        throw std::logic_error("Wrong value type!"s);
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<json::Dict>(value_);
        }
        throw std::logic_error("Wrong value type!"s);
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(value_);
        }
        throw std::logic_error("Wrong value type!"s);
    }

    const string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(value_);
        }
        throw std::logic_error("Wrong value type!"s);
    }

    double Node::AsDouble() const {
        if (IsPureDouble()) {
            return std::get<double>(value_);
        }
        else if (IsInt()) {
            int val = std::get<int>(value_);
            return val;
        }
        throw std::logic_error("Wrong value type!"s);
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(value_);
        }
        throw std::logic_error("Wrong value type!"s);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

    void PrintContext::PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext PrintContext::Indented() const {
        return { out, indent_step, indent_step + indent };
    }

    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, const PrintContext& ctx) {
        ostream& out = ctx.out;
        out << "null"sv;
    }

    // Перегрузка функции PrintValue для вывода значений bool
    void PrintValue(bool value, const PrintContext& ctx) {
        ostream& out = ctx.out;
        out <<std::boolalpha << value;
    }
    
    // Перегрузка функции PrintValue для вывода значений string
    void PrintValue(string val, const PrintContext& ctx) {
        ostream& out = ctx.out;
        
        out << '\"';
        for (const char& ch : val) {

            switch (ch) {
            case '\n':
                out << "\\n";
                break;
            //case 't':
            //    s.push_back('\t');
            //    break;
            case '\r':
                out <<  "\\r";
                break;
            case '\"':
                out  << "\\\"";
                break;
            case '\\':
                out << '\\' << '\\';
                break;
            default:
                out << ch;
            }
        }
        out << '\"';
    }

    // Перегрузка функции PrintValue для вывода значений Array
    void PrintValue(json::Array array, const PrintContext& ctx) {
        ostream& out = ctx.out;
        out << "["sv;
        bool is_first = true;
        for (const Node& el : array) {
            if (is_first) {
                is_first = false;
                PrintNode(el, out);
            }
            else {
                out << ", "sv;
                PrintNode(el, out);
            }
        }
        out << "]"sv;
    }

    // Перегрузка функции PrintValue для вывода значений Dict
    void PrintValue(json::Dict map, const PrintContext& ctx) {
        ostream& out = ctx.out;
        out << "{"sv;
        bool is_first = true;
        for (const auto& [key, val] : map) {
            if (is_first) {
                is_first = false;
                out << "\""sv << key << "\": "sv;
                PrintNode(val, out);
            }
            else {
                out << ", "sv;
                out << "\""sv << key << "\": "sv;
                PrintNode(val, out);
            }
        }
        out << "}"sv;
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
            [&out](const auto& value) { PrintValue(value, { out }); },
            node.GetValue()
        );
    }

}  // namespace json