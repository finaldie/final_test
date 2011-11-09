#!/bin/sh

JSON_DIR = ../test_json/
PB_DIR = ../test_protobuf/
RES = result.log

if ( [ -f result.log ] ); then
    rm -f result.log
fi

echo "speed test start" >> $RES
echo "name      |count  |serialize  |unserialize    |avg    |radio" >> $RES
JSON_DIR/test >> $RES
PB_DIR/test >> $RES
echo "speeded test end" >> $RES
