/***************************************************************************
                          qgsprojectfiletransform.cpp  -  description
                             -------------------
    begin                : Sun 15 dec 2007
    copyright            : (C) 2007 by Magnus Homann
    email                : magnus at homann.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsprojectfiletransform.h"
#include "qgsprojectversion.h"
#include "qgslogger.h"
#include "qgsrasterlayer.h"
#include "qgsreadwritecontext.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QTextStream>
#include <QDomDocument>
#include <QPrinter> //to find out screen resolution
#include <cstdlib>
#include "qgspathresolver.h"
#include "qgsproject.h"
#include "qgsprojectproperty.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterdataprovider.h"
#include "qgsxmlutils.h"

typedef QgsProjectVersion PFV;


QgsProjectFileTransform::TransformItem QgsProjectFileTransform::sTransformers[] =
{
  {PFV( 0, 8, 0 ), PFV( 0, 8, 1 ), &QgsProjectFileTransform::transformNull},
  {PFV( 0, 8, 1 ), PFV( 0, 9, 0 ), &QgsProjectFileTransform::transform081to090},
  {PFV( 0, 9, 0 ), PFV( 0, 9, 1 ), &QgsProjectFileTransform::transformNull},
  {PFV( 0, 9, 1 ), PFV( 0, 10, 0 ), &QgsProjectFileTransform::transform091to0100},
  // Following line is a hack that takes us straight from 0.9.2 to 0.11.0
  // due to an unknown bug in migrating 0.9.2 files which we didn't pursue (TS & GS)
  {PFV( 0, 9, 2 ), PFV( 0, 11, 0 ), &QgsProjectFileTransform::transformNull},
  {PFV( 0, 10, 0 ), PFV( 0, 11, 0 ), &QgsProjectFileTransform::transform0100to0110},
  {PFV( 0, 11, 0 ), PFV( 1, 0, 0 ), &QgsProjectFileTransform::transform0110to1000},
  {PFV( 1, 0, 0 ), PFV( 1, 1, 0 ), &QgsProjectFileTransform::transformNull},
  {PFV( 1, 0, 2 ), PFV( 1, 1, 0 ), &QgsProjectFileTransform::transformNull},
  {PFV( 1, 1, 0 ), PFV( 1, 2, 0 ), &QgsProjectFileTransform::transform1100to1200},
  {PFV( 1, 2, 0 ), PFV( 1, 3, 0 ), &QgsProjectFileTransform::transformNull},
  {PFV( 1, 3, 0 ), PFV( 1, 4, 0 ), &QgsProjectFileTransform::transformNull},
  {PFV( 1, 4, 0 ), PFV( 1, 5, 0 ), &QgsProjectFileTransform::transform1400to1500},
  {PFV( 1, 5, 0 ), PFV( 1, 6, 0 ), &QgsProjectFileTransform::transformNull},
  {PFV( 1, 6, 0 ), PFV( 1, 7, 0 ), &QgsProjectFileTransform::transformNull},
  {PFV( 1, 7, 0 ), PFV( 1, 8, 0 ), &QgsProjectFileTransform::transformNull},
  {PFV( 1, 8, 0 ), PFV( 1, 9, 0 ), &QgsProjectFileTransform::transform1800to1900},
  {PFV( 1, 9, 0 ), PFV( 2, 0, 0 ), &QgsProjectFileTransform::transformNull},
  {PFV( 2, 0, 0 ), PFV( 2, 1, 0 ), &QgsProjectFileTransform::transformNull},
  {PFV( 2, 1, 0 ), PFV( 2, 2, 0 ), &QgsProjectFileTransform::transformNull},
  {PFV( 2, 2, 0 ), PFV( 2, 3, 0 ), &QgsProjectFileTransform::transform2200to2300},
  // A transformer with a NULL from version means that it should be run when upgrading
  // from any version and will take care that it's not going to cause trouble if it's
  // run several times on the same file.
  {PFV(), PFV( 2, 99, 0 ), &QgsProjectFileTransform::transform2990},
};

bool QgsProjectFileTransform::updateRevision( const QgsProjectVersion &newVersion )
{
  Q_UNUSED( newVersion );
  bool returnValue = false;

  if ( !mDom.isNull() )
  {
    for ( std::size_t i = 0; i < sizeof( sTransformers ) / sizeof( TransformItem ); i++ )
    {
      const TransformItem &transformer = sTransformers[i];
      if ( transformer.from == mCurrentVersion || transformer.from.isNull() )
      {
        // Run the transformer, and update the revision in every case
        ( this->*( transformer.transformFunc ) )();
        mCurrentVersion = transformer.to;
        returnValue = true;
      }
    }
  }
  return returnValue;
}

void QgsProjectFileTransform::dump()
{
  QgsDebugMsg( QString( "Current project file version is %1.%2.%3" )
               .arg( mCurrentVersion.majorVersion() )
               .arg( mCurrentVersion.minorVersion() )
               .arg( mCurrentVersion.subVersion() ) );
#ifdef QGISDEBUG
  // Using QgsDebugMsg() didn't print the entire mDom...
  std::cout << mDom.toString( 2 ).toLatin1().constData(); // OK
#endif
}

/*
 *  Transformers below!
 */

