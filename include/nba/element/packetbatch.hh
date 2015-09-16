#ifndef __NBA_PACKETBATCH_HH__
#define __NBA_PACKETBATCH_HH__
#include <nba/core/intrinsic.hh>
#include <nba/framework/config.hh>
#include <nba/framework/datablock.hh>
#include <nba/element/annotation.hh>
#include <cstdint>
#include <cstring>
#include <vector>
#include <rte_config.h>
#include <rte_memory.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

extern "C" {
struct rte_ring;
}

namespace nba {

class Element;

enum BatchDisposition {
    KEPT_BY_ELEMENT = -1,
    CONTINUE_TO_PROCESS = 0,
};

class PacketBatch {
public:
    PacketBatch()
        : count(0), drop_count(0), datablock_states(nullptr), recv_timestamp(0),
          generation(0), batch_id(0), element(nullptr), input_port(0), has_results(false),
          has_dropped(false), delay_start(0), compute_time(0)
    {
        #ifdef DEBUG
        memset(&results[0], 0xdd, sizeof(int) * NBA_MAX_COMP_BATCH_SIZE);
        memset(&excluded[0], 0xcc, sizeof(bool) * NBA_MAX_COMP_BATCH_SIZE);
        memset(&packets[0], 0xbb, sizeof(struct rte_mbuf*) * NBA_MAX_COMP_BATCH_SIZE);
        #endif
    }

    virtual ~PacketBatch()
    {
    }

    /**
     * Moves excluded packets to the end of batches, by swapping them
     * with the tail packets, to reduce branching overheads when iterating
     * over the packet batch in many places.
     * (We assume that this "in-batch" reordering does not incur performance
     * overheads for transport layers.)
     * It stores the number of dropped packets to drop_count member
     * variable.  Later, ElementGraph refer this value to actually free
     * the excluded packets.
     *
     * This should only be called right after doing Element::_process_batch()
     * or moving packets to other batches in ElementGraph.
     * This may be called multiple times until reaching the next element.
     */
    void collect_excluded_packets();

    /**
     * Moves the collected excluded packets at the tail to drop_queue,
     * and resets drop_count to zero.
     */
    void clean_drops(struct rte_ring *drop_queue);

    unsigned count;
    unsigned drop_count;
    struct datablock_tracker *datablock_states;
    uint64_t recv_timestamp;
    uint64_t generation;
    uint64_t batch_id;
    Element* element;
    int input_port;
    bool has_results;
    bool has_dropped;
    uint64_t delay_start;
    uint64_t delay_time;
    double compute_time;

    struct annotation_set banno __rte_cache_aligned;  /** Batch-level annotations. */
    bool excluded[NBA_MAX_COMP_BATCH_SIZE] __rte_cache_aligned;
    struct rte_mbuf *packets[NBA_MAX_COMP_BATCH_SIZE] __rte_cache_aligned;
    int results[NBA_MAX_COMP_BATCH_SIZE] __rte_cache_aligned;
};

}

#endif

// vim: ts=8 sts=4 sw=4 et
