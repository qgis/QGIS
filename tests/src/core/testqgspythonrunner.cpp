/***************************************************************************
     testqgspythonrunner.cpp
     -----------------------
    Date                 : June 2026
    Copyright            : (C) 2026 by Francesco Mazzi
    Email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspythonrunner.h"
#include "qgstest.h"

using namespace Qt::StringLiterals;

class TestPythonRunnerImpl : public QgsPythonRunner
{
  public:
    QString lastCommand;
    QString lastFile;
    QString commandError;
    QString fileError;

  protected:
    bool runCommand( QString command, QString messageOnError = QString() ) override
    {
      Q_UNUSED( messageOnError )
      lastCommand = command;
      return true;
    }

    bool runFileCommand( const QString &filename, const QString &messageOnError = QString() ) override
    {
      Q_UNUSED( messageOnError )
      lastFile = filename;
      return true;
    }

    bool runFileCaptureErrorCommand( const QString &filename, QString &errorOut ) override
    {
      lastFile = filename;
      errorOut = fileError;
      return errorOut.isEmpty();
    }

    bool runCaptureErrorCommand( const QString &command, QString &errorOut ) override
    {
      lastCommand = command;
      errorOut = commandError;
      return errorOut.isEmpty();
    }

    bool evalCommand( QString command, QString &result ) override
    {
      lastCommand = command;
      result = u"ok"_s;
      return true;
    }

    bool setArgvCommand( const QStringList &arguments, const QString &messageOnError = QString() ) override
    {
      Q_UNUSED( arguments )
      Q_UNUSED( messageOnError )
      return true;
    }
};

class TestQgsPythonRunner : public QObject
{
    Q_OBJECT

  private slots:
    void cleanup();
    void captureErrorWithoutRunner();
    void captureErrorFromRunner();
};

void TestQgsPythonRunner::cleanup()
{
  QgsPythonRunner::setInstance( nullptr );
}

void TestQgsPythonRunner::captureErrorWithoutRunner()
{
  QgsPythonRunner::setInstance( nullptr );

  QString error;
  QVERIFY( !QgsPythonRunner::runFileCaptureError( u"/tmp/missing.py"_s, error ) );
  QVERIFY2( !error.isEmpty(), qPrintable( error ) );

  error.clear();
  QVERIFY( !QgsPythonRunner::runCaptureError( u"1 + "_s, error ) );
  QVERIFY2( !error.isEmpty(), qPrintable( error ) );
}

void TestQgsPythonRunner::captureErrorFromRunner()
{
  auto runner = new TestPythonRunnerImpl();
  QgsPythonRunner::setInstance( runner );

  runner->fileError = u"Traceback from file"_s;
  QString error;
  QVERIFY( !QgsPythonRunner::runFileCaptureError( u"/tmp/bad.py"_s, error ) );
  QCOMPARE( error, u"Traceback from file"_s );
  QCOMPARE( runner->lastFile, u"/tmp/bad.py"_s );

  runner->fileError.clear();
  error.clear();
  QVERIFY( QgsPythonRunner::runFileCaptureError( u"/tmp/ok.py"_s, error ) );
  QVERIFY( error.isEmpty() );
  QCOMPARE( runner->lastFile, u"/tmp/ok.py"_s );

  runner->commandError = u"Traceback from command"_s;
  QVERIFY( !QgsPythonRunner::runCaptureError( u"bad code"_s, error ) );
  QCOMPARE( error, u"Traceback from command"_s );
  QCOMPARE( runner->lastCommand, u"bad code"_s );

  runner->commandError.clear();
  error.clear();
  QVERIFY( QgsPythonRunner::runCaptureError( u"print('ok')"_s, error ) );
  QVERIFY( error.isEmpty() );
  QCOMPARE( runner->lastCommand, u"print('ok')"_s );
}

QGSTEST_MAIN( TestQgsPythonRunner )
#include "testqgspythonrunner.moc"
