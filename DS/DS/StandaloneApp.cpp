

/* file: hello.c, a stand-alone application */
#include "hellop.c"

void main(void)
{
	char * pszString = "Hello, World";
	HelloProc(pszString);
}