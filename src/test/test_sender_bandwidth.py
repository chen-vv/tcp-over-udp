"""Run this script to measure the bandwidth usage of the sender."""

import os
import subprocess
import time


def measure_bandwidth():
    start = time.time()

    sender_process = subprocess.Popen(
        [
            "./sender",
            "europa.eomielan-194296.elec331sp2024.emulab.net",
            "12345",
            "src/test/output.txt",
        ]
    )
    sender_process.wait()

    end = time.time()

    duration = end - start
    bandwidth_in_bytes = os.path.getsize("src/test/output.txt")
    bandwidth_in_bits = bandwidth_in_bytes * 8
    bandwidth_in_mbps = (bandwidth_in_bits / duration) / (1024 * 1024)

    return bandwidth_in_mbps, duration


if __name__ == "__main__":
    bandwidth, duration = measure_bandwidth()
    print("Bandwidth usage over {:.10f} seconds: {:.2f} Mbps".format(duration, bandwidth))
