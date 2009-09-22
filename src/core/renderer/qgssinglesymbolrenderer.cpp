/***************************************************************************
                         qgssinglesymbolrenderer.cpp  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id: qgssinglesymbolrenderer.cpp 5371 2006-04-25 01:52:13Z wonder $ */

#include "qgis.h"
#include "qgssinglesymbolrenderer.h"

#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgssymbol.h"
#include "qgssymbologyutils.h"
#include "qgsvectorlayer.h"
#include "qgsrendercontext.h"

#include <QDomNode>
#include <QImage>
#include <QPainter>
#include <QString>
#include <math.h>

QgsSingleSymbolRenderer::QgsSingleSymbolRenderer( QGis::GeometryType type )
{
  mGeometryType = type;

  //initial setting based on random color
  QgsSymbol* sy = new QgsSymbol( mGeometryType );

  //random fill colors for points and polygons and pen colors for lines
  int red = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
  int green = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
  int blue = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );

  if ( type == QGis::Line )
  {
    sy->setColor( QColor( red, green, blue ) );
  }
  else
  {
    sy->setFillColor( QColor( red, green, blue ) );
    sy->setFillStyle( Qt::SolidPattern );
    sy->setColor( QColor( 0, 0, 0 ) );
  }
  mSymbol0 = sy;
  mSymbols[ QString()] = sy;
  updateSymbolAttributes();
}

QgsSingleSymbolRenderer::QgsSingleSymbolRenderer( const QgsSingleSymbolRenderer& other )
{
  *this = other;
}

QgsSingleSymbolRenderer& QgsSingleSymbolRenderer::operator=( const QgsSingleSymbolRenderer & other )
{
  if ( this != &other )
  {
    mGeometryType = other.mGeometryType;

    for ( QMap<QString, QgsSymbol *>::const_iterator it = other.mSymbols.begin(); it != other.mSymbols.end(); it++ )
      mSymbols[ it.key()] = new QgsSymbol( *it.value() );

    if ( mSymbols.size() > 0 )
    {
      mSymbol0 = mSymbols[0];
    }
    else
    {
      mSymbol0 = 0;
    }
  }
  updateSymbolAttributes();
  return *this;
}

QgsSingleSymbolRenderer::~QgsSingleSymbolRenderer()
{
  for ( QMap<QString, QgsSymbol *>::iterator it = mSymbols.begin(); it != mSymbols.end(); it++ )
    delete it.value();
}

void QgsSingleSymbolRenderer::addSymbol( QgsSymbol *sy )
{
  for ( QMap<QString, QgsSymbol *>::iterator it = mSymbols.begin(); it != mSymbols.end(); it++ )
    delete it.value();

  mSymbol0 = sy;
  mSymbols[ QString()] = sy;

  updateSymbolAttributes();
}

