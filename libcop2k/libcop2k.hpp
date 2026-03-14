#ifndef COP2K_H_INCLUDED
#define COP2K_H_INCLUDED

#include <array>
#include <bitset>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include <format>

#include "libopcode.hpp"

namespace COP2K
{
    class Register
    {
        public:
            constexpr Register(uint8_t val, const std::string &name = std::string()) :
                name(name),
                val(val)
            {}

            constexpr uint8_t get() const
            {
                return val;
            }

            constexpr void set(uint8_t val_)
            {
                val = val_;
            }

            constexpr std::string to_string() const
            {
                return std::format("\"{}\"'s value: 0x{:02X}\n", name, get());
            }

        private:
            std::string name;
            uint8_t val;
    };

    class RegisterWithCallback
    {
        public:
            // initialization will not trigger callback
            RegisterWithCallback(uint8_t val, const std::string &name = std::string()) :
                val(val),
                name(name),
                callback([](RegisterWithCallback &) {})
            {}

            constexpr uint8_t get() const
            {
                return val;
            }

            void set(uint8_t val_)
            {
                val = val_;
                callback(*this);
            }

            void set_callback(const std::function<void(RegisterWithCallback &)> &func)
            {
                callback = func;
            }

            void clear_callback()
            {
                callback = [](RegisterWithCallback &) {};
            }

            constexpr std::string to_string() const
            {
                return std::format("\"{}\"'s value: 0x{:02X}\n", name, get());
            }

        private:
            uint8_t val;
            std::string name;
            std::function<void(RegisterWithCallback &)> callback;
    };

    class Flag
    {
        public:
            Flag(bool val, const std::string &name = std::string()) :
                name(name),
                val(val)
            {}

            constexpr bool get() const
            {
                return val;
            }

            constexpr void pos()
            {
                val = true;
            }

            constexpr void neg()
            {
                val = false;
            }

            constexpr void set(bool val_)
            {
                val = val_;
            }

            constexpr std::string to_string() const
            {
                return std::format("\"{}\"'s flag value: {}\n", name, get());
            }

        private:
            std::string name;
            bool val : 1;
    };

    class FlagWithCallback
    {
        public:
            FlagWithCallback(bool val, const std::string &name = std::string()) :
                name(name),
                callback([](FlagWithCallback &) {}),
                     val(val)
            {}

            constexpr bool get() const
            {
                return val;
            }

            void pos()
            {
                val = true;
                callback(*this);
            }

            void neg()
            {
                val = false;
                callback(*this);
            }

            void set(bool val_)
            {
                val = val_;
                callback(*this);
            }

            void set_callback(const std::function<void(FlagWithCallback &)> &func)
            {
                callback = func;
            }

            void clear_callback()
            {
                callback = [](FlagWithCallback &) {};
            }

            constexpr std::string to_string() const
            {
                return std::format("\"{}\"'s flag value: {}\n", name, get());
            }


        private:
            std::string name;
            std::function<void(FlagWithCallback &)> callback;
            bool val : 1;
    };

    class ALU
    {
        public:
            // DO NOT MODIFY
            // THIS IS DEFINED BY THE MACHINE
            enum class CalcTypes : uint8_t {
                ADD,
                SUB,
                OR,
                AND,
                CARRY_ADD,
                CARRY_SUB,
                NOT,
                DIRECT_A
            };

            constexpr ALU() :
                cy(false, "Cy IN"),
                cn(false, "CN"),
                z(false, "Z"),
                fen(false, "FEN")
            {}

            constexpr void set_calc_type(CalcTypes val)
            {
                calc_type = val;
            }

            constexpr CalcTypes get_calc_type() const
            {
                return calc_type;
            }

