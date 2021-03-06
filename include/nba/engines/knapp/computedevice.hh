#ifndef __KNAPP_ENGINE_HH__
#define __KNAPP_ENGINE_HH__

#include <string>
#include <vector>
#include <unordered_map>
#include <deque>

#include <nba/framework/computedevice.hh>
#include <nba/framework/computecontext.hh>
#include <nba/core/threading.hh>
#include <nba/engines/knapp/defs.hh>
#include <scif.h>

namespace nba {

namespace knapp {

class RMABuffer;

} // endns(knapp)

class KnappComputeContext;

class KnappComputeDevice: public ComputeDevice
{
public:
    friend class KnappComputeContext;

    KnappComputeDevice(unsigned node_id, unsigned device_id, size_t num_contexts);
    virtual ~KnappComputeDevice();

    int get_spec(struct compute_device_spec *spec);
    int get_utilization(struct compute_device_util *util);
    host_mem_t alloc_host_buffer(size_t size, int flags);
    dev_mem_t alloc_device_buffer(size_t size, int flags, host_mem_t &assoc_host_buf);
    void free_host_buffer(host_mem_t hbuf);
    void free_device_buffer(dev_mem_t dbuf);
    void *unwrap_host_buffer(const host_mem_t hbuf);
    void *unwrap_device_buffer(const dev_mem_t dbuf);
    void memwrite(host_mem_t hbuf, dev_mem_t duf, size_t offset, size_t size);
    void memread(host_mem_t hbuf, dev_mem_t duf, size_t offset, size_t size);

private:
    ComputeContext *_get_available_context();
    void _return_context(ComputeContext *ctx);

    uint32_t find_new_buffer_id();

    scif_epd_t ctrl_epd;

    std::deque<KnappComputeContext *> _ready_contexts;
    std::deque<KnappComputeContext *> _active_contexts;
    std::unordered_map<uint32_t, knapp::RMABuffer *> buffer_registry;
    Lock _lock;
    CondVar _ready_cond;
};

} //endns(nba)

#endif

// vim: ts=8 sts=4 sw=4 et