void QgsProjectFileTransform::transform081to090()
{
  QgsDebugMsg( "Entering..." );
  if ( ! mDom.isNull() )
  {
    // Start with inserting a mapcanvas element and populate it

    QDomElement mapCanvas; // A null element.

    // there should only be one <qgis>
    QDomNode qgis = mDom.firstChildElement( QStringLiteral( "qgis" ) );
    if ( ! qgis.isNull() )
    {
      QgsDebugMsg( "Populating new mapcanvas" );

      // Create a mapcanvas
      mapCanvas = mDom.createElement( QStringLiteral( "mapcanvas" ) );
      // Append mapcanvas to parent 'qgis'.
      qgis.appendChild( mapCanvas );
      // Re-parent units
      mapCanvas.appendChild( qgis.namedItem( QStringLiteral( "units" ) ) );
      // Re-parent extent
      mapCanvas.appendChild( qgis.namedItem( QStringLiteral( "extent" ) ) );

      // See if we can find if projection is on.

      QDomElement properties = qgis.firstChildElement( QStringLiteral( "properties" ) );
      QDomElement spatial = properties.firstChildElement( QStringLiteral( "SpatialRefSys" ) );
      QDomElement hasCrsTransformEnabled = spatial.firstChildElement( QStringLiteral( "ProjectionsEnabled" ) );
      // Type is 'int', and '1' if on.
      // Create an element
      QDomElement projection = mDom.createElement( QStringLiteral( "projections" ) );
      QgsDebugMsg( QString( "Projection flag: " ) + hasCrsTransformEnabled.text() );
      // Set flag from ProjectionsEnabled
      projection.appendChild( mDom.createTextNode( hasCrsTransformEnabled.text() ) );
      // Set new element as child of <mapcanvas>
      mapCanvas.appendChild( projection );

    }


    // Transforming coordinate-transforms
    // Create a list of all map layers
    QDomNodeList mapLayers = mDom.elementsByTagName( QStringLiteral( "maplayer" ) );
    bool doneDestination = false;
    for ( int i = 0; i < mapLayers.count(); i++ )
    {
      QDomNode mapLayer = mapLayers.item( i );
      // Find the coordinatetransform
      QDomNode coordinateTransform = mapLayer.namedItem( QStringLiteral( "coordinatetransform" ) );
      // Find the sourcesrs
      QDomNode sourceCrs = coordinateTransform.namedItem( QStringLiteral( "sourcesrs" ) );
      // Rename to srs
      sourceCrs.toElement().setTagName( QStringLiteral( "srs" ) );
      // Re-parent to maplayer
      mapLayer.appendChild( sourceCrs );
      // Re-move coordinatetransform
      // Take the destination CRS of the first layer and use for mapcanvas projection
      if ( ! doneDestination )
      {
        // Use destination CRS from the last layer
        QDomNode destinationCRS = coordinateTransform.namedItem( QStringLiteral( "destinationsrs" ) );
        // Re-parent the destination CRS to the mapcanvas
        // If mapcanvas wasn't set, nothing will happen.
        mapCanvas.appendChild( destinationCRS );
        // Only do this once
        doneDestination = true;
      }
      mapLayer.removeChild( coordinateTransform );
      //QDomNode id = mapLayer.namedItem("id");
      //QgsDebugMsg(QString("Found maplayer ") + id.toElement().text());

    }

    // Set the flag 'visible' to match the status of 'checked'
    QDomNodeList legendLayerFiles = mDom.elementsByTagName( QStringLiteral( "legendlayerfile" ) );
    QgsDebugMsg( QString( "Legend layer file entries: " ) + QString::number( legendLayerFiles.count() ) );
    for ( int i = 0; i < mapLayers.count(); i++ )
    {
      // Get one maplayer element from list
      QDomElement mapLayer = mapLayers.item( i ).toElement();
      // Find it's id.
      QString id = mapLayer.firstChildElement( QStringLiteral( "id" ) ).text();
      QgsDebugMsg( QString( "Handling layer " + id ) );
      // Now, look it up in legend
      for ( int j = 0; j < legendLayerFiles.count(); j++ )
      {
        QDomElement legendLayerFile = legendLayerFiles.item( j ).toElement();
        if ( id == legendLayerFile.attribute( QStringLiteral( "layerid" ) ) )
        {
          // Found a the legend layer that matches the maplayer
          QgsDebugMsg( "Found matching id" );

          // Set visible flag from maplayer to legendlayer
          legendLayerFile.setAttribute( QStringLiteral( "visible" ), mapLayer.attribute( QStringLiteral( "visible" ) ) );

          // Set overview flag from maplayer to legendlayer
          legendLayerFile.setAttribute( QStringLiteral( "isInOverview" ), mapLayer.attribute( QStringLiteral( "showInOverviewFlag" ) ) );
        }
      }
    }
  }
}

void QgsProjectFileTransform::transform091to0100()
{
  if ( ! mDom.isNull() )
  {
    // Insert transforms here!
    QDomNodeList rasterPropertyList = mDom.elementsByTagName( QStringLiteral( "rasterproperties" ) );
    QgsDebugMsg( QString( "Raster properties file entries: " ) + QString::number( rasterPropertyList.count() ) );
    for ( int i = 0; i < rasterPropertyList.count(); i++ )
    {
      // Get one rasterproperty element from list, and rename the sub-properties.
      QDomNode rasterProperty = rasterPropertyList.item( i );
      // rasterProperty.namedItem("").toElement().setTagName("");

      rasterProperty.namedItem( QStringLiteral( "stdDevsToPlotDouble" ) ).toElement().setTagName( QStringLiteral( "mStandardDeviations" ) );

      rasterProperty.namedItem( QStringLiteral( "invertHistogramFlag" ) ).toElement().setTagName( QStringLiteral( "mInvertPixelsFlag" ) );
      rasterProperty.namedItem( QStringLiteral( "showDebugOverLayFlag" ) ).toElement().setTagName( QStringLiteral( "mDebugOverLayFlag" ) );

      rasterProperty.namedItem( QStringLiteral( "redBandNameQString" ) ).toElement().setTagName( QStringLiteral( "mRedBandName" ) );
      rasterProperty.namedItem( QStringLiteral( "blueBandNameQString" ) ).toElement().setTagName( QStringLiteral( "mBlueBandName" ) );
      rasterProperty.namedItem( QStringLiteral( "greenBandNameQString" ) ).toElement().setTagName( QStringLiteral( "mGreenBandName" ) );
      rasterProperty.namedItem( QStringLiteral( "grayBandNameQString" ) ).toElement().setTagName( QStringLiteral( "mGrayBandName" ) );
    }

    // Changing symbol size for hard: symbols
    QDomNodeList symbolPropertyList = mDom.elementsByTagName( QStringLiteral( "symbol" ) );
    for ( int i = 0; i < symbolPropertyList.count(); i++ )
    {
      // Get the <poinmtsymbol> to check for 'hard:' for each <symbol>
      QDomNode symbolProperty = symbolPropertyList.item( i );

      QDomElement pointSymbol = symbolProperty.firstChildElement( QStringLiteral( "pointsymbol" ) );
      if ( pointSymbol.text().startsWith( QLatin1String( "hard:" ) ) )
      {
        // Get pointsize and line width
        int lineWidth = symbolProperty.firstChildElement( QStringLiteral( "outlinewidth" ) ).text().toInt();
        int pointSize = symbolProperty.firstChildElement( QStringLiteral( "pointsize" ) ).text().toInt();
        // Just a precaution, checking for 0
        if ( pointSize != 0 )
        {
          // int r = (s-2*lw)/2-1 --> 2r = (s-2*lw)-2 --> 2r+2 = s-2*lw
          // --> 2r+2+2*lw = s
          // where '2r' is the old size.
          pointSize = pointSize + 2 + 2 * lineWidth;
          QgsDebugMsg( QString( "Setting point size to %1" ).arg( pointSize ) );
          QDomElement newPointSizeProperty = mDom.createElement( QStringLiteral( "pointsize" ) );
          QDomText newPointSizeTxt = mDom.createTextNode( QString::number( pointSize ) );
          newPointSizeProperty.appendChild( newPointSizeTxt );
          symbolProperty.replaceChild( newPointSizeProperty, pointSymbol );
        }
      }
    }

  }
}

