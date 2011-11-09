/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  test for json
 *
 *        Version:  1.0
 *        Created:  11/08/2011 08:42:19 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <jsoncpp/json.h>
#include <string>
#include <iostream>
#include <sys/time.h>

using namespace std;

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

int stat_serialize(int count, string& res/*out*/)
{
    Json::FastWriter fWriter;
    
    Json::Value root;
    root["id"] = Json::Value(100);
    root["name"] = Json::Value("hyz");
    string json_str;

    my_time t1, t2;
    get_cur_time(&t1);

    for( int i=0; i<count; i++ ){
        res = fWriter.write(root);
        //cout << "json str:" << res << endl;
    } 

    get_cur_time(&t2);
    return get_diff_time(&t1, &t2);
}

int stat_unserialize(string& src, int count)
{
    Json::Reader fReader;
    Json::Value decode;

    my_time t1, t2;
    get_cur_time(&t1);

    for( int i=0; i<count; i++ ){
        bool sucess = fReader.parse(src, decode);
        if( !sucess ){
            printf("freader failed\n");
            exit(1);
        } 
    } 

    get_cur_time(&t2);

    //cout << "decode id=" << decode["id"].asInt() << endl;
    //cout << "decode name=" << decode["name"].asString() << endl;

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
            "JSON",
            count,
            serialize_time, 
            unserialize_time,
            serialize_time + unserialize_time,
            serialize_time/count,
            unserialize_time/count,
            serialize_res.size());


    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
