#include <Lox/Chunk.h>

int main() {
    Chunk chunk;
    chunk.Push(OpCode::Return);
    chunk.Disassemble();
}