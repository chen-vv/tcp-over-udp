import os
import socket
import subprocess
import time

# import pytest

NUM_INSTANCES = 2
CONVERGENCE_THRESHOLD = 0.1  # 10% threshold for fairness
MAX_RTT_COUNT = 100


def start_instances():
    receiver_procs = []
    sender_procs = []

    for i in range(NUM_INSTANCES):
        udp_port = 5000 + i
        filename_to_write = f"received_file_{i}.txt"
        receiver_proc = subprocess.Popen(["../../receiver", str(udp_port), filename_to_write])
        receiver_procs.append(receiver_proc)

        receiver_hostname = "localhost"
        receiver_port = 5000 + i
        filename = "send1.txt"
        bytes_to_transfer = os.path.getsize("send1.txt")
        sender_proc = subprocess.Popen(
            [
                "../../sender",
                receiver_hostname,
                str(receiver_port),
                filename,
                str(bytes_to_transfer),
            ]
        )
        sender_procs.append(sender_proc)

    return sender_procs, receiver_procs


def measure_throughput(udp_port):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(("localhost", udp_port))
    total_bytes_received = 0
    start_time = time.time()
    rtt_count = 0

    while True:
        data, _ = s.recvfrom(1024)
        total_bytes_received += len(data)
        rtt_count += 1

        if rtt_count >= MAX_RTT_COUNT:
            break

    end_time = time.time()
    elapsed_time = end_time - start_time
    throughput = total_bytes_received / elapsed_time
    return throughput


def test_fairness():
    sender_procs, receiver_procs = start_instances()

    time.sleep(0.5)

    throughputs = []
    for i in range(NUM_INSTANCES):
        udp_port = 5000 + i
        throughput = measure_throughput(udp_port)
        throughputs.append(throughput)

    fairness_ratio = max(throughputs) / min(throughputs)
    print(fairness_ratio)

    assert fairness_ratio <= 1 + CONVERGENCE_THRESHOLD
    assert fairness_ratio >= 1 - CONVERGENCE_THRESHOLD

    for sender_proc in sender_procs:
        sender_proc.terminate()
    for receiver_proc in receiver_procs:
        receiver_proc.terminate()


if __name__ == "__main__":
    test_fairness()
    # pytest.main(["-v"])
