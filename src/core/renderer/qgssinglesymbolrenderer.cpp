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

#include <QDomNode>
#include <QImage>
#include <QPainter>
#include <QString>
#include <math.h>

QgsSingleSymbolRenderer::QgsSingleSymbolRenderer( QGis::VectorType type )
{
  mVectorType = type;

  //initial setting based on random color
  QgsSymbol* sy = new QgsSymbol( mVectorType );

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
  sy->setLineWidth( 0.4 );
  mSymbol = sy;
  updateSymbolAttributes();
}

QgsSingleSymbolRenderer::QgsSingleSymbolRenderer( const QgsSingleSymbolRenderer& other )
{
  mVectorType = other.mVectorType;
  mSymbol = new QgsSymbol( *other.mSymbol );
  updateSymbolAttributes();
}

QgsSingleSymbolRenderer& QgsSingleSymbolRenderer::operator=( const QgsSingleSymbolRenderer & other )
{
  if ( this != &other )
  {
    mVectorType = other.mVectorType;
    delete mSymbol;
    mSymbol = new QgsSymbol( *other.mSymbol );
  }
  updateSymbolAttributes();
  return *this;
}

QgsSingleSymbolRenderer::~QgsSingleSymbolRenderer()
{
  delete mSymbol;
}

void QgsSingleSymbolRenderer::addSymbol( QgsSymbol* sy )
{
  delete mSymbol;
  mSymbol = sy;
  updateSymbolAttributes();
}

void QgsSingleSymbolRenderer::renderFeature( QPainter * p, QgsFeature & f, QImage* img, bool selected, double widthScale, double rasterScaleFactor )
{
  // Point
  if ( img && mVectorType == QGis::Point )
  {

    // If scale field is non-negative, use it to scale.
    double fieldScale = 1.0;
    double rotation = 0.0;

    if ( mSymbol->scaleClassificationField() >= 0 )
    {
      //first find out the value for the scale classification attribute
      const QgsAttributeMap& attrs = f.attributeMap();
      fieldScale = sqrt( fabs( attrs[mSymbol->scaleClassificationField()].toDouble() ) );
      QgsDebugMsgLevel( QString( "Feature has field scale factor %1" ).arg( fieldScale ), 3 );
    }
    if ( mSymbol->rotationClassificationField() >= 0 )
    {
      const QgsAttributeMap& attrs = f.attributeMap();
      rotation = attrs[mSymbol->rotationClassificationField()].toDouble();
      QgsDebugMsgLevel( QString( "Feature has rotation factor %1" ).arg( rotation ), 3 );
    }

    *img = mSymbol->getPointSymbolAsImage( widthScale, selected, mSelectionColor, fieldScale, rotation, rasterScaleFactor );
  }


  // Line, polygon
  if ( mVectorType != QGis::Point )
  {
    if ( !selected )
    {
      QPen pen = mSymbol->pen();
      pen.setWidthF( widthScale * pen.widthF() );
      p->setPen( pen );
      p->setBrush( mSymbol->brush() );
    }
    else
    {
      QPen pen = mSymbol->pen();
      pen.setWidthF( widthScale * pen.widthF() );
      // We set pen color in case it is an area with no brush (transparent).
      // Previously, this was only done for lines. Why?
      pen.setColor( mSelectionColor );
      QBrush brush = mSymbol->brush();
      brush.setColor( mSelectionColor );
      p->setPen( pen );
      p->setBrush( brush );
    }
  }
}

void QgsSingleSymbolRenderer::readXML( const QDomNode& rnode, QgsVectorLayer& vl )
{
  mVectorType = vl.vectorType();
  QgsSymbol* sy = new QgsSymbol( mVectorType );

  QDomNode synode = rnode.namedItem( "symbol" );

  if ( synode.isNull() )
  {
    QgsDebugMsg( "No symbol node in project file's renderitem Dom" );
    // XXX abort?
  }
  else
  {
    sy->readXML( synode );
  }
  updateSymbolAttributes();

  //create a renderer and add it to the vector layer
  this->addSymbol( sy );
  vl.setRenderer( this );
}

bool QgsSingleSymbolRenderer::writeXML( QDomNode & layer_node, QDomDocument & document ) const
{
  bool returnval = false;
  QDomElement singlesymbol = document.createElement( "singlesymbol" );
  layer_node.appendChild( singlesymbol );

  if ( mSymbol )
  {
    returnval = mSymbol->writeXML( singlesymbol, document );
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
  int rotationField = mSymbol->rotationClassificationField();
  if ( rotationField >= 0 && !( mSymbolAttributes.contains( rotationField ) ) )
  {
    mSymbolAttributes.append( rotationField );
  }
  int scaleField = mSymbol->scaleClassificationField();
  if ( scaleField >= 0 && !( mSymbolAttributes.contains( scaleField ) ) )
  {
    mSymbolAttributes.append( scaleField );
  }
}

QString QgsSingleSymbolRenderer::name() const
{
  return "Single Symbol";
}

const QList<QgsSymbol*> QgsSingleSymbolRenderer::symbols() const
{
  QList<QgsSymbol*> list;
  list.append( mSymbol );
  return list;
}

QgsRenderer* QgsSingleSymbolRenderer::clone() const
{
  QgsSingleSymbolRenderer* r = new QgsSingleSymbolRenderer( *this );
  return r;
}
