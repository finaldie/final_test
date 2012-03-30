/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  setjmp/longjmp
 *
 *        Version:  1.0
 *        Created:  03/30/2012 11:05:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */
#include    <stdio.h>
#include    <stdlib.h>
#include    <setjmp.h>

int       max_iteration, iter;

jmp_buf   Main, PointA, PointB;

void      Ping(void);
void      Pong(void);

int  main(int  argc, char* argv[])
{
    max_iteration = abs(atoi(argv[1]));
    iter = 1;
    if (setjmp(Main) == 0)
        Ping();
    if (setjmp(Main) == 0)
        Pong();
    longjmp(PointA, 1);      

    return 0;
}

void  Ping(void)
{
    if (setjmp(PointA) == 0)
        longjmp(Main, 1);
    while (1) {
        printf("%3d : Ping-", iter);
        if (setjmp(PointA) == 0)
            longjmp(PointB, 1);
    }
}

void  Pong(void)
{
    if (setjmp(PointB) == 0)
        longjmp(Main, 1);
    while (1) {
        printf("Pong\n");
        iter++;
        if (iter > max_iteration)
            exit(0);
        if (setjmp(PointB) == 0)
            longjmp(PointA, 1);
    }
}
