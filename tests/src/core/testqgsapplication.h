#ifndef TESTQGSAPPLICATION_H
#define TESTQGSAPPLICATION_H
#include <QtTest/QtTest>
#include <QObject>

class TestQgsApplication: public QObject
{
  Q_OBJECT;
  public:
  private slots:
    void authorsFilePath();
};

#endif
