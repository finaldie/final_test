#!/bin/sh

JSON_DIR='./test_json'
PB_DIR='./test_protobuf'
RES='result.log'
TMP='tmp.log'
COUNT=1000000

if ( [ -f $RES ] ); then
    rm -f $RES
fi

echo "test sample: c struct{ int32 id = 1, char name[] = \"hyz\"}" >> $RES
echo "unit of time: usec, unit of lengh: byte" >> $RES
echo "compiler: g++ v4.1.2" >> $RES
echo "CPPFLAGS: -O2" >>$RES
echo "librarys: jsoncpp-0.5.0.4, google protobuf last stable v2.4" >> $RES
echo "" >> $RES

echo "statistics result" >> $RES
echo "name      |count     |serialize |unserialize|total     |ser/per   |unser/per |size      |radio" >> $RES
$JSON_DIR/test $COUNT >> $RES
$PB_DIR/test $COUNT >> $RES
rm -f $TMP
