/***************************************************************************
     testqgsrunprocess.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:30:36 AKDT 2007
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
#include <QtTest>
#include <QObject>
#include <QString>
#include <QObject>
//header for class being tested
#include <qgsrunprocess.h>

class TestQgsRunProcess: public QObject
{
  Q_OBJECT;
  private slots:
        void QgsRunProcessQgsRunProcess()
{

};
    void QgsRunProcessdie()
{

};
    void QgsRunProcessstdoutAvailable()
{

};
    void QgsRunProcessstderrAvailable()
{

};
    void QgsRunProcessprocessExit()
{

};
    void QgsRunProcessdialogGone()
{

};

};

QTEST_MAIN(TestQgsRunProcess)
#include "testqgsrunprocess.moc.cpp"




