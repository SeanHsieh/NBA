#! /usr/bin/env python3

# 이 설정은 PCIe bus contention이 없는 상태에서도 NIC/GPU 동시 사용이 성능
# 저하 일으키는지 확인하기 위한 것으로, 연산과 GPU는 node 0의 것을 사용하되
# 네트워크 카드는 모두 node 1의 것만을 사용.
# CPU-only로 돌렸을 때와 GPU-only로 돌렸을 때 상황 비교.

import nba, os
import sys

for netdev in nba.get_netdevices():
    print(netdev)
for coproc in nba.get_coprocessors():
    print(coproc)
node_cpus = nba.get_cpu_node_mapping()
for node_id, cpus in enumerate(node_cpus):
    print('Cores in NUMA node {0}: [{1}]'.format(node_id, ', '.join(map(str, cpus))))

# The values read by the framework are:
# - system_params
# - io_threads
# - comp_threads
# - coproc_threads
# - queues
# - thread_connections

# 64, 64, 192 is optimal parameter for ipv4-router

system_params = {
    'IO_BATCH_SIZE': int(os.environ.get('NBA_IO_BATCH_SIZE', 64)),
    'COMP_BATCH_SIZE': int(os.environ.get('NBA_COMP_BATCH_SIZE', 64)),
    'COPROC_PPDEPTH': int(os.environ.get('NBA_COPROC_PPDEPTH', 32)),
    'COPROC_CTX_PER_COMPTHREAD': 1,
}
print("# logical cores: {0}, # physical cores {1} (hyperthreading {2})".format(
    nba.num_logical_cores, nba.num_physical_cores,
    "enabled" if nba.ht_enabled else "disabled"
))
_ht_diff = nba.num_physical_cores if nba.ht_enabled else 0

io_threads = [
    # core_id, list of (port_id, rxq_idx)
    nba.IOThread(core_id=node_cpus[0][0], attached_rxqs=[(0, 0), (1, 0), (2, 0), (3, 0)], mode='normal'),
    nba.IOThread(core_id=node_cpus[0][1], attached_rxqs=[(0, 1), (1, 1), (2, 1), (3, 1)], mode='normal'),
    nba.IOThread(core_id=node_cpus[0][2], attached_rxqs=[(0, 2), (1, 2), (2, 2), (3, 2)], mode='normal'),
    nba.IOThread(core_id=node_cpus[0][3], attached_rxqs=[(0, 3), (1, 3), (2, 3), (3, 3)], mode='normal'),
    nba.IOThread(core_id=node_cpus[0][4], attached_rxqs=[(0, 4), (1, 4), (2, 4), (3, 4)], mode='normal'),
    nba.IOThread(core_id=node_cpus[0][5], attached_rxqs=[(0, 5), (1, 5), (2, 5), (3, 5)], mode='normal'),
    nba.IOThread(core_id=node_cpus[0][6], attached_rxqs=[(0, 6), (1, 6), (2, 6), (3, 6)], mode='normal'),
]
comp_threads = [
    # core_id
    nba.CompThread(core_id=node_cpus[0][0] + _ht_diff),
    nba.CompThread(core_id=node_cpus[0][1] + _ht_diff),
    nba.CompThread(core_id=node_cpus[0][2] + _ht_diff),
    nba.CompThread(core_id=node_cpus[0][3] + _ht_diff),
    nba.CompThread(core_id=node_cpus[0][4] + _ht_diff),
    nba.CompThread(core_id=node_cpus[0][5] + _ht_diff),
    nba.CompThread(core_id=node_cpus[0][6] + _ht_diff),
]

coproc_threads = [
    # core_id, device_id
    nba.CoprocThread(core_id=node_cpus[0][7] + _ht_diff, device_id=0),
]

comp_input_queues = [
    # node_id, template
    nba.Queue(node_id=0, template='swrx'),
    nba.Queue(node_id=0, template='swrx'),
    nba.Queue(node_id=0, template='swrx'),
    nba.Queue(node_id=0, template='swrx'),
    nba.Queue(node_id=0, template='swrx'),
    nba.Queue(node_id=0, template='swrx'),
    nba.Queue(node_id=0, template='swrx'),
]

coproc_input_queues = [
    # node_id, template
    nba.Queue(node_id=0, template='taskin'),
]

coproc_completion_queues = [
    # node_id, template
    nba.Queue(node_id=0, template='taskout'),
    nba.Queue(node_id=0, template='taskout'),
    nba.Queue(node_id=0, template='taskout'),
    nba.Queue(node_id=0, template='taskout'),
    nba.Queue(node_id=0, template='taskout'),
    nba.Queue(node_id=0, template='taskout'),
    nba.Queue(node_id=0, template='taskout'),
]

queues = comp_input_queues + coproc_input_queues + coproc_completion_queues

thread_connections = [
    # from-thread, to-thread, queue-instance
    (io_threads[0], comp_threads[0], comp_input_queues[0]),
    (io_threads[1], comp_threads[1], comp_input_queues[1]),
    (io_threads[2], comp_threads[2], comp_input_queues[2]),
    (io_threads[3], comp_threads[3], comp_input_queues[3]),
    (io_threads[4], comp_threads[4], comp_input_queues[4]),
    (io_threads[5], comp_threads[5], comp_input_queues[5]),
    (io_threads[6], comp_threads[6], comp_input_queues[6]),
    (comp_threads[0], coproc_threads[0], coproc_input_queues[0]),
    (comp_threads[1], coproc_threads[0], coproc_input_queues[0]),
    (comp_threads[2], coproc_threads[0], coproc_input_queues[0]),
    (comp_threads[3], coproc_threads[0], coproc_input_queues[0]),
    (comp_threads[4], coproc_threads[0], coproc_input_queues[0]),
    (comp_threads[5], coproc_threads[0], coproc_input_queues[0]),
    (comp_threads[6], coproc_threads[0], coproc_input_queues[0]),
    (coproc_threads[0], comp_threads[0], coproc_completion_queues[0]),
    (coproc_threads[0], comp_threads[1], coproc_completion_queues[1]),
    (coproc_threads[0], comp_threads[2], coproc_completion_queues[2]),
    (coproc_threads[0], comp_threads[3], coproc_completion_queues[3]),
    (coproc_threads[0], comp_threads[4], coproc_completion_queues[4]),
    (coproc_threads[0], comp_threads[5], coproc_completion_queues[5]),
    (coproc_threads[0], comp_threads[6], coproc_completion_queues[6]),
]
