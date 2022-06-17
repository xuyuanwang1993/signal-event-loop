#include "imp/changelog-format.h"
#include <vector>
using namespace aimy;
int main(int argc,char *argv[])
{
    return aimy::ChangelogFormatter::handleCommandLine(argc,argv);
}
