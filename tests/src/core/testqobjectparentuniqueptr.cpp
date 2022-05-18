/***************************************************************************
  testqobjectparentuniqueptr.cpp
  --------------------------------------
  Date                 :
  Copyright            : (C) 2022 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
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
#include <QPointer>

class TestQObjectChild;
class TestQObjectOwner : public QObject
{
    Q_OBJECT
  public:

    ~TestQObjectOwner() override;

    void setChild( TestQObjectChild *child )
    {
      mChild = child;
    }

  private:

    TestQObjectChild *mChild = nullptr;

};

class TestQObjectChild
{
  public:

    TestQObjectChild( TestQObjectOwner *parent )
      : mParent( parent )
    {
      parent->setChild( this );
    }

    ~TestQObjectChild()
    {
      if ( mParent )
        mParent->setChild( nullptr );
    }

    void setParent( TestQObjectOwner *parent )
    {
      mParent = parent;
    }

    void setValue( int value ) { mValue = value; }
    int value() const { return mValue; }

  private:

    TestQObjectOwner *mParent = nullptr;
    int mValue = 0;

};


TestQObjectOwner::~TestQObjectOwner()
{
  if ( mChild )
    mChild->setParent( nullptr );

  delete mChild;
}


class TestQObjectParentUniquePtr : public QObject
{
    Q_OBJECT

  private slots:
    void testMemLeak();
    void testParentDeletedFirst();
    void testParentDeletedAfter();
    void testOperatorBool();
    void testOperatorArrow();
    void testDeleteLater();
};

void TestQObjectParentUniquePtr::testMemLeak()
{
  TestQObjectOwner parent;
  TestQObjectChild *child = new TestQObjectChild( &parent );
  const QObjectParentUniquePtr<TestQObjectChild> obj( child, &parent );
}

void TestQObjectParentUniquePtr::testParentDeletedFirst()
{
  TestQObjectOwner *parent = new TestQObjectOwner();
  TestQObjectChild *child = new TestQObjectChild( parent );

  const QObjectParentUniquePtr<TestQObjectChild> obj( child, parent );
  QVERIFY( !obj.isNull() );
  QVERIFY( obj );
  QCOMPARE( child, obj.get() );
  QCOMPARE( child, obj.data() );

  delete parent;
  QVERIFY( obj.isNull() );
  QVERIFY( !obj );
}

void TestQObjectParentUniquePtr::testParentDeletedAfter()
{
  TestQObjectOwner *parent = new TestQObjectOwner();
  TestQObjectChild *child = new TestQObjectChild( parent );

  {
    const QObjectParentUniquePtr<TestQObjectChild> obj( child, parent );
  }

  // Basically shouldn't crash because of double delete on this line
  delete parent;
}

void TestQObjectParentUniquePtr::testOperatorBool()
{
  const QObjectParentUniquePtr<TestQObjectChild> obj;
  QVERIFY( !obj );

  TestQObjectOwner parent;
  const QObjectParentUniquePtr<TestQObjectChild> obj2( new TestQObjectChild( &parent ), &parent );
  QVERIFY( obj2 );
}

void TestQObjectParentUniquePtr::testOperatorArrow()
{
  TestQObjectOwner parent;
  TestQObjectChild *child = new TestQObjectChild( &parent );

  child->setValue( 5 );
  const QObjectParentUniquePtr<TestQObjectChild> obj( child, &parent );
  QCOMPARE( obj->value(), 5 );
}

void TestQObjectParentUniquePtr::testDeleteLater()
{
  TestQObjectOwner *o = new TestQObjectOwner();
  TestQObjectOwner *o2 = new TestQObjectOwner();

  TestQObjectChild *child1 = new TestQObjectChild( o );
  TestQObjectChild *child2 = new TestQObjectChild( o2 );

  const QObjectParentUniquePtr<TestQObjectChild> obj( child1, o );
  QObjectParentUniquePtr<TestQObjectChild> obj2( child2, o2 );

  o->deleteLater();
  o2->deleteLater();

  obj2.reset();

  connect( o, &QObject::destroyed, QgsApplication::instance(), &QgsApplication::quit );
  QgsApplication::exec();
  QVERIFY( obj.isNull() );
  QVERIFY( obj2.isNull() );
}



QGSTEST_MAIN( TestQObjectParentUniquePtr )
#include "testqobjectparentuniqueptr.moc"
