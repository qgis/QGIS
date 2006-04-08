#include <QtTest>
#include <QObject>
#include <QString>
#include <QObject>
//header for class being tested
#include <qgsapplication.h>

class TestQgsApplication: public QObject
{
    Q_OBJECT;
      public:
      private slots:
            void authorsFilePath();
};


void TestQgsApplication::authorsFilePath()
{
  QString myPath = QgsApplication::authorsFilePath();
  QVERIFY(!myPath.isNull());
}

QTEST_MAIN(TestQgsApplication)
#include "testqgsapplication.moc.cpp"




