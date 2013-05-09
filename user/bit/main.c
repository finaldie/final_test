#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


int main(int argc, char** argv)
{
	uint32_t a = (uint32_t)(~0) << 8;
	printf("0x%x\n", a);
	return 0;
}
