/***************************************************************************
                         qgsuniquevaluerenderer.cpp  -  description
                             -------------------
    begin                : July 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id: qgsuniquevaluerenderer.cpp 5371 2006-04-25 01:52:13Z wonder $ */

#include "qgsuniquevaluerenderer.h"
#include "qgsfeature.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsrendercontext.h"
#include "qgssymbol.h"
#include "qgssymbologyutils.h"
#include "qgslogger.h"
#include <math.h>
#include <QDomNode>
#include <QPainter>
#include <QImage>
#include <vector>

QgsUniqueValueRenderer::QgsUniqueValueRenderer( QGis::GeometryType type ): mClassificationField( 0 )
{
  mGeometryType = type;
  mSymbolAttributesDirty = false;
}

QgsUniqueValueRenderer::QgsUniqueValueRenderer( const QgsUniqueValueRenderer& other )
{
  mGeometryType = other.mGeometryType;
  mClassificationField = other.mClassificationField;
  QMap<QString, QgsSymbol*> s = other.mSymbols;
  for ( QMap<QString, QgsSymbol*>::iterator it = s.begin(); it != s.end(); ++it )
  {
    QgsSymbol* s = new QgsSymbol( * it.value() );
    insertValue( it.key(), s );
  }
  updateSymbolAttributes();
}

QgsUniqueValueRenderer& QgsUniqueValueRenderer::operator=( const QgsUniqueValueRenderer & other )
{
  if ( this != &other )
  {
    mGeometryType = other.mGeometryType;
    mClassificationField = other.mClassificationField;
    clearValues();
    for ( QMap<QString, QgsSymbol*>::iterator it = mSymbols.begin(); it != mSymbols.end(); ++it )
    {
      QgsSymbol* s = new QgsSymbol( *it.value() );
      insertValue( it.key(), s );
    }
    updateSymbolAttributes();
  }
  return *this;
}

QgsUniqueValueRenderer::~QgsUniqueValueRenderer()
{
  for ( QMap<QString, QgsSymbol*>::iterator it = mSymbols.begin(); it != mSymbols.end(); ++it )
  {
    delete it.value();
  }
}

void QgsUniqueValueRenderer::insertValue( QString name, QgsSymbol* symbol )
{
  mSymbols.insert( name, symbol );
  mSymbolAttributesDirty = true;
}

void QgsUniqueValueRenderer::setClassificationField( int field )
{
  mClassificationField = field;
}

int QgsUniqueValueRenderer::classificationField() const
{
  return mClassificationField;
}

bool QgsUniqueValueRenderer::willRenderFeature( QgsFeature *f )
{
  return ( symbolForFeature( f ) != 0 );
}

void QgsUniqueValueRenderer::renderFeature( QgsRenderContext &renderContext, QgsFeature& f, QImage* img, bool selected, double opacity )
{
  QPainter *p = renderContext.painter();
  QgsSymbol* symbol = symbolForFeature( &f );
  if ( !symbol ) //no matching symbol
  {
    if ( img && mGeometryType == QGis::Point )
    {
      img->fill( 0 );
    }
    else if ( mGeometryType != QGis::Point )
    {
      p->setPen( Qt::NoPen );
      p->setBrush( Qt::NoBrush );
    }
    return;
  }

  // Point
  if ( img && mGeometryType == QGis::Point )
  {
    double fieldScale = 1.0;
    double rotation = 0.0;

    if ( symbol->scaleClassificationField() >= 0 )
    {
      //first find out the value for the scale classification attribute
      const QgsAttributeMap& attrs = f.attributeMap();
      fieldScale = sqrt( fabs( attrs[symbol->scaleClassificationField()].toDouble() ) );
    }
    if ( symbol->rotationClassificationField() >= 0 )
    {
      const QgsAttributeMap& attrs = f.attributeMap();
      rotation = attrs[symbol->rotationClassificationField()].toDouble();
    }

    QString oldName;

    if ( symbol->symbolField() >= 0 )
    {
      const QgsAttributeMap& attrs = f.attributeMap();
      QString name = attrs[symbol->symbolField()].toString();
      oldName = symbol->pointSymbolName();
      symbol->setNamedPointSymbol( name );
    }

    *img = symbol->getPointSymbolAsImage( renderContext.scaleFactor(), selected, mSelectionColor,
                                          fieldScale, rotation, renderContext.rasterScaleFactor(),
                                          opacity );
    if ( !oldName.isNull() )
    {
      symbol->setNamedPointSymbol( oldName );
    }
  }
  // Line, polygon
  else if ( mGeometryType != QGis::Point )
  {
    if ( !selected )
    {
      QPen pen = symbol->pen();
      pen.setWidthF( renderContext.scaleFactor() * pen.widthF() );
      p->setPen( pen );
      if ( mGeometryType == QGis::Polygon )
      {
        QBrush brush = symbol->brush();
        scaleBrush( brush, renderContext.rasterScaleFactor() ); //scale brush content for printout
        p->setBrush( brush );
      }
    }
    else
    {
      QPen pen = symbol->pen();
      pen.setWidthF( renderContext.scaleFactor() * pen.widthF() );
      if ( mGeometryType == QGis::Polygon )
      {
        QBrush brush = symbol->brush();
        scaleBrush( brush, renderContext.rasterScaleFactor() ); //scale brush content for printout
        brush.setColor( mSelectionColor );
        p->setBrush( brush );
      }
      else //don't draw outlines of polygons in selection colour otherwise they appear merged
      {
        pen.setColor( mSelectionColor );
      }
      p->setPen( pen );
    }
  }
}

