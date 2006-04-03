#ifndef TESTQGSAPPLICATION_H
#define TESTQGSAPPLICATION_H
#include <QtTest>
#include <QObject>

class TestQgsApplication: public QObject
{
  Q_OBJECT;
  public:
    TestQgsApplication();
    ~TestQgsApplication();
  private slots:
    void authorsFilePath();
};

#endif
