/***************************************************************************
     testqgsmaptoolutils.h
     ---------------------
    Date                 : January 2018
    Copyright            : (C) 2017 by Martin Dobias
                           (C) 2018 by Paul Blottiere
    Email                : wonder dot sk at gmail dot com
                           paul.blottiere@oslandia.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgisapp.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"


/**
 * \ingroup UnitTests
 */
class TestQgsMapToolAdvancedDigitizingUtils
{
  public:
    TestQgsMapToolAdvancedDigitizingUtils( QgsMapToolAdvancedDigitizing *mapTool )
      : mMapTool( mapTool )
    {
    }

    QSet<QgsFeatureId> existingFeatureIds()
    {
      QSet<QgsFeatureId> fids;
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mMapTool->canvas()->currentLayer() );

      if ( vl )
      {
        QgsFeature f;
        QgsFeatureIterator it = vl->getFeatures();
        while ( it.nextFeature( f ) )
          fids << f.id();
      }

      return fids;
    }

    QgsFeatureId newFeatureId( QSet<QgsFeatureId> oldFids = QSet<QgsFeatureId>() )
    {
      QSet<QgsFeatureId> newFids = existingFeatureIds();
      QSet<QgsFeatureId> diffFids = newFids.subtract( oldFids );
      Q_ASSERT( diffFids.count() == 1 );
      return *diffFids.constBegin();
    }

    QPoint mapToScreen( double mapX, double mapY )
    {
      QgsPointXY pt = mMapTool->canvas()->mapSettings().mapToPixel().transform( mapX, mapY );
      return QPoint( std::round( pt.x() ), std::round( pt.y() ) );
    }

    void mouseMove( double mapX, double mapY )
    {
      QgsMapMouseEvent e( mMapTool->canvas(), QEvent::MouseMove, mapToScreen( mapX, mapY ) );
      mMapTool->cadCanvasMoveEvent( &e );
    }

    void mousePress( double mapX, double mapY, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers(), bool snap = false )
    {
      QgsMapMouseEvent e1( mMapTool->canvas(), QEvent::MouseButtonPress, mapToScreen( mapX, mapY ), button, button, stateKey );

      if ( snap )
        e1.snapPoint();

      mMapTool->cadCanvasPressEvent( &e1 );
    }

    void mouseRelease( double mapX, double mapY, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers(), bool snap = false )
    {
      QgsMapMouseEvent e2( mMapTool->canvas(), QEvent::MouseButtonRelease, mapToScreen( mapX, mapY ), button, Qt::MouseButton(), stateKey );

      if ( snap )
        e2.snapPoint();

      mMapTool->cadCanvasReleaseEvent( &e2 );
    }

    void mouseClick( double mapX, double mapY, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers(), bool snap = false )
    {
      mousePress( mapX, mapY, button, stateKey, snap );
      mouseRelease( mapX, mapY, button, stateKey, snap );
    }

    void keyClick( int key )
    {
      QKeyEvent e1( QEvent::KeyPress, key, Qt::KeyboardModifiers() );
      mMapTool->keyPressEvent( &e1 );

      QKeyEvent e2( QEvent::KeyRelease, key, Qt::KeyboardModifiers() );
      mMapTool->keyReleaseEvent( &e2 );
    }

  private:
    QgsMapToolAdvancedDigitizing *mMapTool = nullptr;
};
