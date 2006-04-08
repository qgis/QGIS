#include <QtTest>
#include <QObject>
#include <QString>
#include <QObject>
//header for class being tested
#include <[testClassLowerCaseName].h>

class Test[testClassCamelCaseName]: public QObject
{
  Q_OBJECT;
  private slots:
    [TestMethods]
};

QTEST_MAIN(Test[testClassCamelCaseName])
#include "test[testClassLowerCaseName].moc.cpp"




