#include <string>
#include <cstdio>
#include <vector>
#include <map>
#include <cctype>
using namespace std;


// defines all possible token types
enum Token {
    tok_eof,
    tok_eol,
    tok_import,
    tok_func,
    tok_identifier,
    tok_number
};

// used for tree printing
static void pad_output(int level) {
    while (level--) {
        printf("  ");
    }
}


// describes a node of abstract syntax tree
class ExpressionNode {
public:
    virtual ~ExpressionNode() = default;
    virtual void print_description(int tab=0) = 0;
};

class NumberExpressionNode : public ExpressionNode {
    double value;

public:
    NumberExpressionNode(double _value) : value(_value) {}

    void print_description(int tab) {
        pad_output(tab);
        printf("number: %.4f\n", value);
    }
};

class VariableExpressionNode : public ExpressionNode {
    string name;

public:
    VariableExpressionNode(const string &_name) : name(_name) {}

    void print_description(int tab) {
        pad_output(tab);
        printf("variable: %s\n", name.c_str());
    }
};

class BinaryExpressionNode : public ExpressionNode {
    char operation;
    ExpressionNode *lhs, *rhs;

public:
    BinaryExpressionNode(char _operation, ExpressionNode *_lhs, ExpressionNode *_rhs)
            : operation(_operation), lhs(_lhs), rhs(_rhs) {}

    ~BinaryExpressionNode() {
        delete lhs;
        delete rhs;
    }

    void print_description(int tab) {
        pad_output(tab);
        printf("binary operation: %c\n", operation);
        lhs->print_description(tab+1);
        rhs->print_description(tab+1);
    }
};

class FunctionCallExpressionNode : public ExpressionNode {
    string function_name;
    vector<ExpressionNode *> arguments;

public:
    FunctionCallExpressionNode(const string &_name, const vector<ExpressionNode *> &_args)
            : function_name(_name), arguments(_args) {}

    ~FunctionCallExpressionNode() {
        for (ExpressionNode *argument : arguments) {
            delete argument;
        }
    }

    void print_description(int tab) {
        pad_output(tab);
        printf("function name: %s\n", function_name.c_str());
        pad_output(tab);
        printf("arguments:\n");
        for (ExpressionNode *node : arguments) {
            node->print_description(tab+1);
        }
    }
};

class FunctionPrototypeNode {
    string function_name;
    vector<string> arguments;

public:
    FunctionPrototypeNode(const string &_name, const vector<string> &_args)
            : function_name(_name), arguments(_args) {}

    void print_description(int tab) {
        pad_output(tab);
        printf("function name: %s\n", function_name.c_str());
        pad_output(tab);
        printf("accepts arguments: [");
        for (int i = 0; i < (int)arguments.size(); i++) {
            if (i > 0) printf(", ");
            printf("%s", arguments[i].c_str());
        }
        printf("]\n");
    }
};

class FunctionDefinitionNode {
    FunctionPrototypeNode *prototype;
    ExpressionNode *body;

public:
    FunctionDefinitionNode(FunctionPrototypeNode *_proto, ExpressionNode *_body)
            : prototype(_proto), body(_body) {}

    ~FunctionDefinitionNode() {
        delete prototype;
        delete body;
    }

    void print_description(int tab) {
        pad_output(tab);
        printf("prototype:\n");
        prototype->print_description(tab+1);
        pad_output(tab);
        printf("body:\n");
        body->print_description(tab+1);
    }
};


// global variables that hold data parsed by the lexer
double number_value;
string identifier_value;
// stores last read token
int current_token;

// constants used by parser
const map<int, int> BINARY_OPERATION_PRECEDENCE = {
        { '<', 10 },
        { '>', 10 },
        { '=', 10 },
        { '+', 20 },
        { '-', 20 },
        { '*', 40 },
        { '/', 40 }
};


// lexer routines
static int get_token();
static int get_next_token();
static int get_token_precedence();