            // left, direct, right
            constexpr std::tuple<uint8_t, uint8_t, uint8_t> calc(uint8_t A, uint8_t W)
            {
                int result = 0; // must use `int` to test overflow

                switch (calc_type) {
                    case CalcTypes::ADD:
                        result = A + W;
                        break;

                    case CalcTypes::SUB:
                        result = A - W;
                        break;

                    case CalcTypes::OR:
                        result = A | W;
                        break;

                    case CalcTypes::AND:
                        result = A & W;
                        break;

                    case CalcTypes::CARRY_ADD:
                        result = A + W + cy.get();
                        break;

                    case CalcTypes::CARRY_SUB:
                        result = A - W - cy.get();
                        break;

                    case CalcTypes::NOT:
                        result = ~A;
                        break;

                    case CalcTypes::DIRECT_A:
                        result = A;
                        break;
                }

                if (fen.get()) {
                    cy.set(result < -128 || result > 127);
                    z.set(!result);
                }

                return std::make_tuple<uint8_t, uint8_t, uint8_t>(
                           (result << 1) | (cy.get() & cn.get()),
                           result,
                           (result >> 1) | ((cy.get() & cn.get()) << 7)
                       );
            }

            constexpr std::string to_string() const
            {
                const char *calc_type_str = nullptr;

                switch (calc_type) {
                    case CalcTypes::ADD:
                        calc_type_str = "Add A and W (A + W)";
                        break;

                    case CalcTypes::SUB:
                        calc_type_str = "Subtract W from A (A - W)";
                        break;

                    case CalcTypes::OR:
                        calc_type_str = "Or A and W (A | W)";
                        break;

                    case CalcTypes::AND:
                        calc_type_str = "And A and W (A & W)";
                        break;

                    case CalcTypes::CARRY_ADD:
                        calc_type_str = "Add A and W with carry (A + W + C)";
                        break;

                    case CalcTypes::CARRY_SUB:
                        calc_type_str = "Subtract W from A with carry (A - W - C)";
                        break;

                    case CalcTypes::NOT:
                        calc_type_str = "Not A (~A)";
                        break;

                    case CalcTypes::DIRECT_A:
                        calc_type_str = "Direct A (A)";
                        break;
                }

                return std::format(
                           "ALU's status:\n"
                           "Calculation type: {}\n"
                           "{}\n"
                           "{}\n"
                           "{}\n"
                           "{}\n",
                           calc_type_str,
                           cy.to_string(),
                           cn.to_string(),
                           z.to_string(),
                           fen.to_string()
                       );
            }

            Flag cy;
            Flag cn;
            Flag z;
            Flag fen;
        private:
            CalcTypes calc_type : 3;
    };

    // ReaderType means "read from bus"
    // WriterType means "write to bus"
    template<typename ReaderType, typename WriterType>
    class Bus
    {
        public:
            constexpr bool has_writer() const
            {
                return writer != WriterType::NONE;
            }

            constexpr bool has_reader() const
            {
                return !reader.empty();
            }

            constexpr WriterType get_writer() const
            {
                return writer;
            }

            constexpr const std::vector<ReaderType> &get_reader() const
            {
                return reader;
            }

            constexpr void set_writer(WriterType val)
            {
                if (has_writer())
                    throw std::logic_error("this bus already has a writer");

                writer = val;
            }

            constexpr void add_reader(ReaderType val)
            {
                reader.push_back(val);
            }

            constexpr void clear_writer()
            {
                writer = WriterType::NONE;
            }

            constexpr void clear_reader()
            {
                reader.clear();
            }

            constexpr uint8_t get_data() const
            {
                if (!has_writer())
                    throw std::logic_error("this bus has no writer");

                return data;
            }

            constexpr void set_data(uint8_t val)
            {
                if (!has_writer())
                    throw std::logic_error("this bus has no writer");

                data = val;
            }

            virtual constexpr std::string to_string() const = 0;

        private:
            std::vector<ReaderType> reader;
            WriterType writer;
            uint8_t data;
    };

