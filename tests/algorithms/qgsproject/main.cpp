#include <iostream>

//#include <qapplication.h>
//#include <cppunit/ui/qt/TestRunner.h>

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>


#include "projecttest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( ProjectTest );

int main( int , char **)
{
    std::cout << "\n" 
              << "QgsProject TESTS...................\n";

    CppUnit::TextUi::TestRunner runner;

    CppUnit::TestFactoryRegistry &registry = 
        CppUnit::TestFactoryRegistry::getRegistry();

    runner.addTest( registry.makeTest() );

    runner.run();

    return 0;
}
