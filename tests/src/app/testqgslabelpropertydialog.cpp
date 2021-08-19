/***************************************************************************
     testqgslabelpropertydialog.cpp
     ------------------------------
    Date                 : Feb 2020
    Copyright            : (C) 2020 by Paul Blottiere
    Email                : blottiere dot paul at gmail dot com
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

#include "qgisapp.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgslabelingengine.h"
#include "qgsauxiliarystorage.h"
#include "qgslabelpropertydialog.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsmapcanvas.h"

class TestQgsLabelPropertyDialog : public QObject
{
    Q_OBJECT

  public:
    TestQgsLabelPropertyDialog() = default;

  private:
    QString mTestDataDir;
    QgisApp *mQgisApp = nullptr;

  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
      mQgisApp = new QgisApp();

      const QString myDataDir( TEST_DATA_DIR );
      mTestDataDir = myDataDir + '/';
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void test()
    {
      // init vector layer
      const QString pointFileName = mTestDataDir + "points.shp";
      const QFileInfo pointFileInfo( pointFileName );
      QgsVectorLayer *vl = new QgsVectorLayer( pointFileInfo.filePath(),
          pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
      QgsProject::instance()->addMapLayer( vl );

      // activate labeling
      const QgsPalLayerSettings settings;
      vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );

      // create auxiliary layer
      const QgsField field = vl->fields().field( 1 );
      QgsAuxiliaryLayer *al = QgsProject::instance()->auxiliaryStorage()->createAuxiliaryLayer( field, vl );
      vl->setAuxiliaryLayer( al );

      // create auxiliary field for BufferDraw
      QgsAuxiliaryLayer::createProperty( QgsPalLayerSettings::BufferDraw, vl );
      const QgsPropertyDefinition def = QgsPalLayerSettings::propertyDefinitions()[QgsPalLayerSettings::BufferDraw];
      const QString propName = QgsAuxiliaryLayer::nameFromProperty( def, true );
      QCOMPARE( int( al->featureCount() ), 0 );

      const QgsFeatureId fid = 0;
      QVariant val = vl->getFeature( fid ).attribute( propName );

      const std::unique_ptr< QgsMapCanvas > mapCanvas = std::make_unique< QgsMapCanvas >();

      // init label property dialog and togle buffer draw
      QgsLabelPropertyDialog dialog( vl->id(), QString(), fid, QFont(), QString(), false, settings, mapCanvas.get() );
      dialog.bufferDrawToggled( true );

      // apply changes
      QgsAttributeMap changes = dialog.changedProperties();
      QgsAttributeMap::const_iterator changeIt = changes.constBegin();
      for ( ; changeIt != changes.constEnd(); ++changeIt )
      {
        vl->changeAttributeValue( fid, changeIt.key(), changeIt.value() );
      }

      // check auxiliary values
      QCOMPARE( int( al->featureCount() ), 1 );
      val = vl->getFeature( fid ).attribute( propName );
      QCOMPARE( val.toInt(), 1 );

      // toggle false
      dialog.bufferDrawToggled( false );

      changes = dialog.changedProperties();
      changeIt = changes.constBegin();
      for ( ; changeIt != changes.constEnd(); ++changeIt )
      {
        vl->changeAttributeValue( fid, changeIt.key(), changeIt.value() );
      }

      val = vl->getFeature( fid ).attribute( propName );
      QCOMPARE( val.toInt(), 0 );
    }
};

QGSTEST_MAIN( TestQgsLabelPropertyDialog )
#include "testqgslabelpropertydialog.moc"