    enum class DBusReaderType {
        MAR,
        OUT,
        ST,
        PC,
        A,
        W,
        REG,
        EM
    };
    enum class DBusWriterType {
        NONE,
        IN,
        IA,
        ST,
        PC,
        D,
        L,
        R,
        REG,
        EM,
        MANUAL
    };
    class DBus : public Bus<DBusReaderType, DBusWriterType>
    {
        public:
            constexpr std::string to_string() const override
            {
                std::string reader_str;
                const char *writer_str = nullptr;

                switch (get_writer()) {
                    case DBusWriterType::NONE:
                        writer_str = "No writer";
                        break;

                    case DBusWriterType::IN:
                        writer_str = "User IN (IN)";
                        break;

                    case DBusWriterType::IA:
                        writer_str = "Interrupt address (IA)";
                        break;

                    case DBusWriterType::ST:
                        writer_str = "Stack pointer (ST)";
                        break;

                    case DBusWriterType::PC:
                        writer_str = "Program count (PC)";
                        break;

                    case DBusWriterType::D:
                        writer_str = "ALU direct (D)";
                        break;

                    case DBusWriterType::L:
                        writer_str = "ALU shift left (L)";
                        break;

                    case DBusWriterType::R:
                        writer_str = "ALU shift right (R)";
                        break;

                    case DBusWriterType::REG:
                        writer_str = "Register (R?)";
                        break;

                    case DBusWriterType::EM:
                        writer_str = "Memory (EM)";
                        break;

                    case DBusWriterType::MANUAL:
                        writer_str = "Manual input";
                        break;
                }

                for (DBusReaderType i : get_reader())
                    switch (i) {
                        case DBusReaderType::MAR:
                            reader_str.append("    Memory address register (MAR)\n");
                            break;

                        case DBusReaderType::OUT:
                            reader_str.append("    User OUT (OUT)\n");
                            break;

                        case DBusReaderType::ST:
                            reader_str.append("    Stack pointer (ST)\n");
                            break;

                        case DBusReaderType::PC:
                            reader_str.append("    Program count (PC)\n");
                            break;

                        case DBusReaderType::A:
                            reader_str.append("    A\n");
                            break;

                        case DBusReaderType::W:
                            reader_str.append("    W\n");
                            break;

                        case DBusReaderType::REG:
                            reader_str.append("    Register (R?)\n");
                            break;

                        case DBusReaderType::EM:
                            reader_str.append("    Memory (EM)\n");
                            break;
                    }

                return std::format(
                           "Data bus's status:\n"
                           "Writer: {}\n"
                           "Readers:\n"
                           "{}\n",
                           writer_str,
                           reader_str
                       );
            }
    };

    enum class ABusReaderType {
        EM
    };
    enum class ABusWriterType {
        NONE,
        PC,
        MAR
    };
    class ABus : public Bus<ABusReaderType, ABusWriterType>
    {
        public:
            constexpr std::string to_string() const override
            {
                std::string reader_str;
                const char *writer_str = nullptr;

                switch (get_writer()) {
                    case ABusWriterType::NONE:
                        writer_str = "No writer";
                        break;

                    case ABusWriterType::PC:
                        writer_str = "Program count (PC)";
                        break;

                    case ABusWriterType::MAR:
                        writer_str = "Memory address register (MAR)";
                        break;
                }

                for (ABusReaderType i : get_reader())
                    switch (i) {
                        case ABusReaderType::EM:
                            reader_str.append("    Memory (EM)\n");
                            break;
                    }

                return std::format(
                           "Address bus's status:\n"
                           "Writer: {}\n"
                           "Readers:\n"
                           "{}\n",
                           writer_str,
                           reader_str
                       );
            }
    };

    enum class IBusReaderType {
        UPC,
        IR
    };
    enum class IBusWriterType {
        NONE,
        EM,
        INTERRUPT
    };
    class IBus : public Bus<IBusReaderType, IBusWriterType>
    {
        public:
            constexpr std::string to_string() const override
            {
                std::string reader_str;
                const char *writer_str = nullptr;

                switch (get_writer()) {
                    case IBusWriterType::NONE:
                        writer_str = "No writer";
                        break;

                    case IBusWriterType::EM:
                        writer_str = "Memory (EM)";

                    case IBusWriterType::INTERRUPT:
                        writer_str = "Interrupt special (0xB8)";
                }

                for (IBusReaderType i : get_reader())
                    switch (i) {
                        case IBusReaderType::UPC:
                            reader_str.append("    Micro program counter (UPC)\n");
                            break;

                        case IBusReaderType::IR:
                            reader_str.append("    Instruction register (IR)\n");
                            break;
                    }

                return std::format(
                           "Address bus's status:\n"
                           "Writer: {}\n"
                           "Readers:\n"
                           "{}\n",
                           writer_str,
                           reader_str
                       );
            }
    };

