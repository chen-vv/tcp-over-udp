# ELEC 331 Programming Assignment 1: TCP

## Overview

This project uses UDP to implement our own version of TCP. Our implementation is able to tolerate packet drops, allow other concurrent connections a fair chance, and is not overly nice to other connections, i.e. does not give up the entire bandwidth to other connections.

## Additional Notes

TODO: specify more info about build commands, testing etc

### Documentation

Doxygen was used to document our code.
To generate new documentation, do the following:

1. Install [Doxygen](https://www.doxygen.nl/).
2. In the command line, navigate to the doxygen directory using `cd /doxygen`.
3. Next, run `doxygen Doxyfile` to generate the documentation.
4. The documentation can be viewed by opening the file `doxygen/html/index.html` in a browser.
