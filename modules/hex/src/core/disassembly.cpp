#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

namespace hex {
using namespace mana::vm;

/**
 * @brief Reads a 16-bit little-endian payload from the bytecode.
 */
static u16 ReadPayload(const u8 first_byte, const u8 second_byte) {
    return static_cast<u16>(first_byte | (second_byte << 8));
}

void PrintBytecode(const ByteCode& s) {
    const auto& code = s.Instructions();

    for (i64 i = 0; i < code.size(); ++i) {
        const i64 offset = i;
        const auto op    = static_cast<Op>(code[i]);
        const auto name  = magic_enum::enum_name(op);

        // Helper to read 2-byte payloads and advance the loop counter
        auto read = [&] {
            u16 val = ReadPayload(code[i + 1], code[i + 2]);

            i += 2;
            return val;
        };

        switch (op) {
            using enum Op;
        case Halt:
        case Err:
            Log->debug("{:04} | {}", offset, name);
            break;

        case Return: {
            const u16 reg = read();
            Log->debug("{:04} | {} R{}", offset, name, reg);
            break;
        }


        case LoadConstant: {
            const u16 reg   = read();
            const u16 idx   = read();
            const auto& val = s.Constants()[idx];

            const auto log_val = [&](auto v) {
                Log->debug("{:04} | {} R{} <- {} [constant index: {}]", offset, name, reg, v, idx);
            };

            switch (val.GetType()) {
            case mana::PrimitiveType::Float64:
                log_val(val.AsFloat());
                break;
            case mana::PrimitiveType::Int64:
                log_val(val.AsInt());
                break;
            case mana::PrimitiveType::Uint64:
                log_val(val.AsUint());
                break;
            case mana::PrimitiveType::Bool:
                log_val(val.AsBool());
                break;
            case mana::PrimitiveType::None:
                log_val("none");
                break;
            default:
                log_val("???");
                break;
            }
            break;
        }

        case Move:
        case Negate:
        case Not: {
            const u16 dst = read();
            const u16 src = read();
            Log->debug("{:04} | {} R{}, R{}", offset, name, dst, src);
            break;
        }

        case Add:
        case Sub:
        case Div:
        case Mul:
        case Mod:
        case Cmp_Greater:
        case Cmp_GreaterEq:
        case Cmp_Lesser:
        case Cmp_LesserEq:
        case Equals:
        case NotEquals: {
            const u16 dst = read();
            const u16 lhs = read();
            const u16 rhs = read();
            Log->debug("{:04} | {} R{}, R{}, R{}", offset, name, dst, lhs, rhs);
            break;
        }

        case Jump: {
            const i16 dist = static_cast<i16>(read());
            // Offset + Opcode (1) + Payload (2) + Distance
            Log->debug("{:04} | {} => {:04}", offset, name, offset + 3 + dist);
            break;
        }

        case JumpWhenTrue:
        case JumpWhenFalse: {
            const u16 reg  = read();
            const i16 dist = static_cast<i16>(read());
            // Offset + Opcode (1) + Reg (2) + Destination (2)
            Log->debug("{:04} | {} R{} => {:04}", offset, name, reg, offset + 5 + dist);
            break;
        }

        default:
            Log->debug("{:04} | ??? ({})", offset, static_cast<u8>(op));
            break;
        }
    }
}
} // namespace hex