    class Memory
    {
        public:
            constexpr void set_addr(uint8_t val)
            {
                addr = val;
            }

            constexpr void set_data(uint8_t val)
            {
                mem.at(addr) = val;
            }

            constexpr uint8_t get_addr() const
            {
                return addr;
            }

            constexpr uint8_t get_data() const
            {
                return mem.at(addr);
            }

            constexpr void clear()
            {
                for (unsigned i = 0; i < 256; i++)
                    set_data_at(i, 0);

                set_addr(0);
            }

            // bypass normal addr lookup mode
            constexpr void set_data_at(uint8_t actaddr, uint8_t val)
            {
                mem.at(actaddr) = val;
            }

            constexpr uint8_t get_data_at(uint8_t actaddr) const
            {
                return mem.at(actaddr);
            }

            constexpr std::string dump_content() const
            {
                std::string ret;

                for (unsigned i = 0; i < 256; i++)
                    ret.push_back(get_data_at(i));

                return ret;
            }

            constexpr std::string to_string() const
            {
                std::string ret("Memory status:");

                for (unsigned i = 0; i < 256; i++) {
                    if (!(i & 0xf))
                        ret.append(std::format("\n0x{:02X}: ", i));

                    ret.append(std::format("{:02X} ", get_data_at(i)));
                }

                ret.push_back('\n');
                return ret;
            }

        private:
            std::array<uint8_t, 256> mem;
            uint8_t addr;
    };

    class MicroProgramMemory
    {
        public:
            constexpr void set_addr(uint8_t val)
            {
                addr = val;
            }

            constexpr void set_data(const std::bitset<24> &val)
            {
                mem.at(addr) = val;
            }

            constexpr uint8_t get_addr() const
            {
                return addr;
            }

            constexpr const std::bitset<24> &get_data() const
            {
                return mem.at(addr);
            }

            constexpr void clear()
            {
                std::bitset<24> all_on;
                all_on.set();

                for (unsigned i = 0; i < 256; i++)
                    set_data_at(i, all_on);

                set_addr(0);
            }

            // bypass normal addr lookup mode
            constexpr void set_data_at(uint8_t actaddr, const std::bitset<24> &val)
            {
                mem.at(actaddr) = val;
            }

            constexpr void set_data_at(uint8_t actaddr, unsigned bit_pos, bool val)
            {
                mem.at(actaddr).set(bit_pos, val);
            }

            constexpr const std::bitset<24> &get_data_at(uint8_t actaddr) const
            {
                return mem.at(actaddr);
            }

            constexpr std::string dump_content() const
            {
                std::string ret;

                for (unsigned i = 0; i < 256; i++) {
                    unsigned a = get_data_at(i).to_ulong();
                    ret.push_back((a >> 16) & 0xff);
                    ret.push_back((a >> 8) & 0xff);
                    ret.push_back(a & 0xff);
                }

                return ret;
            }

            constexpr std::string to_string() const
            {
                std::string ret("Micro program memory status:");

                for (unsigned i = 0; i < 256; i++) {
                    if (!(i & 0xf))
                        ret.append(std::format("\n0x{:02X}: ", i));

                    ret.append(std::format("{:06X} ", get_data_at(i).to_ulong()));
                }

                ret.push_back('\n');
                return ret;
            }

        private:
            std::array<std::bitset<24>, 256> mem;
            uint8_t addr;
    };

