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
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QTextStream>
#include <QDomDocument>
#include <QPrinter> //to find out screen resolution
#include <cstdlib>
#include "qgsprojectproperty.h"

typedef QgsProjectVersion PFV;


QgsProjectFileTransform::transform QgsProjectFileTransform::transformers[] =
{
  {PFV( 0, 8, 0 ), PFV( 0, 8, 1 ), &QgsProjectFileTransform::transformNull},
  {PFV( 0, 8, 1 ), PFV( 0, 9, 0 ), &QgsProjectFileTransform::transform081to090},
  {PFV( 0, 9, 0 ), PFV( 0, 9, 1 ), &QgsProjectFileTransform::transformNull},
  {PFV( 0, 9, 1 ), PFV( 0, 10, 0 ), &QgsProjectFileTransform::transform091to0100},
  {PFV( 0, 9, 2 ), PFV( 0, 10, 0 ), &QgsProjectFileTransform::transformNull},
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
};

bool QgsProjectFileTransform::updateRevision( QgsProjectVersion newVersion )
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
  QgsDebugMsg( "entering" );
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
      QDomText newPointSizeText = mDom.createTextNode( QString::number(( int )pointSize ) );
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
      QgsFieldMap theFieldMap = theProvider->fields();

      //read classificationfield
      QDomNodeList classificationFieldList = layerElem.elementsByTagName( "classificationfield" );
      for ( int j = 0; j < classificationFieldList.size(); ++j )
      {
        QDomElement classificationFieldElem = classificationFieldList.at( j ).toElement();
        int fieldNumber = classificationFieldElem.text().toInt();
        QgsFieldMap::const_iterator field_it = theFieldMap.find( fieldNumber );
        if ( field_it != theFieldMap.constEnd() )
        {
          QDomText fieldName = mDom.createTextNode( field_it.value().name() );
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
      if ( vectorClassificationList.size() > 0 ) //we guess it is a vector layer
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

