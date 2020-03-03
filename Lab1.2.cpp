#include <iostream>
#include <cstdio>
using namespace std;

enum class Token {  //Types of tokens
    END_OF_FILE, // long name caused by collision with C library EOF definition
    DEFINITION,
    DEFINITION_END,
    IDENTIFIER,
    NUMBER,
    OPERATION,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    CHARACTER
};

// some variables which would be assigned by get_token function
double number_value;      //global variables
string identifier_value;  //ex. some_function
char operation_value;
char character_value;

Token get_token() {     //identifies the type of the next token
    static char last_char = ' ';

    while (isspace(last_char))
        last_char = getchar();

    if (isalpha(last_char) || last_char == '_') {
        identifier_value = last_char;
        while ((last_char = getchar()) && (isalnum(last_char) || last_char == '_')) {
            identifier_value += last_char; // concatenation
        }

        if (identifier_value == "def")
            return Token::DEFINITION;
        if (identifier_value == "end")
            return Token::DEFINITION_END;
        return Token::IDENTIFIER;
    }

    if (isdigit(last_char) || last_char == '.') {
        string number;
        while (isdigit(last_char) || last_char == '.') {
            number += last_char;
            last_char = getchar();
        }

        number_value = strtod(number.c_str(), 0);
        return Token::NUMBER;
    }

    character_value = last_char;
    last_char = getchar();

    if (character_value == EOF)
        return Token::END_OF_FILE;
    if (character_value == '(')
        return Token::OPEN_BRACKET;
    if (character_value == ')')
        return Token::CLOSE_BRACKET;
    if (character_value == '+' || character_value == '-'
        || character_value == '*' || character_value == '/' || character_value == '=')
        return Token::OPERATION;
    return Token::CHARACTER;
}


int main() {
    // temporary bind stdin to local file
    freopen(".\\Lab1.2sample-program.txt", "r", stdin);

    Token token;
    while ((token = get_token()) != Token::END_OF_FILE) {
        switch (token) {
            case Token::DEFINITION:
                cout << "definition " << identifier_value << endl;
                break;
            case Token::DEFINITION_END:
                cout << "definition_end" << endl;
                break;
            case Token::IDENTIFIER:
                cout << "identifier " << identifier_value << endl;
                break;
            case Token::NUMBER:
                cout << "number " << number_value << endl;
                break;
            case Token::OPERATION:
                cout << "operator " << character_value << endl;
                break;
            case Token::OPEN_BRACKET:
                cout << "open_bracket" << endl;
                break;
            case Token::CLOSE_BRACKET:
                cout << "close_bracket" << endl;
                break;
        }
    }

    return 0;
}