    class COP2K
    {
        public:
            COP2K() :
                l(0, "L"),
                d(0, "D"),
                r(0, "R"),
                r0(0, "R0"),
                r1(0, "R1"),
                r2(0, "R2"),
                r3(0, "R3"),
                manual_dbus(true, "Data bus input is manual"),
                sa(false, "SA"),
                sb(false, "SB"),
                ireq(false, "IREQ"),
                iack(false, "IACK"),
                running_manually(true, "Running manually"),
                halt(true, "Halt"),
                manual_dbus_input(0, "Data bus manual input"),
                upc(0, "UPC"),
                pc(0, "PC"),
                mar(0, "MAR"),
                ia(0, "IA"),
                st(0, "ST"),
                in(0, "IN"),
                out(0, "OUT"),
                ir(0, "IR"),
                a(0, "A"),
                w(0, "W"),
                emwr(true, "EMWR"),
                emrd(true, "EMRD"),
                pcoe(true, "PCOE"),
                emen(true, "EMEN"),
                iren(true, "IREN"),
                eint(true, "EINT"),
                elp(true, "ELP"),
                maren(true, "MAREN"),
                maroe(true, "MAROE"),
                outen(true, "OUTEN"),
                sten(true, "STEN"),
                rrd(true, "RRD"),
                rwr(true, "RWR"),
                x2(true, "X2"),
                x1(true, "X1"),
                x0(true, "X0"),
                wen(true, "WEN"),
                aen(true, "AEN"),
                s2(true, "S2"),
                s1(true, "S1"),
                s0(true, "S0")
            {
                a.set_callback([this](RegisterWithCallback &) {
                    update_alu();
                });
                w.set_callback([this](RegisterWithCallback &) {
                    update_alu();
                });
                s0.set_callback([this](FlagWithCallback &) {
                    update_alu();
                });
                s1.set_callback([this](FlagWithCallback &) {
                    update_alu();
                });
                s2.set_callback([this](FlagWithCallback &) {
                    update_alu();
                });
                update_alu();
                pos_fen();
                pos_cn();
            }

            constexpr void run_forever()
            {
                while (!halt.get())
                    run_clock();
            }

            constexpr void run_clock()
            {
                get_control_signal();
                set_bus_status();
                modify_bus_data();
            }

            constexpr void run_instruction()
            {
                // NOTE: we assume user has loaded opcode
                unsigned char clock_count = opcode.get_from_byte(upc.get()).signal_count;

                while (clock_count--)
                    run_clock();
            }

            constexpr void trigger_interrupt()
            {
                ireq.pos();
            }

            constexpr void pos_fen()
            {
                alu.fen.pos();
            }

            constexpr void pos_cn()
            {
                alu.cn.pos();
            }

            constexpr void pos_cy()
            {
                alu.cy.pos();
            }

            constexpr void neg_fen()
            {
                alu.fen.neg();
            }

            constexpr void neg_cn()
            {
                alu.cn.neg();
            }

            constexpr void neg_cy()
            {
                alu.cy.neg();
            }

            constexpr void set_fen(bool val)
            {
                alu.fen.set(val);
            }

            constexpr void set_cn(bool val)
            {
                alu.cn.set(val);
            }

            constexpr void set_cy(bool val)
            {
                alu.cy.set(val);
            }

            constexpr bool get_fen() const
            {
                return alu.fen.get();
            }

            constexpr bool get_cn() const
            {
                return alu.cn.get();
            }

            constexpr bool get_cy() const
            {
                return alu.cy.get();
            }

            constexpr uint8_t get_em_data(uint8_t addr) const
            {
                return em.get_data_at(addr);
            }

            constexpr void set_em_data(uint8_t addr, uint8_t val)
            {
                em.set_data_at(addr, val);
            }

            constexpr void clear_em()
            {
                em.clear();
            }

            constexpr const std::bitset<24> &get_um_data(uint8_t addr) const
            {
                return um.get_data_at(addr);
            }

            constexpr void set_um_data(uint8_t addr, const std::bitset<24> &val)
            {
                um.set_data_at(addr, val);
                opcode.patch_um(addr, val);
            }

            constexpr void set_um_data(uint8_t addr, unsigned bit_pos, bool val)
            {
                um.set_data_at(addr, bit_pos, val);
                opcode.patch_um(addr, bit_pos, val);
            }

            constexpr void clear_um()
            {
                um.clear();
                opcode.clear();
            }

            constexpr void load_instruction(FILE *in)
            {
                opcode.load_instr_txt(in);

                for (const Opcode::Instruction &i : opcode)
                    for (unsigned char j = 0; j < 4; j++)
                        um.set_data_at(i.byte | j, i.microprogram.at(j));
            }

