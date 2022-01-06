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
#include "qgspathresolver.h"
#include "qgsproject.h"
#include "qgsprojectproperty.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterdataprovider.h"
#include "qgsxmlutils.h"

#include <QTextStream>
#include <QDomDocument>
#include <QRegularExpression>
#ifndef QT_NO_PRINTER
#include <QPrinter> //to find out screen resolution
#endif
#include <cstdlib>

typedef QgsProjectVersion PFV;

// Transformer functions below. Declare functions here,
// define them in qgsprojectfiletransform.cpp and add them
// to the transformArray with proper version number
void transformNull( QgsProjectFileTransform *pft ) { Q_UNUSED( pft ) } // Do absolutely nothing
void transform081to090( QgsProjectFileTransform *pft );
void transform091to0100( QgsProjectFileTransform *pft );
void transform0100to0110( QgsProjectFileTransform *pft );
void transform0110to1000( QgsProjectFileTransform *pft );
void transform1100to1200( QgsProjectFileTransform *pft );
void transform1400to1500( QgsProjectFileTransform *pft );
void transform1800to1900( QgsProjectFileTransform *pft );
void transform2200to2300( QgsProjectFileTransform *pft );
void transform3000( QgsProjectFileTransform *pft );

//helper functions
int rasterBandNumber( const QDomElement &rasterPropertiesElem, const QString &bandName, QgsRasterLayer *rlayer );
void transformContrastEnhancement( QDomDocument &doc, const QDomElement &rasterproperties, QDomElement &rendererElem );
void transformRasterTransparency( QDomDocument &doc, const QDomElement &orig, QDomElement &rendererElem );

typedef struct
{
  QgsProjectVersion from;
  QgsProjectVersion to;
  void ( * transformFunc )( QgsProjectFileTransform * );
} TransformItem;

typedef std::vector<TransformItem> Transformers;

bool QgsProjectFileTransform::updateRevision( const QgsProjectVersion &newVersion )
{
  Q_UNUSED( newVersion )
  bool returnValue = false;

  static const Transformers transformers(
  {
    {PFV( 0, 8, 0 ), PFV( 0, 8, 1 ), &transformNull},
    {PFV( 0, 8, 1 ), PFV( 0, 9, 0 ), &transform081to090},
    {PFV( 0, 9, 0 ), PFV( 0, 9, 1 ), &transformNull},
    {PFV( 0, 9, 1 ), PFV( 0, 10, 0 ), &transform091to0100},
    // Following line is a hack that takes us straight from 0.9.2 to 0.11.0
    // due to an unknown bug in migrating 0.9.2 files which we didn't pursue (TS & GS)
    {PFV( 0, 9, 2 ), PFV( 0, 11, 0 ), &transformNull},
    {PFV( 0, 10, 0 ), PFV( 0, 11, 0 ), &transform0100to0110},
    {PFV( 0, 11, 0 ), PFV( 1, 0, 0 ), &transform0110to1000},
    {PFV( 1, 0, 0 ), PFV( 1, 1, 0 ), &transformNull},
    {PFV( 1, 0, 2 ), PFV( 1, 1, 0 ), &transformNull},
    {PFV( 1, 1, 0 ), PFV( 1, 2, 0 ), &transform1100to1200},
    {PFV( 1, 2, 0 ), PFV( 1, 3, 0 ), &transformNull},
    {PFV( 1, 3, 0 ), PFV( 1, 4, 0 ), &transformNull},
    {PFV( 1, 4, 0 ), PFV( 1, 5, 0 ), &transform1400to1500},
    {PFV( 1, 5, 0 ), PFV( 1, 6, 0 ), &transformNull},
    {PFV( 1, 6, 0 ), PFV( 1, 7, 0 ), &transformNull},
    {PFV( 1, 7, 0 ), PFV( 1, 8, 0 ), &transformNull},
    {PFV( 1, 8, 0 ), PFV( 1, 9, 0 ), &transform1800to1900},
    {PFV( 1, 9, 0 ), PFV( 2, 0, 0 ), &transformNull},
    {PFV( 2, 0, 0 ), PFV( 2, 1, 0 ), &transformNull},
    {PFV( 2, 1, 0 ), PFV( 2, 2, 0 ), &transformNull},
    {PFV( 2, 2, 0 ), PFV( 2, 3, 0 ), &transform2200to2300},
    // A transformer with a NULL from version means that it should be run when upgrading
    // from any version and will take care that it's not going to cause trouble if it's
    // run several times on the same file.
    {PFV(), PFV( 3, 0, 0 ), &transform3000},
  } );

  if ( !mDom.isNull() )
  {
    for ( const TransformItem &transformer : transformers )
    {
      if ( transformer.to >= mCurrentVersion && ( transformer.from == mCurrentVersion || transformer.from.isNull() ) )
      {
        // Run the transformer, and update the revision in every case
        ( *( transformer.transformFunc ) )( this );
        mCurrentVersion = transformer.to;
        returnValue = true;
      }
    }
  }
  return returnValue;
}

void QgsProjectFileTransform::dump()
{
  QgsDebugMsg( QStringLiteral( "Current project file version is %1.%2.%3" )
               .arg( mCurrentVersion.majorVersion() )
               .arg( mCurrentVersion.minorVersion() )
               .arg( mCurrentVersion.subVersion() ) );
#ifdef QGISDEBUG
  // Using QgsDebugMsg() didn't print the entire pft->dom()...
  std::cout << mDom.toString( 2 ).toLatin1().constData(); // OK
#endif
}

/*
 *  Transformers below!
 */

