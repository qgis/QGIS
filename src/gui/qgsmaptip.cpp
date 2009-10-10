/***************************************************************************
    qgsmaptips.cpp  -  Query a layer and show a maptip on the canvas
    ---------------------
    begin                : October 2007
    copyright            : (C) 2007 by Gary Sherman
    email                : sherman @ mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// QGIS includes
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsvectordataprovider.h>
#include <qgsfield.h>

// Qt includes
#include <QPoint>
#include <QToolTip>
#include <QSettings>

#include "qgsmaptip.h"

QgsMapTip::QgsMapTip()
{
  // init the visible flag
  mMapTipVisible = false;
}

QgsMapTip::~QgsMapTip()
{

}

void QgsMapTip::showMapTip( QgsMapLayer * thepLayer,
                            QgsPoint & theMapPosition,
                            QPoint & thePixelPosition,
                            QgsMapCanvas *thepMapCanvas )
{
  // Do the search using the active layer and the preferred label
  // field for the layer. The label field must be defined in the layer configuration
  // file/database. The code required to do this is similar to identify, except
  // we only want the first qualifying feature and we will only display the
  // field defined as the label field in the layer configuration file/database.
  //
  // TODO: Define the label (display) field for each map layer in the map configuration file/database

  // Show the maptip on the canvas
  QString myTipText = fetchFeature( thepLayer, theMapPosition, thepMapCanvas );
  if ( myTipText.length() > 0 )
  {
    mMapTipVisible = true;
    QToolTip::showText( thepMapCanvas->mapToGlobal( thePixelPosition ), myTipText, thepMapCanvas );
    // store the point so we can use it to clear the maptip later
    mLastPosition = thePixelPosition;
  }
  else
  {
    mMapTipVisible = false;
  }

}

void QgsMapTip::clear( QgsMapCanvas *mpMapCanvas )
{
  if ( mMapTipVisible )
  {
    // set the maptip to blank
    QToolTip::showText( mpMapCanvas->mapToGlobal( mLastPosition ), "", mpMapCanvas );
    // reset the visible flag
    mMapTipVisible = false;
  }
}

QString QgsMapTip::fetchFeature( QgsMapLayer *layer, QgsPoint & mapPosition, QgsMapCanvas *mpMapCanvas )
{
  // Default return value
  QString maptipText = "";
  // Protection just in case we get passed a null layer
  if ( layer )
  {
    // Get the setting for the search radius from user preferences, if it exists
    QSettings settings;
    double identifyValue = settings.value( "/Map/identifyRadius", QGis::DEFAULT_IDENTIFY_RADIUS ).toDouble();

    // create the search rectangle
    double searchRadius = mpMapCanvas->extent().width() * ( identifyValue / 100.0 );
    QgsRectangle r;
    r.setXMinimum( mapPosition.x() - searchRadius );
    r.setXMaximum( mapPosition.x() + searchRadius );
    r.setYMinimum( mapPosition.y() - searchRadius );
    r.setYMaximum( mapPosition.y() + searchRadius );

    // Get the data provider
    QgsVectorDataProvider* dataProvider = qobject_cast<QgsVectorLayer *>( layer )->dataProvider();
    // Fetch the attribute list for the layer
    QgsAttributeList allAttributes = dataProvider->attributeIndexes();
    // Select all attributes within the search radius
    dataProvider->select( allAttributes, r, true, true );
    // Feature to hold the results of the fetch
    QgsFeature feature;
    // Get the field list for the layer
    const QgsFieldMap& fields = dataProvider->fields();
    // Get the label (display) field for the layer
    QString fieldIndex = qobject_cast<QgsVectorLayer *>( layer )->displayField();
    if ( dataProvider->nextFeature( feature ) )
    {
      // if we get a feature, pull out the display field and set the maptip text to its value
      QgsAttributeMap attributes = feature.attributeMap();
      for ( QgsAttributeMap::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
      {

        if ( fields[it.key()].name() == fieldIndex )
        {
          maptipText = it->toString();

        }

      }
    }
  }
  // return the map tip
  return  maptipText;
}