void QgsProjectFileTransform::transform0100to0110()
{
  if ( ! mDom.isNull() )
  {
#ifndef QT_NO_PRINTER
    //Change 'outlinewidth' in QgsSymbol
    QPrinter myPrinter( QPrinter::ScreenResolution );
    int screenDpi = myPrinter.resolution();
    double widthScaleFactor = 25.4 / screenDpi;

    QDomNodeList outlineWidthList = mDom.elementsByTagName( QStringLiteral( "outlinewidth" ) );
    for ( int i = 0; i < outlineWidthList.size(); ++i )
    {
      //calculate new width
      QDomElement currentOutlineElem = outlineWidthList.at( i ).toElement();
      double outlineWidth = currentOutlineElem.text().toDouble();
      outlineWidth *= widthScaleFactor;

      //replace old text node
      QDomNode outlineTextNode = currentOutlineElem.firstChild();
      QDomText newOutlineText = mDom.createTextNode( QString::number( outlineWidth ) );
      currentOutlineElem.replaceChild( newOutlineText, outlineTextNode );

    }

    //Change 'pointsize' in QgsSymbol
    QDomNodeList pointSizeList = mDom.elementsByTagName( QStringLiteral( "pointsize" ) );
    for ( int i = 0; i < pointSizeList.size(); ++i )
    {
      //calculate new size
      QDomElement currentPointSizeElem = pointSizeList.at( i ).toElement();
      double pointSize = currentPointSizeElem.text().toDouble();
      pointSize *= widthScaleFactor;

      //replace old text node
      QDomNode pointSizeTextNode = currentPointSizeElem.firstChild();
      QDomText newPointSizeText = mDom.createTextNode( QString::number( static_cast< int >( pointSize ) ) );
      currentPointSizeElem.replaceChild( newPointSizeText, pointSizeTextNode );
    }
#endif
  }
}

void QgsProjectFileTransform::transform0110to1000()
{
  if ( ! mDom.isNull() )
  {
    QDomNodeList layerList = mDom.elementsByTagName( QStringLiteral( "maplayer" ) );
    for ( int i = 0; i < layerList.size(); ++i )
    {
      QDomElement layerElem = layerList.at( i ).toElement();
      QString typeString = layerElem.attribute( QStringLiteral( "type" ) );
      if ( typeString != QLatin1String( "vector" ) )
      {
        continue;
      }

      //datasource
      QDomNode dataSourceNode = layerElem.namedItem( QStringLiteral( "datasource" ) );
      if ( dataSourceNode.isNull() )
      {
        return;
      }
      QString dataSource = dataSourceNode.toElement().text();

      //provider key
      QDomNode providerNode = layerElem.namedItem( QStringLiteral( "provider" ) );
      if ( providerNode.isNull() )
      {
        return;
      }
      QString providerKey = providerNode.toElement().text();

      //create the layer to get the provider for int->fieldName conversion
      QgsVectorLayer::LayerOptions options;
      options.loadDefaultStyle = false;
      QgsVectorLayer *layer = new QgsVectorLayer( dataSource, QLatin1String( "" ), providerKey, options );
      if ( !layer->isValid() )
      {
        delete layer;
        return;
      }

      QgsVectorDataProvider *provider = layer->dataProvider();
      if ( !provider )
      {
        return;
      }
      QgsFields fields = provider->fields();

      //read classificationfield
      QDomNodeList classificationFieldList = layerElem.elementsByTagName( QStringLiteral( "classificationfield" ) );
      for ( int j = 0; j < classificationFieldList.size(); ++j )
      {
        QDomElement classificationFieldElem = classificationFieldList.at( j ).toElement();
        int fieldNumber = classificationFieldElem.text().toInt();
        if ( fieldNumber >= 0 && fieldNumber < fields.count() )
        {
          QDomText fieldName = mDom.createTextNode( fields.at( fieldNumber ).name() );
          QDomNode nameNode = classificationFieldElem.firstChild();
          classificationFieldElem.replaceChild( fieldName, nameNode );
        }
      }

    }
  }
}

void QgsProjectFileTransform::transform1100to1200()
{
  QgsDebugMsg( "Entering..." );
  if ( mDom.isNull() )
    return;

  QDomNode qgis = mDom.firstChildElement( QStringLiteral( "qgis" ) );
  if ( qgis.isNull() )
    return;

  QDomElement properties = qgis.firstChildElement( QStringLiteral( "properties" ) );
  if ( properties.isNull() )
    return;

  QDomElement digitizing = properties.firstChildElement( QStringLiteral( "Digitizing" ) );
  if ( digitizing.isNull() )
    return;

  QDomElement tolList = digitizing.firstChildElement( QStringLiteral( "LayerSnappingToleranceList" ) );
  if ( tolList.isNull() )
    return;

  QDomElement tolUnitList = digitizing.firstChildElement( QStringLiteral( "LayerSnappingToleranceUnitList" ) );
  if ( !tolUnitList.isNull() )
    return;

  QStringList units;
  for ( int i = 0; i < tolList.childNodes().count(); i++ )
    units << QStringLiteral( "0" );

  QgsProjectPropertyValue value( units );
  value.writeXml( QStringLiteral( "LayerSnappingToleranceUnitList" ), digitizing, mDom );
}

void QgsProjectFileTransform::transform1400to1500()
{
  //Adapt the XML description of the composer legend model to version 1.5
  if ( mDom.isNull() )
  {
    return;
  }
  //Add layer id to <VectorClassificationItem>
  QDomNodeList layerItemList = mDom.elementsByTagName( QStringLiteral( "LayerItem" ) );
  QDomElement currentLayerItemElem;
  QString currentLayerId;

  for ( int i = 0; i < layerItemList.size(); ++i )
  {
    currentLayerItemElem = layerItemList.at( i ).toElement();
    if ( currentLayerItemElem.isNull() )
    {
      continue;
    }
    currentLayerId = currentLayerItemElem.attribute( QStringLiteral( "layerId" ) );

    QDomNodeList vectorClassificationList = currentLayerItemElem.elementsByTagName( QStringLiteral( "VectorClassificationItem" ) );
    QDomElement currentClassificationElem;
    for ( int j = 0; j < vectorClassificationList.size(); ++j )
    {
      currentClassificationElem = vectorClassificationList.at( j ).toElement();
      if ( !currentClassificationElem.isNull() )
      {
        currentClassificationElem.setAttribute( QStringLiteral( "layerId" ), currentLayerId );
      }
    }

    //replace the text items with VectorClassification or RasterClassification items
    QDomNodeList textItemList = currentLayerItemElem.elementsByTagName( QStringLiteral( "TextItem" ) );
    QDomElement currentTextItem;

    for ( int j = 0; j < textItemList.size(); ++j )
    {
      currentTextItem = textItemList.at( j ).toElement();
      if ( currentTextItem.isNull() )
      {
        continue;
      }

      QDomElement classificationElement;
      if ( !vectorClassificationList.isEmpty() ) //we guess it is a vector layer
      {
        classificationElement = mDom.createElement( QStringLiteral( "VectorClassificationItem" ) );
      }
      else
      {
        classificationElement = mDom.createElement( QStringLiteral( "RasterClassificationItem" ) );
      }

      classificationElement.setAttribute( QStringLiteral( "layerId" ), currentLayerId );
      classificationElement.setAttribute( QStringLiteral( "text" ), currentTextItem.attribute( QStringLiteral( "text" ) ) );
      currentLayerItemElem.replaceChild( classificationElement, currentTextItem );
    }
  }
}

