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
    void testOperatorBool();
    void testSwap();
    void testOperatorArrow();
    void testDeleteLater();
};

void TestQObjectUniquePtr::testMemLeak()
{
  QObject *myobj = new QObject();
  const QObjectUniquePtr<QObject> obj( myobj );
}

void TestQObjectUniquePtr::testParentDeletedFirst()
{
  QObject *parent = new QObject();
  QObject *child = new QObject( parent );

  const QObjectUniquePtr<QObject> obj( child );
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
  const QPointer<QObject> observer( child );

  {
    const QObjectUniquePtr<QObject> obj( child );
    QVERIFY( !observer.isNull() );
  }
  QVERIFY( observer.isNull() );


  // Basically shouldn't crash because of double delete on this line
  delete parent;
  QVERIFY( observer.isNull() );
}

void TestQObjectUniquePtr::testOperatorBool()
{
  const QObjectUniquePtr<QObject> obj;
  QVERIFY( !obj );
  const QObjectUniquePtr<QObject> obj2( new QObject() );
  QVERIFY( obj2 );
}

void TestQObjectUniquePtr::testSwap()
{
  QObject *o = new QObject();
  QObjectUniquePtr<QObject> obj;
  QObjectUniquePtr<QObject> obj2( o );
  obj.swap( obj2 );
  QCOMPARE( o, obj.get() );
  QCOMPARE( nullptr, obj2.get() );

  QObject *o2 = new QObject();
  QObjectUniquePtr<QObject> obj3( o2 );
  obj.swap( obj3 );
  QCOMPARE( o, obj3.get() );
  QCOMPARE( o2, obj.get() );
}

void TestQObjectUniquePtr::testOperatorArrow()
{
  QObject *o = new QObject();
  o->setObjectName( "Teddy" );
  const QObjectUniquePtr<QObject> obj( o );
  QCOMPARE( obj->objectName(), QStringLiteral( "Teddy" ) );
}

void TestQObjectUniquePtr::testDeleteLater()
{
  QObject *o = new QObject();
  QObject *o2 = new QObject();

  const QObjectUniquePtr<QObject> obj( o );
  QObjectUniquePtr<QObject> obj2( o2 );

  obj2->deleteLater();
  obj->deleteLater();

  obj2.reset();

  connect( o, &QObject::destroyed, QgsApplication::instance(), &QgsApplication::quit );
  QgsApplication::exec();
  QVERIFY( obj.isNull() );
  QVERIFY( obj2.isNull() );
}

QGSTEST_MAIN( TestQObjectUniquePtr )
#include "testqobjectuniqueptr.moc"
