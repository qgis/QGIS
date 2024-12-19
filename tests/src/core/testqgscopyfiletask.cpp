/***************************************************************************
     testqgscopyfiletask.cpp
     ----------------------
    Date                 : June 2021
    Copyright            : (C) 2021 Julien Cabieces
    Author               : Julien Cabieces
    Email                : julien cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QTemporaryFile>
#include <QTemporaryDir>

#include "qgscopyfiletask.h"

/**
 * \ingroup UnitTests
 * Unit tests for QgsCopyFileTask
 */
class TestQgsCopyFileTask: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init() {}
    void cleanup() {}

    void testCopy();
    void testCopySourceDoesNotExist();
    void testCopyDestinationDoesNotExist();
    void testCopyFileAlreadyExist();
};


void TestQgsCopyFileTask::initTestCase()
{
}

void TestQgsCopyFileTask::cleanupTestCase()
{
}

void TestQgsCopyFileTask::testCopy()
{
  QgsTaskManager manager;

  QTemporaryFile testFile;
  QVERIFY( testFile.open() );
  testFile.write( QByteArray( "Some Content" ) );
  testFile.close();

  const QString source = testFile.fileName();

  const QTemporaryDir dir;
  const QString destination = dir.filePath( "dest.txt" );

  QgsCopyFileTask *task = new QgsCopyFileTask( source, destination );
  manager.addTask( task );
  task->waitForFinished();

  QCOMPARE( task->status(), QgsTask::Complete );

  QFile destFile( destination );
  QVERIFY( destFile.exists() );
  QVERIFY( destFile.open( QIODevice::ReadOnly ) );
  QCOMPARE( destFile.readAll(), QByteArray( "Some Content" ) );
}

void TestQgsCopyFileTask::testCopySourceDoesNotExist()
{
  QgsTaskManager manager;

  const QTemporaryDir dir;
  const QString destination = dir.filePath( "dest.txt" );

  QgsCopyFileTask *task = new QgsCopyFileTask( QStringLiteral( "/not/existing/file.txt" ), destination );
  manager.addTask( task );
  task->waitForFinished();

  QCOMPARE( task->status(), QgsTask::Terminated );

  const QFile destFile( destination );
  QVERIFY( !destFile.exists() );
}

void TestQgsCopyFileTask::testCopyDestinationDoesNotExist()
{
  QgsTaskManager manager;

  QTemporaryFile testFile;
  QVERIFY( testFile.open() );
  testFile.write( QByteArray( "Some Content" ) );
  testFile.close();

  const QString source = testFile.fileName();
  const QString destination = QStringLiteral( "/not/existing/destination/" );

  QgsCopyFileTask *task = new QgsCopyFileTask( source, destination );
  manager.addTask( task );
  task->waitForFinished();

  QCOMPARE( task->status(), QgsTask::Terminated );

  const QFile destFile( destination );
  QVERIFY( !destFile.exists() );
}

void TestQgsCopyFileTask::testCopyFileAlreadyExist()
{
  QgsTaskManager manager;

  QTemporaryFile testFile;
  QVERIFY( testFile.open() );
  testFile.write( QByteArray( "Some Content" ) );
  testFile.close();

  const QString source = testFile.fileName();

  QTemporaryFile destFile;
  QVERIFY( destFile.open() );
  destFile.write( QByteArray( "Already exists" ) );
  destFile.close();

  const QString destination = destFile.fileName();

  QgsCopyFileTask *task = new QgsCopyFileTask( source, destination );
  manager.addTask( task );
  task->waitForFinished();

  QCOMPARE( task->status(), QgsTask::Terminated );

  QFile destFile2( destination );
  QVERIFY( destFile2.exists() );
  QVERIFY( destFile2.open( QIODevice::ReadOnly ) );
  QCOMPARE( destFile2.readAll(), QByteArray( "Already exists" ) );
}

QGSTEST_MAIN( TestQgsCopyFileTask )
#include "testqgscopyfiletask.moc"
