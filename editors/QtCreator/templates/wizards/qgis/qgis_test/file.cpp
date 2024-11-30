/***************************************************************************
                   %{SrcFileName} - %{CN}
                         --------------
    begin                : %{CurrentDate:dd-MM-yyyy}
@if %{HostOs:isWindows}
    copyright            : (C) %{CurrentDate:yyyy} by %{Env:USERNAME}
@else
    copyright            : (C) %{CurrentDate:yyyy} by %{Env:USER}
@endif
    email                : [ YOUR EMAIL]
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include <qgsapplication.h>

class % { CN } : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();    // will be called before the first testfunction is executed.
  void cleanupTestCase(); // will be called after the last testfunction was executed.
  void init();            // will be called before each testfunction is executed.
  void cleanup();         // will be called after every testfunction.

  // Add your test methods here
};

void % { CN }
::initTestCase()
{
}

void % { CN }
::cleanupTestCase()
{
}

void % { CN }
::init()
{
}

void % { CN }
::cleanup()
{
}

QGSTEST_MAIN( % { CN } )
#include "%{JS: Cpp.classToFileName('%{Class}', '.moc')}"
