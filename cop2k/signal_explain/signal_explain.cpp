#include <iostream>
#include <cstring>

#include "libopcode_yacc.hpp" // struct Signals
#include "signal_explain.hpp"

int main(int argc, char **argv)
{
    COP2K::SignalExplain signal_explain;

    if (argc > 1 && !strcmp(argv[1], "--help")) {
        std::cerr << "Usage: signal_explain [[!]<signal>]..." << std::endl;
        return EXIT_FAILURE;
    }

    struct Signals signals;
    signals.clear();

    for (int i = 1; i < argc; i++) {
#define SET_SIGNAL(signal_name) \
    if (!strcasecmp(argv[i], #signal_name)) \
        signals.signal_name = true; \
    else if (!strcasecmp(argv[i], "!" #signal_name)) \
        signals.signal_name = false
        SET_SIGNAL(emwr);
        else
            SET_SIGNAL(emrd);

        else
            SET_SIGNAL(pcoe);

        else
            SET_SIGNAL(emen);

        else
            SET_SIGNAL(iren);

        else
            SET_SIGNAL(eint);

        else
            SET_SIGNAL(elp);

        else
            SET_SIGNAL(maren);

        else
            SET_SIGNAL(maroe);

        else
            SET_SIGNAL(outen);

        else
            SET_SIGNAL(sten);

        else
            SET_SIGNAL(rrd);

        else
            SET_SIGNAL(rwr);

        else
            SET_SIGNAL(x2);

        else
            SET_SIGNAL(x1);

        else
            SET_SIGNAL(x0);

        else
            SET_SIGNAL(wen);

        else
            SET_SIGNAL(aen);

        else
            SET_SIGNAL(s2);

        else
            SET_SIGNAL(s1);

        else
            SET_SIGNAL(s0);

        else
            std::cerr << "unknown signal: " << argv[i] << std::endl;
    }

    std::cout << signal_explain.explain(signals) << std::endl;
}
