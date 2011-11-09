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
 *       Compiler:  gcc
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

using namespace std;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int
main ( int argc, char *argv[] )
{
    Json::Reader fReader;
    Json::FastWriter fWriter;

    Json::Value root;
    root["name"] = Json::Value("hyz");
    root["id"] = Json::Value(1);
    string json_str;
    json_str = fWriter.write(root);
    cout << "json str:" << json_str << endl;

    Json::Value decode;
    bool sucess = fReader.parse(json_str, decode);
    if( !sucess ){
        printf("freader failed\n");
        exit(1);
    } 

    cout << "decode name=" << decode["name"].asString() << endl;
    cout << "decode id=" << decode["id"].asInt() << endl;

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
