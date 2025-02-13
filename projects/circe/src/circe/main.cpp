#include <circe/core/logger.hpp>
#include <circe/main-visitor.hpp>

#include <sigil/ast/lexer.hpp>
#include <sigil/ast/nodes.hpp>
#include <sigil/ast/parser.hpp>

#include <mana/vm/slice.hpp>

#include <fstream>

using namespace mana::literals;
using namespace circe;

constexpr auto FILE_TO_TOKENIZE = "assets/samples/expr-a.mn";

// int R() {
//     using namespace circe;
//     Log->debug("Hello from Circe!");
//
//     sigil::Lexer lexer;
//     if (lexer.Tokenize(FILE_TO_TOKENIZE)) {
//         Log->debug("Tokenized file from '{}'", FILE_TO_TOKENIZE);
//     }
//
//     sigil::Parser parser(lexer.RelinquishTokens());
//     if (parser.Parse()) {
//         Log->debug("Parsed the file.");
//     }
//
//     mana::vm::Slice slice;
//     const auto&     ast = parser.ViewAST();
//     MainVisitor     visitor;
//
//     ast->Accept(visitor);
//
//     // for now
//     slice = visitor.GetSlice();
//     slice.Write(mana::vm::Op::Halt);
//     constexpr auto output_path = "../hex/test-circe.mhm";
//     std::ofstream  out_file(output_path, std::ios::binary);
//     Log->debug("Output file to: {}", output_path);
//
//     const auto output = slice.Serialize();
//     out_file.write(reinterpret_cast<const char*>(output.data()), output.size());
//
//     if (not out_file) {
//         Log->error("Failed to write to file.");
//         return 3;
//     }
//
//     out_file.close();
// }

#include <array>

template <typename T>
consteval T ToSerializable(T) {
    return T();
}

consteval u32 ToSerializable(const f32 v) {
    return v;
}

consteval u64 ToSerializable(const f64 v) {
    return v;
}

consteval u8 ToSerializable(const bool v) {
    return v;
}

template <typename T>
concept SupportedType = std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<T, bool>;

template <SupportedType T>
void Serialize(std::vector<u8>& dst, T val) {
    const auto v = std::bit_cast<decltype(ToSerializable(T()))>(val);
    for (i64 i = 0; i < sizeof(T); ++i) {
        dst.push_back((v >> i * 8) & 0xFF);
    }
}

template <SupportedType T>
T Deserialize(std::vector<u8>& src) {
    std::array<u8, sizeof(T)> ret;

    for (i64 i = 0; i < ret.size(); ++i) {
        ret[i] = src[i];
    }

    return std::bit_cast<T>(ret);
}

int main() {
    std::vector<u8> bytes;

    f64 val = 57.1274;
    Serialize(bytes, val);
    f64 deser = Deserialize<f64>(bytes);

    Log->debug("Original value: {}", val);
    Log->debug("Serializing...");
    for (auto b : bytes) {
        Log->debug(b);
    }
    Log->debug("Deserialized value: {}", deser);
    Log->debug("");

    bytes.clear();
    u32 val_u = 275649;
    Serialize(bytes, val_u);
    u32 deser_u = Deserialize<u32>(bytes);

    Log->debug("Original value: {}", val_u);
    Log->debug("Serializing...");
    for (auto b : bytes) {
        Log->debug(b);
    }
    Log->debug(deser_u);
    Log->debug("Deserialized value: {}", deser_u);
}