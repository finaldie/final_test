#!/bin/sh

JSON_DIR = ../test_json/
PB_DIR = ../test_protobuf/
RES = result.log

if ( [ -f $RES ] ); then
    rm -f $RES
fi

echo "speed test start" >> $RES
echo "name      |count  |serialize  |unserialize    |avg    |radio" >> $RES
$JSON_DIR/test >> $RES
$PB_DIR/test >> $RES
echo "speeded test end" >> $RES
