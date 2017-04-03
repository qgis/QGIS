% {Cpp: LicenseTemplate}\
#include "qgstest.h"
#include <QObject>
#include <qgsapplication.h>

class % {CN}: public QObject
{

  Q_OBJECT

private slots:
  void initTestCase(); // will be called before the first testfunction is executed.
  void cleanupTestCase() {} // will be called after the last testfunction was executed.
  void init() {} // will be called before each testfunction is executed.
  void cleanup() {} // will be called after every testfunction.

  // Add your test methods here
};

void % {CN}::initTestCase()
{

}

void % {CN}::cleanupTestCase()
{

}

void % {CN}::init()
{

}

void % {CN}::cleanup()
{

}

QGSTEST_MAIN( % {CN} )
#include "%{JS: Cpp.classToFileName('%{Class}', '.moc')}"
