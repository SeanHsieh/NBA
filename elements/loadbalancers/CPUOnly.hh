#ifndef __NBA_ELEMENT_CPUONLY_HH__
#define __NBA_ELEMENT_CPUONLY_HH__

#include <nba/element/element.hh>
#include <nba/element/annotation.hh>
#include <vector>
#include <string>
#include <rte_errno.h>

namespace nba {

class CPUOnly : public SchedulableElement, PerBatchElement {
public:
    CPUOnly() : SchedulableElement(), PerBatchElement()
    { }

    virtual ~CPUOnly()
    { }

    const char *class_name() const { return "CPUOnly"; }
    const char *port_count() const { return "1/1"; }
    int get_type() const { return SchedulableElement::get_type() | PerBatchElement::get_type(); }

    int initialize() { return 0; }
    int initialize_global() { return 0; }
    int initialize_per_node() { return 0; }

    int configure(comp_thread_context *ctx, std::vector<std::string> &args)
    {
        Element::configure(ctx, args);
        return 0;
    }

    int process_batch(int input_port, PacketBatch *batch)
    {
        anno_set(&batch->banno, NBA_BANNO_LB_DECISION, -1);
        return 0;
    }

    int dispatch(uint64_t loop_count, PacketBatch*& out_batch, uint64_t &next_delay)
    {
        next_delay = 1e6L;
        out_batch = nullptr;
        return 0;
    }

};

EXPORT_ELEMENT(CPUOnly);

}

#endif

// vim: ts=8 sts=4 sw=4 et
