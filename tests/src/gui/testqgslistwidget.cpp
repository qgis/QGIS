/***************************************************************************
    testqgslistwidget.cpp
     --------------------------------------
    Date                 : 08 09 2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick dot valsecchi at camptocamp dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsconfig.h"

#include "editorwidgets/core/qgseditorwidgetwrapper.h"
#include "editorwidgets/qgslistwidgetfactory.h"
#include "editorwidgets/qgslistwidgetwrapper.h"
#include "qgsapplication.h"
#include "qgslistwidget.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QSignalSpy>

class TestQgsListWidget : public QObject
{
    Q_OBJECT
  private:
#ifdef ENABLE_PGTEST
    QString dbConn;
#endif

  private slots:
    void initTestCase() // will be called before the first testfunction is executed.
    {
      QgsApplication::init();
      QgsApplication::initQgis();
#ifdef ENABLE_PGTEST
      dbConn = getenv( "QGIS_PGTEST_DB" );
      if ( dbConn.isEmpty() )
      {
        dbConn = "service=\"qgis_test\"";
      }
#endif
    }

    void cleanupTestCase() // will be called after the last testfunction was executed.
    {
#ifdef ENABLE_PGTEST
      // delete new features in db from postgres test
      QgsVectorLayer *vl_array_int = new QgsVectorLayer( u"%1 sslmode=disable key=\"pk\" table=\"qgis_test\".\"array_tbl\" sql="_s.arg( dbConn ), u"json"_s, u"postgres"_s );
      vl_array_int->startEditing();
      const QgsFeatureIds delete_ids = QSet<QgsFeatureId>() << Q_INT64_C( 997 ) << Q_INT64_C( 998 ) << Q_INT64_C( 999 );
      vl_array_int->deleteFeatures( delete_ids );
      vl_array_int->commitChanges( false );
      QgsVectorLayer *vl_array_str = new QgsVectorLayer( u"%1 sslmode=disable key=\"pk\" table=\"qgis_test\".\"string_array\" sql="_s.arg( dbConn ), u"json"_s, u"postgres"_s );
      vl_array_str->startEditing();
      vl_array_str->deleteFeatures( delete_ids );
      vl_array_str->commitChanges( false );
#endif
      QgsApplication::exitQgis();
    }

    void testStringUpdate()
    {
      const QgsListWidgetFactory factory( u"testList"_s );
      QgsVectorLayer vl( u"Point?field=fld:string[]"_s, u"test"_s, u"memory"_s );
      QgsEditorWidgetWrapper *wrapper = factory.create( &vl, 0, nullptr, nullptr );
      QVERIFY( wrapper );
      const QSignalSpy spy( wrapper, SIGNAL( valueChanged( const QVariant & ) ) );

      QgsListWidget *widget = wrapper->widget()->findChild<QgsListWidget *>();
      QVERIFY( widget );

      QStringList initial;
      initial << u"one"_s << u"two"_s;
      wrapper->setValues( initial, QVariantList() );

      const QVariant value = wrapper->value();
      QCOMPARE( int( static_cast<QMetaType::Type>( value.userType() ) ), int( QMetaType::Type::QStringList ) );
      QCOMPARE( value.toStringList(), initial );
      QCOMPARE( spy.count(), 0 );

      QAbstractItemModel *model = widget->tableView->model();
      model->setData( model->index( 0, 0 ), "hello" );
      QCOMPARE( spy.count(), 1 );
      QVERIFY( widget->valid() );

      QStringList expected = initial;
      expected[0] = u"hello"_s;
      const QVariant eventValue = spy.at( 0 ).at( 0 ).value<QVariant>();
      QCOMPARE( int( static_cast<QMetaType::Type>( eventValue.userType() ) ), int( QMetaType::Type::QStringList ) );
      QCOMPARE( eventValue.toStringList(), expected );
      QCOMPARE( wrapper->value().toStringList(), expected );
      QCOMPARE( spy.count(), 1 );
      QVERIFY( widget->valid() );
    }

    void testIntUpdate()
    {
      const QgsListWidgetFactory factory( u"testList"_s );
      QgsVectorLayer vl( u"Point?field=fld:int[]"_s, u"test"_s, u"memory"_s );
      QgsEditorWidgetWrapper *wrapper = factory.create( &vl, 0, nullptr, nullptr );
      QVERIFY( wrapper );
      QSignalSpy spy( wrapper, SIGNAL( valueChanged( const QVariant & ) ) );

      QgsListWidget *widget = wrapper->widget()->findChild<QgsListWidget *>();
      QVERIFY( widget );

      QVariantList initial;
      initial << 1 << -2;
      wrapper->setValues( initial, QVariantList() );

      const QVariant value = wrapper->value();
      QCOMPARE( int( static_cast<QMetaType::Type>( value.userType() ) ), int( QMetaType::Type::QVariantList ) );
      QCOMPARE( value.toList(), initial );
      QCOMPARE( spy.count(), 0 );

      QAbstractItemModel *model = widget->tableView->model();
      model->setData( model->index( 0, 0 ), 3 );
      QCOMPARE( spy.count(), 1 );

      QVariantList expected = initial;
      expected[0] = 3;
      QCOMPARE( spy.count(), 1 );
      QVariant eventValue = spy.at( 0 ).at( 0 ).value<QVariant>();
      QCOMPARE( int( static_cast<QMetaType::Type>( eventValue.userType() ) ), int( QMetaType::Type::QVariantList ) );
      QCOMPARE( eventValue.toList(), expected );
      QCOMPARE( wrapper->value().toList(), expected );
      QVERIFY( widget->valid() );

      model->setData( model->index( 0, 0 ), "a" );
      expected = initial;
      expected.removeAt( 0 );
      QVERIFY( !widget->valid() );
      QCOMPARE( wrapper->value().toList(), expected );

      spy.clear();
      model->setData( model->index( 0, 0 ), 56 );
      expected = initial;
      expected[0] = 56;
      QCOMPARE( spy.count(), 1 );
      eventValue = spy.at( 0 ).at( 0 ).value<QVariant>();
      QCOMPARE( eventValue.toList(), expected );
      QCOMPARE( wrapper->value().toList(), expected );
      QVERIFY( widget->valid() );
    }

#ifdef ENABLE_PGTEST
    void testPostgres()
    {
      //create pg layers
      QgsVectorLayer *vl_array_int = new QgsVectorLayer( u"%1 sslmode=disable key=\"pk\" table=\"qgis_test\".\"array_tbl\" sql="_s.arg( dbConn ), u"json"_s, u"postgres"_s );

      connect( vl_array_int, &QgsVectorLayer::raiseError, this, []( const QString &msg ) { qWarning() << msg; } );
      QVERIFY( vl_array_int->isValid() );

      QgsListWidgetWrapper w_array_int( vl_array_int, vl_array_int->fields().indexOf( "location"_L1 ), nullptr, nullptr );
      QgsListWidget *widget = w_array_int.widget()->findChild<QgsListWidget *>();

      vl_array_int->startEditing();
      QVariantList newList;
      newList.append( u"100"_s );
      widget->setList( QList<QVariant>() << 100 );
      QVERIFY( w_array_int.value().isValid() );
      QCOMPARE( widget->list(), QList<QVariant>() << 100 );
      // save value and check it is saved properly in postges
      QgsFeature new_rec_997 { vl_array_int->fields(), 997 };
      new_rec_997.setAttribute( 0, QVariant( 997 ) );
      vl_array_int->addFeature( new_rec_997, QgsFeatureSink::RollBackOnErrors );
      vl_array_int->commitChanges( false );
      QVERIFY( vl_array_int->changeAttributeValue( 997, 1, w_array_int.value(), QVariant(), false ) );
      QVERIFY( vl_array_int->commitChanges( false ) );

      w_array_int.setFeature( vl_array_int->getFeature( 997 ) );
      QCOMPARE( widget->list(), QList<QVariant>() << 100 );

      // alter two values at a time which triggered old bug (#38784)
      widget->setList( QList<QVariant>() << 4 << 5 << 6 );
      QgsFeature new_rec_998 { vl_array_int->fields(), 998 };
      new_rec_998.setAttribute( 0, QVariant( 998 ) );
      new_rec_998.setAttribute( 1, w_array_int.value() );
      vl_array_int->addFeature( new_rec_998, QgsFeatureSink::RollBackOnErrors );

      widget->setList( QList<QVariant>() << 10 << 11 << 12 );
      QgsFeature new_rec_999 { vl_array_int->fields(), 999 };
      new_rec_999.setAttribute( 0, QVariant( 999 ) );
      new_rec_999.setAttribute( 1, w_array_int.value() );
      vl_array_int->addFeature( new_rec_999, QgsFeatureSink::RollBackOnErrors );
      vl_array_int->commitChanges( false );

      w_array_int.setFeature( vl_array_int->getFeature( 998 ) );
      QCOMPARE( widget->list(), QList<QVariant>() << 4 << 5 << 6 );

      w_array_int.setFeature( vl_array_int->getFeature( 999 ) );
      QCOMPARE( widget->list(), QList<QVariant>() << 10 << 11 << 12 );

      // do similar for array of strings
      QgsVectorLayer *vl_array_str = new QgsVectorLayer( u"%1 sslmode=disable key=\"pk\" table=\"qgis_test\".\"string_array\" sql="_s.arg( dbConn ), u"json"_s, u"postgres"_s );
      QVERIFY( vl_array_str->isValid() );

      QgsListWidgetWrapper w_array_str( vl_array_str, vl_array_str->fields().indexOf( "value"_L1 ), nullptr, nullptr );
      widget = w_array_str.widget()->findChild<QgsListWidget *>();
      vl_array_str->startEditing();
      QVariantList newListStr;

      // test quotes
      newListStr.append( u"10\"0"_s );
      widget->setList( newListStr );
      QVERIFY( w_array_str.value().isValid() );
      QCOMPARE( widget->list(), QList<QVariant>() << u"10\"0"_s );
      // save value and check it is saved properly in postges
      QgsFeature new_rec_997_str { vl_array_str->fields(), 997 };
      new_rec_997_str.setAttribute( 0, QVariant( 997 ) );
      vl_array_str->addFeature( new_rec_997_str, QgsFeatureSink::RollBackOnErrors );
      vl_array_str->commitChanges( false );
      QVERIFY( vl_array_str->changeAttributeValue( 997, 1, w_array_str.value(), QVariant(), false ) );
      QVERIFY( vl_array_str->commitChanges( false ) );

      w_array_str.setFeature( vl_array_str->getFeature( 997 ) );
      QCOMPARE( widget->list(), QList<QVariant>() << u"10\"0"_s );

      // alter two values at a time which triggered old bug (#38784)
      widget->setList( QList<QVariant>() << u"four"_s << u"five"_s << u"six"_s );
      QgsFeature new_rec_998_str { vl_array_str->fields(), 998 };
      new_rec_998_str.setAttribute( 0, QVariant( 998 ) );
      new_rec_998_str.setAttribute( 1, w_array_str.value() );
      vl_array_str->addFeature( new_rec_998_str, QgsFeatureSink::RollBackOnErrors );

      widget->setList( QList<QVariant>() << u"ten"_s << u"eleven"_s << u"twelve"_s );
      QgsFeature new_rec_999_str { vl_array_str->fields(), 999 };
      new_rec_999_str.setAttribute( 0, QVariant( 999 ) );
      new_rec_999_str.setAttribute( 1, w_array_str.value() );
      vl_array_str->addFeature( new_rec_999_str, QgsFeatureSink::RollBackOnErrors );
      vl_array_str->commitChanges( false );

      w_array_str.setFeature( vl_array_str->getFeature( 998 ) );
      QCOMPARE( widget->list(), QList<QVariant>() << u"four"_s << u"five"_s << u"six"_s );

      w_array_str.setFeature( vl_array_str->getFeature( 999 ) );
      QCOMPARE( widget->list(), QList<QVariant>() << u"ten"_s << u"eleven"_s << u"twelve"_s );
    }
#endif
};

QGSTEST_MAIN( TestQgsListWidget )
#include "testqgslistwidget.moc"
