#include <iostream>
#include <cppunit/ui/text/TestRunner.h>
#include "projectioncshandlingtest.h"


int main( int , char **)
{
    std::cout << std::endl << "PROJECTION TESTS..................." << std::endl;  
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( ProjectionCsHandlingTest::suite() );
    runner.run();
    return 0;
}
