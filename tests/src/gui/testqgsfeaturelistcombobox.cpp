/***************************************************************************
  testqgsfeaturelistcombobox.cpp

 ---------------------
 begin                : 3.10.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgsapplication.h"
#include "qgsfeaturelistcombobox.h"
#include "qgsvectorlayer.h"
#include "qgsfeaturefiltermodel.h"
#include "qgsgui.h"

#include <memory>

#include <QLineEdit>

class TestQgsFeatureListComboBox : public QObject
{
    Q_OBJECT
  public:
    TestQgsFeatureListComboBox() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testSetGetLayer();
    void testSetGetForeignKey();
    void testAllowNull();

  private:
    void waitForLoaded( QgsFeatureListComboBox *cb );

    std::unique_ptr<QgsVectorLayer> mLayer;
};

void TestQgsFeatureListComboBox::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsFeatureListComboBox::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsFeatureListComboBox::init()
{
  // create layer
  mLayer.reset( new QgsVectorLayer( QStringLiteral( "LineString?field=pk:int&field=material:string&field=diameter:int&field=raccord:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) ) );

  // add features
  mLayer->startEditing();

  QgsFeature ft1( mLayer->fields() );
  ft1.setAttribute( QStringLiteral( "pk" ), 10 );
  ft1.setAttribute( QStringLiteral( "material" ), "iron" );
  ft1.setAttribute( QStringLiteral( "diameter" ), 120 );
  ft1.setAttribute( QStringLiteral( "raccord" ), "brides" );
  mLayer->addFeature( ft1 );

  QgsFeature ft2( mLayer->fields() );
  ft2.setAttribute( QStringLiteral( "pk" ), 11 );
  ft2.setAttribute( QStringLiteral( "material" ), "iron" );
  ft2.setAttribute( QStringLiteral( "diameter" ), 120 );
  ft2.setAttribute( QStringLiteral( "raccord" ), "sleeve" );
  mLayer->addFeature( ft2 );

  QgsFeature ft3( mLayer->fields() );
  ft3.setAttribute( QStringLiteral( "pk" ), 12 );
  ft3.setAttribute( QStringLiteral( "material" ), "steel" );
  ft3.setAttribute( QStringLiteral( "diameter" ), 120 );
  ft3.setAttribute( QStringLiteral( "raccord" ), "collar" );
  mLayer->addFeature( ft3 );

  mLayer->commitChanges();
}

void TestQgsFeatureListComboBox::cleanup()
{
}

void TestQgsFeatureListComboBox::testSetGetLayer()
{
  std::unique_ptr<QgsFeatureListComboBox> cb( new QgsFeatureListComboBox() );

  Q_ASSERT( cb->sourceLayer() == nullptr );
  cb->setSourceLayer( mLayer.get() );
  QCOMPARE( cb->sourceLayer(), mLayer.get() );
}

void TestQgsFeatureListComboBox::testSetGetForeignKey()
{
  QgsFeatureListComboBox *cb = new QgsFeatureListComboBox();
  // std::unique_ptr<QgsFeatureListComboBox> cb( new QgsFeatureListComboBox() );

  Q_ASSERT( cb->identifierValue().isNull() );

  cb->setSourceLayer( mLayer.get() );
  cb->setDisplayExpression( "\"material\"" );
  cb->lineEdit()->setText( "ro" );
  emit cb->lineEdit()->textChanged( "ro" );
  Q_ASSERT( cb->identifierValue().isNull() );

  waitForLoaded( cb );

  Q_ASSERT( cb->identifierValue().isNull() );

  cb->setIdentifierValue( 20 );
  QCOMPARE( cb->identifierValue(), QVariant( 20 ) );
}

void TestQgsFeatureListComboBox::testAllowNull()
{
  Q_ASSERT( false );
  // Note to self: implement this!
}

void TestQgsFeatureListComboBox::waitForLoaded( QgsFeatureListComboBox *cb )
{
  QgsFeatureFilterModel *model = qobject_cast<QgsFeatureFilterModel *>( cb->model() );

  // Wait
  while ( model->isLoading() )
  {}
}

QGSTEST_MAIN( TestQgsFeatureListComboBox )
#include "testqgsfeaturelistcombobox.moc"
