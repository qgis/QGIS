/***************************************************************************
                          qgsmaptoolprofilecurvefromfeature.cpp
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmaptoolprofilecurvefromfeature.h"
#include "qgsmapcanvas.h"
#include "qgsidentifymenu.h"
#include "qgsmapmouseevent.h"
#include "qgsapplication.h"

QgsMapToolProfileCurveFromFeature::QgsMapToolProfileCurveFromFeature( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::CrossHair ) );
}

QgsMapTool::Flags QgsMapToolProfileCurveFromFeature::flags() const
{
  return QgsMapTool::Flag::AllowZoomRect | QgsMapTool::Flag::ShowContextMenu;
}

void QgsMapToolProfileCurveFromFeature::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( e->button() == Qt::LeftButton )
  {
    e->ignore();

    const QList<QgsMapToolIdentify::IdentifyResult> results = QgsIdentifyMenu::findFeaturesOnCanvas( e, mCanvas, { QgsWkbTypes::LineGeometry } );
    if ( results.empty( ) )
      return;

    QgsIdentifyMenu *menu = new QgsIdentifyMenu( mCanvas );
    menu->setAllowMultipleReturn( false );
    menu->setExecWithSingleResult( false );

    const QPoint globalPos = mCanvas->mapToGlobal( QPoint( e->pos().x() + 5, e->pos().y() + 5 ) );
    const QList<QgsMapToolIdentify::IdentifyResult> selectedFeatures = menu->exec( results, globalPos );

    menu->deleteLater();

    if ( !selectedFeatures.empty() && selectedFeatures[0].mFeature.hasGeometry() )
    {
      const QgsCoordinateTransform transform = mCanvas->mapSettings().layerTransform( selectedFeatures.at( 0 ).mLayer );
      QgsGeometry geom = selectedFeatures[0].mFeature.geometry();
      try
      {
        geom.transform( transform );
      }
      catch ( QgsCsException & )
      {
        QgsDebugMsg( QStringLiteral( "Could not transform geometry from layer CRS" ) );
      }
      emit curveCaptured( geom );
    }
  }
}