void QgsProjectFileTransform::transform1800to1900()
{
  if ( mDom.isNull() )
  {
    return;
  }

  QgsReadWriteContext context;
  context.setPathResolver( QgsProject::instance()->pathResolver() );

  QDomNodeList layerItemList = mDom.elementsByTagName( QStringLiteral( "rasterproperties" ) );
  for ( int i = 0; i < layerItemList.size(); ++i )
  {
    QDomElement rasterPropertiesElem = layerItemList.at( i ).toElement();
    QDomNode layerNode = rasterPropertiesElem.parentNode();
    QDomElement dataSourceElem = layerNode.firstChildElement( QStringLiteral( "datasource" ) );
    QDomElement layerNameElem = layerNode.firstChildElement( QStringLiteral( "layername" ) );
    QgsRasterLayer rasterLayer;
    // TODO: We have to use more data from project file to read the layer it correctly,
    // OTOH, we should not read it until it was converted
    rasterLayer.readLayerXml( layerNode.toElement(), context );
    convertRasterProperties( mDom, layerNode, rasterPropertiesElem, &rasterLayer );
  }

  //composer: replace mGridAnnotationPosition with mLeftGridAnnotationPosition & co.
  // and mGridAnnotationDirection with mLeftGridAnnotationDirection & co.
  QDomNodeList composerMapList = mDom.elementsByTagName( QStringLiteral( "ComposerMap" ) );
  for ( int i = 0; i < composerMapList.size(); ++i )
  {
    QDomNodeList gridList = composerMapList.at( i ).toElement().elementsByTagName( QStringLiteral( "Grid" ) );
    for ( int j = 0; j < gridList.size(); ++j )
    {
      QDomNodeList annotationList = gridList.at( j ).toElement().elementsByTagName( QStringLiteral( "Annotation" ) );
      for ( int k = 0; k < annotationList.size(); ++k )
      {
        QDomElement annotationElem = annotationList.at( k ).toElement();

        //position
        if ( annotationElem.hasAttribute( QStringLiteral( "position" ) ) )
        {
          int pos = annotationElem.attribute( QStringLiteral( "position" ) ).toInt();
          annotationElem.setAttribute( QStringLiteral( "leftPosition" ), pos );
          annotationElem.setAttribute( QStringLiteral( "rightPosition" ), pos );
          annotationElem.setAttribute( QStringLiteral( "topPosition" ), pos );
          annotationElem.setAttribute( QStringLiteral( "bottomPosition" ), pos );
          annotationElem.removeAttribute( QStringLiteral( "position" ) );
        }

        //direction
        if ( annotationElem.hasAttribute( QStringLiteral( "direction" ) ) )
        {
          int dir = annotationElem.attribute( QStringLiteral( "direction" ) ).toInt();
          if ( dir == 2 )
          {
            annotationElem.setAttribute( QStringLiteral( "leftDirection" ), 0 );
            annotationElem.setAttribute( QStringLiteral( "rightDirection" ), 0 );
            annotationElem.setAttribute( QStringLiteral( "topDirection" ), 1 );
            annotationElem.setAttribute( QStringLiteral( "bottomDirection" ), 1 );
          }
          else if ( dir == 3 )
          {
            annotationElem.setAttribute( QStringLiteral( "leftDirection" ), 1 );
            annotationElem.setAttribute( QStringLiteral( "rightDirection" ), 1 );
            annotationElem.setAttribute( QStringLiteral( "topDirection" ), 0 );
            annotationElem.setAttribute( QStringLiteral( "bottomDirection" ), 0 );
          }
          else
          {
            annotationElem.setAttribute( QStringLiteral( "leftDirection" ), dir );
            annotationElem.setAttribute( QStringLiteral( "rightDirection" ), dir );
            annotationElem.setAttribute( QStringLiteral( "topDirection" ), dir );
            annotationElem.setAttribute( QStringLiteral( "bottomDirection" ), dir );
          }
          annotationElem.removeAttribute( QStringLiteral( "direction" ) );
        }
      }
    }
  }

  //Composer: move all items under Composition element
  QDomNodeList composerList = mDom.elementsByTagName( QStringLiteral( "Composer" ) );
  for ( int i = 0; i < composerList.size(); ++i )
  {
    QDomElement composerElem = composerList.at( i ).toElement();

    //find <QgsComposition element
    QDomElement compositionElem = composerElem.firstChildElement( QStringLiteral( "Composition" ) );
    if ( compositionElem.isNull() )
    {
      continue;
    }

    QDomNodeList composerChildren = composerElem.childNodes();

    if ( composerChildren.size() < 1 )
    {
      continue;
    }

    for ( int j = composerChildren.size() - 1; j >= 0; --j )
    {
      QDomElement childElem = composerChildren.at( j ).toElement();
      if ( childElem.tagName() == QLatin1String( "Composition" ) )
      {
        continue;
      }

      composerElem.removeChild( childElem );
      compositionElem.appendChild( childElem );

    }
  }

  // SimpleFill symbol layer v2: avoid double transparency
  // replacing alpha value of symbol layer's color with 255 (the
  // transparency value is already stored as symbol transparency).
  QDomNodeList rendererList = mDom.elementsByTagName( QStringLiteral( "renderer-v2" ) );
  for ( int i = 0; i < rendererList.size(); ++i )
  {
    QDomNodeList layerList = rendererList.at( i ).toElement().elementsByTagName( QStringLiteral( "layer" ) );
    for ( int j = 0; j < layerList.size(); ++j )
    {
      QDomElement layerElem = layerList.at( j ).toElement();
      if ( layerElem.attribute( QStringLiteral( "class" ) ) == QLatin1String( "SimpleFill" ) )
      {
        QDomNodeList propList = layerElem.elementsByTagName( QStringLiteral( "prop" ) );
        for ( int k = 0; k < propList.size(); ++k )
        {
          QDomElement propElem = propList.at( k ).toElement();
          if ( propElem.attribute( QStringLiteral( "k" ) ) == QLatin1String( "color" ) || propElem.attribute( QStringLiteral( "k" ) ) == QLatin1String( "color_border" ) )
          {
            propElem.setAttribute( QStringLiteral( "v" ), propElem.attribute( QStringLiteral( "v" ) ).section( ',', 0, 2 ) + ",255" );
          }
        }
      }
    }
  }

  QgsDebugMsg( mDom.toString() );
}

void QgsProjectFileTransform::transform2200to2300()
{
  //composer: set placement for all picture items to middle, to mimic <=2.2 behavior
  QDomNodeList composerPictureList = mDom.elementsByTagName( QStringLiteral( "ComposerPicture" ) );
  for ( int i = 0; i < composerPictureList.size(); ++i )
  {
    QDomElement picture = composerPictureList.at( i ).toElement();
    picture.setAttribute( QStringLiteral( "anchorPoint" ), QString::number( 4 ) );
  }
}

