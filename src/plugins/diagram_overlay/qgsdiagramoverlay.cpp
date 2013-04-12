/***************************************************************************
                         qgsdiagramoverlay.cpp  -  description
                         ---------------------
    begin                : January 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdiagramoverlay.h"
#include "qgscoordinatetransform.h"
#include "qgsdiagramfactory.h"
#include "qgsbardiagramfactory.h"
#include "qgspiediagramfactory.h"
#include "qgssvgdiagramfactory.h"
#include "qgsdiagramrenderer.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsmaptopixel.h"
#include "qgsoverlayobject.h"
#include "qgsproject.h"
#include "qgsrendercontext.h"
#include "qgsvectordataprovider.h"
#include "qgslogger.h"

#include <QPainter>
#include <QDomNode>

QgsDiagramOverlay::QgsDiagramOverlay( QgsVectorLayer* vl ): QgsVectorOverlay( vl ), mDiagramRenderer( 0 )
{

}

QgsDiagramOverlay::~QgsDiagramOverlay()
{
  //memory cleanup
  for ( QMap<QgsFeatureId, QgsOverlayObject*>::iterator it = mOverlayObjects.begin(); it != mOverlayObjects.end(); ++it )
  {
    delete it.value();
  }
  delete mDiagramRenderer;
}

void QgsDiagramOverlay::setDiagramRenderer( QgsDiagramRenderer* r )
{
  delete mDiagramRenderer;
  mDiagramRenderer = r;
}

const QgsDiagramRenderer* QgsDiagramOverlay::diagramRenderer() const
{
  return mDiagramRenderer;
}

void QgsDiagramOverlay::createOverlayObjects( const QgsRenderContext& renderContext )
{
  if ( !mDisplayFlag )
  {
    return;
  }

  //memory cleanup
  for ( QMap<QgsFeatureId, QgsOverlayObject*>::iterator it = mOverlayObjects.begin(); it != mOverlayObjects.end(); ++it )
  {
    delete it.value();
  }
  mOverlayObjects.clear();

  //go through all the features and fill the multimap (query mDiagramRenderer for the correct sizes)
  if ( mVectorLayer && mDiagramRenderer )
  {
    //set spatial filter on data provider
    QgsFeatureIterator fit = mVectorLayer->getFeatures( QgsFeatureRequest().setFilterRect( renderContext.extent() ).setSubsetOfAttributes( mAttributes ) );

    QgsFeature currentFeature;
    QgsGeometry* currentGeometry = 0;

    int width, height;

    std::list<unsigned char*> wkbBuffers;
    std::list<int> wkbSizes;

    std::list<unsigned char*>::iterator bufferIt;
    std::list<int>::iterator sizeIt;

    while ( fit.nextFeature( currentFeature ) )
    {
      //todo: insert more objects for multipart features
      if ( mDiagramRenderer->getDiagramDimensions( width, height, currentFeature, renderContext ) != 0 )
      {
        //error
      }

      currentGeometry = currentFeature.geometryAndOwnership();
      //overlay objects needs the geometry in map coordinates
      if ( currentGeometry && renderContext.coordinateTransform() )
      {
        currentGeometry->transform( *( renderContext.coordinateTransform() ) );
      }
      mOverlayObjects.insert( currentFeature.id(), new QgsOverlayObject( width, height, 0, currentGeometry ) );
    }
  }
}

void QgsDiagramOverlay::drawOverlayObjects( QgsRenderContext& context ) const
{
  if ( !mDisplayFlag )
  {
    return;
  }
  if ( mVectorLayer && mDiagramRenderer )
  {
    //set spatial filter on data provider
    QgsFeatureIterator fit = mVectorLayer->getFeatures( QgsFeatureRequest().setFilterRect( context.extent() ).setSubsetOfAttributes( mAttributes ) );

    QgsFeature currentFeature;
    QImage* currentDiagramImage = 0;

    QPainter* painter = context.painter();

    while ( fit.nextFeature( currentFeature ) )
    {
      //request diagram from renderer
      currentDiagramImage = mDiagramRenderer->renderDiagram( currentFeature, context );
      if ( !currentDiagramImage )
      {
        QgsDebugMsg( "diagram image is 0" );
        continue;
      }
      //search for overlay object in the map
      QMap<QgsFeatureId, QgsOverlayObject*>::const_iterator it = mOverlayObjects.find( currentFeature.id() );
      if ( it != mOverlayObjects.constEnd() )
      {
        if ( it.value() )
        {
          QList<QgsPoint> positionList = it.value()->positions();

          QList<QgsPoint>::const_iterator positionIt = positionList.constBegin();
          for ( ; positionIt != positionList.constEnd(); ++positionIt )
          {
            QgsPoint overlayPosition = *positionIt;
            context.mapToPixel().transform( &overlayPosition );
            int shiftX = currentDiagramImage->width() / 2;
            int shiftY = currentDiagramImage->height() / 2;

            if ( painter )
            {
              painter->save();
              painter->scale( 1.0 / context.rasterScaleFactor(), 1.0 / context.rasterScaleFactor() );
              //painter->drawRect(( int )( overlayPosition.x() * context.rasterScaleFactor() ) - shiftX, ( int )( overlayPosition.y() * context.rasterScaleFactor() ) - shiftY, it.value()->width(), it.value()->height());
              painter->drawImage(( int )( overlayPosition.x() * context.rasterScaleFactor() ) - shiftX, ( int )( overlayPosition.y() * context.rasterScaleFactor() ) - shiftY, *currentDiagramImage );
              painter->restore();
            }
          }
        }
      }

      delete currentDiagramImage;
    }
  }
}

int QgsDiagramOverlay::getOverlayObjectSize( int& width, int& height, double value, const QgsFeature& f, const QgsRenderContext& renderContext ) const
{
  Q_UNUSED( value );
  return mDiagramRenderer->getDiagramDimensions( width, height, f, renderContext );
}

bool QgsDiagramOverlay::readXML( const QDomNode& overlayNode )
{
  QDomElement overlayElem = overlayNode.toElement();

  //set display flag
  if ( overlayElem.attribute( "display" ) == "true" )
  {
    mDisplayFlag = true;
  }
  else
  {
    mDisplayFlag = false;
  }

  //create a renderer object
  QgsDiagramRenderer* theDiagramRenderer = 0;
  QDomNodeList rendererList = overlayNode.toElement().elementsByTagName( "renderer" );
  QDomElement rendererElem;

  QString wellKnownName;
  QgsAttributeList attributeList;
  QList<int> classAttrList;

  //classificationField
  QDomNodeList classificationFieldList = overlayElem.elementsByTagName( "scalingAttribute" );
  for ( int i = 0; i < classificationFieldList.size(); ++i )
  {
    bool conversionSuccess = false;
    classificationFieldList.at( i ).toElement().text().toInt( &conversionSuccess );
    if ( conversionSuccess )
    {
      classAttrList.push_back( classificationFieldList.at( i ).toElement().text().toInt() );
    }
  }

  theDiagramRenderer = new QgsDiagramRenderer( classAttrList );

  //in case of well-known diagrams, the category attributes also need to be fetched
  QDomElement categoryElem;
  QDomNodeList categoryList = overlayElem.elementsByTagName( "category" );

  for ( int i = 0; i < categoryList.size(); ++i )
  {
    categoryElem = categoryList.at( i ).toElement();
    attributeList.push_back( categoryElem.attribute( "attribute" ).toInt() );
  }

  if ( rendererList.size() < 1 )
  {
    return false;
  }
  rendererElem = rendererList.at( 0 ).toElement();

  //read settings about factory
  QDomNode factoryNode = overlayElem.namedItem( "factory" );
  if ( factoryNode.isNull() )
  {
    return false;
  }

  QDomElement factoryElem = factoryNode.toElement();
  QString factoryType = factoryElem.attribute( "type" );
  QgsDiagramFactory* newFactory = 0;
  if ( factoryType == "svg" )
  {
    newFactory = new QgsSVGDiagramFactory();
  }
  else if ( factoryType == "Pie" )
  {
    newFactory = new QgsPieDiagramFactory();
  }
  else if ( factoryType == "Bar" )
  {
    newFactory = new QgsBarDiagramFactory();
  }

  if ( !newFactory )
  {
    return false;
  }

  if ( !newFactory->readXML( factoryElem ) )
  {
    delete newFactory;
    return false;
  }
  newFactory->setScalingAttributes( classAttrList );
  theDiagramRenderer->setFactory( newFactory );

  //Read renderer specific settings
  if ( theDiagramRenderer )
  {
    theDiagramRenderer->readXML( rendererElem );
    setDiagramRenderer( theDiagramRenderer );

    //the overlay may need a different attribute list than the renderer
    QList<int>::const_iterator it = classAttrList.constBegin();
    for ( ; it != classAttrList.constEnd(); ++it )
    {
      if ( !attributeList.contains( *it ) )
      {
        attributeList.push_back( *it );
      }
    }
    setAttributes( attributeList );
    return true;
  }
  return false;
}

bool QgsDiagramOverlay::writeXML( QDomNode& layer_node, QDomDocument& doc ) const
{
  QDomElement overlayElement = doc.createElement( "overlay" );
  overlayElement.setAttribute( "type", "diagram" );
  if ( mDisplayFlag )
  {
    overlayElement.setAttribute( "display", "true" );
  }
  else
  {
    overlayElement.setAttribute( "display", "false" );
  }

  layer_node.appendChild( overlayElement );
  if ( mDiagramRenderer )
  {
    mDiagramRenderer->writeXML( overlayElement, doc );
    QgsDiagramFactory* f = mDiagramRenderer->factory();
    if ( f )
    {
      f->writeXML( overlayElement, doc );
    }

    //write classification attributes
    QList<int> scalingAttributes = mDiagramRenderer->classificationAttributes();
    QList<int>::const_iterator it = scalingAttributes.constBegin();
    for ( ; it != scalingAttributes.constEnd(); ++it )
    {
      QDomElement scalingAttributeElem = doc.createElement( "scalingAttribute" );
      QDomText scalingAttributeText = doc.createTextNode( QString::number( *it ) );
      scalingAttributeElem.appendChild( scalingAttributeText );
      overlayElement.appendChild( scalingAttributeElem );
    }
  }
  return true;
}

int QgsDiagramOverlay::createLegendContent( std::list<std::pair<QString, QImage*> >& content ) const
{
  Q_UNUSED( content );
#if 0
  //first make sure the list is clean
  std::list<std::pair<QString, QImage*> >::iterator it;
  for ( it = content.begin(); it != content.end(); ++it )
  {
    delete( it->second );
  }
  content.clear();

  if ( mDiagramRenderer )
  {
    //first item: name of the classification attribute
    QString classificationName = QgsDiagramOverlay::attributeNameFromIndex( mDiagramRenderer->classificationField(), mVectorLayer );
    content.push_back( std::make_pair( classificationName, ( QImage* )0 ) );

    //then a descriptive symbol (must come from diagram renderer)
    QString legendSymbolText;
    QImage* legendSymbolImage = mDiagramRenderer->getLegendImage( legendSymbolText );
    content.push_back( std::make_pair( legendSymbolText, legendSymbolImage ) );

    //then color/attribute pairs
    std::list<QColor> colorList = mDiagramRenderer->colors();
    std::list<QColor>::const_iterator color_it = colorList.begin();
    QgsAttributeList attributeList = mDiagramRenderer->attributes();
    QgsAttributeList::const_iterator att_it = attributeList.begin();
    QString attributeName;
    QImage* colorImage;
    QPainter p;

    for ( ; att_it != attributeList.constEnd() && color_it != colorList.end(); ++color_it, ++att_it )
    {
      colorImage = new QImage( 15, 15, QImage::Format_ARGB32_Premultiplied );
      colorImage->fill( QColor( 255, 255, 255, 0 ).rgba() );
      p.begin( colorImage );
      p.setPen( Qt::NoPen );
      p.setBrush( *color_it );
      p.drawRect( 0, 0, 15, 15 );
      p.end();
      attributeName = QgsDiagramOverlay::attributeNameFromIndex( *att_it, mVectorLayer );
      content.push_back( std::make_pair( attributeName, colorImage ) );
    }



    return 0;
  }
  else
  {
    return 1;
  }
#endif //0
  return 1; //todo: adapt to new design
}

int QgsDiagramOverlay::indexFromAttributeName( const QString& name, const QgsVectorLayer* vl )
{
  int error = -1;

  if ( !vl )
  {
    return error;
  }

  const QgsVectorDataProvider *provider;

  if (( provider = dynamic_cast<const QgsVectorDataProvider *>( vl->dataProvider() ) ) )
  {
    return provider->fieldNameIndex( name );
  }
  return error;
}

QString QgsDiagramOverlay::attributeNameFromIndex( int index, const QgsVectorLayer* vl )
{
  if ( !vl )
  {
    return "";
  }

  const QgsVectorDataProvider *provider;
  if (( provider = dynamic_cast<const QgsVectorDataProvider *>( vl->dataProvider() ) ) )
  {
    const QgsFields & fields = provider->fields();
    if ( index < 0 || index >= fields.count() )
    {
      return fields[index].name();
    }
  }
  return "";
}
