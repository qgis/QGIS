/***************************************************************************
     testqgsmapsavedialog.cpp
     ------------------------------
    Date                 : September 2025
    Copyright            : (C) 2025 by Germán Carrillo
    Email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsmapsavedialog.h"
#include "qgstest.h"

#include <QObject>

class TestQgsMapSaveDialog : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsMapSaveDialog()
      : QgsTest( u"Map save dialogs"_s )
    {}

  private:
    QgisApp *mQgisApp = nullptr;

  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
      mQgisApp = new QgisApp();
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void testUpdateExtent()
    {
      // Set up base canvas
      QgsMapCanvas canvas;
      canvas.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3857"_s ) );
      canvas.setFrameStyle( QFrame::NoFrame );
      canvas.resize( 800, 600 );
      canvas.show(); // to make the canvas resize
      canvas.hide();
      canvas.setExtent( QgsRectangle( 623913, 5720967, 1215325, 6068610 ) );

      // Set up dialog
      QgsMapSaveDialog dialog( nullptr, &canvas );
      dialog.updateDpi( 96 );
      dialog.mLockScale->setChecked( false ); // Default, let's make it explicit

      // Check initial status
      QCOMPARE( dialog.mScaleWidget->scale(), 2794072 );
      QCOMPARE( dialog.mDpi, 96 );

      // Check update extent without locking the scale
      dialog.mScaleWidget->setScale( 10000 ); // First set a different arbitrary scale
      QCOMPARE( dialog.mScaleWidget->scale(), 10000 );

      QgsRectangle canvasExtent( 1028930.8433, 5910111.234, 1031976.2192, 5912395.266 );
      canvas.setExtent( canvasExtent );
      dialog.mExtentGroupBox->setOutputExtentFromCurrent(); // Same as set extent from "Map Canvas Extent"
      QCOMPARE( dialog.mExtentGroupBox->outputExtent(), canvasExtent );
      QCOMPARE( dialog.mScaleWidget->scale(), 14388 );

      // Check update extent locking the scale
      dialog.mScaleWidget->setScale( 10000 ); // First set a different arbitrary scale
      QCOMPARE( dialog.mScaleWidget->scale(), 10000 );
      dialog.mLockScale->setChecked( true );

      canvas.setExtent( canvasExtent );
      dialog.mExtentGroupBox->setOutputExtentFromCurrent(); // Same as set extent from "Map Canvas Extent"
      QCOMPARE( dialog.mExtentGroupBox->outputExtent(), canvasExtent );
      QCOMPARE( dialog.mScaleWidget->scale(), 10000 ); // Our arbitrary scale is kept!
    }
};

QGSTEST_MAIN( TestQgsMapSaveDialog )
#include "testqgsmapsavedialog.moc"
