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

using namespace std;
using namespace test;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int
main ( int argc, char *argv[] )
{
    hello src;
    string res;
    src.set_id(100);
    src.set_name("hyz");
    src.PrintDebugString();
    if( !src.SerializeToString(res) ){
        cout << "PB test failed" << endl;
        exit(1);
    }

    hello des;
    des.ParseFromString(res);
    des.PrintDebugString();

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
