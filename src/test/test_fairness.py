import os
import subprocess
import time

# import pytest

CONVERGENCE_THRESHOLD = 0.1  # 10% threshold for fairness
MAX_RTT_COUNT = 100
TEST_FILENAME = "output.txt"
TRANSFER_BYTES = os.path.getsize(TEST_FILENAME)
HOSTNAME = "localhost"


def start_and_wait_processes():
    receiver1 = subprocess.Popen(["../../receiver", str(12345), "received1.txt", str(0)])
    sender1 = subprocess.Popen(
        [
            "../../sender",
            HOSTNAME,
            str(12345),
            TEST_FILENAME,
            str(TRANSFER_BYTES),
        ]
    )

    time.sleep(0.001)

    receiver2 = subprocess.Popen(["../../receiver", str(12346), "received2.txt", str(0)])

    sender2 = subprocess.Popen(
        [
            "../../sender",
            HOSTNAME,
            str(12346),
            TEST_FILENAME,
            str(TRANSFER_BYTES),
        ]
    )

    receiver1.wait()
    receiver2.wait()
    sender1.wait()
    sender2.wait()


def test_fairness():
    start = time.time()
    start_and_wait_processes()
    end = time.time()

    throughput1 = TRANSFER_BYTES / (end - start)
    throughput2 = TRANSFER_BYTES / (end - start)
    throughputs = (throughput1, throughput2)

    fairness_ratio = max(throughputs) / min(throughputs)
    print(fairness_ratio)

    assert fairness_ratio <= 1 + CONVERGENCE_THRESHOLD
    assert fairness_ratio >= 1 - CONVERGENCE_THRESHOLD


if __name__ == "__main__":
    test_fairness()
    # pytest.main(["-v"])
