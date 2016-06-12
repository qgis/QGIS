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
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QTextStream>
#include <QDomDocument>
#include <QPrinter> //to find out screen resolution
#include <cstdlib>
#include "qgsproject.h"
#include "qgsprojectproperty.h"

typedef QgsProjectVersion PFV;


QgsProjectFileTransform::transform QgsProjectFileTransform::transformers[] =
{
  {PFV( 0, 8, 0 ), PFV( 0, 8, 1 ), &QgsProjectFileTransform::transformNull},
  {PFV( 0, 8, 1 ), PFV( 0, 9, 0 ), &QgsProjectFileTransform::transform081to090},
  {PFV( 0, 9, 0 ), PFV( 0, 9, 1 ), &QgsProjectFileTransform::transformNull},
  {PFV( 0, 9, 1 ), PFV( 0, 10, 0 ), &QgsProjectFileTransform::transform091to0100},
  // Following line is a hack that takes us straight from 0.9.2 to 0.11.0
  // due to an unknown bug in migrating 0.9.2 files which we didnt pursue (TS & GS)
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
};

bool QgsProjectFileTransform::updateRevision( const QgsProjectVersion& newVersion )
{
  Q_UNUSED( newVersion );
  bool returnValue = false;

  if ( ! mDom.isNull() )
  {
    for ( std::size_t i = 0; i < sizeof( transformers ) / sizeof( transform ); i++ )
    {
      if ( transformers[i].from == mCurrentVersion )
      {
        // Run the transformer, and update the revision in every case
        ( this->*( transformers[i].transformFunc ) )();
        mCurrentVersion = transformers[i].to;
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
    QDomNode qgis = mDom.firstChildElement( "qgis" );
    if ( ! qgis.isNull() )
    {
      QgsDebugMsg( "Populating new mapcanvas" );

      // Create a mapcanvas
      mapCanvas = mDom.createElement( "mapcanvas" );
      // Append mapcanvas to parent 'qgis'.
      qgis.appendChild( mapCanvas );
      // Re-parent units
      mapCanvas.appendChild( qgis.namedItem( "units" ) );
      // Re-parent extent
      mapCanvas.appendChild( qgis.namedItem( "extent" ) );

      // See if we can find if projection is on.

      QDomElement properties = qgis.firstChildElement( "properties" );
      QDomElement spatial = properties.firstChildElement( "SpatialRefSys" );
      QDomElement hasCrsTransformEnabled = spatial.firstChildElement( "ProjectionsEnabled" );
      // Type is 'int', and '1' if on.
      // Create an element
      QDomElement projection = mDom.createElement( "projections" );
      QgsDebugMsg( QString( "Projection flag: " ) + hasCrsTransformEnabled.text() );
      // Set flag from ProjectionsEnabled
      projection.appendChild( mDom.createTextNode( hasCrsTransformEnabled.text() ) );
      // Set new element as child of <mapcanvas>
      mapCanvas.appendChild( projection );

    }


    // Transforming coordinate-transforms
    // Create a list of all map layers
    QDomNodeList mapLayers = mDom.elementsByTagName( "maplayer" );
    bool doneDestination = false;
    for ( int i = 0; i < mapLayers.count(); i++ )
    {
      QDomNode mapLayer = mapLayers.item( i );
      // Find the coordinatetransform
      QDomNode coordinateTransform = mapLayer.namedItem( "coordinatetransform" );
      // Find the sourcesrs
      QDomNode sourceCrs = coordinateTransform.namedItem( "sourcesrs" );
      // Rename to srs
      sourceCrs.toElement().setTagName( "srs" );
      // Re-parent to maplayer
      mapLayer.appendChild( sourceCrs );
      // Re-move coordinatetransform
      // Take the destination CRS of the first layer and use for mapcanvas projection
      if ( ! doneDestination )
      {
        // Use destination CRS from the last layer
        QDomNode destinationCRS = coordinateTransform.namedItem( "destinationsrs" );
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
    QDomNodeList legendLayerFiles = mDom.elementsByTagName( "legendlayerfile" );
    QgsDebugMsg( QString( "Legend layer file entries: " ) + QString::number( legendLayerFiles.count() ) );
    for ( int i = 0; i < mapLayers.count(); i++ )
    {
      // Get one maplayer element from list
      QDomElement mapLayer = mapLayers.item( i ).toElement();
      // Find it's id.
      QString id = mapLayer.firstChildElement( "id" ).text();
      QgsDebugMsg( QString( "Handling layer " + id ) );
      // Now, look it up in legend
      for ( int j = 0; j < legendLayerFiles.count(); j++ )
      {
        QDomElement legendLayerFile = legendLayerFiles.item( j ).toElement();
        if ( id == legendLayerFile.attribute( "layerid" ) )
        {
          // Found a the legend layer that matches the maplayer
          QgsDebugMsg( "Found matching id" );

          // Set visible flag from maplayer to legendlayer
          legendLayerFile.setAttribute( "visible", mapLayer.attribute( "visible" ) );

          // Set overview flag from maplayer to legendlayer
          legendLayerFile.setAttribute( "isInOverview", mapLayer.attribute( "showInOverviewFlag" ) );
        }
      }
    }
  }
  return;

}

void QgsProjectFileTransform::transform091to0100()
{
  if ( ! mDom.isNull() )
  {
    // Insert transforms here!
    QDomNodeList rasterPropertyList = mDom.elementsByTagName( "rasterproperties" );
    QgsDebugMsg( QString( "Raster properties file entries: " ) + QString::number( rasterPropertyList.count() ) );
    for ( int i = 0; i < rasterPropertyList.count(); i++ )
    {
      // Get one rasterproperty element from list, and rename the sub-properties.
      QDomNode rasterProperty = rasterPropertyList.item( i );
      // rasterProperty.namedItem("").toElement().setTagName("");

      rasterProperty.namedItem( "stdDevsToPlotDouble" ).toElement().setTagName( "mStandardDeviations" );

      rasterProperty.namedItem( "invertHistogramFlag" ).toElement().setTagName( "mInvertPixelsFlag" );
      rasterProperty.namedItem( "showDebugOverLayFlag" ).toElement().setTagName( "mDebugOverLayFlag" );

      rasterProperty.namedItem( "redBandNameQString" ).toElement().setTagName( "mRedBandName" );
      rasterProperty.namedItem( "blueBandNameQString" ).toElement().setTagName( "mBlueBandName" );
      rasterProperty.namedItem( "greenBandNameQString" ).toElement().setTagName( "mGreenBandName" );
      rasterProperty.namedItem( "grayBandNameQString" ).toElement().setTagName( "mGrayBandName" );
    }

    // Changing symbol size for hard: symbols
    QDomNodeList symbolPropertyList = mDom.elementsByTagName( "symbol" );
    for ( int i = 0; i < symbolPropertyList.count(); i++ )
    {
      // Get the <poinmtsymbol> to check for 'hard:' for each <symbol>
      QDomNode symbolProperty = symbolPropertyList.item( i );

      QDomElement pointSymbol = symbolProperty.firstChildElement( "pointsymbol" );
      if ( pointSymbol.text().startsWith( "hard:" ) )
      {
        // Get pointsize and line width
        int lineWidth = symbolProperty.firstChildElement( "outlinewidth" ).text().toInt();
        int pointSize = symbolProperty.firstChildElement( "pointsize" ).text().toInt();
        // Just a precaution, checking for 0
        if ( pointSize != 0 )
        {
          // int r = (s-2*lw)/2-1 --> 2r = (s-2*lw)-2 --> 2r+2 = s-2*lw
          // --> 2r+2+2*lw = s
          // where '2r' is the old size.
          pointSize = pointSize + 2 + 2 * lineWidth;
          QgsDebugMsg( QString( "Setting point size to %1" ).arg( pointSize ) );
          QDomElement newPointSizeProperty = mDom.createElement( "pointsize" );
          QDomText newPointSizeTxt = mDom.createTextNode( QString::number( pointSize ) );
          newPointSizeProperty.appendChild( newPointSizeTxt );
          symbolProperty.replaceChild( newPointSizeProperty, pointSymbol );
        }
      }
    }

  }
  return;

}

void QgsProjectFileTransform::transform0100to0110()
{
  if ( ! mDom.isNull() )
  {
    //Change 'outlinewidth' in QgsSymbol
    QPrinter myPrinter( QPrinter::ScreenResolution );
    int screenDpi = myPrinter.resolution();
    double widthScaleFactor = 25.4 / screenDpi;

    QDomNodeList outlineWidthList = mDom.elementsByTagName( "outlinewidth" );
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
    QDomNodeList pointSizeList = mDom.elementsByTagName( "pointsize" );
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
  }
}

void QgsProjectFileTransform::transform0110to1000()
{
  if ( ! mDom.isNull() )
  {
    QDomNodeList layerList = mDom.elementsByTagName( "maplayer" );
    for ( int i = 0; i < layerList.size(); ++i )
    {
      QDomElement layerElem = layerList.at( i ).toElement();
      QString typeString = layerElem.attribute( "type" );
      if ( typeString != "vector" )
      {
        continue;
      }

      //datasource
      QDomNode dataSourceNode = layerElem.namedItem( "datasource" );
      if ( dataSourceNode.isNull() )
      {
        return;
      }
      QString dataSource = dataSourceNode.toElement().text();

      //provider key
      QDomNode providerNode = layerElem.namedItem( "provider" );
      if ( providerNode.isNull() )
      {
        return;
      }
      QString providerKey = providerNode.toElement().text();

      //create the layer to get the provider for int->fieldName conversion
      QgsVectorLayer* theLayer = new QgsVectorLayer( dataSource, "", providerKey, false );
      if ( !theLayer->isValid() )
      {
        delete theLayer;
        return;
      }

      QgsVectorDataProvider* theProvider = theLayer->dataProvider();
      if ( !theProvider )
      {
        return;
      }
      QgsFields theFields = theProvider->fields();

      //read classificationfield
      QDomNodeList classificationFieldList = layerElem.elementsByTagName( "classificationfield" );
      for ( int j = 0; j < classificationFieldList.size(); ++j )
      {
        QDomElement classificationFieldElem = classificationFieldList.at( j ).toElement();
        int fieldNumber = classificationFieldElem.text().toInt();
        if ( fieldNumber >= 0 && fieldNumber < theFields.count() )
        {
          QDomText fieldName = mDom.createTextNode( theFields.at( fieldNumber ).name() );
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

  QDomNode qgis = mDom.firstChildElement( "qgis" );
  if ( qgis.isNull() )
    return;

  QDomElement properties = qgis.firstChildElement( "properties" );
  if ( properties.isNull() )
    return;

  QDomElement digitizing = properties.firstChildElement( "Digitizing" );
  if ( digitizing.isNull() )
    return;

  QDomElement tolList = digitizing.firstChildElement( "LayerSnappingToleranceList" );
  if ( tolList.isNull() )
    return;

  QDomElement tolUnitList = digitizing.firstChildElement( "LayerSnappingToleranceUnitList" );
  if ( !tolUnitList.isNull() )
    return;

  QStringList units;
  for ( int i = 0; i < tolList.childNodes().count(); i++ )
    units << "0";

  QgsPropertyValue value( units );
  value.writeXML( "LayerSnappingToleranceUnitList", digitizing, mDom );
}

void QgsProjectFileTransform::transform1400to1500()
{
  //Adapt the XML description of the composer legend model to version 1.5
  if ( mDom.isNull() )
  {
    return;
  }
  //Add layer id to <VectorClassificationItem>
  QDomNodeList layerItemList = mDom.elementsByTagName( "LayerItem" );
  QDomElement currentLayerItemElem;
  QString currentLayerId;

  for ( int i = 0; i < layerItemList.size(); ++i )
  {
    currentLayerItemElem = layerItemList.at( i ).toElement();
    if ( currentLayerItemElem.isNull() )
    {
      continue;
    }
    currentLayerId = currentLayerItemElem.attribute( "layerId" );

    QDomNodeList vectorClassificationList = currentLayerItemElem.elementsByTagName( "VectorClassificationItem" );
    QDomElement currentClassificationElem;
    for ( int j = 0; j < vectorClassificationList.size(); ++j )
    {
      currentClassificationElem = vectorClassificationList.at( j ).toElement();
      if ( !currentClassificationElem.isNull() )
      {
        currentClassificationElem.setAttribute( "layerId", currentLayerId );
      }
    }

    //replace the text items with VectorClassification or RasterClassification items
    QDomNodeList textItemList = currentLayerItemElem.elementsByTagName( "TextItem" );
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
        classificationElement = mDom.createElement( "VectorClassificationItem" );
      }
      else
      {
        classificationElement = mDom.createElement( "RasterClassificationItem" );
      }

      classificationElement.setAttribute( "layerId", currentLayerId );
      classificationElement.setAttribute( "text", currentTextItem.attribute( "text" ) );
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

  QDomNodeList layerItemList = mDom.elementsByTagName( "rasterproperties" );
  for ( int i = 0; i < layerItemList.size(); ++i )
  {
    QDomElement rasterPropertiesElem = layerItemList.at( i ).toElement();
    QDomNode layerNode = rasterPropertiesElem.parentNode();
    QDomElement dataSourceElem = layerNode.firstChildElement( "datasource" );
    QDomElement layerNameElem = layerNode.firstChildElement( "layername" );
    QgsRasterLayer rasterLayer;
    // TODO: We have to use more data from project file to read the layer it correctly,
    // OTOH, we should not read it until it was converted
    rasterLayer.readLayerXML( layerNode.toElement() );
    convertRasterProperties( mDom, layerNode, rasterPropertiesElem, &rasterLayer );
  }

  //composer: replace mGridAnnotationPosition with mLeftGridAnnotationPosition & co.
  // and mGridAnnotationDirection with mLeftGridAnnotationDirection & co.
  QDomNodeList composerMapList = mDom.elementsByTagName( "ComposerMap" );
  for ( int i = 0; i < composerMapList.size(); ++i )
  {
    QDomNodeList gridList = composerMapList.at( i ).toElement().elementsByTagName( "Grid" );
    for ( int j = 0; j < gridList.size(); ++j )
    {
      QDomNodeList annotationList = gridList.at( j ).toElement().elementsByTagName( "Annotation" );
      for ( int k = 0; k < annotationList.size(); ++k )
      {
        QDomElement annotationElem = annotationList.at( k ).toElement();

        //position
        if ( annotationElem.hasAttribute( "position" ) )
        {
          int pos = annotationElem.attribute( "position" ).toInt();
          annotationElem.setAttribute( "leftPosition", pos );
          annotationElem.setAttribute( "rightPosition", pos );
          annotationElem.setAttribute( "topPosition", pos );
          annotationElem.setAttribute( "bottomPosition", pos );
          annotationElem.removeAttribute( "position" );
        }

        //direction
        if ( annotationElem.hasAttribute( "direction" ) )
        {
          int dir = annotationElem.attribute( "direction" ).toInt();
          if ( dir == 2 )
          {
            annotationElem.setAttribute( "leftDirection", 0 );
            annotationElem.setAttribute( "rightDirection", 0 );
            annotationElem.setAttribute( "topDirection", 1 );
            annotationElem.setAttribute( "bottomDirection", 1 );
          }
          else if ( dir == 3 )
          {
            annotationElem.setAttribute( "leftDirection", 1 );
            annotationElem.setAttribute( "rightDirection", 1 );
            annotationElem.setAttribute( "topDirection", 0 );
            annotationElem.setAttribute( "bottomDirection", 0 );
          }
          else
          {
            annotationElem.setAttribute( "leftDirection", dir );
            annotationElem.setAttribute( "rightDirection", dir );
            annotationElem.setAttribute( "topDirection", dir );
            annotationElem.setAttribute( "bottomDirection", dir );
          }
          annotationElem.removeAttribute( "direction" );
        }
      }
    }
  }

  //Composer: move all items under Composition element
  QDomNodeList composerList = mDom.elementsByTagName( "Composer" );
  for ( int i = 0; i < composerList.size(); ++i )
  {
    QDomElement composerElem = composerList.at( i ).toElement();

    //find <QgsComposition element
    QDomElement compositionElem = composerElem.firstChildElement( "Composition" );
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
      if ( childElem.tagName() == "Composition" )
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
  QDomNodeList rendererList = mDom.elementsByTagName( "renderer-v2" );
  for ( int i = 0; i < rendererList.size(); ++i )
  {
    QDomNodeList layerList = rendererList.at( i ).toElement().elementsByTagName( "layer" );
    for ( int j = 0; j < layerList.size(); ++j )
    {
      QDomElement layerElem = layerList.at( j ).toElement();
      if ( layerElem.attribute( "class" ) == "SimpleFill" )
      {
        QDomNodeList propList = layerElem.elementsByTagName( "prop" );
        for ( int k = 0; k < propList.size(); ++k )
        {
          QDomElement propElem = propList.at( k ).toElement();
          if ( propElem.attribute( "k" ) == "color" || propElem.attribute( "k" ) == "color_border" )
          {
            propElem.setAttribute( "v", propElem.attribute( "v" ).section( ',', 0, 2 ) + ",255" );
          }
        }
      }
    }
  }

  QgsDebugMsg( mDom.toString() );
}

void QgsProjectFileTransform::transform2200to2300()
{
  //composer: set placement for all picture items to middle, to mimic <=2.2 behaviour
  QDomNodeList composerPictureList = mDom.elementsByTagName( "ComposerPicture" );
  for ( int i = 0; i < composerPictureList.size(); ++i )
  {
    QDomElement picture = composerPictureList.at( i ).toElement();
    picture.setAttribute( "anchorPoint", QString::number( 4 ) );
  }
}

void QgsProjectFileTransform::convertRasterProperties( QDomDocument& doc, QDomNode& parentNode,
    QDomElement& rasterPropertiesElem, QgsRasterLayer* rlayer )
{
  //no data
  //TODO: We would need to set no data on all bands, but we don't know number of bands here
  QDomNode noDataNode = rasterPropertiesElem.namedItem( "mNoDataValue" );
  QDomElement noDataElement = noDataNode.toElement();
  if ( !noDataElement.text().isEmpty() )
  {
    QgsDebugMsg( "mNoDataValue = " + noDataElement.text() );
    QDomElement noDataElem = doc.createElement( "noData" );

    QDomElement noDataRangeList = doc.createElement( "noDataRangeList" );
    noDataRangeList.setAttribute( "bandNo", 1 );

    QDomElement noDataRange =  doc.createElement( "noDataRange" );
    noDataRange.setAttribute( "min", noDataElement.text() );
    noDataRange.setAttribute( "max", noDataElement.text() );
    noDataRangeList.appendChild( noDataRange );

    noDataElem.appendChild( noDataRangeList );

    parentNode.appendChild( noDataElem );
  }

  QDomElement rasterRendererElem = doc.createElement( "rasterrenderer" );
  //convert general properties

  //invert color
  rasterRendererElem.setAttribute( "invertColor", "0" );
  QDomElement  invertColorElem = rasterPropertiesElem.firstChildElement( "mInvertColor" );
  if ( !invertColorElem.isNull() )
  {
    if ( invertColorElem.text() == "true" )
    {
      rasterRendererElem.setAttribute( "invertColor", "1" );
    }
  }

  //opacity
  rasterRendererElem.setAttribute( "opacity", "1" );
  QDomElement transparencyElem = parentNode.firstChildElement( "transparencyLevelInt" );
  if ( !transparencyElem.isNull() )
  {
    double transparency = transparencyElem.text().toInt();
    rasterRendererElem.setAttribute( "opacity", QString::number( transparency / 255.0 ) );
  }

  //alphaBand was not saved until now (bug)
  rasterRendererElem.setAttribute( "alphaBand", -1 );

  //gray band is used for several renderers
  int grayBand = rasterBandNumber( rasterPropertiesElem, "mGrayBandName", rlayer );

  //convert renderer specific properties
  QString drawingStyle = rasterPropertiesElem.firstChildElement( "mDrawingStyle" ).text();

  // While PalettedColor should normaly contain only integer values, usually
  // color palette 0-255, it may happen (Tim, issue #7023) that it contains
  // colormap classification with double values and text labels
  // (which should normaly only appear in SingleBandPseudoColor drawingStyle)
  // => we have to check first the values and change drawingStyle if necessary
  if ( drawingStyle == "PalettedColor" )
  {
    QDomElement customColorRampElem = rasterPropertiesElem.firstChildElement( "customColorRamp" );
    QDomNodeList colorRampEntryList = customColorRampElem.elementsByTagName( "colorRampEntry" );

    for ( int i = 0; i < colorRampEntryList.size(); ++i )
    {
      QDomElement colorRampEntryElem = colorRampEntryList.at( i ).toElement();
      QString strValue = colorRampEntryElem.attribute( "value" );
      double value = strValue.toDouble();
      if ( value < 0 || value > 10000 || !qgsDoubleNear( value, static_cast< int >( value ) ) )
      {
        QgsDebugMsg( QString( "forcing SingleBandPseudoColor value = %1" ).arg( value ) );
        drawingStyle = "SingleBandPseudoColor";
        break;
      }
    }
  }

  if ( drawingStyle == "SingleBandGray" )
  {
    rasterRendererElem.setAttribute( "type", "singlebandgray" );
    rasterRendererElem.setAttribute( "grayBand", grayBand );
    transformContrastEnhancement( doc, rasterPropertiesElem, rasterRendererElem );
  }
  else if ( drawingStyle == "SingleBandPseudoColor" )
  {
    rasterRendererElem.setAttribute( "type", "singlebandpseudocolor" );
    rasterRendererElem.setAttribute( "band", grayBand );
    QDomElement newRasterShaderElem = doc.createElement( "rastershader" );
    QDomElement newColorRampShaderElem = doc.createElement( "colorrampshader" );
    newRasterShaderElem.appendChild( newColorRampShaderElem );
    rasterRendererElem.appendChild( newRasterShaderElem );

    //switch depending on mColorShadingAlgorithm
    QString colorShadingAlgorithm = rasterPropertiesElem.firstChildElement( "mColorShadingAlgorithm" ).text();
    if ( colorShadingAlgorithm == "PseudoColorShader" || colorShadingAlgorithm == "FreakOutShader" )
    {
      newColorRampShaderElem.setAttribute( "colorRampType", "INTERPOLATED" );

      //get minmax from rasterlayer
      QgsRasterBandStats rasterBandStats = rlayer->dataProvider()->bandStatistics( grayBand );
      double minValue = rasterBandStats.minimumValue;
      double maxValue = rasterBandStats.maximumValue;
      double breakSize = ( maxValue - minValue ) / 3;

      QStringList colorList;
      if ( colorShadingAlgorithm == "FreakOutShader" )
      {
        colorList << "#ff00ff" << "#00ffff" << "#ff0000" << "#00ff00";
      }
      else //pseudocolor
      {
        colorList << "#0000ff" << "#00ffff" << "#ffff00" << "#ff0000";
      }
      QStringList::const_iterator colorIt = colorList.constBegin();
      double boundValue = minValue;
      for ( ; colorIt != colorList.constEnd(); ++colorIt )
      {
        QDomElement newItemElem = doc.createElement( "item" );
        newItemElem.setAttribute( "value", QString::number( boundValue ) );
        newItemElem.setAttribute( "label", QString::number( boundValue ) );
        newItemElem.setAttribute( "color", *colorIt );
        newColorRampShaderElem.appendChild( newItemElem );
        boundValue += breakSize;
      }
    }
    else if ( colorShadingAlgorithm == "ColorRampShader" )
    {
      QDomElement customColorRampElem = rasterPropertiesElem.firstChildElement( "customColorRamp" );
      QString type = customColorRampElem.firstChildElement( "colorRampType" ).text();
      newColorRampShaderElem.setAttribute( "colorRampType", type );
      QDomNodeList colorNodeList = customColorRampElem.elementsByTagName( "colorRampEntry" );

      QString value, label;
      QColor newColor;
      int red, green, blue;
      QDomElement currentItemElem;
      for ( int i = 0; i < colorNodeList.size(); ++i )
      {
        currentItemElem = colorNodeList.at( i ).toElement();
        value = currentItemElem.attribute( "value" );
        label = currentItemElem.attribute( "label" );
        red = currentItemElem.attribute( "red" ).toInt();
        green = currentItemElem.attribute( "green" ).toInt();
        blue = currentItemElem.attribute( "blue" ).toInt();
        newColor = QColor( red, green, blue );
        QDomElement newItemElem = doc.createElement( "item" );
        newItemElem.setAttribute( "value", value );
        newItemElem.setAttribute( "label", label );
        newItemElem.setAttribute( "color", newColor.name() );
        newColorRampShaderElem.appendChild( newItemElem );
      }
    }
  }
  else if ( drawingStyle == "PalettedColor" )
  {
    rasterRendererElem.setAttribute( "type", "paletted" );
    rasterRendererElem.setAttribute( "band", grayBand );
    QDomElement customColorRampElem = rasterPropertiesElem.firstChildElement( "customColorRamp" );
    QDomNodeList colorRampEntryList = customColorRampElem.elementsByTagName( "colorRampEntry" );
    QDomElement newColorPaletteElem = doc.createElement( "colorPalette" );

    int red = 0;
    int green = 0;
    int blue = 0;
    int value = 0;
    QDomElement colorRampEntryElem;
    for ( int i = 0; i < colorRampEntryList.size(); ++i )
    {
      colorRampEntryElem = colorRampEntryList.at( i ).toElement();
      QDomElement newPaletteElem = doc.createElement( "paletteEntry" );
      value = static_cast< int >( colorRampEntryElem.attribute( "value" ).toDouble() );
      newPaletteElem.setAttribute( "value", value );
      red = colorRampEntryElem.attribute( "red" ).toInt();
      green = colorRampEntryElem.attribute( "green" ).toInt();
      blue = colorRampEntryElem.attribute( "blue" ).toInt();
      newPaletteElem.setAttribute( "color", QColor( red, green, blue ).name() );
      QString label = colorRampEntryElem.attribute( "label" );
      if ( !label.isEmpty() )
      {
        newPaletteElem.setAttribute( "label", label );
      }
      newColorPaletteElem.appendChild( newPaletteElem );
    }
    rasterRendererElem.appendChild( newColorPaletteElem );
  }
  else if ( drawingStyle == "MultiBandColor" )
  {
    rasterRendererElem.setAttribute( "type", "multibandcolor" );

    //red band, green band, blue band
    int redBand = rasterBandNumber( rasterPropertiesElem, "mRedBandName", rlayer );
    int greenBand = rasterBandNumber( rasterPropertiesElem, "mGreenBandName", rlayer );
    int blueBand = rasterBandNumber( rasterPropertiesElem, "mBlueBandName", rlayer );
    rasterRendererElem.setAttribute( "redBand", redBand );
    rasterRendererElem.setAttribute( "greenBand", greenBand );
    rasterRendererElem.setAttribute( "blueBand", blueBand );

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

void QgsProjectFileTransform::transformContrastEnhancement( QDomDocument& doc, const QDomElement& rasterproperties, QDomElement& rendererElem )
{
  if ( rasterproperties.isNull() || rendererElem.isNull() )
  {
    return;
  }

  double minimumValue = 0;
  double maximumValue = 0;
  QDomElement contrastMinMaxElem = rasterproperties.firstChildElement( "contrastEnhancementMinMaxValues" );
  if ( contrastMinMaxElem.isNull() )
  {
    return;
  }

  QDomElement contrastEnhancementAlgorithmElem = rasterproperties.firstChildElement( "mContrastEnhancementAlgorithm" );
  if ( contrastEnhancementAlgorithmElem.isNull() )
  {
    return;
  }

  //convert enhancement name to enumeration
  int algorithmEnum = 0;
  QString algorithmString = contrastEnhancementAlgorithmElem.text();
  if ( algorithmString == "StretchToMinimumMaximum" )
  {
    algorithmEnum = 1;
  }
  else if ( algorithmString == "StretchAndClipToMinimumMaximum" )
  {
    algorithmEnum = 2;
  }
  else if ( algorithmString == "ClipToMinimumMaximum" )
  {
    algorithmEnum = 3;
  }
  else if ( algorithmString == "UserDefinedEnhancement" )
  {
    algorithmEnum = 4;
  }

  QDomNodeList minMaxEntryList = contrastMinMaxElem.elementsByTagName( "minMaxEntry" );
  QStringList enhancementNameList;
  if ( minMaxEntryList.size() == 1 )
  {
    enhancementNameList << "contrastEnhancement";
  }
  if ( minMaxEntryList.size() ==  3 )
  {
    enhancementNameList << "redContrastEnhancement" << "greenContrastEnhancement" << "blueContrastEnhancement";
  }
  if ( minMaxEntryList.size() > enhancementNameList.size() )
  {
    return;
  }

  QDomElement minMaxEntryElem;
  for ( int i = 0; i < minMaxEntryList.size(); ++i )
  {
    minMaxEntryElem = minMaxEntryList.at( i ).toElement();
    QDomElement minElem = minMaxEntryElem.firstChildElement( "min" );
    if ( minElem.isNull() )
    {
      return;
    }
    minimumValue = minElem.text().toDouble();

    QDomElement maxElem = minMaxEntryElem.firstChildElement( "max" );
    if ( maxElem.isNull() )
    {
      return;
    }
    maximumValue = maxElem.text().toDouble();

    QDomElement newContrastEnhancementElem = doc.createElement( enhancementNameList.at( i ) );
    QDomElement newMinValElem = doc.createElement( "minValue" );
    QDomText minText = doc.createTextNode( QString::number( minimumValue ) );
    newMinValElem.appendChild( minText );
    newContrastEnhancementElem.appendChild( newMinValElem );
    QDomElement newMaxValElem = doc.createElement( "maxValue" );
    QDomText maxText = doc.createTextNode( QString::number( maximumValue ) );
    newMaxValElem.appendChild( maxText );
    newContrastEnhancementElem.appendChild( newMaxValElem );

    QDomElement newAlgorithmElem = doc.createElement( "algorithm" );
    QDomText newAlgorithmText = doc.createTextNode( QString::number( algorithmEnum ) );
    newAlgorithmElem.appendChild( newAlgorithmText );
    newContrastEnhancementElem.appendChild( newAlgorithmElem );

    rendererElem.appendChild( newContrastEnhancementElem );
  }
}

void QgsProjectFileTransform::transformRasterTransparency( QDomDocument& doc, const QDomElement& orig, QDomElement& rendererElem )
{
  //soon...
  Q_UNUSED( doc );
  Q_UNUSED( orig );
  Q_UNUSED( rendererElem );
}

