import subprocess
import os
import pytest


@pytest.fixture(autouse=True)
def clear_received_file():
    """Clear received.txt before each test"""
    with open("received.txt", "w"):
        pass


@pytest.mark.parametrize("send_filename", ["send1.txt", "send2.txt"])
def test_udp_file_transfer(send_filename):
    with open("received.txt", "r") as received_file:
        assert received_file.read() == ""

    with open(send_filename, "r") as send_file:
        send_data = send_file.read()

    receiver_process = subprocess.Popen(["../../receiver", "12345", "received.txt", "0"])

    sender_process = subprocess.Popen(
        ["../../sender", "localhost", "12345", send_filename, str(os.path.getsize(send_filename))]
    )

    sender_process.wait()
    receiver_process.wait()

    with open("received.txt", "r") as received_file:
        received_data = received_file.read()

    assert len(send_data) == len(received_data)
    assert send_data == received_data


@pytest.mark.parametrize("send_filename", ["send1.txt", "send2.txt"])
def test_udp_max_timeout(send_filename):
    sender_process = subprocess.Popen(["../../sender", "localhost", "12345", send_filename, "123"])

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