void QgsSingleSymbolRenderer::renderFeature( QgsRenderContext &renderContext, QgsFeature & f, QImage* img, bool selected, double opacity )
{
  QPainter *p = renderContext.painter();

  // Point
  if ( img && mGeometryType == QGis::Point )
  {

    // If scale field is non-negative, use it to scale.
    double fieldScale = 1.0;
    double rotation = 0.0;
    QgsSymbol *sy = mSymbol0;

    if ( mSymbol0->symbolField() >= 0 )
    {
      const QgsAttributeMap& attrs = f.attributeMap();
      QString name = attrs[ mSymbol0->symbolField()].toString();
      QgsDebugMsgLevel( QString( "Feature has name %1" ).arg( name ), 3 );

      if ( !mSymbols.contains( name ) )
      {
        sy = new QgsSymbol( mGeometryType );
        sy->setNamedPointSymbol( name );
        mSymbols[ name ] = sy;
      }
      else
      {
        sy = mSymbols[ name ];
      }

      sy->setPointSize( mSymbol0->pointSize() );
      sy->setPointSizeUnits( mSymbol0->pointSizeUnits() );
    }

    if ( mSymbol0->scaleClassificationField() >= 0 )
    {
      //first find out the value for the scale classification attribute
      const QgsAttributeMap& attrs = f.attributeMap();
      fieldScale = sqrt( fabs( attrs[ mSymbol0->scaleClassificationField()].toDouble() ) );
      QgsDebugMsgLevel( QString( "Feature has field scale factor %1" ).arg( fieldScale ), 3 );
    }
    if ( mSymbol0->rotationClassificationField() >= 0 )
    {
      const QgsAttributeMap& attrs = f.attributeMap();
      rotation = attrs[ mSymbol0->rotationClassificationField()].toDouble();
      QgsDebugMsgLevel( QString( "Feature has rotation factor %1" ).arg( rotation ), 3 );
    }

    double scale = renderContext.scaleFactor();

    if ( sy->pointSizeUnits() )
    {
      /* Calc scale (still not nice) */
      QgsPoint point;
      point = renderContext.mapToPixel().transform( 0, 0 );
      double x1 = point.x();
      point = renderContext.mapToPixel().transform( 1000, 0 );
      double x2 = point.x();

      scale *= ( x2 - x1 ) * 0.001;
    }

    *img = sy->getPointSymbolAsImage( scale, selected, mSelectionColor, fieldScale, rotation, renderContext.rasterScaleFactor(), opacity );
  }

  // Line, polygon
  if ( mGeometryType != QGis::Point )
  {
    if ( !selected )
    {
      QPen pen = mSymbol0->pen();
      pen.setWidthF( renderContext.scaleFactor() * pen.widthF() );
      p->setPen( pen );

      if ( mGeometryType == QGis::Polygon )
      {
        QBrush brush = mSymbol0->brush();
        scaleBrush( brush, renderContext.rasterScaleFactor() ); //scale brush content for printout
        p->setBrush( brush );
      }
    }
    else
    {
      QPen pen = mSymbol0->pen();
      pen.setWidthF( renderContext.scaleFactor() * pen.widthF() );
      if ( mGeometryType == QGis::Polygon )
      {
        QBrush brush = mSymbol0->brush();
        scaleBrush( brush, renderContext.rasterScaleFactor() ); //scale brush content for printout
        brush.setColor( mSelectionColor );
        p->setBrush( brush );
      }
      else //for lines we draw in selection color
      {
        // We set pen color in case it is an area with no brush (transparent).
        // Previously, this was only done for lines. Why?
        pen.setColor( mSelectionColor );
        p->setPen( pen );
      }
    }
  }
}

int QgsSingleSymbolRenderer::readXML( const QDomNode& rnode, QgsVectorLayer& vl )
{
  mGeometryType = vl.geometryType();
  QgsSymbol* sy = new QgsSymbol( mGeometryType );

  QDomNode synode = rnode.namedItem( "symbol" );

  if ( synode.isNull() )
  {
    QgsDebugMsg( "No symbol node in project file's renderitem Dom" );
    // XXX abort?
  }
  else
  {
    sy->readXML( synode, &vl );
  }
  updateSymbolAttributes();

  //create a renderer and add it to the vector layer
  addSymbol( sy );
  vl.setRenderer( this );
  return 0;
}

bool QgsSingleSymbolRenderer::writeXML( QDomNode & layer_node, QDomDocument & document, const QgsVectorLayer& vl ) const
{
  bool returnval = false;
  QDomElement singlesymbol = document.createElement( "singlesymbol" );
  layer_node.appendChild( singlesymbol );

  if ( mSymbol0 )
  {
    returnval = mSymbol0->writeXML( singlesymbol, document, &vl );
  }
  return returnval;
}


QgsAttributeList QgsSingleSymbolRenderer::classificationAttributes() const
{
  return mSymbolAttributes;
}

void QgsSingleSymbolRenderer::updateSymbolAttributes()
{
  // This function is only called after changing field specifier in the GUI.
  // Timing is not so important.

  mSymbolAttributes.clear();
  int rotationField = mSymbol0->rotationClassificationField();
  if ( rotationField >= 0 && !( mSymbolAttributes.contains( rotationField ) ) )
  {
    mSymbolAttributes.append( rotationField );
  }
  int scaleField = mSymbol0->scaleClassificationField();
  if ( scaleField >= 0 && !( mSymbolAttributes.contains( scaleField ) ) )
  {
    mSymbolAttributes.append( scaleField );
  }
  int symbolField = mSymbol0->symbolField();
  if ( symbolField >= 0 && !( mSymbolAttributes.contains( symbolField ) ) )
  {
    mSymbolAttributes.append( symbolField );
  }
}

QString QgsSingleSymbolRenderer::name() const
{
  return "Single Symbol";
}

const QList<QgsSymbol*> QgsSingleSymbolRenderer::symbols() const
{
  return mSymbols.values();
}

QgsRenderer* QgsSingleSymbolRenderer::clone() const
{
  QgsSingleSymbolRenderer* r = new QgsSingleSymbolRenderer( *this );
  return r;
}
