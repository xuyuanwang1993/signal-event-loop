#include "log/aimy-log.h"
#include "core/core-include.h"
#include "imp/udp-echo-server.h"
#include "imp/commandlineTool.h"
#include "imp/utils/common_utils.h"
using namespace aimy;
int main0 ( int argc, char * argv[] )
{
unsigned int i = 0x12345678;
unsigned short int *q = NULL;
unsigned char *p = ( unsigned char * )&i;

*p = 0x00;
q = ( unsigned short int * )( p + 1 );
*q = 0x0000;
return( EXIT_SUCCESS );
}

int main1 ( int argc, char * argv[] )
{
unsigned int i = 0x12345678;
unsigned short int j = 0x0000;

j = *( ( unsigned short int * )( ( ( unsigned char * )&i ) + 1 ) );
return( EXIT_SUCCESS );
}

int main(int argc,char *argv[])
{
    if(AIMY_UTILS::checkIsBuildDay())
    {
        AIMY_INFO("test 1");
    }
    else {
        AIMY_INFO("test 2");
    }
    (void)argc;
    (void)argv;
    int opt=0;
    if(argc>1)opt=std::stoi(argv[1]);
    switch (opt) {
    case 0:
        return main0(argc,argv);
    case 1:
        return main1(argc,argv);
    default:
        return 0;
    }
}