void transform081to090( QgsProjectFileTransform *pft )
{
  QgsDebugMsg( QStringLiteral( "Entering..." ) );
  if ( ! pft->dom().isNull() )
  {
    // Start with inserting a mapcanvas element and populate it

    QDomElement mapCanvas; // A null element.

    // there should only be one <qgis>
    QDomNode qgis = pft->dom().firstChildElement( QStringLiteral( "qgis" ) );
    if ( ! qgis.isNull() )
    {
      QgsDebugMsg( QStringLiteral( "Populating new mapcanvas" ) );

      // Create a mapcanvas
      mapCanvas = pft->dom().createElement( QStringLiteral( "mapcanvas" ) );
      // Append mapcanvas to parent 'qgis'.
      qgis.appendChild( mapCanvas );
      // Re-parent units
      mapCanvas.appendChild( qgis.namedItem( QStringLiteral( "units" ) ) );
      // Re-parent extent
      mapCanvas.appendChild( qgis.namedItem( QStringLiteral( "extent" ) ) );

      // See if we can find if projection is on.

      const QDomElement properties = qgis.firstChildElement( QStringLiteral( "properties" ) );
      const QDomElement spatial = properties.firstChildElement( QStringLiteral( "SpatialRefSys" ) );
      const QDomElement hasCrsTransformEnabled = spatial.firstChildElement( QStringLiteral( "ProjectionsEnabled" ) );
      // Type is 'int', and '1' if on.
      // Create an element
      QDomElement projection = pft->dom().createElement( QStringLiteral( "projections" ) );
      QgsDebugMsg( QStringLiteral( "Projection flag: " ) + hasCrsTransformEnabled.text() );
      // Set flag from ProjectionsEnabled
      projection.appendChild( pft->dom().createTextNode( hasCrsTransformEnabled.text() ) );
      // Set new element as child of <mapcanvas>
      mapCanvas.appendChild( projection );

    }


    // Transforming coordinate-transforms
    // Create a list of all map layers
    const QDomNodeList mapLayers = pft->dom().elementsByTagName( QStringLiteral( "maplayer" ) );
    bool doneDestination = false;
    for ( int i = 0; i < mapLayers.count(); i++ )
    {
      QDomNode mapLayer = mapLayers.item( i );
      // Find the coordinatetransform
      const QDomNode coordinateTransform = mapLayer.namedItem( QStringLiteral( "coordinatetransform" ) );
      // Find the sourcesrs
      const QDomNode sourceCrs = coordinateTransform.namedItem( QStringLiteral( "sourcesrs" ) );
      // Rename to srs
      sourceCrs.toElement().setTagName( QStringLiteral( "srs" ) );
      // Re-parent to maplayer
      mapLayer.appendChild( sourceCrs );
      // Re-move coordinatetransform
      // Take the destination CRS of the first layer and use for mapcanvas projection
      if ( ! doneDestination )
      {
        // Use destination CRS from the last layer
        const QDomNode destinationCRS = coordinateTransform.namedItem( QStringLiteral( "destinationsrs" ) );
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
    const QDomNodeList legendLayerFiles = pft->dom().elementsByTagName( QStringLiteral( "legendlayerfile" ) );
    QgsDebugMsg( QStringLiteral( "Legend layer file entries: " ) + QString::number( legendLayerFiles.count() ) );
    for ( int i = 0; i < mapLayers.count(); i++ )
    {
      // Get one maplayer element from list
      const QDomElement mapLayer = mapLayers.item( i ).toElement();
      // Find it's id.
      const QString id = mapLayer.firstChildElement( QStringLiteral( "id" ) ).text();
      QgsDebugMsg( QStringLiteral( "Handling layer %1" ).arg( id ) );
      // Now, look it up in legend
      for ( int j = 0; j < legendLayerFiles.count(); j++ )
      {
        QDomElement legendLayerFile = legendLayerFiles.item( j ).toElement();
        if ( id == legendLayerFile.attribute( QStringLiteral( "layerid" ) ) )
        {
          // Found a the legend layer that matches the maplayer
          QgsDebugMsg( QStringLiteral( "Found matching id" ) );

          // Set visible flag from maplayer to legendlayer
          legendLayerFile.setAttribute( QStringLiteral( "visible" ), mapLayer.attribute( QStringLiteral( "visible" ) ) );

          // Set overview flag from maplayer to legendlayer
          legendLayerFile.setAttribute( QStringLiteral( "isInOverview" ), mapLayer.attribute( QStringLiteral( "showInOverviewFlag" ) ) );
        }
      }
    }
  }
}

void transform091to0100( QgsProjectFileTransform *pft )
{
  if ( ! pft->dom().isNull() )
  {
    // Insert transforms here!
    const QDomNodeList rasterPropertyList = pft->dom().elementsByTagName( QStringLiteral( "rasterproperties" ) );
    QgsDebugMsg( QStringLiteral( "Raster properties file entries: " ) + QString::number( rasterPropertyList.count() ) );
    for ( int i = 0; i < rasterPropertyList.count(); i++ )
    {
      // Get one rasterproperty element from list, and rename the sub-properties.
      const QDomNode rasterProperty = rasterPropertyList.item( i );
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
    const QDomNodeList symbolPropertyList = pft->dom().elementsByTagName( QStringLiteral( "symbol" ) );
    for ( int i = 0; i < symbolPropertyList.count(); i++ )
    {
      // Get the <poinmtsymbol> to check for 'hard:' for each <symbol>
      QDomNode symbolProperty = symbolPropertyList.item( i );

      const QDomElement pointSymbol = symbolProperty.firstChildElement( QStringLiteral( "pointsymbol" ) );
      if ( pointSymbol.text().startsWith( QLatin1String( "hard:" ) ) )
      {
        // Get pointsize and line width
        const int lineWidth = symbolProperty.firstChildElement( QStringLiteral( "outlinewidth" ) ).text().toInt();
        int pointSize = symbolProperty.firstChildElement( QStringLiteral( "pointsize" ) ).text().toInt();
        // Just a precaution, checking for 0
        if ( pointSize != 0 )
        {
          // int r = (s-2*lw)/2-1 --> 2r = (s-2*lw)-2 --> 2r+2 = s-2*lw
          // --> 2r+2+2*lw = s
          // where '2r' is the old size.
          pointSize = pointSize + 2 + 2 * lineWidth;
          QgsDebugMsg( QStringLiteral( "Setting point size to %1" ).arg( pointSize ) );
          QDomElement newPointSizeProperty = pft->dom().createElement( QStringLiteral( "pointsize" ) );
          const QDomText newPointSizeTxt = pft->dom().createTextNode( QString::number( pointSize ) );
          newPointSizeProperty.appendChild( newPointSizeTxt );
          symbolProperty.replaceChild( newPointSizeProperty, pointSymbol );
        }
      }
    }

  }
}

void transform0100to0110( QgsProjectFileTransform *pft )
{
  if ( ! pft->dom().isNull() )
  {
#ifndef QT_NO_PRINTER
    //Change 'outlinewidth' in QgsSymbol
    const QPrinter myPrinter( QPrinter::ScreenResolution );
    const int screenDpi = myPrinter.resolution();
    const double widthScaleFactor = 25.4 / screenDpi;

    const QDomNodeList outlineWidthList = pft->dom().elementsByTagName( QStringLiteral( "outlinewidth" ) );
    for ( int i = 0; i < outlineWidthList.size(); ++i )
    {
      //calculate new width
      QDomElement currentOutlineElem = outlineWidthList.at( i ).toElement();
      double outlineWidth = currentOutlineElem.text().toDouble();
      outlineWidth *= widthScaleFactor;

      //replace old text node
      const QDomNode outlineTextNode = currentOutlineElem.firstChild();
      const QDomText newOutlineText = pft->dom().createTextNode( QString::number( outlineWidth ) );
      currentOutlineElem.replaceChild( newOutlineText, outlineTextNode );

    }

    //Change 'pointsize' in QgsSymbol
    const QDomNodeList pointSizeList = pft->dom().elementsByTagName( QStringLiteral( "pointsize" ) );
    for ( int i = 0; i < pointSizeList.size(); ++i )
    {
      //calculate new size
      QDomElement currentPointSizeElem = pointSizeList.at( i ).toElement();
      double pointSize = currentPointSizeElem.text().toDouble();
      pointSize *= widthScaleFactor;

      //replace old text node
      const QDomNode pointSizeTextNode = currentPointSizeElem.firstChild();
      const QDomText newPointSizeText = pft->dom().createTextNode( QString::number( static_cast< int >( pointSize ) ) );
      currentPointSizeElem.replaceChild( newPointSizeText, pointSizeTextNode );
    }
#endif
  }
}

void transform0110to1000( QgsProjectFileTransform *pft )
{
  if ( ! pft->dom().isNull() )
  {
    const QDomNodeList layerList = pft->dom().elementsByTagName( QStringLiteral( "maplayer" ) );
    for ( int i = 0; i < layerList.size(); ++i )
    {
      const QDomElement layerElem = layerList.at( i ).toElement();
      const QString typeString = layerElem.attribute( QStringLiteral( "type" ) );
      if ( typeString != QLatin1String( "vector" ) )
      {
        continue;
      }

      //datasource
      const QDomNode dataSourceNode = layerElem.namedItem( QStringLiteral( "datasource" ) );
      if ( dataSourceNode.isNull() )
      {
        return;
      }
      const QString dataSource = dataSourceNode.toElement().text();

      //provider key
      const QDomNode providerNode = layerElem.namedItem( QStringLiteral( "provider" ) );
      if ( providerNode.isNull() )
      {
        return;
      }
      const QString providerKey = providerNode.toElement().text();

      //create the layer to get the provider for int->fieldName conversion
      QgsVectorLayer::LayerOptions options { QgsCoordinateTransformContext() };
      options.loadDefaultStyle = false;
      QgsVectorLayer *layer = new QgsVectorLayer( dataSource, QString(), providerKey, options );
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
      const QgsFields fields = provider->fields();

      //read classificationfield
      const QDomNodeList classificationFieldList = layerElem.elementsByTagName( QStringLiteral( "classificationfield" ) );
      for ( int j = 0; j < classificationFieldList.size(); ++j )
      {
        QDomElement classificationFieldElem = classificationFieldList.at( j ).toElement();
        const int fieldNumber = classificationFieldElem.text().toInt();
        if ( fieldNumber >= 0 && fieldNumber < fields.count() )
        {
          const QDomText fieldName = pft->dom().createTextNode( fields.at( fieldNumber ).name() );
          const QDomNode nameNode = classificationFieldElem.firstChild();
          classificationFieldElem.replaceChild( fieldName, nameNode );
        }
      }

    }
  }
}

void transform1100to1200( QgsProjectFileTransform *pft )
{
  QgsDebugMsg( QStringLiteral( "Entering..." ) );
  if ( pft->dom().isNull() )
    return;

  const QDomNode qgis = pft->dom().firstChildElement( QStringLiteral( "qgis" ) );
  if ( qgis.isNull() )
    return;

  const QDomElement properties = qgis.firstChildElement( QStringLiteral( "properties" ) );
  if ( properties.isNull() )
    return;

  QDomElement digitizing = properties.firstChildElement( QStringLiteral( "Digitizing" ) );
  if ( digitizing.isNull() )
    return;

  const QDomElement tolList = digitizing.firstChildElement( QStringLiteral( "LayerSnappingToleranceList" ) );
  if ( tolList.isNull() )
    return;

  const QDomElement tolUnitList = digitizing.firstChildElement( QStringLiteral( "LayerSnappingToleranceUnitList" ) );
  if ( !tolUnitList.isNull() )
    return;

  QStringList units;
  for ( int i = 0; i < tolList.childNodes().count(); i++ )
    units << QStringLiteral( "0" );

  QgsProjectPropertyValue value( units );
  value.writeXml( QStringLiteral( "LayerSnappingToleranceUnitList" ), digitizing, pft->dom() );
}

void transform1400to1500( QgsProjectFileTransform *pft )
{
  //Adapt the XML description of the composer legend model to version 1.5
  if ( pft->dom().isNull() )
  {
    return;
  }
  //Add layer id to <VectorClassificationItem>
  const QDomNodeList layerItemList = pft->dom().elementsByTagName( QStringLiteral( "LayerItem" ) );
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

    const QDomNodeList vectorClassificationList = currentLayerItemElem.elementsByTagName( QStringLiteral( "VectorClassificationItem" ) );
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
    const QDomNodeList textItemList = currentLayerItemElem.elementsByTagName( QStringLiteral( "TextItem" ) );
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
        classificationElement = pft->dom().createElement( QStringLiteral( "VectorClassificationItem" ) );
      }
      else
      {
        classificationElement = pft->dom().createElement( QStringLiteral( "RasterClassificationItem" ) );
      }

      classificationElement.setAttribute( QStringLiteral( "layerId" ), currentLayerId );
      classificationElement.setAttribute( QStringLiteral( "text" ), currentTextItem.attribute( QStringLiteral( "text" ) ) );
      currentLayerItemElem.replaceChild( classificationElement, currentTextItem );
    }
  }
}

