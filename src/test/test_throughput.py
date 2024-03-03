import socket
import subprocess
import os
import pytest
import time

# Source: https://chat.openai.com/share/0429d84c-bffc-41ac-849e-c13e07669cae

# UDP port numbers for testing
# SENDER_PORT = 12345
# RECEIVER_PORT = 12346
TCP_PORT = 12347

# Test file information
TEST_FILE = "sample.txt"
TEST_FILE_SIZE = 1024
TRANSFERRED_FILE = "received_file.txt"

# Test timeout (in seconds)
TEST_TIMEOUT = 10


# TCP server function
def tcp_server():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.bind(('localhost', TCP_PORT))
        server_socket.listen(1)
        conn, _ = server_socket.accept()
        with conn:
            with open(TEST_FILE, 'rb') as file:
                conn.sendfile(file)

# Test cases
@pytest.mark.parametrize("send_filename", ["sample.txt"])
def test_udp_vs_tcp(send_filename):
    # Run TCP server in a subprocess
    tcp_server_process = subprocess.Popen(tcp_server)

    # Run the TCP client in a subprocess
    tcp_client_process = subprocess.Popen(['nc', '-w', '5', 'localhost', str(TCP_PORT), '>', TRANSFERRED_FILE])
    tcp_client_process.wait()

    # Run the receiver in a subprocess
    receiver_process = subprocess.Popen(["../../receiver", "12345", "received.txt", "0"])
    time.sleep(1)  # Wait for the receiver to start

    # Run the sender in a subprocess
    sender_process = subprocess.Popen(
        ["../../sender", "localhost", "12345", send_filename, str(os.path.getsize(send_filename))]
    )
    sender_process.wait()  # Wait for sender to complete

    # Wait for the receiver to finish
    receiver_process.wait(timeout=TEST_TIMEOUT)

    # Check if the received file exists
    assert os.path.exists(TRANSFERRED_FILE)

    # Check if the received file size matches the original file size
    received_file_size = os.path.getsize(TRANSFERRED_FILE)
    assert received_file_size == TEST_FILE_SIZE

    # Check if the content of the received file matches the content of the original file
    with open(TEST_FILE, "rb") as original_file, open(TRANSFERRED_FILE, "rb") as received_file:
        original_content = original_file.read()
        received_content = received_file.read()
        assert original_content == received_content

    # Clean up
    os.remove(TEST_FILE)
    os.remove(TRANSFERRED_FILE)

    # Terminate TCP server process
    tcp_server_process.terminate()

# Run the tests
if __name__ == "__main__":
    pytest.main([__file__])
