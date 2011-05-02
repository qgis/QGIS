/***************************************************************************
     main.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:20:46 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <iostream>

//#include <qapplication.h>
//#include <cppunit/ui/qt/TestRunner.h>

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>


#include "projecttest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( ProjectTest );

int main( int, char ** )
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