            constexpr std::string reg_to_string() const
            {
                std::string ret;
                ret.append(l.to_string());
                ret.append(d.to_string());
                ret.append(r.to_string());
                ret.append(r0.to_string());
                ret.append(r1.to_string());
                ret.append(r2.to_string());
                ret.append(r3.to_string());
                ret.append(manual_dbus_input.to_string());
                ret.append(upc.to_string());
                ret.append(pc.to_string());
                ret.append(mar.to_string());
                ret.append(ia.to_string());
                ret.append(st.to_string());
                ret.append(in.to_string());
                ret.append(out.to_string());
                ret.append(ir.to_string());
                ret.append(a.to_string());
                ret.append(w.to_string());
                return ret;
            }

            constexpr std::string flag_to_string() const
            {
                std::string ret;
                ret.append(manual_dbus.to_string());
                ret.append(sa.to_string());
                ret.append(sb.to_string());
                ret.append(ireq.to_string());
                ret.append(iack.to_string());
                ret.append(running_manually.to_string());
                ret.append(halt.to_string());
                ret.append(emwr.to_string());
                ret.append(emrd.to_string());
                ret.append(pcoe.to_string());
                ret.append(emen.to_string());
                ret.append(iren.to_string());
                ret.append(eint.to_string());
                ret.append(elp.to_string());
                ret.append(maren.to_string());
                ret.append(maroe.to_string());
                ret.append(outen.to_string());
                ret.append(sten.to_string());
                ret.append(rrd.to_string());
                ret.append(rwr.to_string());
                ret.append(x2.to_string());
                ret.append(x1.to_string());
                ret.append(x0.to_string());
                ret.append(wen.to_string());
                ret.append(aen.to_string());
                ret.append(s2.to_string());
                ret.append(s1.to_string());
                ret.append(s0.to_string());
                return ret;
            }

            constexpr std::string to_string() const
            {
                std::string ret("COP2K's status:\n");
                ret.append("Registers:\n");
                ret.append(reg_to_string());
                ret.append("Flags:\n");
                ret.append(flag_to_string());
                ret.append("Buses:\n");
                ret.append(dbus.to_string());
                ret.append(abus.to_string());
                ret.append(ibus.to_string());
                ret.append("ALU:\n");
                ret.append(alu.to_string());
                ret.append("Memory:\n");
                ret.append(em.to_string());
                ret.append("Micro program memory:\n");
                ret.append(um.to_string());
                return ret;
            }

            Register l, d, r;
            Register r0, r1, r2, r3;

            // valid when TRUE
            Flag manual_dbus;
            Flag sa;
            Flag sb;
            Flag ireq;
            Flag iack;
            Flag running_manually;
            Flag halt;

            Register manual_dbus_input;
            Register upc;
            Register pc;
            Register mar;
            Register ia;
            Register st;
            Register in;
            Register out;
            Register ir;

            RegisterWithCallback a, w;

            // valid when FALSE
            // errr... don't know its usage
            // Flag xrd;
            Flag emwr;
            Flag emrd;
            Flag pcoe;
            Flag emen;
            Flag iren;
            Flag eint;
            Flag elp;
            Flag maren;
            Flag maroe;
            Flag outen;
            Flag sten;
            Flag rrd;
            Flag rwr;
            // set in ALU
            // Flag cn;
            // Flag fen;
            Flag x2, x1, x0;
            Flag wen, aen;
            FlagWithCallback s2, s1, s0;

        private:
            constexpr void update_alu()
            {
                // note: must be careful not to cause another callback to
                // call cthis function
                alu.set_calc_type(
                    static_cast<ALU::CalcTypes>(s2.get() << 2 | s1.get() << 1 | s0.get())
                );
                uint8_t _l, _d, _r;
                std::tie(_l, _d, _r) = alu.calc(a.get(), w.get());
                l.set(_l);
                d.set(_d);
                r.set(_r);
            }

