#ifndef __NBA_ELEMENT_TOOUTPUT_HH__
#define __NBA_ELEMENT_TOOUTPUT_HH__

#include <nba/element/element.hh>
#include <vector>
#include <string>

namespace nba {

class ToOutput : public Element {
    /**
     * ToOutput element is a NULL element, which does nothing.
     * It even does not have any output.
     * It serves as a placeholder in configuration to indicate
     * connections to the TX Common Component.
     */
public:
    ToOutput(): Element()
    {
    }

    ~ToOutput()
    {
    }

    const char *class_name() const { return "ToOutput"; }
    const char *port_count() const { return "1/0"; }
    virtual int get_type() const { return ELEMTYPE_OUTPUT; }

    int initialize() { return 0; };
    int initialize_global() { return 0; };
    int initialize_per_node() { return 0; };

    int configure(comp_thread_context *ctx, std::vector<std::string> &args)
    {
        Element::configure(ctx, args);
        return 0;
    }

    int process(int input_port, Packet *pkt)
    {
        assert(0);  // This is a special element that is not executed.
        return 0;
    }

};

EXPORT_ELEMENT(ToOutput);

}

#endif

// vim: ts=8 sts=4 sw=4 et