void QgsProjectFileTransform::transform2990()
{
  // transform OTF off to "no projection" for project
  QDomElement propsElem = mDom.firstChildElement( QStringLiteral( "qgis" ) ).toElement().firstChildElement( QStringLiteral( "properties" ) );
  QDomNodeList srsNodes = propsElem.elementsByTagName( QStringLiteral( "SpatialRefSys" ) );
  QDomElement srsElem;
  QDomElement projElem;
  if ( srsNodes.count() > 0 )
  {
    srsElem = srsNodes.at( 0 ).toElement();
    QDomNodeList projNodes = srsElem.elementsByTagName( QStringLiteral( "ProjectionsEnabled" ) );
    if ( projNodes.count() == 0 )
    {
      projElem = mDom.createElement( QStringLiteral( "ProjectionsEnabled" ) );
      projElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "int" ) );
      QDomText projText = mDom.createTextNode( QStringLiteral( "0" ) );
      projElem.appendChild( projText );
      srsElem.appendChild( projElem );
    }
  }
  else
  {
    srsElem = mDom.createElement( QStringLiteral( "SpatialRefSys" ) );
    projElem = mDom.createElement( QStringLiteral( "ProjectionsEnabled" ) );
    projElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "int" ) );
    QDomText projText = mDom.createTextNode( QStringLiteral( "0" ) );
    projElem.appendChild( projText );
    srsElem.appendChild( projElem );
    propsElem.appendChild( srsElem );
  }
  // transform map canvas CRS to project CRS - this is because project CRS was inconsistently used
  // prior to 3.0. In >= 3.0 main canvas CRS is forced to match project CRS, so we need to make
  // sure we can read the project CRS correctly
  QDomNodeList canvasNodes = mDom.elementsByTagName( QStringLiteral( "mapcanvas" ) );
  if ( canvasNodes.count() > 0 )
  {
    QDomElement canvasElem = canvasNodes.at( 0 ).toElement();
    QDomNodeList canvasSrsNodes = canvasElem.elementsByTagName( QStringLiteral( "spatialrefsys" ) );
    if ( canvasSrsNodes.count() > 0 )
    {
      QDomElement canvasSrsElem = canvasSrsNodes.at( 0 ).toElement();
      QString proj;
      QString authid;
      QString srsid;

      QDomNodeList proj4Nodes = canvasSrsElem.elementsByTagName( QStringLiteral( "proj4" ) );
      if ( proj4Nodes.count() > 0 )
      {
        QDomElement proj4Node = proj4Nodes.at( 0 ).toElement();
        proj = proj4Node.text();
      }
      QDomNodeList authidNodes = canvasSrsElem.elementsByTagName( QStringLiteral( "authid" ) );
      if ( authidNodes.count() > 0 )
      {
        QDomElement authidNode = authidNodes.at( 0 ).toElement();
        authid = authidNode.text();
      }
      QDomNodeList srsidNodes = canvasSrsElem.elementsByTagName( QStringLiteral( "srsid" ) );
      if ( srsidNodes.count() > 0 )
      {
        QDomElement srsidNode = srsidNodes.at( 0 ).toElement();
        srsid = srsidNode.text();
      }

      // clear existing project CRS nodes
      QDomNodeList oldProjectProj4Nodes = srsElem.elementsByTagName( QStringLiteral( "ProjectCRSProj4String" ) );
      for ( int i = oldProjectProj4Nodes.count(); i >= 0; --i )
      {
        srsElem.removeChild( oldProjectProj4Nodes.at( i ) );
      }
      QDomNodeList oldProjectCrsNodes = srsElem.elementsByTagName( QStringLiteral( "ProjectCrs" ) );
      for ( int i = oldProjectCrsNodes.count(); i >= 0; --i )
      {
        srsElem.removeChild( oldProjectCrsNodes.at( i ) );
      }
      QDomNodeList oldProjectCrsIdNodes = srsElem.elementsByTagName( QStringLiteral( "ProjectCRSID" ) );
      for ( int i = oldProjectCrsIdNodes.count(); i >= 0; --i )
      {
        srsElem.removeChild( oldProjectCrsIdNodes.at( i ) );
      }
      QDomNodeList projectionsEnabledNodes = srsElem.elementsByTagName( QStringLiteral( "ProjectionsEnabled" ) );
      for ( int i = projectionsEnabledNodes.count(); i >= 0; --i )
      {
        srsElem.removeChild( projectionsEnabledNodes.at( i ) );
      }

      QDomElement proj4Elem = mDom.createElement( QStringLiteral( "ProjectCRSProj4String" ) );
      proj4Elem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "QString" ) );
      QDomText proj4Text = mDom.createTextNode( proj );
      proj4Elem.appendChild( proj4Text );
      QDomElement projectCrsElem = mDom.createElement( QStringLiteral( "ProjectCrs" ) );
      projectCrsElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "QString" ) );
      QDomText projectCrsText = mDom.createTextNode( authid );
      projectCrsElem.appendChild( projectCrsText );
      QDomElement projectCrsIdElem = mDom.createElement( QStringLiteral( "ProjectCRSID" ) );
      projectCrsIdElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "int" ) );
      QDomText srsidText = mDom.createTextNode( srsid );
      projectCrsIdElem.appendChild( srsidText );
      QDomElement projectionsEnabledElem = mDom.createElement( QStringLiteral( "ProjectionsEnabled" ) );
      projectionsEnabledElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "int" ) );
      QDomText projectionsEnabledText = mDom.createTextNode( QStringLiteral( "1" ) );
      projectionsEnabledElem.appendChild( projectionsEnabledText );
      srsElem.appendChild( proj4Elem );
      srsElem.appendChild( projectCrsElem );
      srsElem.appendChild( projectCrsIdElem );
      srsElem.appendChild( projectionsEnabledElem );

      QDomNodeList srsNodes = propsElem.elementsByTagName( QStringLiteral( "SpatialRefSys" ) );
      for ( int i = srsNodes.count(); i >= 0; --i )
      {
        propsElem.removeChild( srsNodes.at( i ) );
      }
      propsElem.appendChild( srsElem );
    }
  }


  QDomNodeList mapLayers = mDom.elementsByTagName( QStringLiteral( "maplayer" ) );

  for ( int mapLayerIndex = 0; mapLayerIndex < mapLayers.count(); ++mapLayerIndex )
  {
    QDomElement layerElem = mapLayers.at( mapLayerIndex ).toElement();

    // The newly added fieldConfiguration element
    QDomElement fieldConfigurationElement = mDom.createElement( QStringLiteral( "fieldConfiguration" ) );
    layerElem.appendChild( fieldConfigurationElement );

    QDomNodeList editTypeNodes = layerElem.namedItem( QStringLiteral( "edittypes" ) ).childNodes();
    QDomElement constraintExpressionsElem = mDom.createElement( QStringLiteral( "constraintExpressions" ) );
    layerElem.appendChild( constraintExpressionsElem );

    for ( int i = 0; i < editTypeNodes.size(); ++i )
    {
      QDomNode editTypeNode = editTypeNodes.at( i );
      QDomElement editTypeElement = editTypeNode.toElement();

      QDomElement fieldElement = mDom.createElement( QStringLiteral( "field" ) );
      fieldConfigurationElement.appendChild( fieldElement );

      QString name = editTypeElement.attribute( QStringLiteral( "name" ) );
      fieldElement.setAttribute( QStringLiteral( "name" ), name );
      QDomElement constraintExpressionElem = mDom.createElement( QStringLiteral( "constraint" ) );
      constraintExpressionElem.setAttribute( QStringLiteral( "field" ), name );
      constraintExpressionsElem.appendChild( constraintExpressionElem );

      QDomElement editWidgetElement = mDom.createElement( QStringLiteral( "editWidget" ) );
      fieldElement.appendChild( editWidgetElement );

      QString ewv2Type = editTypeElement.attribute( QStringLiteral( "widgetv2type" ) );
      editWidgetElement.setAttribute( QStringLiteral( "type" ), ewv2Type );

      QDomElement ewv2CfgElem = editTypeElement.namedItem( QStringLiteral( "widgetv2config" ) ).toElement();

      if ( !ewv2CfgElem.isNull() )
      {
        QDomElement editWidgetConfigElement = mDom.createElement( QStringLiteral( "config" ) );
        editWidgetElement.appendChild( editWidgetConfigElement );

        QVariantMap editWidgetConfiguration;

        QDomNamedNodeMap configAttrs = ewv2CfgElem.attributes();
        for ( int configIndex = 0; configIndex < configAttrs.count(); ++configIndex )
        {
          QDomAttr configAttr = configAttrs.item( configIndex ).toAttr();
          if ( configAttr.name() == QStringLiteral( "fieldEditable" ) )
          {
            editWidgetConfigElement.setAttribute( QStringLiteral( "fieldEditable" ), configAttr.value() );
          }
          else if ( configAttr.name() == QStringLiteral( "labelOnTop" ) )
          {
            editWidgetConfigElement.setAttribute( QStringLiteral( "labelOnTop" ), configAttr.value() );
          }
          else if ( configAttr.name() == QStringLiteral( "notNull" ) )
          {
            editWidgetConfigElement.setAttribute( QStringLiteral( "notNull" ), configAttr.value() );
          }
          else if ( configAttr.name() == QStringLiteral( "constraint" ) )
          {
            constraintExpressionElem.setAttribute( QStringLiteral( "exp" ), configAttr.value() );
          }
          else if ( configAttr.name() == QStringLiteral( "constraintDescription" ) )
          {
            constraintExpressionElem.setAttribute( QStringLiteral( "desc" ), configAttr.value() );
          }
          else
          {
            editWidgetConfiguration.insert( configAttr.name(), configAttr.value() );
          }
        }

        if ( ewv2Type == QStringLiteral( "ValueMap" ) )
        {
          QDomNodeList configElements = ewv2CfgElem.childNodes();
          QVariantMap map;
          for ( int configIndex = 0; configIndex < configElements.count(); ++configIndex )
          {
            QDomElement configElem = configElements.at( configIndex ).toElement();
            map.insert( configElem.attribute( QStringLiteral( "key" ) ), configElem.attribute( QStringLiteral( "value" ) ) );
          }
          editWidgetConfiguration.insert( QStringLiteral( "map" ), map );
        }
        else if ( ewv2Type == QStringLiteral( "Photo" ) )
        {
          editWidgetElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "ExternalResource" ) );

          editWidgetConfiguration.insert( QStringLiteral( "DocumentViewer" ), 1 );
          editWidgetConfiguration.insert( QStringLiteral( "DocumentViewerHeight" ), editWidgetConfiguration.value( QStringLiteral( "Height" ) ) );
          editWidgetConfiguration.insert( QStringLiteral( "DocumentViewerWidth" ), editWidgetConfiguration.value( QStringLiteral( "Width" ) ) );
          editWidgetConfiguration.insert( QStringLiteral( "RelativeStorage" ), 1 );
        }
        else if ( ewv2Type == QStringLiteral( "FileName" ) )
        {
          editWidgetElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "ExternalResource" ) );

          editWidgetConfiguration.insert( QStringLiteral( "RelativeStorage" ), 1 );
        }
        else if ( ewv2Type == QStringLiteral( "WebView" ) )
        {
          editWidgetElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "ExternalResource" ) );

          editWidgetConfiguration.insert( QStringLiteral( "DocumentViewerHeight" ), editWidgetConfiguration.value( QStringLiteral( "Height" ) ) );
          editWidgetConfiguration.insert( QStringLiteral( "DocumentViewerWidth" ), editWidgetConfiguration.value( QStringLiteral( "Width" ) ) );
          editWidgetConfiguration.insert( QStringLiteral( "RelativeStorage" ), 1 );
        }

        editWidgetConfigElement.appendChild( QgsXmlUtils::writeVariant( editWidgetConfiguration, mDom ) );
      }
    }
  }
}