            constexpr void get_control_signal()
            {
                // get control signal if running automatically
                if (running_manually.get())
                    return;

                const std::bitset<24> &microprogram = um.get_data();
                s0.set(microprogram.test(0));
                s1.set(microprogram.test(1));
                s2.set(microprogram.test(2));
                aen.set(microprogram.test(3));
                wen.set(microprogram.test(4));
                x0.set(microprogram.test(5));
                x1.set(microprogram.test(6));
                x2.set(microprogram.test(7));
                set_fen(microprogram.test(8));
                set_cn(microprogram.test(9));
                rwr.set(microprogram.test(10));
                rrd.set(microprogram.test(11));
                sten.set(microprogram.test(12));
                outen.set(microprogram.test(13));
                maroe.set(microprogram.test(14));
                maren.set(microprogram.test(15));
                elp.set(microprogram.test(16));
                eint.set(microprogram.test(17));
                iren.set(microprogram.test(18));
                emen.set(microprogram.test(19));
                pcoe.set(microprogram.test(20));
                emrd.set(microprogram.test(21));
                emwr.set(microprogram.test(22));
            }

            constexpr void set_bus_status()
            {
                dbus.clear_reader();
                dbus.clear_writer();
                ibus.clear_reader();
                ibus.clear_writer();
                abus.clear_reader();
                abus.clear_writer();

                // if somebody is interrupting reply to them
                if (ireq.get() && !iack.get()) {
                    ibus.set_writer(IBusWriterType::INTERRUPT);
                    emrd.pos();
                    iack.pos();
                }

                if (!emrd.get())
                    ibus.set_writer(IBusWriterType::EM);

                if (!pcoe.get())
                    abus.set_writer(ABusWriterType::PC);

                if (!emen.get()) {
                    if (!emwr.get())
                        dbus.add_reader(DBusReaderType::EM);

                    if (!emrd.get())
                        dbus.set_writer(DBusWriterType::EM);
                }

                if (!iren.get()) {
                    ibus.add_reader(IBusReaderType::IR);
                    ibus.add_reader(IBusReaderType::UPC);
                }

                if (!eint.get()) {
                    iack.neg();
                    ireq.neg();
                }

                if (!elp.get())
                    dbus.add_reader(DBusReaderType::PC);

                if (!maren.get())
                    dbus.add_reader(DBusReaderType::MAR);

                if (!maroe.get())
                    abus.set_writer(ABusWriterType::MAR);

                if (!outen.get())
                    dbus.add_reader(DBusReaderType::OUT);

                if (!sten.get())
                    dbus.add_reader(DBusReaderType::ST);

                if (!rrd.get())
                    dbus.set_writer(DBusWriterType::REG);

                if (!rwr.get())
                    dbus.add_reader(DBusReaderType::REG);

                if (!wen.get())
                    dbus.add_reader(DBusReaderType::W);

                if (!aen.get())
                    dbus.add_reader(DBusReaderType::A);

                switch (x2.get() << 2 | x1.get() << 1 | x0.get()) {
                    case 0:
                        dbus.set_writer(DBusWriterType::IN);
                        break;

                    case 1:
                        dbus.set_writer(DBusWriterType::IA);
                        break;

                    case 2:
                        dbus.set_writer(DBusWriterType::ST);
                        break;

                    case 3:
                        dbus.set_writer(DBusWriterType::PC);
                        break;

                    case 4:
                        dbus.set_writer(DBusWriterType::D);
                        break;

                    case 5:
                        dbus.set_writer(DBusWriterType::R);
                        break;

                    case 6:
                        dbus.set_writer(DBusWriterType::L);
                        break;

                    case 7:
                        break;
                }

                // manual dbus will override previous writer
                if (manual_dbus.get()) {
                    dbus.clear_writer();
                    dbus.set_writer(DBusWriterType::MANUAL);
                }
            }

