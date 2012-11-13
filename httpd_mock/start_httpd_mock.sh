#!/bin/sh

ulimit -c unlimited -n 65535;
ulimit -a;

./httpd_mock
