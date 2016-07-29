/***************************************************************************
     test_template.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
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
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
//header for class being tested
#include <[testClassLowerCaseName].h>

class Test[testClassCamelCaseName]: public QObject
{
    Q_OBJECT
  private slots:
    [TestMethods]
};

QTEST_MAIN( Test[testClassCamelCaseName] )
#include "test[testClassLowerCaseName].moc"




