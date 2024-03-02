import os
import subprocess
import time

sender_command = "../../sender localhost 12345 send2.txt 1000"
receiver_command = "../../receiver 12345 received.txt"


def run_command(command):
    process = subprocess.Popen(command.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, _ = process.communicate()
    return out.decode().strip()


def measure_bandwidth():
    receiver_process = subprocess.Popen(receiver_command.split())

    time.sleep(1)

    sender_start_time = time.time()
    sender_process = subprocess.Popen(sender_command.split())

    sender_process.wait()
    sender_end_time = time.time()

    time.sleep(1)

    receiver_process.terminate()

    sender_duration = sender_end_time - sender_start_time
    bandwidth_in_bytes = os.path.getsize("send2.txt")
    bandwidth_in_bits = bandwidth_in_bytes * 8
    bandwidth_in_mbps = (bandwidth_in_bits / sender_duration) / (1024 * 1024)

    return bandwidth_in_mbps, sender_duration


while True:
    bandwidth, duration = measure_bandwidth()

    print("Bandwidth usage over {:.10f} seconds: {:.2f} Mbps".format(duration, bandwidth))