            constexpr void modify_bus_data()
            {
                switch (abus.get_writer()) {
                    case ABusWriterType::NONE:
                        break;

                    case ABusWriterType::MAR:
                        abus.set_data(mar.get());
                        break;

                    case ABusWriterType::PC:
                        // it may be subsequently overwritten by !ELP
                        abus.set_data(pc.get());
                        pc.set(pc.get() + 1);
                        break;
                }

                switch (dbus.get_writer()) {
                    case DBusWriterType::NONE:
                        break;

                    case DBusWriterType::IN:
                        dbus.set_data(in.get());
                        break;

                    case DBusWriterType::IA:
                        dbus.set_data(ia.get());
                        break;

                    case DBusWriterType::ST:
                        dbus.set_data(st.get());
                        break;

                    case DBusWriterType::PC:
                        dbus.set_data(pc.get());
                        break;

                    case DBusWriterType::D:
                        dbus.set_data(d.get());
                        break;

                    case DBusWriterType::L:
                        dbus.set_data(l.get());
                        break;

                    case DBusWriterType::R:
                        dbus.set_data(r.get());
                        break;

                    case DBusWriterType::REG:
                        switch (sb.get() << 1 | sa.get()) {
                            case 0:
                                dbus.set_data(r0.get());
                                break;

                            case 1:
                                dbus.set_data(r1.get());
                                break;

                            case 2:
                                dbus.set_data(r2.get());
                                break;

                            case 3:
                                dbus.set_data(r3.get());
                                break;
                        }

                        break;

                    case DBusWriterType::EM:
                        dbus.set_data(em.get_data());
                        break;

                    case DBusWriterType::MANUAL:
                        dbus.set_data(manual_dbus_input.get());
                        break;
                }

                switch (ibus.get_writer()) {
                    case IBusWriterType::NONE:
                        break;

                    case IBusWriterType::EM:
                        ibus.set_data(em.get_data());
                        break;

                    case IBusWriterType::INTERRUPT:
                        ibus.set_data(0xB8);
                        break;
                }

                for (ABusReaderType i : abus.get_reader())
                    switch (i) {
                        case ABusReaderType::EM:
                            em.set_addr(abus.get_data());
                            break;
                    }

                for (DBusReaderType i : dbus.get_reader())
                    switch (i) {
                        case DBusReaderType::MAR:
                            mar.set(dbus.get_data());
                            break;

                        case DBusReaderType::OUT:
                            out.set(dbus.get_data());
                            break;

                        case DBusReaderType::ST:
                            st.set(dbus.get_data());
                            break;

                        case DBusReaderType::PC:
                            if (
                                (ir.get() & 0x8) >> 3 == 1 || // jump unconditionally
                                ((ir.get() & 0xC) >> 2 == 0 && alu.cy.get()) || // jump on carry
                                ((ir.get() & 0xC) >> 2 == 1 && alu.z.get()) // jump on zero
                            )
                                pc.set(dbus.get_data());

                            break;

                        case DBusReaderType::A:
                            a.set(dbus.get_data());
                            break;

                        case DBusReaderType::W:
                            w.set(dbus.get_data());
                            break;

                        case DBusReaderType::REG:
                            switch (sb.get() << 1 | sa.get()) {
                                case 0:
                                    r0.set(dbus.get_data());
                                    break;

                                case 1:
                                    r1.set(dbus.get_data());
                                    break;

                                case 2:
                                    r2.set(dbus.get_data());
                                    break;

                                case 3:
                                    r3.set(dbus.get_data());
                                    break;
                            }

                            break;

                        case DBusReaderType::EM:
                            em.set_data(dbus.get_data());
                            break;
                    }

                bool upc_modify = false;

                for (IBusReaderType i : ibus.get_reader())
                    switch (i) {
                        case IBusReaderType::IR:
                            ir.set(ibus.get_data());
                            sa.set(ibus.get_data() & (1 << 0));
                            sb.set(ibus.get_data() & (1 << 1));
                            break;

                        case IBusReaderType::UPC:
                            upc_modify = true;
                            upc.set(ibus.get_data() & ~0x3);
                            um.set_addr(upc.get());
                            break;
                    }

                if (!upc_modify)
                    upc.set(upc.get() + 1);
            }

            Opcode opcode;
            Memory em;
            MicroProgramMemory um;
            ALU alu;
            DBus dbus;
            ABus abus;
            IBus ibus;
    };

}
#endif // COP2K_H_INCLUDED
