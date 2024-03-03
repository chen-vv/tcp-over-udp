# ELEC 331 Programming Assignment 1: TCP

## Overview

This project uses UDP (User Datagram Protocol) to implement our own version of TCP (Transmission Control Protocol). Our implementation is able to:

- tolerate packet drops
- allow other concurrent connections a fair chance
- ensure that the entire bandwidth is not given up to other connections

In order to account for unreliable network connectivity, we first establish a reliable connection between the sender (`sender.c`) and the receiver (`receiver.c`) through a 3-way handshake process. The sender first sends a SYN, waits for a SYN-ACK from the receiver, then sends a final ACK before sending data packets over the network.

## Installing

Follow these steps to run the program:

1. Install g++ and run it on Ubuntu or macOS.
2. (optional) If you have built the binaries before, run `make clean` to clean the executable files.
3. In the terminal, run `make`.
4. To start the sender, run `./sender receiver_hostname receiver_port filename_to_xfer bytes_to_xfer`
5. To start the receiver, run `./receiver UDP_port filename_to_write [writeRate]`

## Testing

We used [Pytest](https://docs.pytest.org/en/8.0.x/), a Python testing framework, to test our code. These test files can be found in the `src/test` directory. To run the test suite, first ensure that you have Pytest installed, then do the following:

1. In the command line, navigate to the test directory using `cd src/test`.
2. Run `pytest test_transfer.py` to execute the test suite.
3. The results will be displayed on the console.

### Troubleshooting

**Q: FileNotFoundError: [Errno 2] No such file or directory: '../../receiver': '../../receiver'**

A: Make sure to compile the C code and enter `cd src/test` before running the test suite.

<br/>

**Q: Captured stderr call - bind: Address already in use**

A: This means that the port needed for testing is currently being used by another process. You will need to stop the process which is using that port to let the tests run properly. In a Linux environment, use:

- `netstat -tulpn` to display the processes
- `kill <pid>` to terminate the process

## Documentation

Doxygen was used to document our code.
To generate new documentation, do the following:

1. Install [Doxygen](https://www.doxygen.nl/).
2. In the command line, navigate to the doxygen directory using `cd doxygen`.
3. Next, run `doxygen Doxyfile` to generate the documentation.
4. The documentation can be viewed by opening the file `doxygen/html/index.html` in a browser.
