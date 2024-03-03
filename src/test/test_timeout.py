import subprocess

import pytest


@pytest.mark.parametrize("send_filename", ["sample.txt", "hotpot.jpg", "quacks.mp3", "ducks.mp4"])
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
