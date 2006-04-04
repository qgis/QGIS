#include <QtTest/QtTest>
#include <qgsapplication.h>
#include "testqgsapplication.h"
#include <QObject>
#include <QString>


TestQgsApplication::TestQgsApplication() : QObject()
{

}


TestQgsApplication::~TestQgsApplication()
{

}


void TestQgsApplication::authorsFilePath()
{
  QString myPath = QgsApplication::authorsFilePath();
  QVERIFY(!myPath.isNull());
}

QTEST_MAIN(TestQgsApplication)
#include "testqgsapplication.moc.cpp"