// parser low-level routines
static void log_error(const char *message);
static ExpressionNode* parse_expression();
static ExpressionNode* parse_number_expression();
static ExpressionNode* parse_parentheses_expression();
static ExpressionNode* parse_identifier_expression();
static ExpressionNode* parse_primary();
static ExpressionNode* parse_binop_rhs(int min_precedence, ExpressionNode *lhs);
static FunctionPrototypeNode* parse_function_prototype();
static FunctionDefinitionNode* parse_function_definition();
static FunctionDefinitionNode* parse_top_level_expression();
static FunctionPrototypeNode* parse_function_import();

// parser high-level routines
static void handle_function_definition();
static void handle_function_import();
static void handle_top_level_expression();


int main() {

    freopen("Lab2.2ParserInput1.txt", "r", stdin); //change to input 2 for another test
    // initialize the current_token variable
    get_next_token();

    // start main read loop
    while (current_token != tok_eof) {
        switch (current_token) {
            case tok_func:
                handle_function_definition();
                break;

            case tok_import:
                handle_function_import();
                break;

            default:
                handle_top_level_expression();
                break;
        }
    }
    return 0;
}


int get_token() {
    static char last_char = ' ';

    while (isspace(last_char))
        last_char = getchar();

    if (isalpha(last_char) || last_char == '_') {
        identifier_value = last_char;
        while ((last_char = getchar()) && (isalnum(last_char) || last_char == '_')) {
            identifier_value += last_char;
        }

        if (identifier_value == "func")
            return tok_func;
        if (identifier_value == "import")
            return tok_import;
        return tok_identifier;
    }

    if (isdigit(last_char) || last_char == '.') {
        string number;
        while (isdigit(last_char) || last_char == '.') {
            number += last_char;
            last_char = getchar();
        }

        number_value = strtod(number.c_str(), 0);
        return tok_number;
    }

    // if it's a comment, skip until next line
    if (last_char == '#') {
        while (last_char != EOF && last_char != '\n' && last_char != '\r') {
            last_char = getchar();
        }
        if (last_char != EOF) {
            // using recursion to simplify code flow
            return get_token();
        }
    }

    if (last_char == EOF)
        return tok_eof;

    int rc = last_char;
    last_char = getchar();
    return rc;
}

int get_next_token() {
    return (current_token = get_token());
}

int get_token_precedence() {
    //if (!isascii(current_token))
    if (current_token < 0 || current_token > 127){
        return -1;
    }

    if (BINARY_OPERATION_PRECEDENCE.count(current_token) == 0) {
        return -1;
    }

    return BINARY_OPERATION_PRECEDENCE.at(current_token);
}

ExpressionNode* parse_expression() {
    auto lhs = parse_primary();
    if (lhs == nullptr) {
        return nullptr;
    }

    return parse_binop_rhs(0, lhs);
}

ExpressionNode* parse_number_expression() {
    auto node = new NumberExpressionNode(number_value);

    // consume token from the input
    get_next_token();

    return node;
}

ExpressionNode* parse_parentheses_expression() {
    // consume open bracket '(' character
    get_next_token();

    auto expression = parse_expression();
    if (expression == nullptr) {
        return nullptr;
    }

    if (current_token != ')') {
        log_error("expected a ')' character");
        return nullptr;
    }

    // consume close bracket ')' character
    get_next_token();

    return expression;
}

ExpressionNode* parse_identifier_expression() {
    string identifer = identifier_value;

    // skip identifier token
    get_next_token();

    // its a simple variable
    if (current_token != '(') {
        return new VariableExpressionNode(identifer);
    }

    // skip '(' token
    get_next_token();
    vector<ExpressionNode *> arguments;
    while (current_token != ')') {
        if (auto argument = parse_expression()) {
            arguments.emplace_back(argument);
        } else {
            return nullptr;
        }

        if (current_token != ',' && current_token != ')') {
            log_error("expected a comma or a ')' inside function argument list");
            return nullptr;
        }
        if (current_token == ',') {
            get_next_token();
        }
    }

    // skip ')' token
    get_next_token();

    return new FunctionCallExpressionNode(identifer, arguments);
}

