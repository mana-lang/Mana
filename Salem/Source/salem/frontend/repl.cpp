#include <salem/frontend/repl.hpp>
#include <salem/frontend/lexer.hpp>

#include <iostream>
#include <string>

namespace salem {

void repl::run() {
    std::string input;
    lexer lexer;

    while (input != "exit") {
        std::cout << PROMPT;
        std::getline(std::cin, input);

        if (input.back() != '\n') {
            input.push_back('\n');
        }
        lexer.tokenize_line(input);
        lexer.print_tokens();
        input.clear();
    }
}

} // namespace salem
