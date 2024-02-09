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
#include "qgsmasksymbollayer.h"
#include "qgsprojectversion.h"
#include "qgslogger.h"
#include "qgsrasterlayer.h"
#include "qgsreadwritecontext.h"
#include "qgsstyleentityvisitor.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgspathresolver.h"
#include "qgsproject.h"
#include "qgsprojectproperty.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterdataprovider.h"
#include "qgsxmlutils.h"
#include "qgssymbollayerreference.h"
#include "qgssymbollayerutils.h"

#include <QTextStream>
#include <QDomDocument>
#include <QRegularExpression>
#include <cstdlib>

typedef QgsProjectVersion PFV;

// Transformer functions below. Declare functions here,
// define them in qgsprojectfiletransform.cpp and add them
// to the transformArray with proper version number
void transformNull( QgsProjectFileTransform *pft ) { Q_UNUSED( pft ) } // Do absolutely nothing
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
    {PFV( 0, 8, 1 ), PFV( 0, 9, 0 ), &transformNull},
    {PFV( 0, 9, 0 ), PFV( 0, 9, 1 ), &transformNull},
    {PFV( 0, 9, 1 ), PFV( 0, 10, 0 ), &transformNull},
    // Following line is a hack that takes us straight from 0.9.2 to 0.11.0
    // due to an unknown bug in migrating 0.9.2 files which we didn't pursue (TS & GS)
    {PFV( 0, 9, 2 ), PFV( 0, 11, 0 ), &transformNull},
    {PFV( 0, 10, 0 ), PFV( 0, 11, 0 ), &transformNull},
    {PFV( 0, 11, 0 ), PFV( 1, 0, 0 ), &transformNull},
    {PFV( 1, 0, 0 ), PFV( 1, 1, 0 ), &transformNull},
    {PFV( 1, 0, 2 ), PFV( 1, 1, 0 ), &transformNull},
    {PFV( 1, 1, 0 ), PFV( 1, 2, 0 ), &transformNull},
    {PFV( 1, 2, 0 ), PFV( 1, 3, 0 ), &transformNull},
    {PFV( 1, 3, 0 ), PFV( 1, 4, 0 ), &transformNull},
    {PFV( 1, 4, 0 ), PFV( 1, 5, 0 ), &transformNull},
    {PFV( 1, 5, 0 ), PFV( 1, 6, 0 ), &transformNull},
    {PFV( 1, 6, 0 ), PFV( 1, 7, 0 ), &transformNull},
    {PFV( 1, 7, 0 ), PFV( 1, 8, 0 ), &transformNull},
    {PFV( 1, 8, 0 ), PFV( 1, 9, 0 ), &transformNull},
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
  QgsDebugMsgLevel( QStringLiteral( "Current project file version is %1.%2.%3" )
                    .arg( mCurrentVersion.majorVersion() )
                    .arg( mCurrentVersion.minorVersion() )
                    .arg( mCurrentVersion.subVersion() ), 1 );
#ifdef QGISDEBUG
  // Using QgsDebugMsgLevel() didn't print the entire pft->dom()...
  std::cout << mDom.toString( 2 ).toLatin1().constData(); // OK
#endif
}

/*
 *  Transformers below!
 */

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
    QgsDebugMsgLevel( "mNoDataValue = " + noDataElement.text(), 2 );
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
        QgsDebugMsgLevel( QStringLiteral( "forcing SingleBandPseudoColor value = %1" ).arg( value ), 2 );
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

void QgsProjectFileTransform::fixOldSymbolLayerReferences( const QMap<QString, QgsMapLayer *> &mapLayers )
{
  for ( QgsMapLayer *ml : mapLayers )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
    if ( !vl )
      continue;

    auto migrateOldReferences = [&mapLayers]( const QList<QgsSymbolLayerReference> &slRefs )
    {
      QList<QgsSymbolLayerReference> newRefs;
      for ( const QgsSymbolLayerReference &slRef : slRefs )
      {
        const QgsVectorLayer *vlRef = qobject_cast<QgsVectorLayer *>( mapLayers[ slRef.layerId() ] );
        const QgsFeatureRenderer *renderer = vlRef ? vlRef->renderer() : nullptr;
        Q_NOWARN_DEPRECATED_PUSH
        QSet<const QgsSymbolLayer *> symbolLayers = renderer ? QgsSymbolLayerUtils::toSymbolLayerPointers(
              renderer, QSet<QgsSymbolLayerId>() << slRef.symbolLayerId() ) : QSet<const QgsSymbolLayer *>();
        Q_NOWARN_DEPRECATED_POP
        const QString slId = symbolLayers.isEmpty() ? QString() : ( *symbolLayers.constBegin() )->id();
        newRefs << QgsSymbolLayerReference( slRef.layerId(), slId );
      }

      return newRefs;
    };

    if ( QgsAbstractVectorLayerLabeling *labeling = vl->labeling() )
    {
      const QStringList subProviders = labeling->subProviders();
      for ( const QString &provider : subProviders )
      {
        QgsPalLayerSettings settings = labeling->settings( provider );
        QgsTextFormat format = settings.format();
        QList<QgsSymbolLayerReference> newMaskedSymbolLayers = migrateOldReferences( format.mask().maskedSymbolLayers() );
        format.mask().setMaskedSymbolLayers( newMaskedSymbolLayers );
        settings.setFormat( format );
        labeling->setSettings( new QgsPalLayerSettings( settings ), provider );
      }
    }

    if ( QgsFeatureRenderer *renderer = vl->renderer() )
    {

      class SymbolLayerVisitor : public QgsStyleEntityVisitorInterface
      {
        public:
          bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override
          {
            return ( node.type == QgsStyleEntityVisitorInterface::NodeType::SymbolRule );
          }

          void visitSymbol( const QgsSymbol *symbol )
          {
            for ( int idx = 0; idx < symbol->symbolLayerCount(); idx++ )
            {
              const QgsSymbolLayer *sl = symbol->symbolLayer( idx );

              // recurse over sub symbols
              const QgsSymbol *subSymbol = const_cast<QgsSymbolLayer *>( sl )->subSymbol();
              if ( subSymbol )
                visitSymbol( subSymbol );

              if ( const QgsMaskMarkerSymbolLayer *maskLayer = dynamic_cast<const QgsMaskMarkerSymbolLayer *>( sl ) )
                maskSymbolLayers << maskLayer;
            }
          }

          bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &leaf ) override
          {
            if ( leaf.entity && leaf.entity->type() == QgsStyle::SymbolEntity )
            {
              auto symbolEntity = static_cast<const QgsStyleSymbolEntity *>( leaf.entity );
              if ( symbolEntity->symbol() )
                visitSymbol( symbolEntity->symbol() );
            }
            return true;
          }

          QList<const QgsMaskMarkerSymbolLayer *> maskSymbolLayers;
      };

      SymbolLayerVisitor visitor;
      renderer->accept( &visitor );

      for ( const QgsMaskMarkerSymbolLayer *maskSymbolLayer : std::as_const( visitor.maskSymbolLayers ) )
        // Ugly but there is no other proper way to get those layer in order to modify them
        const_cast<QgsMaskMarkerSymbolLayer *>( maskSymbolLayer )->setMasks( migrateOldReferences( maskSymbolLayer->masks() ) );
    }
  }
}
