/***************************************************************************
     main.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:20:40 AKDT 2007
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
#include <cppunit/ui/text/TestRunner.h>
#include "projectioncshandlingtest.h"


int main( int, char ** )
{
  std::cout << std::endl << "CRS TESTS..................." << std::endl;
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( ProjectionCsHandlingTest::suite() );
  runner.run();
  return 0;
}