void transform1800to1900( QgsProjectFileTransform *pft )
{
  if ( pft->dom().isNull() )
  {
    return;
  }

  QgsReadWriteContext context;
  context.setPathResolver( QgsProject::instance()->pathResolver() );

  const QDomNodeList layerItemList = pft->dom().elementsByTagName( QStringLiteral( "rasterproperties" ) );
  for ( int i = 0; i < layerItemList.size(); ++i )
  {
    QDomElement rasterPropertiesElem = layerItemList.at( i ).toElement();
    QDomNode layerNode = rasterPropertiesElem.parentNode();
    const QDomElement dataSourceElem = layerNode.firstChildElement( QStringLiteral( "datasource" ) );
    const QDomElement layerNameElem = layerNode.firstChildElement( QStringLiteral( "layername" ) );
    QgsRasterLayer rasterLayer;
    // TODO: We have to use more data from project file to read the layer it correctly,
    // OTOH, we should not read it until it was converted
    rasterLayer.readLayerXml( layerNode.toElement(), context );
    QgsProjectFileTransform::convertRasterProperties( pft->dom(), layerNode, rasterPropertiesElem, &rasterLayer );
  }

  //composer: replace mGridAnnotationPosition with mLeftGridAnnotationPosition & co.
  // and mGridAnnotationDirection with mLeftGridAnnotationDirection & co.
  const QDomNodeList composerMapList = pft->dom().elementsByTagName( QStringLiteral( "ComposerMap" ) );
  for ( int i = 0; i < composerMapList.size(); ++i )
  {
    const QDomNodeList gridList = composerMapList.at( i ).toElement().elementsByTagName( QStringLiteral( "Grid" ) );
    for ( int j = 0; j < gridList.size(); ++j )
    {
      const QDomNodeList annotationList = gridList.at( j ).toElement().elementsByTagName( QStringLiteral( "Annotation" ) );
      for ( int k = 0; k < annotationList.size(); ++k )
      {
        QDomElement annotationElem = annotationList.at( k ).toElement();

        //position
        if ( annotationElem.hasAttribute( QStringLiteral( "position" ) ) )
        {
          const int pos = annotationElem.attribute( QStringLiteral( "position" ) ).toInt();
          annotationElem.setAttribute( QStringLiteral( "leftPosition" ), pos );
          annotationElem.setAttribute( QStringLiteral( "rightPosition" ), pos );
          annotationElem.setAttribute( QStringLiteral( "topPosition" ), pos );
          annotationElem.setAttribute( QStringLiteral( "bottomPosition" ), pos );
          annotationElem.removeAttribute( QStringLiteral( "position" ) );
        }

        //direction
        if ( annotationElem.hasAttribute( QStringLiteral( "direction" ) ) )
        {
          const int dir = annotationElem.attribute( QStringLiteral( "direction" ) ).toInt();
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
  const QDomNodeList composerList = pft->dom().elementsByTagName( QStringLiteral( "Composer" ) );
  for ( int i = 0; i < composerList.size(); ++i )
  {
    QDomElement composerElem = composerList.at( i ).toElement();

    //find <QgsComposition element
    QDomElement compositionElem = composerElem.firstChildElement( QStringLiteral( "Composition" ) );
    if ( compositionElem.isNull() )
    {
      continue;
    }

    const QDomNodeList composerChildren = composerElem.childNodes();

    if ( composerChildren.size() < 1 )
    {
      continue;
    }

    for ( int j = composerChildren.size() - 1; j >= 0; --j )
    {
      const QDomElement childElem = composerChildren.at( j ).toElement();
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
  const QDomNodeList rendererList = pft->dom().elementsByTagName( QStringLiteral( "renderer-v2" ) );
  for ( int i = 0; i < rendererList.size(); ++i )
  {
    const QDomNodeList layerList = rendererList.at( i ).toElement().elementsByTagName( QStringLiteral( "layer" ) );
    for ( int j = 0; j < layerList.size(); ++j )
    {
      const QDomElement layerElem = layerList.at( j ).toElement();
      if ( layerElem.attribute( QStringLiteral( "class" ) ) == QLatin1String( "SimpleFill" ) )
      {
        const QDomNodeList propList = layerElem.elementsByTagName( QStringLiteral( "prop" ) );
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

  QgsDebugMsgLevel( pft->dom().toString(), 2 );
}

void transform2200to2300( QgsProjectFileTransform *pft )
{
  //composer: set placement for all picture items to middle, to mimic <=2.2 behavior
  const QDomNodeList composerPictureList = pft->dom().elementsByTagName( QStringLiteral( "ComposerPicture" ) );
  for ( int i = 0; i < composerPictureList.size(); ++i )
  {
    QDomElement picture = composerPictureList.at( i ).toElement();
    picture.setAttribute( QStringLiteral( "anchorPoint" ), QString::number( 4 ) );
  }
}

void transform3000( QgsProjectFileTransform *pft )
{
  // transform OTF off to "no projection" for project
  QDomElement propsElem = pft->dom().firstChildElement( QStringLiteral( "qgis" ) ).toElement().firstChildElement( QStringLiteral( "properties" ) );
  if ( !propsElem.isNull() )
  {
    const QDomNodeList srsNodes = propsElem.elementsByTagName( QStringLiteral( "SpatialRefSys" ) );
    QDomElement srsElem;
    QDomElement projElem;
    if ( srsNodes.count() > 0 )
    {
      srsElem = srsNodes.at( 0 ).toElement();
      const QDomNodeList projNodes = srsElem.elementsByTagName( QStringLiteral( "ProjectionsEnabled" ) );
      if ( projNodes.count() == 0 )
      {
        projElem = pft->dom().createElement( QStringLiteral( "ProjectionsEnabled" ) );
        projElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "int" ) );
        const QDomText projText = pft->dom().createTextNode( QStringLiteral( "0" ) );
        projElem.appendChild( projText );
        srsElem.appendChild( projElem );
      }
    }
    else
    {
      srsElem = pft->dom().createElement( QStringLiteral( "SpatialRefSys" ) );
      projElem = pft->dom().createElement( QStringLiteral( "ProjectionsEnabled" ) );
      projElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "int" ) );
      const QDomText projText = pft->dom().createTextNode( QStringLiteral( "0" ) );
      projElem.appendChild( projText );
      srsElem.appendChild( projElem );
      propsElem.appendChild( srsElem );
    }

    // transform map canvas CRS to project CRS - this is because project CRS was inconsistently used
    // prior to 3.0. In >= 3.0 main canvas CRS is forced to match project CRS, so we need to make
    // sure we can read the project CRS correctly
    const QDomNodeList canvasNodes = pft->dom().elementsByTagName( QStringLiteral( "mapcanvas" ) );
    if ( canvasNodes.count() > 0 )
    {
      const QDomElement canvasElem = canvasNodes.at( 0 ).toElement();
      const QDomNodeList canvasSrsNodes = canvasElem.elementsByTagName( QStringLiteral( "spatialrefsys" ) );
      if ( canvasSrsNodes.count() > 0 )
      {
        const QDomElement canvasSrsElem = canvasSrsNodes.at( 0 ).toElement();
        QString proj;
        QString authid;
        QString srsid;

        const QDomNodeList proj4Nodes = canvasSrsElem.elementsByTagName( QStringLiteral( "proj4" ) );
        if ( proj4Nodes.count() > 0 )
        {
          const QDomElement proj4Node = proj4Nodes.at( 0 ).toElement();
          proj = proj4Node.text();
        }
        const QDomNodeList authidNodes = canvasSrsElem.elementsByTagName( QStringLiteral( "authid" ) );
        if ( authidNodes.count() > 0 )
        {
          const QDomElement authidNode = authidNodes.at( 0 ).toElement();
          authid = authidNode.text();
        }
        const QDomNodeList srsidNodes = canvasSrsElem.elementsByTagName( QStringLiteral( "srsid" ) );
        if ( srsidNodes.count() > 0 )
        {
          const QDomElement srsidNode = srsidNodes.at( 0 ).toElement();
          srsid = srsidNode.text();
        }

        // clear existing project CRS nodes
        const QDomNodeList oldProjectProj4Nodes = srsElem.elementsByTagName( QStringLiteral( "ProjectCRSProj4String" ) );
        for ( int i = oldProjectProj4Nodes.count(); i >= 0; --i )
        {
          srsElem.removeChild( oldProjectProj4Nodes.at( i ) );
        }
        const QDomNodeList oldProjectCrsNodes = srsElem.elementsByTagName( QStringLiteral( "ProjectCrs" ) );
        for ( int i = oldProjectCrsNodes.count(); i >= 0; --i )
        {
          srsElem.removeChild( oldProjectCrsNodes.at( i ) );
        }
        const QDomNodeList oldProjectCrsIdNodes = srsElem.elementsByTagName( QStringLiteral( "ProjectCRSID" ) );
        for ( int i = oldProjectCrsIdNodes.count(); i >= 0; --i )
        {
          srsElem.removeChild( oldProjectCrsIdNodes.at( i ) );
        }
        const QDomNodeList projectionsEnabledNodes = srsElem.elementsByTagName( QStringLiteral( "ProjectionsEnabled" ) );
        for ( int i = projectionsEnabledNodes.count(); i >= 0; --i )
        {
          srsElem.removeChild( projectionsEnabledNodes.at( i ) );
        }

        QDomElement proj4Elem = pft->dom().createElement( QStringLiteral( "ProjectCRSProj4String" ) );
        proj4Elem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "QString" ) );
        const QDomText proj4Text = pft->dom().createTextNode( proj );
        proj4Elem.appendChild( proj4Text );
        QDomElement projectCrsElem = pft->dom().createElement( QStringLiteral( "ProjectCrs" ) );
        projectCrsElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "QString" ) );
        const QDomText projectCrsText = pft->dom().createTextNode( authid );
        projectCrsElem.appendChild( projectCrsText );
        QDomElement projectCrsIdElem = pft->dom().createElement( QStringLiteral( "ProjectCRSID" ) );
        projectCrsIdElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "int" ) );
        const QDomText srsidText = pft->dom().createTextNode( srsid );
        projectCrsIdElem.appendChild( srsidText );
        QDomElement projectionsEnabledElem = pft->dom().createElement( QStringLiteral( "ProjectionsEnabled" ) );
        projectionsEnabledElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "int" ) );
        const QDomText projectionsEnabledText = pft->dom().createTextNode( QStringLiteral( "1" ) );
        projectionsEnabledElem.appendChild( projectionsEnabledText );
        srsElem.appendChild( proj4Elem );
        srsElem.appendChild( projectCrsElem );
        srsElem.appendChild( projectCrsIdElem );
        srsElem.appendChild( projectionsEnabledElem );

        const QDomNodeList srsNodes = propsElem.elementsByTagName( QStringLiteral( "SpatialRefSys" ) );
        for ( int i = srsNodes.count(); i >= 0; --i )
        {
          propsElem.removeChild( srsNodes.at( i ) );
        }
        propsElem.appendChild( srsElem );
      }
    }
  }


  const QDomNodeList mapLayers = pft->dom().elementsByTagName( QStringLiteral( "maplayer" ) );

  for ( int mapLayerIndex = 0; mapLayerIndex < mapLayers.count(); ++mapLayerIndex )
  {
    QDomElement layerElem = mapLayers.at( mapLayerIndex ).toElement();

    // The newly added fieldConfiguration element
    QDomElement fieldConfigurationElement = pft->dom().createElement( QStringLiteral( "fieldConfiguration" ) );
    layerElem.appendChild( fieldConfigurationElement );

    const QDomNodeList editTypeNodes = layerElem.namedItem( QStringLiteral( "edittypes" ) ).childNodes();
    QDomElement constraintExpressionsElem = pft->dom().createElement( QStringLiteral( "constraintExpressions" ) );
    layerElem.appendChild( constraintExpressionsElem );

    for ( int i = 0; i < editTypeNodes.size(); ++i )
    {
      const QDomNode editTypeNode = editTypeNodes.at( i );
      const QDomElement editTypeElement = editTypeNode.toElement();

      QDomElement fieldElement = pft->dom().createElement( QStringLiteral( "field" ) );
      fieldConfigurationElement.appendChild( fieldElement );

      const QString name = editTypeElement.attribute( QStringLiteral( "name" ) );
      fieldElement.setAttribute( QStringLiteral( "name" ), name );
      QDomElement constraintExpressionElem = pft->dom().createElement( QStringLiteral( "constraint" ) );
      constraintExpressionElem.setAttribute( QStringLiteral( "field" ), name );
      constraintExpressionsElem.appendChild( constraintExpressionElem );

      QDomElement editWidgetElement = pft->dom().createElement( QStringLiteral( "editWidget" ) );
      fieldElement.appendChild( editWidgetElement );

      const QString ewv2Type = editTypeElement.attribute( QStringLiteral( "widgetv2type" ) );
      editWidgetElement.setAttribute( QStringLiteral( "type" ), ewv2Type );

      const QDomElement ewv2CfgElem = editTypeElement.namedItem( QStringLiteral( "widgetv2config" ) ).toElement();

      if ( !ewv2CfgElem.isNull() )
      {
        QDomElement editWidgetConfigElement = pft->dom().createElement( QStringLiteral( "config" ) );
        editWidgetElement.appendChild( editWidgetConfigElement );

        QVariantMap editWidgetConfiguration;

        const QDomNamedNodeMap configAttrs = ewv2CfgElem.attributes();
        for ( int configIndex = 0; configIndex < configAttrs.count(); ++configIndex )
        {
          const QDomAttr configAttr = configAttrs.item( configIndex ).toAttr();
          if ( configAttr.name() == QLatin1String( "fieldEditable" ) )
          {
            editWidgetConfigElement.setAttribute( QStringLiteral( "fieldEditable" ), configAttr.value() );
          }
          else if ( configAttr.name() == QLatin1String( "labelOnTop" ) )
          {
            editWidgetConfigElement.setAttribute( QStringLiteral( "labelOnTop" ), configAttr.value() );
          }
          else if ( configAttr.name() == QLatin1String( "notNull" ) )
          {
            editWidgetConfigElement.setAttribute( QStringLiteral( "notNull" ), configAttr.value() );
          }
          else if ( configAttr.name() == QLatin1String( "constraint" ) )
          {
            constraintExpressionElem.setAttribute( QStringLiteral( "exp" ), configAttr.value() );
          }
          else if ( configAttr.name() == QLatin1String( "constraintDescription" ) )
          {
            constraintExpressionElem.setAttribute( QStringLiteral( "desc" ), configAttr.value() );
          }
          else
          {
            editWidgetConfiguration.insert( configAttr.name(), configAttr.value() );
          }
        }

        if ( ewv2Type == QLatin1String( "ValueMap" ) )
        {
          const QDomNodeList configElements = ewv2CfgElem.childNodes();
          QVariantMap map;
          for ( int configIndex = 0; configIndex < configElements.count(); ++configIndex )
          {
            const QDomElement configElem = configElements.at( configIndex ).toElement();
            map.insert( configElem.attribute( QStringLiteral( "key" ) ), configElem.attribute( QStringLiteral( "value" ) ) );
          }
          editWidgetConfiguration.insert( QStringLiteral( "map" ), map );
        }
        else if ( ewv2Type == QLatin1String( "Photo" ) )
        {
          editWidgetElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "ExternalResource" ) );

          editWidgetConfiguration.insert( QStringLiteral( "DocumentViewer" ), 1 );
          editWidgetConfiguration.insert( QStringLiteral( "DocumentViewerHeight" ), editWidgetConfiguration.value( QStringLiteral( "Height" ) ) );
          editWidgetConfiguration.insert( QStringLiteral( "DocumentViewerWidth" ), editWidgetConfiguration.value( QStringLiteral( "Width" ) ) );
          editWidgetConfiguration.insert( QStringLiteral( "RelativeStorage" ), 1 );
        }
        else if ( ewv2Type == QLatin1String( "FileName" ) )
        {
          editWidgetElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "ExternalResource" ) );

          editWidgetConfiguration.insert( QStringLiteral( "RelativeStorage" ), 1 );
        }
        else if ( ewv2Type == QLatin1String( "WebView" ) )
        {
          editWidgetElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "ExternalResource" ) );

          editWidgetConfiguration.insert( QStringLiteral( "DocumentViewerHeight" ), editWidgetConfiguration.value( QStringLiteral( "Height" ) ) );
          editWidgetConfiguration.insert( QStringLiteral( "DocumentViewerWidth" ), editWidgetConfiguration.value( QStringLiteral( "Width" ) ) );
          editWidgetConfiguration.insert( QStringLiteral( "RelativeStorage" ), 1 );
        }

        editWidgetConfigElement.appendChild( QgsXmlUtils::writeVariant( editWidgetConfiguration, pft->dom() ) );
      }
    }
  }
}

