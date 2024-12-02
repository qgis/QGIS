/***************************************************************************
    qgsannotationmanager.cpp
    ------------------------
    Date                 : January 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationmanager.h"
#include "moc_qgsannotationmanager.cpp"
#include "qgsproject.h"
#include "qgsannotation.h"
#include "qgsannotationregistry.h"
#include "qgsapplication.h"
#include "qgsstyleentityvisitor.h"
#include "qgsannotationitem.h"
#include "qgsannotationlayer.h"
#include "qgssvgannotation.h"
#include "qgsannotationpictureitem.h"
#include "qgsannotationrectangletextitem.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"

QgsAnnotationManager::QgsAnnotationManager( QgsProject *project )
  : QObject( project )
  , mProject( project )
{

}

QgsAnnotationManager::~QgsAnnotationManager()
{
  clear();
}

bool QgsAnnotationManager::addAnnotation( QgsAnnotation *annotation )
{
  if ( !annotation )
    return false;

  if ( mAnnotations.contains( annotation ) )
    return true;

  mAnnotations << annotation;
  emit annotationAdded( annotation );
  mProject->setDirty( true );
  return true;
}

bool QgsAnnotationManager::removeAnnotation( QgsAnnotation *annotation )
{
  if ( !annotation )
    return false;

  if ( !mAnnotations.contains( annotation ) )
    return false;

  emit annotationAboutToBeRemoved( annotation );
  mAnnotations.removeAll( annotation );
  delete annotation;
  emit annotationRemoved();
  mProject->setDirty( true );
  return true;
}

void QgsAnnotationManager::clear()
{
  for ( auto *a : std::as_const( mAnnotations ) )
  {
    removeAnnotation( a );
  }
}

QList<QgsAnnotation *> QgsAnnotationManager::annotations() const
{
  return mAnnotations;
}

QList<QgsAnnotation *> QgsAnnotationManager::cloneAnnotations() const
{
  QList<QgsAnnotation *> results;
  for ( const auto *a : std::as_const( mAnnotations ) )
  {
    results << a->clone();
  }
  return results;
}

bool QgsAnnotationManager::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  return readXmlPrivate( element, context, nullptr, QgsCoordinateTransformContext() );
}

bool QgsAnnotationManager::readXmlAndUpgradeToAnnotationLayerItems( const QDomElement &element, const QgsReadWriteContext &context, QgsAnnotationLayer *layer, const QgsCoordinateTransformContext &transformContext )
{
  return readXmlPrivate( element, context, layer, transformContext );
}

bool QgsAnnotationManager::readXmlPrivate( const QDomElement &element, const QgsReadWriteContext &context, QgsAnnotationLayer *layer, const QgsCoordinateTransformContext &transformContext )
{
  clear();
  //restore each annotation
  bool result = true;

  auto createAnnotationFromElement = [this, &context, layer, &transformContext]( const QDomElement & element )
  {
    std::unique_ptr< QgsAnnotation > annotation( createAnnotationFromXml( element, context ) );
    if ( !annotation )
      return;

    if ( layer )
    {
      std::unique_ptr< QgsAnnotationItem > annotationItem = convertToAnnotationItem( annotation.get(), layer, transformContext );
      if ( annotationItem )
      {
        layer->addItem( annotationItem.release() );
      }
      else
      {
        // could not convert to QgsAnnotationItem, just leave as QgsAnnotation
        addAnnotation( annotation.release() );
      }
    }
    else
    {
      addAnnotation( annotation.release() );
    }
  };

  QDomElement annotationsElem = element.firstChildElement( QStringLiteral( "Annotations" ) );

  QDomElement annotationElement = annotationsElem.firstChildElement( QStringLiteral( "Annotation" ) );
  while ( ! annotationElement.isNull() )
  {
    createAnnotationFromElement( annotationElement );
    annotationElement = annotationElement.nextSiblingElement( QStringLiteral( "Annotation" ) );
  }

  // restore old (pre 3.0) project annotations
  if ( annotationElement.isNull() )
  {
    QDomNodeList oldItemList = element.elementsByTagName( QStringLiteral( "TextAnnotationItem" ) );
    for ( int i = 0; i < oldItemList.size(); ++i )
    {
      createAnnotationFromElement( oldItemList.at( i ).toElement() );
    }
    oldItemList = element.elementsByTagName( QStringLiteral( "FormAnnotationItem" ) );
    for ( int i = 0; i < oldItemList.size(); ++i )
    {
      createAnnotationFromElement( oldItemList.at( i ).toElement() );
    }
    oldItemList = element.elementsByTagName( QStringLiteral( "HtmlAnnotationItem" ) );
    for ( int i = 0; i < oldItemList.size(); ++i )
    {
      createAnnotationFromElement( oldItemList.at( i ).toElement() );
    }
    oldItemList = element.elementsByTagName( QStringLiteral( "SVGAnnotationItem" ) );
    for ( int i = 0; i < oldItemList.size(); ++i )
    {
      createAnnotationFromElement( oldItemList.at( i ).toElement() );
    }
  }

  return result;
}

std::unique_ptr<QgsAnnotationItem> QgsAnnotationManager::convertToAnnotationItem( QgsAnnotation *annotation, QgsAnnotationLayer *layer, const QgsCoordinateTransformContext &transformContext )
{
  auto setCommonProperties = [layer, &transformContext]( const QgsAnnotation * source, QgsAnnotationItem * destination ) -> bool
  {
    destination->setEnabled( source->isVisible() );
    if ( source->hasFixedMapPosition() )
    {
      QgsPointXY mapPosition = source->mapPosition();
      QgsCoordinateTransform transform( source->mapPositionCrs(), layer->crs(), transformContext );
      try
      {
        mapPosition = transform.transform( mapPosition );
      }
      catch ( QgsCsException & )
      {
        QgsDebugError( QStringLiteral( "Error transforming annotation position" ) );
        return false;
      }

      destination->setCalloutAnchor( QgsGeometry::fromPointXY( mapPosition ) );

      std::unique_ptr< QgsBalloonCallout > callout = std::make_unique< QgsBalloonCallout >();
      if ( QgsFillSymbol *fill = source->fillSymbol() )
        callout->setFillSymbol( fill->clone() );

      if ( QgsMarkerSymbol *marker = source->markerSymbol() )
        callout->setMarkerSymbol( marker->clone() );
      callout->setMargins( source->contentsMargin() );
      callout->setMarginsUnit( Qgis::RenderUnit::Millimeters );
      destination->setCallout( callout.release() );
    }

    if ( source->mapLayer() )
      layer->setLinkedVisibilityLayer( source->mapLayer() );

    return true;
  };

  if ( const QgsSvgAnnotation *svg = dynamic_cast< const QgsSvgAnnotation *>( annotation ) )
  {
    QgsPointXY mapPosition = svg->mapPosition();
    QgsCoordinateTransform transform( svg->mapPositionCrs(), layer->crs(), transformContext );
    try
    {
      transform.transform( mapPosition );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "Error transforming annotation position" ) );
    }

    std::unique_ptr< QgsAnnotationPictureItem > item = std::make_unique< QgsAnnotationPictureItem >( Qgis::PictureFormat::SVG,
        svg->filePath(), QgsRectangle::fromCenterAndSize( mapPosition, 1, 1 ) );
    if ( !setCommonProperties( annotation, item.get() ) )
      return nullptr;

    const QgsMargins margins = svg->contentsMargin();
    item->setFixedSize( QSizeF( svg->frameSizeMm().width() - margins.left() - margins.right(),
                                svg->frameSizeMm().height() - margins.top() - margins.bottom() ) );
    item->setFixedSizeUnit( Qgis::RenderUnit::Millimeters );

    if ( svg->hasFixedMapPosition() )
    {
      item->setPlacementMode( Qgis::AnnotationPlacementMode::FixedSize );

      item->setOffsetFromCallout( QSizeF( svg->frameOffsetFromReferencePointMm().x() + margins.left(),
                                          svg->frameOffsetFromReferencePointMm().y() + margins.top() ) );
      item->setOffsetFromCalloutUnit( Qgis::RenderUnit::Millimeters );
    }
    else
    {
      item->setPlacementMode( Qgis::AnnotationPlacementMode::RelativeToMapFrame );
      item->setBounds( QgsRectangle( svg->relativePosition().x(), svg->relativePosition().y(),
                                     svg->relativePosition().x(), svg->relativePosition().y() ) );
      if ( QgsFillSymbol *fill = svg->fillSymbol() )
      {
        item->setBackgroundEnabled( true );
        item->setBackgroundSymbol( fill->clone() );
      }
    }

    return item;
  }
  else if ( const QgsTextAnnotation *text = dynamic_cast< const QgsTextAnnotation *>( annotation ) )
  {
    QgsPointXY mapPosition = text->mapPosition();
    QgsCoordinateTransform transform( text->mapPositionCrs(), layer->crs(), transformContext );
    try
    {
      transform.transform( mapPosition );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "Error transforming annotation position" ) );
    }

    std::unique_ptr< QgsAnnotationRectangleTextItem > item = std::make_unique< QgsAnnotationRectangleTextItem >( text->document()->toHtml(), QgsRectangle::fromCenterAndSize( mapPosition, 1, 1 ) );
    if ( !setCommonProperties( annotation, item.get() ) )
      return nullptr;

    QgsTextFormat format = item->format();
    format.setAllowHtmlFormatting( true );
    item->setFormat( format );

    const QgsMargins margins = text->contentsMargin();
    item->setFixedSize( QSizeF( text->frameSizeMm().width() - margins.left() - margins.right(),
                                text->frameSizeMm().height() - margins.top() - margins.bottom() ) );
    item->setFixedSizeUnit( Qgis::RenderUnit::Millimeters );

    if ( text->hasFixedMapPosition() )
    {
      item->setPlacementMode( Qgis::AnnotationPlacementMode::FixedSize );

      item->setOffsetFromCallout( QSizeF( text->frameOffsetFromReferencePointMm().x() + margins.left(),
                                          text->frameOffsetFromReferencePointMm().y() + margins.top() ) );
      item->setOffsetFromCalloutUnit( Qgis::RenderUnit::Millimeters );
      item->setBackgroundEnabled( false );
      item->setFrameEnabled( false );
    }
    else
    {
      item->setPlacementMode( Qgis::AnnotationPlacementMode::RelativeToMapFrame );
      item->setBounds( QgsRectangle( text->relativePosition().x(), text->relativePosition().y(),
                                     text->relativePosition().x(), text->relativePosition().y() ) );
      if ( QgsFillSymbol *fill = text->fillSymbol() )
      {
        item->setBackgroundEnabled( true );
        item->setBackgroundSymbol( fill->clone() );
      }
    }

    return item;
  }

  return nullptr;
}

QDomElement QgsAnnotationManager::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement annotationsElem = doc.createElement( QStringLiteral( "Annotations" ) );
  QListIterator<QgsAnnotation *> i( mAnnotations );
  // save lowermost annotation (at end of list) first
  i.toBack();
  while ( i.hasPrevious() )
  {
    QgsAnnotation *annotation = i.previous();

    if ( !annotation )
    {
      continue;
    }

    annotation->writeXml( annotationsElem, doc, context );
  }
  return annotationsElem;
}

bool QgsAnnotationManager::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mAnnotations.empty() )
    return true;

  // NOTE: if visitEnter returns false it means "don't visit any annotations", not "abort all further visitations"
  if ( !visitor->visitEnter( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Annotations, QStringLiteral( "annotations" ), tr( "Annotations" ) ) ) )
    return true;

  for ( QgsAnnotation *a : mAnnotations )
  {
    if ( !a->accept( visitor ) )
      return false;
  }

  if ( !visitor->visitExit( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Annotations, QStringLiteral( "annotations" ), tr( "Annotations" ) ) ) )
    return false;

  return true;
}

QgsAnnotation *QgsAnnotationManager::createAnnotationFromXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  QString type = element.tagName();
  QgsAnnotation *annotation = QgsApplication::annotationRegistry()->create( type );
  if ( !annotation )
    return nullptr;

  annotation->readXml( element, context );

  if ( !annotation->mapPositionCrs().isValid() )
  {
    annotation->setMapPositionCrs( mProject->crs() );
  }

  return annotation;
}
