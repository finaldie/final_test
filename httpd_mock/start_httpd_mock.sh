#!/bin/sh

ulimit -c unlimited;
ulimit -n 65535;
ulimit -a;
sysctl -w net.ipv4.ip_local_port_range="18000 61000";
sysctl -w net.ipv4.tcp_tw_reuse=1;

./httpd_mock -c default_httpd_mock.cfg
