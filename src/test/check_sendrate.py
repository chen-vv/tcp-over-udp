"""
Run this script to start and measure the data sendrate of the sender. Should be run on cloudlab
within the juno vm, and the hostname of the receiver should be updated."
"""

import os
import subprocess
import time

SEND_FILENAME = "quacks.mp3"
RECEIVER_HOSTNAME = "europa.eomielan-194296.elec331sp2024.emulab.net"


def measure_bandwidth():
    start = time.time()

    sender_process = subprocess.Popen(
        [
            "../../sender",
            RECEIVER_HOSTNAME,
            "12345",
            SEND_FILENAME,
            str(os.path.getsize(SEND_FILENAME)),
        ]
    )
    sender_process.wait()

    end = time.time()

    duration = end - start
    bandwidth_in_bytes = os.path.getsize(SEND_FILENAME)
    bandwidth_in_bits = bandwidth_in_bytes * 8
    bandwidth_in_mbps = (bandwidth_in_bits / duration) / (1024 * 1024)

    return bandwidth_in_mbps, duration


if __name__ == "__main__":
    bandwidth, duration = measure_bandwidth()
    print("Sendrate usage over {:.10f} seconds: {:.2f} Mbps".format(duration, bandwidth))
