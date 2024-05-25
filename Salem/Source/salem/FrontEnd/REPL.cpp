#include <Salem/FrontEnd/REPL.hpp>
#include <Salem/FrontEnd/Lexer.hpp>

#include <iostream>
#include <string>

namespace salem {

void REPL::Start() {
    std::string input;
    Lexer lexer;

    while (input != "exit") {
        std::cout << PROMPT;
        std::cin >> input;
    }
}

} // namespace salem