void QgsProjectFileTransform::convertRasterProperties( QDomDocument &doc, QDomNode &parentNode,
    QDomElement &rasterPropertiesElem, QgsRasterLayer *rlayer )
{
  //no data
  //TODO: We would need to set no data on all bands, but we don't know number of bands here
  const QDomNode noDataNode = rasterPropertiesElem.namedItem( QStringLiteral( "mNoDataValue" ) );
  const QDomElement noDataElement = noDataNode.toElement();
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
  const QDomElement  invertColorElem = rasterPropertiesElem.firstChildElement( QStringLiteral( "mInvertColor" ) );
  if ( !invertColorElem.isNull() )
  {
    if ( invertColorElem.text() == QLatin1String( "true" ) )
    {
      rasterRendererElem.setAttribute( QStringLiteral( "invertColor" ), QStringLiteral( "1" ) );
    }
  }

  //opacity
  rasterRendererElem.setAttribute( QStringLiteral( "opacity" ), QStringLiteral( "1" ) );
  const QDomElement transparencyElem = parentNode.firstChildElement( QStringLiteral( "transparencyLevelInt" ) );
  if ( !transparencyElem.isNull() )
  {
    const double transparency = transparencyElem.text().toInt();
    rasterRendererElem.setAttribute( QStringLiteral( "opacity" ), QString::number( transparency / 255.0 ) );
  }

  //alphaBand was not saved until now (bug)
  rasterRendererElem.setAttribute( QStringLiteral( "alphaBand" ), -1 );

  //gray band is used for several renderers
  const int grayBand = rasterBandNumber( rasterPropertiesElem, QStringLiteral( "mGrayBandName" ), rlayer );

  //convert renderer specific properties
  QString drawingStyle = rasterPropertiesElem.firstChildElement( QStringLiteral( "mDrawingStyle" ) ).text();

  // While PalettedColor should normally contain only integer values, usually
  // color palette 0-255, it may happen (Tim, issue #7023) that it contains
  // colormap classification with double values and text labels
  // (which should normally only appear in SingleBandPseudoColor drawingStyle)
  // => we have to check first the values and change drawingStyle if necessary
  if ( drawingStyle == QLatin1String( "PalettedColor" ) )
  {
    const QDomElement customColorRampElem = rasterPropertiesElem.firstChildElement( QStringLiteral( "customColorRamp" ) );
    const QDomNodeList colorRampEntryList = customColorRampElem.elementsByTagName( QStringLiteral( "colorRampEntry" ) );

    for ( int i = 0; i < colorRampEntryList.size(); ++i )
    {
      const QDomElement colorRampEntryElem = colorRampEntryList.at( i ).toElement();
      const QString strValue = colorRampEntryElem.attribute( QStringLiteral( "value" ) );
      const double value = strValue.toDouble();
      if ( value < 0 || value > 10000 || !qgsDoubleNear( value, static_cast< int >( value ) ) )
      {
        QgsDebugMsg( QStringLiteral( "forcing SingleBandPseudoColor value = %1" ).arg( value ) );
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
    const QString colorShadingAlgorithm = rasterPropertiesElem.firstChildElement( QStringLiteral( "mColorShadingAlgorithm" ) ).text();
    if ( colorShadingAlgorithm == QLatin1String( "PseudoColorShader" ) || colorShadingAlgorithm == QLatin1String( "FreakOutShader" ) )
    {
      newColorRampShaderElem.setAttribute( QStringLiteral( "colorRampType" ), QStringLiteral( "INTERPOLATED" ) );

      //get minmax from rasterlayer
      const QgsRasterBandStats rasterBandStats = rlayer->dataProvider()->bandStatistics( grayBand );
      const double minValue = rasterBandStats.minimumValue;
      const double maxValue = rasterBandStats.maximumValue;
      const double breakSize = ( maxValue - minValue ) / 3;

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
      const QDomElement customColorRampElem = rasterPropertiesElem.firstChildElement( QStringLiteral( "customColorRamp" ) );
      const QString type = customColorRampElem.firstChildElement( QStringLiteral( "colorRampType" ) ).text();
      newColorRampShaderElem.setAttribute( QStringLiteral( "colorRampType" ), type );
      const QDomNodeList colorNodeList = customColorRampElem.elementsByTagName( QStringLiteral( "colorRampEntry" ) );

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
    const QDomElement customColorRampElem = rasterPropertiesElem.firstChildElement( QStringLiteral( "customColorRamp" ) );
    const QDomNodeList colorRampEntryList = customColorRampElem.elementsByTagName( QStringLiteral( "colorRampEntry" ) );
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
      const QString label = colorRampEntryElem.attribute( QStringLiteral( "label" ) );
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
    const int redBand = rasterBandNumber( rasterPropertiesElem, QStringLiteral( "mRedBandName" ), rlayer );
    const int greenBand = rasterBandNumber( rasterPropertiesElem, QStringLiteral( "mGreenBandName" ), rlayer );
    const int blueBand = rasterBandNumber( rasterPropertiesElem, QStringLiteral( "mBlueBandName" ), rlayer );
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

QDomDocument &QgsProjectFileTransform::dom()
{
  return mDom;
}

QgsProjectVersion QgsProjectFileTransform::currentVersion() const
{
  return mCurrentVersion;
}

int rasterBandNumber( const QDomElement &rasterPropertiesElem, const QString &bandName, QgsRasterLayer *rlayer )
{
  if ( !rlayer )
  {
    return -1;
  }

  const int band = -1;
  const QDomElement rasterBandElem = rasterPropertiesElem.firstChildElement( bandName );
  if ( !rasterBandElem.isNull() )
  {
    const thread_local QRegularExpression re( "(\\d+)" );
    const QRegularExpressionMatch match = re.match( rasterBandElem.text() );
    if ( match.hasMatch() )
    {
      return match.captured( 1 ).toInt();
    }
  }
  return band;
}

void transformContrastEnhancement( QDomDocument &doc, const QDomElement &rasterproperties, QDomElement &rendererElem )
{
  if ( rasterproperties.isNull() || rendererElem.isNull() )
  {
    return;
  }

  double minimumValue = 0;
  double maximumValue = 0;
  const QDomElement contrastMinMaxElem = rasterproperties.firstChildElement( QStringLiteral( "contrastEnhancementMinMaxValues" ) );
  if ( contrastMinMaxElem.isNull() )
  {
    return;
  }

  const QDomElement contrastEnhancementAlgorithmElem = rasterproperties.firstChildElement( QStringLiteral( "mContrastEnhancementAlgorithm" ) );
  if ( contrastEnhancementAlgorithmElem.isNull() )
  {
    return;
  }

  //convert enhancement name to enumeration
  int algorithmEnum = 0;
  const QString algorithmString = contrastEnhancementAlgorithmElem.text();
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

  const QDomNodeList minMaxEntryList = contrastMinMaxElem.elementsByTagName( QStringLiteral( "minMaxEntry" ) );
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
    const QDomElement minElem = minMaxEntryElem.firstChildElement( QStringLiteral( "min" ) );
    if ( minElem.isNull() )
    {
      return;
    }
    minimumValue = minElem.text().toDouble();

    const QDomElement maxElem = minMaxEntryElem.firstChildElement( QStringLiteral( "max" ) );
    if ( maxElem.isNull() )
    {
      return;
    }
    maximumValue = maxElem.text().toDouble();

    QDomElement newContrastEnhancementElem = doc.createElement( enhancementNameList.at( i ) );
    QDomElement newMinValElem = doc.createElement( QStringLiteral( "minValue" ) );
    const QDomText minText = doc.createTextNode( QString::number( minimumValue ) );
    newMinValElem.appendChild( minText );
    newContrastEnhancementElem.appendChild( newMinValElem );
    QDomElement newMaxValElem = doc.createElement( QStringLiteral( "maxValue" ) );
    const QDomText maxText = doc.createTextNode( QString::number( maximumValue ) );
    newMaxValElem.appendChild( maxText );
    newContrastEnhancementElem.appendChild( newMaxValElem );

    QDomElement newAlgorithmElem = doc.createElement( QStringLiteral( "algorithm" ) );
    const QDomText newAlgorithmText = doc.createTextNode( QString::number( algorithmEnum ) );
    newAlgorithmElem.appendChild( newAlgorithmText );
    newContrastEnhancementElem.appendChild( newAlgorithmElem );

    rendererElem.appendChild( newContrastEnhancementElem );
  }
}
