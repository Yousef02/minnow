#!/bin/bash
echo "Running send_connect"
./build/tests/send_connect
echo "Running send_transmit"
./build/tests/send_transmit
echo "Running send_retx"
./build/tests/send_retx
echo "Running send_window"
./build/tests/send_window
echo "Running send_ack"
./build/tests/send_ack
echo "Running send_close"
./build/tests/send_close
echo "Running send_extra"
./build/tests/send_extra