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


QGis::UnitType QgsXmlUtils::readMapUnits( const QDomElement& element )
{
  if ( "meters" == element.text() )
  {
    return QGis::Meters;
  }
  else if ( "feet" == element.text() )
  {
    return QGis::Feet;
  }
  else if ( "nautical miles" == element.text() )
  {
    return QGis::NauticalMiles;
  }
  else if ( "degrees" == element.text() )
  {
    return QGis::Degrees;
  }
  else if ( "unknown" == element.text() )
  {
    return QGis::UnknownUnit;
  }
  else
  {
    QgsDebugMsg( "Unknown map unit type " + element.text() );
    return QGis::Degrees;
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



QDomElement QgsXmlUtils::writeMapUnits( QGis::UnitType units, QDomDocument& doc )
{
  QString unitsString;
  switch ( units )
  {
    case QGis::Meters:
      unitsString = "meters";
      break;
    case QGis::Feet:
      unitsString = "feet";
      break;
    case QGis::NauticalMiles:
      unitsString = "nautical miles";
      break;
    case QGis::Degrees:
      unitsString = "degrees";
      break;
    case QGis::UnknownUnit:
    default:
      unitsString = "unknown";
      break;
  }

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
