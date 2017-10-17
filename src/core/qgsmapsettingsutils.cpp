/***************************************************************************
                         qgsmapsettingsutils.cpp
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapsettings.h"
#include "qgsmapsettingsutils.h"
#include "qgspallabeling.h"
#include "qgstextrenderer.h"
#include "qgsvectorlayer.h"

#include <QString>

const QStringList QgsMapSettingsUtils::containsAdvancedEffects( const QgsMapSettings &mapSettings )
{
  QSet< QString > layers;

  QgsTextFormat layerFormat;
  Q_FOREACH ( QgsMapLayer *layer, mapSettings.layers() )
  {
    if ( layer )
    {
      if ( layer->blendMode() != QPainter::CompositionMode_SourceOver )
      {
        layers << layer->name();
      }
      // if vector layer, check labels and feature blend mode
      QgsVectorLayer *currentVectorLayer = qobject_cast<QgsVectorLayer *>( layer );
      if ( currentVectorLayer )
      {
        if ( !qgsDoubleNear( currentVectorLayer->opacity(), 1.0 ) )
        {
          layers << layer->name();
        }
        if ( currentVectorLayer->featureBlendMode() != QPainter::CompositionMode_SourceOver )
        {
          layers << layer->name();
        }
        // check label blend modes
        if ( QgsPalLabeling::staticWillUseLayer( currentVectorLayer ) )
        {
          // Check all label blending properties
          layerFormat.readFromLayer( currentVectorLayer );
          if ( layerFormat.containsAdvancedEffects() )
          {
            layers << layer->name();
          }
        }
      }
    }
  }

  return layers.toList();
}

QString QgsMapSettingsUtils::worldFileContent( const QgsMapSettings &mapSettings )
{
  QgsMapSettings ms = mapSettings;

  double rotation = ms.rotation();
  double alpha = rotation / 180 * M_PI;

  // reset rotation to 0 to calculate world file parameters
  ms.setRotation( 0 );

  double xOrigin = ms.visibleExtent().xMinimum() + ( ms.mapUnitsPerPixel() / 2 );
  double yOrigin = ms.visibleExtent().yMaximum() - ( ms.mapUnitsPerPixel() / 2 );

  double xCenter = ms.visibleExtent().center().x();
  double yCenter = ms.visibleExtent().center().y();

  // scaling matrix
  double s[6];
  s[0] = ms.mapUnitsPerPixel();
  s[1] = 0;
  s[2] = xOrigin;
  s[3] = 0;
  s[4] = ms.mapUnitsPerPixel();
  s[5] = yOrigin;

  // rotation matrix
  double r[6];
  r[0] = std::cos( alpha );
  r[1] = -std::sin( alpha );
  r[2] = xCenter * ( 1 - std::cos( alpha ) ) + yCenter * std::sin( alpha );
  r[3] = std::sin( alpha );
  r[4] = std::cos( alpha );
  r[5] = - xCenter * std::sin( alpha ) + yCenter * ( 1 - std::cos( alpha ) );

  // result = rotation x scaling = rotation(scaling(X))
  double a = r[0] * s[0] + r[1] * s[3];
  double b = r[0] * s[1] + r[1] * s[4];
  double c = r[0] * s[2] + r[1] * s[5] + r[2];
  double d = r[3] * s[0] + r[4] * s[3];
  double e = r[3] * s[1] + r[4] * s[4];
  double f = r[3] * s[2] + r[4] * s[5] + r[5];

  QString content;
  // Pixel XDim
  content += qgsDoubleToString( a ) + "\r\n";
  // Rotation on y axis
  content += qgsDoubleToString( d ) + "\r\n";
  // Rotation on x axis
  content += qgsDoubleToString( -b ) + "\r\n";
  // Pixel YDim - almost always negative
  // See https://en.wikipedia.org/wiki/World_file#cite_ref-3
  content += "-" + qgsDoubleToString( e ) + "\r\n";
  // Origin X (center of top left cell)
  content += qgsDoubleToString( c ) + "\r\n";
  // Origin Y (center of top left cell)
  content += qgsDoubleToString( f ) + "\r\n";

  return content;
}