QgsSymbol *QgsUniqueValueRenderer::symbolForFeature( const QgsFeature *f )
{
  //first find out the value
  const QgsAttributeMap& attrs = f->attributeMap();
  QString value = attrs[mClassificationField].toString();

  QMap<QString, QgsSymbol*>::iterator it = mSymbols.find( value );
  if ( it == mSymbols.end() )
  {
    it = mSymbols.find( QString::null );
  }

  if ( it == mSymbols.end() )
  {
    return 0;
  }
  else
  {
    return it.value();
  }
}

int QgsUniqueValueRenderer::readXML( const QDomNode& rnode, QgsVectorLayer& vl )
{
  mGeometryType = vl.geometryType();
  QDomNode classnode = rnode.namedItem( "classificationfield" );
  QString classificationField = classnode.toElement().text();

  QgsVectorDataProvider* theProvider = vl.dataProvider();
  if ( !theProvider )
  {
    return 1;
  }

  int classificationId = theProvider->fieldNameIndex( classificationField );
  if ( classificationId == -1 )
  {
    return 2; //@todo: handle gracefully in gui situation where user needs to nominate field
  }
  setClassificationField( classificationId );

  QDomNode symbolnode = rnode.namedItem( "symbol" );
  while ( !symbolnode.isNull() )
  {
    QgsSymbol* msy = new QgsSymbol( mGeometryType );
    msy->readXML( symbolnode, &vl );
    insertValue( msy->lowerValue(), msy );
    symbolnode = symbolnode.nextSibling();
  }
  updateSymbolAttributes();
  vl.setRenderer( this );
  return 0;
}

void QgsUniqueValueRenderer::clearValues()
{
  for ( QMap<QString, QgsSymbol*>::iterator it = mSymbols.begin(); it != mSymbols.end(); ++it )
  {
    delete it.value();
  }
  mSymbols.clear();
  updateSymbolAttributes();
}

void QgsUniqueValueRenderer::updateSymbolAttributes()
{
  mSymbolAttributesDirty = false;

  mSymbolAttributes.clear();

  QMap<QString, QgsSymbol*>::iterator it;
  for ( it = mSymbols.begin(); it != mSymbols.end(); ++it )
  {
    int rotationField = ( *it )->rotationClassificationField();
    if ( rotationField >= 0 && !( mSymbolAttributes.contains( rotationField ) ) )
    {
      mSymbolAttributes.append( rotationField );
    }
    int scaleField = ( *it )->scaleClassificationField();
    if ( scaleField >= 0 && !( mSymbolAttributes.contains( scaleField ) ) )
    {
      mSymbolAttributes.append( scaleField );
    }
    int symbolField = ( *it )->symbolField();
    if ( symbolField >= 0 && !mSymbolAttributes.contains( symbolField ) )
    {
      mSymbolAttributes.append( symbolField );
    }
  }
}

QString QgsUniqueValueRenderer::name() const
{
  return "Unique Value";
}

QgsAttributeList QgsUniqueValueRenderer::classificationAttributes() const
{
  QgsAttributeList list( mSymbolAttributes );
  if ( ! list.contains( mClassificationField ) )
  {
    list.append( mClassificationField );
  }
  return list;
}

bool QgsUniqueValueRenderer::writeXML( QDomNode & layer_node, QDomDocument & document, const QgsVectorLayer& vl ) const
{
  const QgsVectorDataProvider* theProvider = vl.dataProvider();
  if ( !theProvider )
  {
    return false;
  }

  QString classificationFieldName;
  QgsFieldMap::const_iterator field_it = theProvider->fields().find( mClassificationField );
  if ( field_it != theProvider->fields().constEnd() )
  {
    classificationFieldName = field_it.value().name();
  }

  bool returnval = true;
  QDomElement uniquevalue = document.createElement( "uniquevalue" );
  layer_node.appendChild( uniquevalue );
  QDomElement classificationfield = document.createElement( "classificationfield" );
  QDomText classificationfieldtxt = document.createTextNode( classificationFieldName );
  classificationfield.appendChild( classificationfieldtxt );
  uniquevalue.appendChild( classificationfield );
  for ( QMap<QString, QgsSymbol*>::const_iterator it = mSymbols.begin(); it != mSymbols.end(); ++it )
  {
    if ( !( it.value()->writeXML( uniquevalue, document, &vl ) ) )
    {
      returnval = false;
    }
  }
  return returnval;
}

QgsRenderer* QgsUniqueValueRenderer::clone() const
{
  QgsUniqueValueRenderer* r = new QgsUniqueValueRenderer( *this );
  return r;
}
