/***************************************************************************
    qgsdigitizingguidelayer.cpp
    ----------------------
    begin                : August 2023
    copyright            : (C) Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsdigitizingguidelayer.h"

#include "qgsannotationlineitem.h"
#include "qgsannotationlinetextitem.h"
#include "qgsannotationmarkeritem.h"
#include "qgsannotationpointtextitem.h"
#include "qgscurve.h"
#include "qgsdigitizingguidemodel.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsthreadingutils.h"
#include "qgsapplication.h"
#include "qgsannotationitemregistry.h"
#include "qgsmaplayerfactory.h"


QgsDigitizingGuideLayer::QgsDigitizingGuideLayer( const QString &name, const LayerOptions &options )
  : QgsAnnotationLayer( name, options )
{
  mModel = new QgsDigitizingGuideModel( this );
}

void QgsDigitizingGuideLayer::setEnabled( bool enabled )
{
  // TODO
  setOpacity( enabled ? 1 : 0 );
  triggerRepaint();
}

void QgsDigitizingGuideLayer::addPointGuide( const QgsPoint &point, const QString &title, QList<QgsAnnotationItem *> details, const QDateTime &creation )
{
  QgsAnnotationMarkerItem *guideItem = new QgsAnnotationMarkerItem( point );
  guideItem->setSymbol( pointGuideSymbol() );
  QString guideItemId = addItem( guideItem );

  QString titleItemId;
  if ( !title.isEmpty() )
  {
    QgsAnnotationPointTextItem *titleItem = new QgsAnnotationPointTextItem( title, point );
    titleItem->setEnabled( false );
    titleItemId = addItem( titleItem );
  }

  QStringList detailItemIds = addDetails( details );

  mModel->addPointGuide( guideItemId, title, titleItemId, detailItemIds, creation );
}

void QgsDigitizingGuideLayer::addLineGuide( QgsCurve *curve, const QString &title, QList<QgsAnnotationItem *> details, const QDateTime &creation )
{
  QString titleItemId;
  if ( !title.isEmpty() )
  {
    // cloning first, curve will be transferred in the main annotation
    QgsAnnotationLineTextItem *titleItem = new QgsAnnotationLineTextItem( title, curve->clone() );
    titleItem->setEnabled( false );
    titleItemId = addItem( titleItem );
  }

  QgsAnnotationLineItem *guideItem = new QgsAnnotationLineItem( curve );
  QString guideItemId = addItem( guideItem );

  QStringList detailItemIds = addDetails( details );

  mModel->addLineGuide( guideItemId, title, titleItemId, detailItemIds, creation );
}

QgsAnnotationItem *QgsDigitizingGuideLayer::createDetailsPoint( const QgsPoint &point )
{
  QgsAnnotationMarkerItem *item = new QgsAnnotationMarkerItem( point );
  item->setSymbol( detailsPointSymbol() );
  item->setEnabled( false );
  return item;
}

QgsAnnotationItem *QgsDigitizingGuideLayer::createDetailsLine( QgsCurve *curve )
{
  QgsAnnotationLineItem *item = new QgsAnnotationLineItem( curve );
  item->setSymbol( detailsLineSymbol() );
  item->setEnabled( false );
  return item;
}

QgsAnnotationItem *QgsDigitizingGuideLayer::createDetailsPointTextGuide( const QString &text, const QgsPoint &point, double angle )
{
  QgsAnnotationPointTextItem *item = new QgsAnnotationPointTextItem( text, point );
  item->setAngle( angle );
  item->setRotationMode( Qgis::SymbolRotationMode::RespectMapRotation );
  item->setEnabled( false );
  return item;
}

void QgsDigitizingGuideLayer::setGuideHighlight( const QString &guideId )
{
  if ( !mHighlightItemId.isEmpty() )
    removeItem( mHighlightItemId );

  const QgsAnnotationItem *guide = item( guideId );
  if ( !guide )
    return;

  if ( guide->type() == QStringLiteral( "marker" ) )
  {
    QgsAnnotationMarkerItem *pointGuide = dynamic_cast<QgsAnnotationMarkerItem *>( guide->clone() );
    if ( pointGuide )
    {
      pointGuide = pointGuide->clone();
      pointGuide->setSymbol( highlightedPointGuideSymbol() );
      mHighlightItemId = addItem( pointGuide );
    }
  }
  /*
      if (guide->type() == QStringLiteral( "linestring" ))
      {
          const QgsAnnotationLineItem *lineGuide = dynamic_cast<const QgsAnnotationLineItem*>(guide);
          if (lineGuide)
          {

          }
      }
  */
}

