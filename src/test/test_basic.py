import subprocess

import pytest


@pytest.fixture(autouse=True)
def clear_received_file():
    """Clear received.txt before each test"""
    with open("received.txt", "w"):
        pass


@pytest.mark.parametrize("file_size", [30, 36])
def test_udp_file_transfer(file_size):
    with open("received.txt", "r") as received_file:
        assert received_file.read() == ""

    receiver_process = subprocess.Popen(["../../receiver", "12345", "received.txt"])

    sender_process = subprocess.Popen(
        ["../../sender", "localhost", "12345", "send.txt", str(file_size)]
    )

    sender_process.wait()

    try:
        receiver_process.wait(timeout=1)
    except subprocess.TimeoutExpired:
        receiver_process.kill()
        receiver_process.wait()

    with open("received.txt", "r") as received_file:
        received_data = received_file.read()

    with open("send.txt", "r") as send_file:
        send_data = send_file.read()

    assert len(send_data) == len(received_data)
    assert send_data == received_data


def test_udp_max_timeout():
    sender_process = subprocess.Popen(
        ["../../sender", "localhost", "12345", "send.txt", "123"]
    )

    sender_process.wait()
    exit_code = sender_process.returncode

    try:
        max_retry_timeout = 1.5
        sender_process.wait(timeout=max_retry_timeout)
    except subprocess.TimeoutExpired:
        sender_process.kill()
        sender_process.wait()

    assert exit_code == 1

if __name__ == "__main__":
    pytest.main(["-v"])
