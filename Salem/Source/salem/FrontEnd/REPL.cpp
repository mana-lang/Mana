#include <Salem/FrontEnd/REPL.hpp>
#include <Salem/FrontEnd/Lexer.hpp>

#include <iostream>
#include <string>

namespace salem {

void REPL::Run() {
    std::string input;
    Lexer lexer;

    while (input != "exit") {
        std::cout << PROMPT;
        std::getline(std::cin, input);

        if (input.back() != '\n') {
            input.push_back('\n');
        }
        lexer.TokenizeLine(input);
        lexer.PrintTokens();
        input.clear();
    }
}

} // namespace salem
