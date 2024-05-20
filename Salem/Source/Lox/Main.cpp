#include <Lox/Chunk.h>

int main() {
    Chunk chunk;
    chunk.Push(OpCode::Op_Return);
    chunk.Disassemble();
}