void QgsProjectFileTransform::convertRasterProperties( QDomDocument &doc, QDomNode &parentNode,
    QDomElement &rasterPropertiesElem, QgsRasterLayer *rlayer )
{
  //no data
  //TODO: We would need to set no data on all bands, but we don't know number of bands here
  QDomNode noDataNode = rasterPropertiesElem.namedItem( QStringLiteral( "mNoDataValue" ) );
  QDomElement noDataElement = noDataNode.toElement();
  if ( !noDataElement.text().isEmpty() )
  {
    QgsDebugMsg( "mNoDataValue = " + noDataElement.text() );
    QDomElement noDataElem = doc.createElement( QStringLiteral( "noData" ) );

    QDomElement noDataRangeList = doc.createElement( QStringLiteral( "noDataRangeList" ) );
    noDataRangeList.setAttribute( QStringLiteral( "bandNo" ), 1 );

    QDomElement noDataRange = doc.createElement( QStringLiteral( "noDataRange" ) );
    noDataRange.setAttribute( QStringLiteral( "min" ), noDataElement.text() );
    noDataRange.setAttribute( QStringLiteral( "max" ), noDataElement.text() );
    noDataRangeList.appendChild( noDataRange );

    noDataElem.appendChild( noDataRangeList );

    parentNode.appendChild( noDataElem );
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  //convert general properties

  //invert color
  rasterRendererElem.setAttribute( QStringLiteral( "invertColor" ), QStringLiteral( "0" ) );
  QDomElement  invertColorElem = rasterPropertiesElem.firstChildElement( QStringLiteral( "mInvertColor" ) );
  if ( !invertColorElem.isNull() )
  {
    if ( invertColorElem.text() == QLatin1String( "true" ) )
    {
      rasterRendererElem.setAttribute( QStringLiteral( "invertColor" ), QStringLiteral( "1" ) );
    }
  }

  //opacity
  rasterRendererElem.setAttribute( QStringLiteral( "opacity" ), QStringLiteral( "1" ) );
  QDomElement transparencyElem = parentNode.firstChildElement( QStringLiteral( "transparencyLevelInt" ) );
  if ( !transparencyElem.isNull() )
  {
    double transparency = transparencyElem.text().toInt();
    rasterRendererElem.setAttribute( QStringLiteral( "opacity" ), QString::number( transparency / 255.0 ) );
  }

  //alphaBand was not saved until now (bug)
  rasterRendererElem.setAttribute( QStringLiteral( "alphaBand" ), -1 );

  //gray band is used for several renderers
  int grayBand = rasterBandNumber( rasterPropertiesElem, QStringLiteral( "mGrayBandName" ), rlayer );

  //convert renderer specific properties
  QString drawingStyle = rasterPropertiesElem.firstChildElement( QStringLiteral( "mDrawingStyle" ) ).text();

  // While PalettedColor should normally contain only integer values, usually
  // color palette 0-255, it may happen (Tim, issue #7023) that it contains
  // colormap classification with double values and text labels
  // (which should normally only appear in SingleBandPseudoColor drawingStyle)
  // => we have to check first the values and change drawingStyle if necessary
  if ( drawingStyle == QLatin1String( "PalettedColor" ) )
  {
    QDomElement customColorRampElem = rasterPropertiesElem.firstChildElement( QStringLiteral( "customColorRamp" ) );
    QDomNodeList colorRampEntryList = customColorRampElem.elementsByTagName( QStringLiteral( "colorRampEntry" ) );

    for ( int i = 0; i < colorRampEntryList.size(); ++i )
    {
      QDomElement colorRampEntryElem = colorRampEntryList.at( i ).toElement();
      QString strValue = colorRampEntryElem.attribute( QStringLiteral( "value" ) );
      double value = strValue.toDouble();
      if ( value < 0 || value > 10000 || !qgsDoubleNear( value, static_cast< int >( value ) ) )
      {
        QgsDebugMsg( QString( "forcing SingleBandPseudoColor value = %1" ).arg( value ) );
        drawingStyle = QStringLiteral( "SingleBandPseudoColor" );
        break;
      }
    }
  }

  if ( drawingStyle == QLatin1String( "SingleBandGray" ) )
  {
    rasterRendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "singlebandgray" ) );
    rasterRendererElem.setAttribute( QStringLiteral( "grayBand" ), grayBand );
    transformContrastEnhancement( doc, rasterPropertiesElem, rasterRendererElem );
  }
  else if ( drawingStyle == QLatin1String( "SingleBandPseudoColor" ) )
  {
    rasterRendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "singlebandpseudocolor" ) );
    rasterRendererElem.setAttribute( QStringLiteral( "band" ), grayBand );
    QDomElement newRasterShaderElem = doc.createElement( QStringLiteral( "rastershader" ) );
    QDomElement newColorRampShaderElem = doc.createElement( QStringLiteral( "colorrampshader" ) );
    newRasterShaderElem.appendChild( newColorRampShaderElem );
    rasterRendererElem.appendChild( newRasterShaderElem );

    //switch depending on mColorShadingAlgorithm
    QString colorShadingAlgorithm = rasterPropertiesElem.firstChildElement( QStringLiteral( "mColorShadingAlgorithm" ) ).text();
    if ( colorShadingAlgorithm == QLatin1String( "PseudoColorShader" ) || colorShadingAlgorithm == QLatin1String( "FreakOutShader" ) )
    {
      newColorRampShaderElem.setAttribute( QStringLiteral( "colorRampType" ), QStringLiteral( "INTERPOLATED" ) );

      //get minmax from rasterlayer
      QgsRasterBandStats rasterBandStats = rlayer->dataProvider()->bandStatistics( grayBand );
      double minValue = rasterBandStats.minimumValue;
      double maxValue = rasterBandStats.maximumValue;
      double breakSize = ( maxValue - minValue ) / 3;

      QStringList colorList;
      if ( colorShadingAlgorithm == QLatin1String( "FreakOutShader" ) )
      {
        colorList << QStringLiteral( "#ff00ff" ) << QStringLiteral( "#00ffff" ) << QStringLiteral( "#ff0000" ) << QStringLiteral( "#00ff00" );
      }
      else //pseudocolor
      {
        colorList << QStringLiteral( "#0000ff" ) << QStringLiteral( "#00ffff" ) << QStringLiteral( "#ffff00" ) << QStringLiteral( "#ff0000" );
      }
      QStringList::const_iterator colorIt = colorList.constBegin();
      double boundValue = minValue;
      for ( ; colorIt != colorList.constEnd(); ++colorIt )
      {
        QDomElement newItemElem = doc.createElement( QStringLiteral( "item" ) );
        newItemElem.setAttribute( QStringLiteral( "value" ), QString::number( boundValue ) );
        newItemElem.setAttribute( QStringLiteral( "label" ), QString::number( boundValue ) );
        newItemElem.setAttribute( QStringLiteral( "color" ), *colorIt );
        newColorRampShaderElem.appendChild( newItemElem );
        boundValue += breakSize;
      }
    }
    else if ( colorShadingAlgorithm == QLatin1String( "ColorRampShader" ) )
    {
      QDomElement customColorRampElem = rasterPropertiesElem.firstChildElement( QStringLiteral( "customColorRamp" ) );
      QString type = customColorRampElem.firstChildElement( QStringLiteral( "colorRampType" ) ).text();
      newColorRampShaderElem.setAttribute( QStringLiteral( "colorRampType" ), type );
      QDomNodeList colorNodeList = customColorRampElem.elementsByTagName( QStringLiteral( "colorRampEntry" ) );

      QString value, label;
      QColor newColor;
      int red, green, blue;
      QDomElement currentItemElem;
      for ( int i = 0; i < colorNodeList.size(); ++i )
      {
        currentItemElem = colorNodeList.at( i ).toElement();
        value = currentItemElem.attribute( QStringLiteral( "value" ) );
        label = currentItemElem.attribute( QStringLiteral( "label" ) );
        red = currentItemElem.attribute( QStringLiteral( "red" ) ).toInt();
        green = currentItemElem.attribute( QStringLiteral( "green" ) ).toInt();
        blue = currentItemElem.attribute( QStringLiteral( "blue" ) ).toInt();
        newColor = QColor( red, green, blue );
        QDomElement newItemElem = doc.createElement( QStringLiteral( "item" ) );
        newItemElem.setAttribute( QStringLiteral( "value" ), value );
        newItemElem.setAttribute( QStringLiteral( "label" ), label );
        newItemElem.setAttribute( QStringLiteral( "color" ), newColor.name() );
        newColorRampShaderElem.appendChild( newItemElem );
      }
    }
  }
  else if ( drawingStyle == QLatin1String( "PalettedColor" ) )
  {
    rasterRendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "paletted" ) );
    rasterRendererElem.setAttribute( QStringLiteral( "band" ), grayBand );
    QDomElement customColorRampElem = rasterPropertiesElem.firstChildElement( QStringLiteral( "customColorRamp" ) );
    QDomNodeList colorRampEntryList = customColorRampElem.elementsByTagName( QStringLiteral( "colorRampEntry" ) );
    QDomElement newColorPaletteElem = doc.createElement( QStringLiteral( "colorPalette" ) );

    int red = 0;
    int green = 0;
    int blue = 0;
    int value = 0;
    QDomElement colorRampEntryElem;
    for ( int i = 0; i < colorRampEntryList.size(); ++i )
    {
      colorRampEntryElem = colorRampEntryList.at( i ).toElement();
      QDomElement newPaletteElem = doc.createElement( QStringLiteral( "paletteEntry" ) );
      value = static_cast< int >( colorRampEntryElem.attribute( QStringLiteral( "value" ) ).toDouble() );
      newPaletteElem.setAttribute( QStringLiteral( "value" ), value );
      red = colorRampEntryElem.attribute( QStringLiteral( "red" ) ).toInt();
      green = colorRampEntryElem.attribute( QStringLiteral( "green" ) ).toInt();
      blue = colorRampEntryElem.attribute( QStringLiteral( "blue" ) ).toInt();
      newPaletteElem.setAttribute( QStringLiteral( "color" ), QColor( red, green, blue ).name() );
      QString label = colorRampEntryElem.attribute( QStringLiteral( "label" ) );
      if ( !label.isEmpty() )
      {
        newPaletteElem.setAttribute( QStringLiteral( "label" ), label );
      }
      newColorPaletteElem.appendChild( newPaletteElem );
    }
    rasterRendererElem.appendChild( newColorPaletteElem );
  }
  else if ( drawingStyle == QLatin1String( "MultiBandColor" ) )
  {
    rasterRendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "multibandcolor" ) );

    //red band, green band, blue band
    int redBand = rasterBandNumber( rasterPropertiesElem, QStringLiteral( "mRedBandName" ), rlayer );
    int greenBand = rasterBandNumber( rasterPropertiesElem, QStringLiteral( "mGreenBandName" ), rlayer );
    int blueBand = rasterBandNumber( rasterPropertiesElem, QStringLiteral( "mBlueBandName" ), rlayer );
    rasterRendererElem.setAttribute( QStringLiteral( "redBand" ), redBand );
    rasterRendererElem.setAttribute( QStringLiteral( "greenBand" ), greenBand );
    rasterRendererElem.setAttribute( QStringLiteral( "blueBand" ), blueBand );

    transformContrastEnhancement( doc, rasterPropertiesElem, rasterRendererElem );
  }
  else
  {
    return;
  }

  //replace rasterproperties element with rasterrenderer element
  if ( !parentNode.isNull() )
  {
    parentNode.replaceChild( rasterRendererElem, rasterPropertiesElem );
  }
}