ExpressionNode* parse_primary() {
    switch (current_token) {
        default:
            log_error("unexpected token in place of a primary expression");
            return nullptr;

        case tok_identifier:
            return parse_identifier_expression();

        case tok_number:
            return parse_number_expression();

        case '(':
            return parse_parentheses_expression();
    }
}

ExpressionNode* parse_binop_rhs(int min_precedence, ExpressionNode *lhs) {
    while (true) {
        int token_precedence = get_token_precedence();
        if (token_precedence < min_precedence) {
            return lhs;
        }

        int binary_operation = current_token;
        // skip operation token
        get_next_token();

        auto rhs = parse_primary();
        if (rhs == nullptr) {
            return nullptr;
        }

        int next_precedence = get_token_precedence();
        if (token_precedence < next_precedence) {
            rhs = parse_binop_rhs(token_precedence+1, rhs);
            if (rhs == nullptr) {
                return nullptr;
            }
        }

        lhs = new BinaryExpressionNode(binary_operation, lhs, rhs);
    }
    return nullptr;
}

FunctionPrototypeNode* parse_function_prototype() {
    if (current_token != tok_func) {
        log_error("expected a 'func' token");
        return nullptr;
    }
    get_next_token();

    if (current_token != tok_identifier) {
        log_error("expected an identifier after 'func' keyword");
        return nullptr;
    }

    string function_name = identifier_value;
    get_next_token();

    if (current_token != '(') {
        log_error("exptected argument list after function name");
        return nullptr;
    }
    get_next_token();

    vector<string> arguments;
    while (current_token != ')') {
        if (current_token != tok_identifier) {
            log_error("exptected an identifier inside function argument list");
            return nullptr;
        }
        arguments.emplace_back(identifier_value);
        get_next_token();

        if (current_token != ')' && current_token != ',') {
            log_error("exptected a ')' or a comma");
            return nullptr;
        }
        if (current_token == ',') {
            get_next_token();
        }
    }
    get_next_token();

    return new FunctionPrototypeNode(function_name, arguments);
}

FunctionDefinitionNode* parse_function_definition() {
    auto prototype = parse_function_prototype();
    if (prototype == nullptr) {
        return nullptr;
    }

    if (auto body = parse_expression()) {
        return new FunctionDefinitionNode(prototype, body);
    }
    return nullptr;
}

FunctionDefinitionNode* parse_top_level_expression() {
    if (auto expression = parse_expression()) {
        auto prototype = new FunctionPrototypeNode("__top_level", vector<string>());
        return new FunctionDefinitionNode(prototype, expression);
    }
    return nullptr;
}

FunctionPrototypeNode* parse_function_import() {
    // skip 'import' token
    get_next_token();
    return parse_function_prototype();
}

void handle_function_definition() {
    if (auto function = parse_function_definition()) {
        printf("info: parsed function definition\n");
        function->print_description(0);
        printf("\n");
        delete function;
    } else {
        // if input contains erroneous token, skip it
        get_next_token();
    }
}

void handle_function_import() {
    if (auto import = parse_function_import()) {
        printf("info: parsed function import declaration\n");
        import->print_description(0);
        printf("\n");
        delete import;
    } else {
        // if input contains erroneous token, skip it
        get_next_token();
    }
}

void handle_top_level_expression() {
    if (auto expression = parse_top_level_expression()) {
        printf("info: parsed top level expression\n");
        expression->print_description(0);
        printf("\n");
        delete expression;
    } else {
        // if input contains erroneous token, skip it
        get_next_token();
    }
}

void log_error(const char *message) {
    printf("error: %s\n", message);
}
