/***************************************************************************
  testqobjectuniqueptr.cpp
  --------------------------------------
  Date                 :
  Copyright            : (C) 2019 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qobjectuniqueptr.h"

#include "qgstest.h"

class TestQObjectUniquePtr : public QObject
{
    Q_OBJECT

  private slots:
    void testMemLeak();
    void testParentDeletedFirst();
    void testParentDeletedAfter();
};



void TestQObjectUniquePtr::testMemLeak()
{
  QObject *myobj = new QObject();
  QObjectUniquePtr<QObject> obj( myobj );
}

void TestQObjectUniquePtr::testParentDeletedFirst()
{
  QObject *parent = new QObject();
  QObject *child = new QObject( parent );

  QObjectUniquePtr<QObject> obj( child );
  QVERIFY( !obj.isNull() );
  QVERIFY( obj );
  QCOMPARE( child, obj.get() );
  QCOMPARE( child, obj.data() );

  delete parent;
  QVERIFY( obj.isNull() );
  QVERIFY( !obj );
}

void TestQObjectUniquePtr::testParentDeletedAfter()
{
  QObject *parent = new QObject();
  QObject *child = new QObject( parent );
  QPointer<QObject> observer( child );

  {
    QObjectUniquePtr<QObject> obj( child );
    QVERIFY( !observer.isNull() );
  }
  QVERIFY( observer.isNull() );


  // Basically shouldn't crash because of double delete on this line
  delete parent;
  QVERIFY( observer.isNull() );
}

QGSTEST_MAIN( TestQObjectUniquePtr )
#include "testqobjectuniqueptr.moc"