void QgsDigitizingGuideLayer::clear()
{
  mModel->clear();
  QgsAnnotationLayer::clear();
}

std::pair<QList<QgsPointXY>, QList<const QgsCurve *> > QgsDigitizingGuideLayer::guides() const
{
  std::pair<QList<QgsPointXY>, QList<const QgsCurve *> > guides;
  for ( auto it = mModel->mGuides.constBegin(); it != mModel->mGuides.constEnd(); it++ )
  {
    if ( it->mType == QStringLiteral( "point-guide" ) )
    {
      const QgsAnnotationMarkerItem *guideItem = dynamic_cast<const QgsAnnotationMarkerItem *>( item( it->mGuideItemId ) );
      if ( !guideItem )
        continue;
      guides.first.append( guideItem->geometry() );
    }
    else if ( it->mType == QStringLiteral( "line-guide" ) )
    {
      const QgsAnnotationLineItem *guideItem = dynamic_cast<const QgsAnnotationLineItem *>( item( it->mGuideItemId ) );
      if ( !guideItem )
        continue;
      guides.second.append( guideItem->geometry() );
    }
  }
  return guides;
}

bool QgsDigitizingGuideLayer::readXml( const QDomNode &node, QgsReadWriteContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  clear();
  mModel->clear();

  QStringList annotationsItemsIds;

  const QDomNodeList guidesElements = node.toElement().elementsByTagName( QStringLiteral( "guides" ) );

  if ( guidesElements.size() == 0 )
    return false;

  const QDomNodeList itemsList = guidesElements.at( 0 ).childNodes();
  for ( int i = 0; i < itemsList.size(); ++i )
  {
    const QDomElement itemElement = itemsList.at( i ).toElement();
    const QString title = itemElement.attribute( QStringLiteral( "title" ) );
    const QDateTime creation = QDateTime::fromString( itemElement.attribute( QStringLiteral( "creation" ) ), Qt::ISODate );
    const QString type = itemElement.attribute( QStringLiteral( "type" ) );
    const QString wkt = itemElement.attribute( QStringLiteral( "wkt" ) );
    const QgsGeometry geometry = QgsGeometry::fromWkt( wkt );

    QList<QgsAnnotationItem *> detailItems;
    const QDomNodeList detailsElements = itemElement.elementsByTagName( QStringLiteral( "details" ) );
    const QDomNodeList details = detailsElements.at( 0 ).childNodes();
    for ( int j = 0; j < details.size(); ++j )
    {
      const QDomElement detailElement = details.at( j ).toElement();
      const QString detailType = detailElement.attribute( QStringLiteral( "type" ) );
      QgsAnnotationItem *detailItem = QgsApplication::annotationItemRegistry()->createItem( detailType );
      if ( detailItem )
      {
        detailItem->readXml( detailElement, context );
        detailItem->setEnabled( false );
        detailItems << detailItem;
      }
    }

    if ( type == QStringLiteral( "point-guide" ) )
    {
      addPointGuide( QgsPoint( geometry.asPoint() ), title, detailItems, creation );
    }
    else if ( type == QStringLiteral( "line-guide" ) )
    {
      QgsCurve *curve = qgsgeometry_cast< QgsCurve *>( geometry.constGet() );
      if ( curve )
        addLineGuide( curve, title, detailItems, creation );
    }

  }

  triggerRepaint( true );

  return true;
}

