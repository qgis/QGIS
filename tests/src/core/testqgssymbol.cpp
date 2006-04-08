#include <QtTest>
#include <QObject>
#include <QString>
#include <QObject>
//header for class being tested
#include <qgssymbol.h>

class TestQgsSymbol: public QObject
{
  Q_OBJECT;
  private slots:
    void setPointSize()
    {
      QgsSymbol mySymbol;
      mySymbol.setPointSize(4);
      int mySize = mySymbol.pointSize();
      QCOMPARE(mySize,4);
    }
};

QTEST_MAIN(TestQgsSymbol)
#include "testqgssymbol.moc.cpp"