int QgsProjectFileTransform::rasterBandNumber( const QDomElement &rasterPropertiesElem, const QString &bandName,
    QgsRasterLayer *rlayer )
{
  if ( !rlayer )
  {
    return -1;
  }

  int band = -1;
  QDomElement rasterBandElem = rasterPropertiesElem.firstChildElement( bandName );
  if ( !rasterBandElem.isNull() )
  {
    QRegExp re( "(\\d+)" );

    if ( re.indexIn( rasterBandElem.text() ) >= 0 )
    {
      return re.cap( 1 ).toInt();
    }
  }
  return band;
}

void QgsProjectFileTransform::transformContrastEnhancement( QDomDocument &doc, const QDomElement &rasterproperties, QDomElement &rendererElem )
{
  if ( rasterproperties.isNull() || rendererElem.isNull() )
  {
    return;
  }

  double minimumValue = 0;
  double maximumValue = 0;
  QDomElement contrastMinMaxElem = rasterproperties.firstChildElement( QStringLiteral( "contrastEnhancementMinMaxValues" ) );
  if ( contrastMinMaxElem.isNull() )
  {
    return;
  }

  QDomElement contrastEnhancementAlgorithmElem = rasterproperties.firstChildElement( QStringLiteral( "mContrastEnhancementAlgorithm" ) );
  if ( contrastEnhancementAlgorithmElem.isNull() )
  {
    return;
  }

  //convert enhancement name to enumeration
  int algorithmEnum = 0;
  QString algorithmString = contrastEnhancementAlgorithmElem.text();
  if ( algorithmString == QLatin1String( "StretchToMinimumMaximum" ) )
  {
    algorithmEnum = 1;
  }
  else if ( algorithmString == QLatin1String( "StretchAndClipToMinimumMaximum" ) )
  {
    algorithmEnum = 2;
  }
  else if ( algorithmString == QLatin1String( "ClipToMinimumMaximum" ) )
  {
    algorithmEnum = 3;
  }
  else if ( algorithmString == QLatin1String( "UserDefinedEnhancement" ) )
  {
    algorithmEnum = 4;
  }

  QDomNodeList minMaxEntryList = contrastMinMaxElem.elementsByTagName( QStringLiteral( "minMaxEntry" ) );
  QStringList enhancementNameList;
  if ( minMaxEntryList.size() == 1 )
  {
    enhancementNameList << QStringLiteral( "contrastEnhancement" );
  }
  if ( minMaxEntryList.size() == 3 )
  {
    enhancementNameList << QStringLiteral( "redContrastEnhancement" ) << QStringLiteral( "greenContrastEnhancement" ) << QStringLiteral( "blueContrastEnhancement" );
  }
  if ( minMaxEntryList.size() > enhancementNameList.size() )
  {
    return;
  }

  QDomElement minMaxEntryElem;
  for ( int i = 0; i < minMaxEntryList.size(); ++i )
  {
    minMaxEntryElem = minMaxEntryList.at( i ).toElement();
    QDomElement minElem = minMaxEntryElem.firstChildElement( QStringLiteral( "min" ) );
    if ( minElem.isNull() )
    {
      return;
    }
    minimumValue = minElem.text().toDouble();

    QDomElement maxElem = minMaxEntryElem.firstChildElement( QStringLiteral( "max" ) );
    if ( maxElem.isNull() )
    {
      return;
    }
    maximumValue = maxElem.text().toDouble();

    QDomElement newContrastEnhancementElem = doc.createElement( enhancementNameList.at( i ) );
    QDomElement newMinValElem = doc.createElement( QStringLiteral( "minValue" ) );
    QDomText minText = doc.createTextNode( QString::number( minimumValue ) );
    newMinValElem.appendChild( minText );
    newContrastEnhancementElem.appendChild( newMinValElem );
    QDomElement newMaxValElem = doc.createElement( QStringLiteral( "maxValue" ) );
    QDomText maxText = doc.createTextNode( QString::number( maximumValue ) );
    newMaxValElem.appendChild( maxText );
    newContrastEnhancementElem.appendChild( newMaxValElem );

    QDomElement newAlgorithmElem = doc.createElement( QStringLiteral( "algorithm" ) );
    QDomText newAlgorithmText = doc.createTextNode( QString::number( algorithmEnum ) );
    newAlgorithmElem.appendChild( newAlgorithmText );
    newContrastEnhancementElem.appendChild( newAlgorithmElem );

    rendererElem.appendChild( newContrastEnhancementElem );
  }
}

void QgsProjectFileTransform::transformRasterTransparency( QDomDocument &doc, const QDomElement &orig, QDomElement &rendererElem )
{
  //soon...
  Q_UNUSED( doc );
  Q_UNUSED( orig );
  Q_UNUSED( rendererElem );
}

