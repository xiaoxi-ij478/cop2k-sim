#ifndef LIBOPCODE_YACC_HPP_INCLUDED
#define LIBOPCODE_YACC_HPP_INCLUDED

struct Signals {
    bool s0, s1, s2, aen, wen, x0, x1, x2, fen, cn, rwr, rrd, sten, outen,
         maroe, maren, elp, eint, iren, emen, pcoe, emrd, emwr;

    bool empty()
    {
        return s0 && s1 && s2 && aen && wen && x0 && x1 && x2 && fen && cn && rwr && rrd && sten && outen && maroe && maren && elp && eint && iren && emen && pcoe && emrd && emwr;
    }

    void clear()
    {
        s0 = s1 = s2 = aen = wen = x0 = x1 = x2 = fen = cn = rwr = rrd = sten =
        outen = maroe = maren = elp = eint = iren = emen = pcoe = emrd = emwr = true;
    }

    std::bitset<24> to_bitset()
    {
        return std::bitset<24>(
            (s0    << 0)  |
            (s1    << 1)  |
            (s2    << 2)  |
            (aen   << 3)  |
            (wen   << 4)  |
            (x0    << 5)  |
            (x1    << 6)  |
            (x2    << 7)  |
            (fen   << 8)  |
            (cn    << 9)  |
            (rwr   << 10) |
            (rrd   << 11) |
            (sten  << 12) |
            (outen << 13) |
            (maroe << 14) |
            (maren << 15) |
            (elp   << 16) |
            (eint  << 17) |
            (iren  << 18) |
            (emen  << 19) |
            (pcoe  << 20) |
            (emrd  << 21) |
            (emwr  << 22) |
            (0     << 23)
        );
    }
};

struct MicroProgram {
    unsigned char signal_count;
    struct Signals signals[4];

    void clear()
    {
        signal_count = 0;

        for (struct Signals &i : signals)
            i.clear();
    }
};

#endif // LIBOPCODE_YACC_HPP_INCLUDED
