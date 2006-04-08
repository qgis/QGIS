#include <QtTest>
#include <qgsapplication.h>
#include "testqgsapplication.h"
#include <QObject>
#include <QString>


void TestQgsApplication::authorsFilePath()
{
  QString myPath = QgsApplication::authorsFilePath();
  QVERIFY(!myPath.isNull());
}

QTEST_MAIN(TestQgsApplication)
#include "testqgsapplication.moc.cpp"