bool QgsDigitizingGuideLayer::writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // first get the layer element so that we can append the type attribute
  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() )
  {
    QgsDebugMsgLevel( QStringLiteral( "can't find maplayer node" ), 2 );
    return false;
  }

  mapLayerNode.setAttribute( QStringLiteral( "type" ), QgsMapLayerFactory::typeToString( Qgis::LayerType::Annotation ) );
  QDomElement guidesElement = doc.createElement( QStringLiteral( "guides" ) );

  for ( auto it = mModel->mGuides.constBegin(); it != mModel->mGuides.constEnd(); ++it )
  {
    QDomElement guideElement = doc.createElement( QStringLiteral( "guide" ) );
    guideElement.setAttribute( QStringLiteral( "title" ), it->mTitle );
    guideElement.setAttribute( QStringLiteral( "creation" ), it->mCreation.toString( Qt::DateFormat::ISODate ) );
    guideElement.setAttribute( QStringLiteral( "type" ), it->mType );

    if ( it->mType == QStringLiteral( "point-guide" ) )
    {
      const QgsAnnotationMarkerItem *guideItem = dynamic_cast<const QgsAnnotationMarkerItem *>( item( it->mGuideItemId ) );
      if ( !guideItem )
        continue;
      guideElement.setAttribute( QStringLiteral( "wkt" ), guideItem->geometry().asWkt() );
    }
    else if ( it->mType == QStringLiteral( "line-guide" ) )
    {
      const QgsAnnotationLineItem *guideItem = dynamic_cast<const QgsAnnotationLineItem *>( item( it->mGuideItemId ) );
      if ( !guideItem )
        continue;
      guideElement.setAttribute( QStringLiteral( "wkt" ), guideItem->geometry()->asWkt() );
    }
    else
    {
      continue;
    }

    QDomElement detailsElement = doc.createElement( QStringLiteral( "details" ) );
    for ( const QString &detailId : std::as_const( it->mDetails ) )
    {
      QDomElement detailElement = doc.createElement( QStringLiteral( "detail" ) );

      const QgsAnnotationItem *detailItem = item( detailId );
      if ( !detailItem )
        continue;
      detailElement.setAttribute( QStringLiteral( "type" ), detailItem->type() );
      detailItem->writeXml( detailElement, doc, context );
      detailsElement.appendChild( detailElement );
    }

    guideElement.appendChild( detailsElement );

    guidesElement.appendChild( guideElement );
  }

  QString errorMsg;
  layer_node.appendChild( guidesElement );

  return writeSymbology( layer_node, doc, errorMsg, context );
}

QStringList QgsDigitizingGuideLayer::addDetails( QList<QgsAnnotationItem *> details )
{
  QStringList detailItemIds;
  while ( !details.isEmpty() )
  {
    detailItemIds << addItem( details.takeFirst() );
  }
  return detailItemIds;
}

QgsMarkerSymbol *QgsDigitizingGuideLayer::pointGuideSymbol() const
{
  QgsSimpleMarkerSymbolLayer *markerSymbolLayer = new QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape::Cross );
  markerSymbolLayer->setSize( 3 );
  markerSymbolLayer->setColor( Qt::red );
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol( QgsSymbolLayerList() << markerSymbolLayer );
  return markerSymbol;
}

QgsMarkerSymbol *QgsDigitizingGuideLayer::highlightedPointGuideSymbol() const
{
  QgsSimpleMarkerSymbolLayer *markerSymbolLayer = new QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape::Cross );
  markerSymbolLayer->setSize( 5 );
  markerSymbolLayer->setColor( Qt::blue );
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol( QgsSymbolLayerList() << markerSymbolLayer );
  return markerSymbol;
}

QgsMarkerSymbol *QgsDigitizingGuideLayer::detailsPointSymbol() const
{
  QgsSimpleMarkerSymbolLayer *markerSymbolLayer = new QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape::Cross );
  markerSymbolLayer->setSize( 2 );
  markerSymbolLayer->setColor( Qt::gray );
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol( QgsSymbolLayerList() << markerSymbolLayer );
  return markerSymbol;
}

QgsLineSymbol *QgsDigitizingGuideLayer::lineGuideSymbol() const
{
  QgsSimpleLineSymbolLayer *lineSymbolLayer = new QgsSimpleLineSymbolLayer( QColor( Qt::gray ), 0.1, Qt::PenStyle::DashLine );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol( QgsSymbolLayerList() << lineSymbolLayer );
  return lineSymbol;
}

QgsLineSymbol *QgsDigitizingGuideLayer::highlightedLineGuideSymbol() const
{
  QgsSimpleLineSymbolLayer *lineSymbolLayer = new QgsSimpleLineSymbolLayer( QColor( Qt::blue ), 0.2 );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol( QgsSymbolLayerList() << lineSymbolLayer );
  return lineSymbol;
}

QgsLineSymbol *QgsDigitizingGuideLayer::detailsLineSymbol() const
{
  QgsSimpleLineSymbolLayer *lineSymbolLayer = new QgsSimpleLineSymbolLayer( QColor( Qt::gray ), 0.2, Qt::PenStyle::DashLine );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol( QgsSymbolLayerList() << lineSymbolLayer );
  return lineSymbol;
}
