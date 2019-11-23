/***************************************************************************
    qgsprojectviewsettings.cpp
    -----------------------------
    begin                : October 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectviewsettings.h"
#include "qgis.h"
#include <QDomElement>

QgsProjectViewSettings::QgsProjectViewSettings( QObject *parent )
  : QObject( parent )
{

}

void QgsProjectViewSettings::reset()
{
  mDefaultViewExtent = QgsReferencedRectangle();
  if ( mUseProjectScales || !mMapScales.empty() )
  {
    mUseProjectScales = false;
    mMapScales.clear();
    emit mapScalesChanged();
  }
}

QgsReferencedRectangle QgsProjectViewSettings::defaultViewExtent() const
{
  return mDefaultViewExtent;
}

void QgsProjectViewSettings::setDefaultViewExtent( const QgsReferencedRectangle &extent )
{
  mDefaultViewExtent = extent;
}

void QgsProjectViewSettings::setMapScales( const QVector<double> &scales )
{
  // sort scales in descending order
  QVector< double > sorted = scales;
  std::sort( sorted.begin(), sorted.end(), std::greater<double>() );

  if ( sorted == mapScales() )
    return;

  mMapScales = sorted;

  emit mapScalesChanged();
}

QVector<double> QgsProjectViewSettings::mapScales() const
{
  return mMapScales;
}

void QgsProjectViewSettings::setUseProjectScales( bool enabled )
{
  if ( enabled == useProjectScales() )
    return;

  mUseProjectScales = enabled;
  emit mapScalesChanged();
}

bool QgsProjectViewSettings::useProjectScales() const
{
  return mUseProjectScales;
}

bool QgsProjectViewSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  bool useProjectScale = element.attribute( QStringLiteral( "UseProjectScales" ), QStringLiteral( "0" ) ).toInt();

  QDomNodeList scalesNodes = element.elementsByTagName( QStringLiteral( "Scales" ) );
  QVector< double > newScales;
  if ( !scalesNodes.isEmpty() )
  {
    QDomElement scalesElement = scalesNodes.at( 0 ).toElement();
    QDomNodeList scalesNodes = scalesElement.elementsByTagName( QStringLiteral( "Scale" ) );
    for ( int i = 0; i < scalesNodes.count(); i++ )
    {
      QDomElement scaleElement = scalesNodes.at( i ).toElement();
      newScales.append( scaleElement.attribute( QStringLiteral( "Value" ) ).toDouble() );
    }
  }
  if ( useProjectScale != mUseProjectScales || newScales != mMapScales )
  {
    mMapScales = newScales;
    mUseProjectScales = useProjectScale;
    emit mapScalesChanged();
  }

  QDomElement defaultViewElement = element.firstChildElement( QStringLiteral( "DefaultViewExtent" ) );
  if ( !defaultViewElement.isNull() )
  {
    double xMin = defaultViewElement.attribute( QStringLiteral( "xmin" ) ).toDouble();
    double yMin = defaultViewElement.attribute( QStringLiteral( "ymin" ) ).toDouble();
    double xMax = defaultViewElement.attribute( QStringLiteral( "xmax" ) ).toDouble();
    double yMax = defaultViewElement.attribute( QStringLiteral( "ymax" ) ).toDouble();
    QgsCoordinateReferenceSystem crs;
    crs.readXml( defaultViewElement );
    mDefaultViewExtent = QgsReferencedRectangle( QgsRectangle( xMin, yMin, xMax, yMax ), crs );
  }
  else
  {
    mDefaultViewExtent = QgsReferencedRectangle();
  }

  return true;
}

QDomElement QgsProjectViewSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "ProjectViewSettings" ) );
  element.setAttribute( QStringLiteral( "UseProjectScales" ), mUseProjectScales ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  QDomElement scales = doc.createElement( QStringLiteral( "Scales" ) );
  for ( double scale : mMapScales )
  {
    QDomElement scaleElement = doc.createElement( QStringLiteral( "Scale" ) );
    scaleElement.setAttribute( QStringLiteral( "Value" ), qgsDoubleToString( scale ) );
    scales.appendChild( scaleElement );
  }
  element.appendChild( scales );

  if ( !mDefaultViewExtent.isNull() )
  {
    QDomElement defaultViewElement = doc.createElement( QStringLiteral( "DefaultViewExtent" ) );
    defaultViewElement.setAttribute( QStringLiteral( "xmin" ), qgsDoubleToString( mDefaultViewExtent.xMinimum() ) );
    defaultViewElement.setAttribute( QStringLiteral( "ymin" ), qgsDoubleToString( mDefaultViewExtent.yMinimum() ) );
    defaultViewElement.setAttribute( QStringLiteral( "xmax" ), qgsDoubleToString( mDefaultViewExtent.xMaximum() ) );
    defaultViewElement.setAttribute( QStringLiteral( "ymax" ), qgsDoubleToString( mDefaultViewExtent.yMaximum() ) );
    mDefaultViewExtent.crs().writeXml( defaultViewElement, doc );
    element.appendChild( defaultViewElement );
  }

  return element;
}
