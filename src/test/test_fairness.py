import os
import queue
import subprocess
import threading
import time

import pytest

CONVERGENCE_THRESHOLD = 0.1  # 10% threshold for fairness
MAX_RTT_COUNT = 100
TEST_FILENAME = "output.txt"
TRANSFER_BYTES = os.path.getsize(TEST_FILENAME)
HOSTNAME = "localhost"


def start1(result_queue):
    start = time.time()
    receiver = subprocess.Popen(["../../receiver", str(12345), "received1.txt", str(0)])
    sender = subprocess.Popen(
        [
            "../../sender",
            HOSTNAME,
            str(12345),
            TEST_FILENAME,
            str(TRANSFER_BYTES),
        ]
    )
    receiver.wait()
    sender.wait()
    end = time.time()
    result_queue.put(end - start)


def start2(result_queue):
    start = time.time()
    receiver = subprocess.Popen(["../../receiver", str(12346), "received2.txt", str(0)])
    sender = subprocess.Popen(
        [
            "../../sender",
            HOSTNAME,
            str(12346),
            TEST_FILENAME,
            str(TRANSFER_BYTES),
        ]
    )
    receiver.wait()
    sender.wait()
    end = time.time()
    result_queue.put(end - start)


def test_fairness():
    for _ in range(5):
        result_queue = queue.Queue()

        thread1 = threading.Thread(target=start1, args=(result_queue,))
        thread2 = threading.Thread(target=start2, args=(result_queue,))

        thread1.start()
        thread2.start()

        thread1.join()
        thread2.join()

        time1 = result_queue.get()
        time2 = result_queue.get()

        throughput1 = TRANSFER_BYTES / time1
        throughput2 = TRANSFER_BYTES / time2
        throughputs = (throughput1, throughput2)

        fairness_ratio = max(throughputs) / min(throughputs)
        print(fairness_ratio)

        assert fairness_ratio <= 1 + CONVERGENCE_THRESHOLD
        assert fairness_ratio >= 1 - CONVERGENCE_THRESHOLD


if __name__ == "__main__":
    pytest.main(["-v"])
