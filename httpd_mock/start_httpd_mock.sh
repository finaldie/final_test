#!/bin/sh

ulimit -c unlimited;
ulimit -n 65535;
ulimit -a;

./httpd_mock -c default_httpd_mock.cfg
