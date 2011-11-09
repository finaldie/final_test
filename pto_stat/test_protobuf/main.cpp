/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  test protobuf
 *
 *        Version:  1.0
 *        Created:  11/09/2011 02:56:25 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sys/time.h>

#include "test.hello.pb.h"
using namespace std;
using namespace test;

typedef struct{
	struct timeval tv;
	struct timezone tz;
}my_time;
void get_cur_time(my_time*);
int  get_diff_time(my_time* time1, my_time* time2);

void get_cur_time(my_time* time){
	gettimeofday(&time->tv, &time->tz);
}

int	get_diff_time(my_time* time1, my_time* time2){
	int diff_sec = time2->tv.tv_sec - time1->tv.tv_sec;
	if ( diff_sec > 0 )
		return diff_sec * 1000000 + time2->tv.tv_usec - time1->tv.tv_usec;
	else
		return time2->tv.tv_usec - time1->tv.tv_usec;
}

int stat_serialize(int count, string& res)
{
    hello src;
    src.set_id(100);
    src.set_name("hyz");
    //src.PrintDebugString();

    my_time t1, t2;
    get_cur_time(&t1);

    for( int i=0; i<count; i++ ){
        //res.clear();
        if( !src.SerializeToString(&res) ){
            cout << "PB test failed" << endl;
            exit(1);
        }
    }

    get_cur_time(&t2);
    return get_diff_time(&t1, &t2);
}

int stat_unserialize(string& src, int count)
{
    hello des;

    my_time t1, t2;
    get_cur_time(&t1);

    for( int i=0; i<count; i++ ){
        des.ParseFromString(src);
        //des.PrintDebugString();
    }

    get_cur_time(&t2);
    return get_diff_time(&t1, &t2);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int
main ( int argc, char *argv[] )
{
    if( argc != 2 ){
        printf("miss count\n");
        exit(0);
    }  
    
    int count = atoi(argv[1]);
    string serialize_res;
    int serialize_time = stat_serialize(count, serialize_res);
    int unserialize_time = stat_unserialize(serialize_res, count);

    //printf("name    |count  |serialize  |unserialize    |total|ser_per    |unser_per  |ser_size   |radio\n"); 
    printf("%-10s|%-10d|%-10d|%-11d|%-10d|%-10d|%-10d|%-10zu|\n", 
            "PB",
            count,
            serialize_time, 
            unserialize_time,
            serialize_time + unserialize_time,
            serialize_time/count,
            unserialize_time/count,
            serialize_res.size());

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
