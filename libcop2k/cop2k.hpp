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

namespace COP2K
{
    class BusConflict : public std::logic_error
    {
        public:
            BusConflict() : std::logic_error("this bus already has a writer") {}
    };

    class BusNoWriter : public std::logic_error
    {
        public:
            BusNoWriter() : std::logic_error("this bus has no writer") {}
    };

    class Register
    {
        public:
            constexpr uint8_t get() const
            {
                return data;
            }

            constexpr void set(uint8_t val)
            {
                data = val;
            }

        private:
            uint8_t data;
    };

    class RegisterWithCallback
    {
        public:
            RegisterWithCallback() : data(), callback([](RegisterWithCallback &) {}) {}

            constexpr uint8_t get() const
            {
                return data;
            }

            void set(uint8_t val)
            {
                data = val;
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

        private:
            uint8_t data;
            std::function<void(RegisterWithCallback &)> callback;
    };

    class Flag
    {
        public:
            constexpr bool get() const
            {
                return data;
            }

            constexpr void pos()
            {
                data = true;
            }

            constexpr void neg()
            {
                data = false;
            }

            constexpr void set(bool val)
            {
                data = val;
            }

        private:
            bool data : 1;
    };

    class FlagWithCallback
    {
        public:
            FlagWithCallback() : callback([](FlagWithCallback &) {}), data() {}

            constexpr bool get() const
            {
                return data;
            }

            void pos()
            {
                data = true;
                callback(*this);
            }

            void neg()
            {
                data = false;
                callback(*this);
            }

            void set(bool val)
            {
                data = val;
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

        private:
            std::function<void(FlagWithCallback &)> callback;
            bool data : 1;

    };

    class ALU
    {
        public:
            // DO NOT MODIFY
            // THIS IS DEFINED BY THE MACHINE
            enum class CalcTypes : uint8_t {
                ADD,
                SUB,
                AND,
                OR,
                CARRY_ADD,
                CARRY_SUB,
                NOT,
                DIRECT_A
            };

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

            FlagWithCallback cy;
            FlagWithCallback cn;
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

            // bypass normal addr lookup mode
            constexpr void set_data_at(uint8_t actaddr, uint8_t val)
            {
                mem.at(actaddr) = val;
            }

            constexpr uint8_t get_data_at(uint8_t actaddr) const
            {
                return mem.at(actaddr);
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

        private:
            std::array<std::bitset<24>, 256> mem;
            uint8_t addr;
    };

    class COP2K
    {
        public:
            COP2K()
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
                alu.cy.set_callback([this](FlagWithCallback &) {
                    update_alu();
                });
                alu.cn.set_callback([this](FlagWithCallback &) {
                    update_alu();
                });
                manual_dbus_input.set(0);
                upc.set(0);
                pc.set(0);
                mar.set(0);
                ia.set(0xE0);
                st.set(0);
                in.set(0);
                out.set(0);
                ir.set(0);
                r0.set(0);
                r1.set(0);
                r2.set(0);
                r3.set(0);
                emwr.pos();
                emrd.pos();
                pcoe.pos();
                emen.pos();
                iren.pos();
                eint.pos();
                elp.pos();
                maren.pos();
                maroe.pos();
                outen.pos();
                sten.pos();
                rrd.pos();
                rwr.pos();
                x2.pos();
                x1.pos();
                x0.pos();
                wen.pos();
                aen.pos();
                s2.pos();
                s1.pos();
                s0.pos();
                sa.neg();
                sb.neg();
                pos_fen();
                pos_cn();
                ireq.neg();
                iack.neg();
                manual_dbus.pos();
                running_manually.pos();
                halt.pos();
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

            constexpr void trigger_interrupt()
            {
                ireq.pos();
            }

            constexpr void set_dbus_manual_input(uint8_t val)
            {
                manual_dbus_input.set(val);
            }

            constexpr void pos_fen()
            {
                alu.fen.pos();
            }

            constexpr void pos_cn()
            {
                alu.cn.pos();
            }

            constexpr void neg_fen()
            {
                alu.fen.neg();
            }

            constexpr void neg_cn()
            {
                alu.cn.neg();
            }

            constexpr void set_fen(bool val)
            {
                alu.fen.set(val);
            }

            constexpr void set_cn(bool val)
            {
                alu.cn.set(val);
            }

            constexpr bool get_fen() const
            {
                return alu.fen.get();
            }

            constexpr bool get_cn() const
            {
                return alu.cn.get();
            }

            Memory em;
            Register l, d, r;
            Register r0, r1, r2, r3;
            MicroProgramMemory um;

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
                            mar.set(dbus.get_data());
                            break;

                        case DBusReaderType::OUT:
                            out.set(dbus.get_data());
                            break;

                        case DBusReaderType::ST:
                            st.set(dbus.get_data());
                            break;

                        case DBusReaderType::PC:
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

                for (IBusReaderType i : ibus.get_reader())
                    switch (i) {
                        case IBusReaderType::NONE:
                            upc.set(upc.get() + 1);
                            break;

                        case IBusReaderType::IR:
                            ir.set(ibus.get_data());
                            sa.set(ibus.get_data() & (1 << 0));
                            sb.set(ibus.get_data() & (1 << 1));
                            break;

                        case IBusReaderType::UPC:
                            um.set_addr(ibus.get_data() & ~0x3);
                            break;
                    }
            }

            ALU alu;
            DBus dbus;
            ABus abus;
            IBus ibus;
    };

}
#endif // COP2K_H_INCLUDED
