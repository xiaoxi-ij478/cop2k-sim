#ifndef COP2K_H_INCLUDED
#define COP2K_H_INCLUDED

#include <array>
#include <bitset>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace COP2K
{
    class BusConflict : std::logic_error
    {
        public:
            BusConflict() : std::logic_error("this bus already has a writer") {}
    };

    class BusNoWriter : std::logic_error
    {
        public:
            BusNoWriter() : std::logic_error("this bus has no writer") {}
    };

    class ALU
    {
        public:
            // DO NOT MODIFY
            // THIS IS DEFINED BY THE MACHINE
            enum class CalcTypes : unsigned {
                ADD,
                SUB,
                AND,
                OR,
                CARRY_ADD,
                CARRY_SUB,
                NOT,
                DIRECT_A
            };
            // DO NOT MODIFY
            // THIS IS DEFINED BY THE MACHINE
            enum class OutputType : unsigned {
                IN,
                IA,
                ST,
                PC,
                D,
                R,
                L,
                NONE
            };

            constexpr void set_calc_type(CalcTypes new_type)
            {
                calc_type = new_type;
            }

            constexpr void set_output_type(OutputType new_type)
            {
                output_type = new_type;
            }

#define READ_WRITE_FLAG(flag) \
    constexpr bool get_##flag() const \
    { \
        return _##flag; \
    } \
    \
    constexpr void set_##flag() \
    { \
        _##flag = true; \
    } \
    \
    constexpr void clear_##flag() \
    { \
        _##flag = false; \
    } \
    \
    constexpr void set_##flag(bool val) \
    { \
        _##flag = val; \
    }

            READ_WRITE_FLAG(fen)
            READ_WRITE_FLAG(cn)
            READ_WRITE_FLAG(cy)
            READ_WRITE_FLAG(z)

#undef READ_WRITE_FLAG

            // left, direct, right
            constexpr std::tuple<uint8_t, uint8_t, uint8_t> calc(uint8_t A, uint8_t W)
            {
                int result = 0;

                switch (calc_type) {
                    case CalcTypes::ADD:
                        result = A + W;
                        break;

                    case CalcTypes::SUB:
                        result = A - W;
                        break;

                    case CalcTypes::AND:
                        result = A & W;
                        break;

                    case CalcTypes::OR:
                        result = A | W;
                        break;

                    case CalcTypes::CARRY_ADD:
                        result = A + W + get_cy();
                        break;

                    case CalcTypes::CARRY_SUB:
                        result = A - W - get_cy();
                        break;

                    case CalcTypes::NOT:
                        result = ~A;
                        break;

                    case CalcTypes::DIRECT_A:
                        result = A;
                        break;
                }

                if (get_fen()) {
                    set_cy(result < -128 || result > 127);
                    set_z(!result);
                }

                return std::make_tuple<uint8_t, uint8_t, uint8_t>(
                           (result << 1) | (get_cy() & get_cn()),
                           result,
                           (result >> 1) | ((get_cy() & get_cn()) << 7)
                       );
            }

        private:
            CalcTypes calc_type : 3;
            OutputType output_type : 3;
            bool _cy : 1;
            bool _z : 1;
            bool _fen : 1;
            bool _cn : 1;
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
                    throw BusConflict();

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
                    throw BusNoWriter();

                return data;
            }

            constexpr void set_data(uint8_t val)
            {
                if (!has_writer())
                    throw BusNoWriter();

                data = val;
            }

        private:
            std::vector<ReaderType> reader;
            WriterType writer;
            uint8_t data;
    };

    enum class DBusReaderType {
        NONE,
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
    class DBus : public Bus<DBusReaderType, DBusWriterType> {};

    enum class ABusReaderType {
        NONE,
        EM
    };
    enum class ABusWriterType {
        NONE,
        PC,
        MAR
    };
    class ABus : public Bus<ABusReaderType, ABusWriterType> {};

    enum class IBusReaderType {
        NONE,
        UPC,
        IR
    };
    enum class IBusWriterType {
        NONE,
        EM,
        INTERRUPT
    };
    class IBus : public Bus<IBusReaderType, IBusWriterType> {};

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

        private:
            std::array<std::bitset<24>, 256> mem;
            uint8_t addr;
    };

    class COP2K
    {
        public:
            constexpr COP2K()
            {
                set_manual_dbus_input(0);
                set_upc(0);
                set_pc(0);
                set_mar(0);
                set_ia(0xE0);
                set_st(0);
                set_in(0);
                set_out(0);
                set_ir(0);
                set_r0(0);
                set_r1(0);
                set_r2(0);
                set_r3(0);
                set_emwr();
                set_emrd();
                set_pcoe();
                set_emen();
                set_iren();
                set_eint();
                set_elp();
                set_maren();
                set_maroe();
                set_outen();
                set_sten();
                set_rrd();
                set_rwr();
                set_x2();
                set_x1();
                set_x0();
                set_wen();
                set_aen();
                set_s2();
                set_s1();
                set_s0();
                clear_sa();
                clear_sb();
                clear_ireq();
                clear_iack();
                set_running_manually();
                set_halt();
            }

            constexpr void run_forever()
            {
                while (!get_halt())
                    run_clock();
            }

            constexpr void run_clock()
            {
                get_control_signal();
                set_bus_status();
                modify_bus_data();
            }

            constexpr void trigger_interrupt()
            {
                set_ireq();
            }

#define READ_WRITE_REGISTER(register) \
    constexpr uint8_t get_##register() const \
    { \
        return _##register; \
    } \
    \
    constexpr void set_##register(uint8_t val) \
    { \
        _##register = val; \
    }

#define READ_WRITE_FLAG_FALSE_VALID(flag) \
    constexpr bool get_##flag() const \
    { \
        return _##flag; \
    } \
    \
    constexpr void set_##flag() \
    { \
        _##flag = false; \
    } \
    \
    constexpr void clear_##flag() \
    { \
        _##flag = true; \
    } \
    \
    constexpr void set_##flag(bool val) \
    { \
        _##flag = val; \
    }

#define READ_WRITE_FLAG_TRUE_VALID(flag) \
    constexpr bool get_##flag() const \
    { \
        return _##flag; \
    } \
    \
    constexpr void set_##flag() \
    { \
        _##flag = true; \
    } \
    \
    constexpr void clear_##flag() \
    { \
        _##flag = false; \
    } \
    \
    constexpr void set_##flag(bool val) \
    { \
        _##flag = val; \
    }

            READ_WRITE_REGISTER(manual_dbus_input)
            READ_WRITE_REGISTER(upc)
            READ_WRITE_REGISTER(pc)
            READ_WRITE_REGISTER(mar)
            READ_WRITE_REGISTER(ia)
            READ_WRITE_REGISTER(st)
            READ_WRITE_REGISTER(in)
            READ_WRITE_REGISTER(out)
            READ_WRITE_REGISTER(ir)
            READ_WRITE_REGISTER(r0)
            READ_WRITE_REGISTER(r1)
            READ_WRITE_REGISTER(r2)
            READ_WRITE_REGISTER(r3)
            READ_WRITE_FLAG_FALSE_VALID(emwr)
            READ_WRITE_FLAG_FALSE_VALID(emrd)
            READ_WRITE_FLAG_FALSE_VALID(pcoe)
            READ_WRITE_FLAG_FALSE_VALID(emen)
            READ_WRITE_FLAG_FALSE_VALID(iren)
            READ_WRITE_FLAG_FALSE_VALID(eint)
            READ_WRITE_FLAG_FALSE_VALID(elp)
            READ_WRITE_FLAG_FALSE_VALID(maren)
            READ_WRITE_FLAG_FALSE_VALID(maroe)
            READ_WRITE_FLAG_FALSE_VALID(outen)
            READ_WRITE_FLAG_FALSE_VALID(sten)
            READ_WRITE_FLAG_FALSE_VALID(rrd)
            READ_WRITE_FLAG_FALSE_VALID(rwr)
            READ_WRITE_FLAG_FALSE_VALID(x2)
            READ_WRITE_FLAG_FALSE_VALID(x1)
            READ_WRITE_FLAG_FALSE_VALID(x0)
            READ_WRITE_FLAG_FALSE_VALID(wen)
            READ_WRITE_FLAG_FALSE_VALID(aen)
            READ_WRITE_FLAG_FALSE_VALID(s2)
            READ_WRITE_FLAG_FALSE_VALID(s1)
            READ_WRITE_FLAG_FALSE_VALID(s0)
            READ_WRITE_FLAG_TRUE_VALID(sa)
            READ_WRITE_FLAG_TRUE_VALID(sb)
            READ_WRITE_FLAG_TRUE_VALID(ireq)
            READ_WRITE_FLAG_TRUE_VALID(iack)
            READ_WRITE_FLAG_TRUE_VALID(running_manually)
            READ_WRITE_FLAG_TRUE_VALID(halt)

            // these need to be treated specially
            constexpr uint8_t get_l() const
            {
                return _l;
            }

            constexpr uint8_t get_d() const
            {
                return _d;
            }

            constexpr uint8_t get_r() const
            {
                return _r;
            }

            constexpr uint8_t get_a() const
            {
                return _a;
            }

            constexpr void set_a(uint8_t val)
            {
                _a = val;
                std::tie(_l, _d, _r) = alu.calc(_a, _w);
            }

            constexpr uint8_t get_w() const
            {
                return _w;
            }

            constexpr void set_w(uint8_t val)
            {
                _w = val;
                std::tie(_l, _d, _r) = alu.calc(_a, _w);
            }

#undef READ_WRITE_FLAG
#undef READ_WRITE_FLAG_TRUE_VALID
#undef READ_WRITE_FLAG_FALSE_VALID

        private:
            constexpr void get_control_signal()
            {
                // get control signal if running automatically
                if (get_running_manually())
                    return;

                const std::bitset<24> &microprogram = um.get_data();
                set_s0(microprogram.test(0));
                set_s1(microprogram.test(1));
                set_s2(microprogram.test(2));
                set_aen(microprogram.test(3));
                set_wen(microprogram.test(4));
                set_x0(microprogram.test(5));
                set_x1(microprogram.test(6));
                set_x2(microprogram.test(7));
                alu.set_fen(microprogram.test(8));
                alu.set_cn(microprogram.test(9));
                set_rwr(microprogram.test(10));
                set_rrd(microprogram.test(11));
                set_sten(microprogram.test(12));
                set_outen(microprogram.test(13));
                set_maroe(microprogram.test(14));
                set_maren(microprogram.test(15));
                set_elp(microprogram.test(16));
                set_eint(microprogram.test(17));
                set_iren(microprogram.test(18));
                set_emen(microprogram.test(19));
                set_pcoe(microprogram.test(20));
                set_emrd(microprogram.test(21));
                set_emwr(microprogram.test(22));
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
                if (get_ireq() && !get_iack()) {
                    ibus.set_writer(IBusWriterType::INTERRUPT);
                    clear_emrd();
                    set_iack();
                }

                if (!get_emrd())
                    ibus.set_writer(IBusWriterType::EM);

                if (!get_pcoe())
                    abus.set_writer(ABusWriterType::PC);

                if (!get_emen()) {
                    if (!get_emwr())
                        dbus.add_reader(DBusReaderType::EM);

                    if (!get_emrd())
                        dbus.set_writer(DBusWriterType::EM);
                }

                if (!get_iren()) {
                    ibus.add_reader(IBusReaderType::IR);
                    ibus.add_reader(IBusReaderType::UPC);
                }

                if (!get_eint()) {
                    clear_iack();
                    clear_ireq();
                }

                if (!get_elp())
                    dbus.add_reader(DBusReaderType::PC);

                if (!get_maren())
                    dbus.add_reader(DBusReaderType::MAR);

                if (!get_maroe())
                    abus.set_writer(ABusWriterType::MAR);

                if (!get_outen())
                    dbus.add_reader(DBusReaderType::OUT);

                if (!get_sten())
                    dbus.add_reader(DBusReaderType::ST);

                if (!get_rrd())
                    dbus.set_writer(DBusWriterType::REG);

                if (!get_rwr())
                    dbus.add_reader(DBusReaderType::REG);

                if (!get_wen())
                    dbus.add_reader(DBusReaderType::W);

                if (!get_aen())
                    dbus.add_reader(DBusReaderType::A);
            }

            constexpr void modify_bus_data()
            {
                switch (abus.get_writer()) {
                    case ABusWriterType::NONE:
                        break;

                    case ABusWriterType::MAR:
                        abus.set_data(get_mar());
                        break;

                    case ABusWriterType::PC:
                        abus.set_data(get_pc());
                        // it may be subsequently overwritten by ELP
                        set_pc(get_pc() + 1);
                        break;
                }

                switch (dbus.get_writer()) {
                    case DBusWriterType::NONE:
                        break;

                    case DBusWriterType::IN:
                        dbus.set_data(get_in());
                        break;

                    case DBusWriterType::IA:
                        dbus.set_data(get_ia());
                        break;

                    case DBusWriterType::ST:
                        dbus.set_data(get_st());
                        break;

                    case DBusWriterType::PC:
                        dbus.set_data(get_pc());
                        break;

                    case DBusWriterType::D:
                        dbus.set_data(get_d());
                        break;

                    case DBusWriterType::L:
                        dbus.set_data(get_l());
                        break;

                    case DBusWriterType::R:
                        dbus.set_data(get_r());
                        break;

                    case DBusWriterType::REG:
                        switch ((get_sb() << 1) | get_sa()) {
                            case 0:
                                dbus.set_data(get_r0());
                                break;

                            case 1:
                                dbus.set_data(get_r1());
                                break;

                            case 2:
                                dbus.set_data(get_r2());
                                break;

                            case 3:
                                dbus.set_data(get_r3());
                                break;
                        }

                        break;

                    case DBusWriterType::EM:
                        dbus.set_data(em.get_data());
                        break;

                    case DBusWriterType::MANUAL:
                        dbus.set_data(get_manual_dbus_input());
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
                        case ABusReaderType::NONE:
                            break;

                        case ABusReaderType::EM:
                            em.set_addr(abus.get_data());
                            break;
                    }

                for (DBusReaderType i : dbus.get_reader())
                    switch (i) {
                        case DBusReaderType::NONE:
                            break;

                        case DBusReaderType::MAR:
                            set_mar(dbus.get_data());
                            break;

                        case DBusReaderType::OUT:
                            set_out(dbus.get_data());
                            break;

                        case DBusReaderType::ST:
                            set_st(dbus.get_data());
                            break;

                        case DBusReaderType::PC:
                            set_pc(dbus.get_data());
                            break;

                        case DBusReaderType::A:
                            set_a(dbus.get_data());
                            break;

                        case DBusReaderType::W:
                            set_w(dbus.get_data());
                            break;

                        case DBusReaderType::REG:
                            switch ((get_sb() << 1) | get_sa()) {
                                case 0:
                                    set_r0(dbus.get_data());
                                    break;

                                case 1:
                                    set_r1(dbus.get_data());
                                    break;

                                case 2:
                                    set_r2(dbus.get_data());
                                    break;

                                case 3:
                                    set_r3(dbus.get_data());
                                    break;
                            }

                            break;

                        case DBusReaderType::EM:
                            em.set_data(dbus.get_data());
                            break;
                    }

                bool upc_set = false;

                for (IBusReaderType i : ibus.get_reader())
                    switch (i) {
                        case IBusReaderType::NONE:
                            break;

                        case IBusReaderType::IR:
                            set_ir(ibus.get_data());
                            break;

                        case IBusReaderType::UPC:
                            upc_set = true;
                            set_upc(ibus.get_data());
                            break;
                    }

                if (!upc_set)
                    set_upc(get_upc() + 1);
            }

            Memory em;
            MicroProgramMemory um;
            ALU alu;
            DBus dbus;
            ABus abus;
            IBus ibus;

            uint8_t _manual_dbus_input;
            uint8_t _upc;
            uint8_t _pc;
            uint8_t _mar;
            uint8_t _ia;
            uint8_t _st;
            uint8_t _in;
            uint8_t _out;
            uint8_t _ir;

            uint8_t _l, _d, _r;
            uint8_t _a, _w;
            uint8_t _r0, _r1, _r2, _r3;

            // valid when FALSE
            // errr... don't know its usage
            // bool _xrd : 1;
            bool _emwr : 1;
            bool _emrd : 1;
            bool _pcoe : 1;
            bool _emen : 1;
            bool _iren : 1;
            bool _eint : 1;
            bool _elp : 1;
            bool _maren : 1;
            bool _maroe : 1;
            bool _outen : 1;
            bool _sten : 1;
            bool _rrd : 1;
            bool _rwr : 1;
            // set in ALU
            // bool _cn : 1;
            // bool _fen : 1;
            bool _x2 : 1;
            bool _x1 : 1;
            bool _x0 : 1;
            bool _wen : 1;
            bool _aen : 1;
            bool _s2 : 1;
            bool _s1 : 1;
            bool _s0 : 1;

            // valid when TRUE
            bool _sa : 1;
            bool _sb : 1;
            bool _ireq : 1;
            bool _iack : 1;
            bool _running_manually : 1;
            bool _halt : 1;

    };

}
#endif // COP2K_H_INCLUDED
