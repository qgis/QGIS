/***************************************************************************
    qgsxmlutils.cpp
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsxmlutils.h"

#include <QDomElement>

#include "qgslogger.h"
#include "qgsrectangle.h"


QgsUnitTypes::DistanceUnit QgsXmlUtils::readMapUnits( const QDomElement& element )
{
  if ( "unknown" == element.text() )
  {
    return QgsUnitTypes::DistanceUnknownUnit;
  }
  else
  {
    QgsUnitTypes::DistanceUnit unit = QgsUnitTypes::decodeDistanceUnit( element.text() );
    return unit == QgsUnitTypes::DistanceUnknownUnit ? QgsUnitTypes::DistanceDegrees : unit;
  }
}

QgsRectangle QgsXmlUtils::readRectangle( const QDomElement& element )
{
  QgsRectangle aoi;

  QDomNode xminNode = element.namedItem( "xmin" );
  QDomNode yminNode = element.namedItem( "ymin" );
  QDomNode xmaxNode = element.namedItem( "xmax" );
  QDomNode ymaxNode = element.namedItem( "ymax" );

  QDomElement exElement = xminNode.toElement();
  double xmin = exElement.text().toDouble();
  aoi.setXMinimum( xmin );

  exElement = yminNode.toElement();
  double ymin = exElement.text().toDouble();
  aoi.setYMinimum( ymin );

  exElement = xmaxNode.toElement();
  double xmax = exElement.text().toDouble();
  aoi.setXMaximum( xmax );

  exElement = ymaxNode.toElement();
  double ymax = exElement.text().toDouble();
  aoi.setYMaximum( ymax );

  return aoi;
}



QDomElement QgsXmlUtils::writeMapUnits( QgsUnitTypes::DistanceUnit units, QDomDocument& doc )
{
  QString unitsString = QgsUnitTypes::encodeUnit( units );
  // maintain compatibility with old projects
  if ( units == QgsUnitTypes::DistanceUnknownUnit )
    unitsString = "unknown";

  QDomElement unitsNode = doc.createElement( "units" );
  unitsNode.appendChild( doc.createTextNode( unitsString ) );
  return unitsNode;
}

QDomElement QgsXmlUtils::writeRectangle( const QgsRectangle& rect, QDomDocument& doc )
{
  QDomElement xMin = doc.createElement( "xmin" );
  QDomElement yMin = doc.createElement( "ymin" );
  QDomElement xMax = doc.createElement( "xmax" );
  QDomElement yMax = doc.createElement( "ymax" );

  QDomText xMinText = doc.createTextNode( qgsDoubleToString( rect.xMinimum() ) );
  QDomText yMinText = doc.createTextNode( qgsDoubleToString( rect.yMinimum() ) );
  QDomText xMaxText = doc.createTextNode( qgsDoubleToString( rect.xMaximum() ) );
  QDomText yMaxText = doc.createTextNode( qgsDoubleToString( rect.yMaximum() ) );

  xMin.appendChild( xMinText );
  yMin.appendChild( yMinText );
  xMax.appendChild( xMaxText );
  yMax.appendChild( yMaxText );

  QDomElement extentNode = doc.createElement( "extent" );
  extentNode.appendChild( xMin );
  extentNode.appendChild( yMin );
  extentNode.appendChild( xMax );
  extentNode.appendChild( yMax );
  return extentNode;
}
