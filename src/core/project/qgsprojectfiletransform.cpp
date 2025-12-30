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

#include <cstdlib>

#include "qgslogger.h"
#include "qgsmasksymbollayer.h"
#include "qgspathresolver.h"
#include "qgsproject.h"
#include "qgsprojectproperty.h"
#include "qgsprojectversion.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsreadwritecontext.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbollayerreference.h"
#include "qgssymbollayerutils.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsxmlutils.h"

#include <QDomDocument>
#include <QRegularExpression>
#include <QTextStream>

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
  QgsDebugMsgLevel( u"Current project file version is %1.%2.%3"_s
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
  const QDomNodeList composerPictureList = pft->dom().elementsByTagName( u"ComposerPicture"_s );
  for ( int i = 0; i < composerPictureList.size(); ++i )
  {
    QDomElement picture = composerPictureList.at( i ).toElement();
    picture.setAttribute( u"anchorPoint"_s, QString::number( 4 ) );
  }
}

void transform3000( QgsProjectFileTransform *pft )
{
  // transform OTF off to "no projection" for project
  QDomElement propsElem = pft->dom().firstChildElement( u"qgis"_s ).toElement().firstChildElement( u"properties"_s );
  if ( !propsElem.isNull() )
  {
    const QDomNodeList srsNodes = propsElem.elementsByTagName( u"SpatialRefSys"_s );
    QDomElement srsElem;
    QDomElement projElem;
    if ( srsNodes.count() > 0 )
    {
      srsElem = srsNodes.at( 0 ).toElement();
      const QDomNodeList projNodes = srsElem.elementsByTagName( u"ProjectionsEnabled"_s );
      if ( projNodes.count() == 0 )
      {
        projElem = pft->dom().createElement( u"ProjectionsEnabled"_s );
        projElem.setAttribute( u"type"_s, u"int"_s );
        const QDomText projText = pft->dom().createTextNode( u"0"_s );
        projElem.appendChild( projText );
        srsElem.appendChild( projElem );
      }
    }
    else
    {
      srsElem = pft->dom().createElement( u"SpatialRefSys"_s );
      projElem = pft->dom().createElement( u"ProjectionsEnabled"_s );
      projElem.setAttribute( u"type"_s, u"int"_s );
      const QDomText projText = pft->dom().createTextNode( u"0"_s );
      projElem.appendChild( projText );
      srsElem.appendChild( projElem );
      propsElem.appendChild( srsElem );
    }

    // transform map canvas CRS to project CRS - this is because project CRS was inconsistently used
    // prior to 3.0. In >= 3.0 main canvas CRS is forced to match project CRS, so we need to make
    // sure we can read the project CRS correctly
    const QDomNodeList canvasNodes = pft->dom().elementsByTagName( u"mapcanvas"_s );
    if ( canvasNodes.count() > 0 )
    {
      const QDomElement canvasElem = canvasNodes.at( 0 ).toElement();
      const QDomNodeList canvasSrsNodes = canvasElem.elementsByTagName( u"spatialrefsys"_s );
      if ( canvasSrsNodes.count() > 0 )
      {
        const QDomElement canvasSrsElem = canvasSrsNodes.at( 0 ).toElement();
        QString proj;
        QString authid;
        QString srsid;

        const QDomNodeList proj4Nodes = canvasSrsElem.elementsByTagName( u"proj4"_s );
        if ( proj4Nodes.count() > 0 )
        {
          const QDomElement proj4Node = proj4Nodes.at( 0 ).toElement();
          proj = proj4Node.text();
        }
        const QDomNodeList authidNodes = canvasSrsElem.elementsByTagName( u"authid"_s );
        if ( authidNodes.count() > 0 )
        {
          const QDomElement authidNode = authidNodes.at( 0 ).toElement();
          authid = authidNode.text();
        }
        const QDomNodeList srsidNodes = canvasSrsElem.elementsByTagName( u"srsid"_s );
        if ( srsidNodes.count() > 0 )
        {
          const QDomElement srsidNode = srsidNodes.at( 0 ).toElement();
          srsid = srsidNode.text();
        }

        // clear existing project CRS nodes
        const QDomNodeList oldProjectProj4Nodes = srsElem.elementsByTagName( u"ProjectCRSProj4String"_s );
        for ( int i = oldProjectProj4Nodes.count(); i >= 0; --i )
        {
          srsElem.removeChild( oldProjectProj4Nodes.at( i ) );
        }
        const QDomNodeList oldProjectCrsNodes = srsElem.elementsByTagName( u"ProjectCrs"_s );
        for ( int i = oldProjectCrsNodes.count(); i >= 0; --i )
        {
          srsElem.removeChild( oldProjectCrsNodes.at( i ) );
        }
        const QDomNodeList oldProjectCrsIdNodes = srsElem.elementsByTagName( u"ProjectCRSID"_s );
        for ( int i = oldProjectCrsIdNodes.count(); i >= 0; --i )
        {
          srsElem.removeChild( oldProjectCrsIdNodes.at( i ) );
        }
        const QDomNodeList projectionsEnabledNodes = srsElem.elementsByTagName( u"ProjectionsEnabled"_s );
        for ( int i = projectionsEnabledNodes.count(); i >= 0; --i )
        {
          srsElem.removeChild( projectionsEnabledNodes.at( i ) );
        }

        QDomElement proj4Elem = pft->dom().createElement( u"ProjectCRSProj4String"_s );
        proj4Elem.setAttribute( u"type"_s, u"QString"_s );
        const QDomText proj4Text = pft->dom().createTextNode( proj );
        proj4Elem.appendChild( proj4Text );
        QDomElement projectCrsElem = pft->dom().createElement( u"ProjectCrs"_s );
        projectCrsElem.setAttribute( u"type"_s, u"QString"_s );
        const QDomText projectCrsText = pft->dom().createTextNode( authid );
        projectCrsElem.appendChild( projectCrsText );
        QDomElement projectCrsIdElem = pft->dom().createElement( u"ProjectCRSID"_s );
        projectCrsIdElem.setAttribute( u"type"_s, u"int"_s );
        const QDomText srsidText = pft->dom().createTextNode( srsid );
        projectCrsIdElem.appendChild( srsidText );
        QDomElement projectionsEnabledElem = pft->dom().createElement( u"ProjectionsEnabled"_s );
        projectionsEnabledElem.setAttribute( u"type"_s, u"int"_s );
        const QDomText projectionsEnabledText = pft->dom().createTextNode( u"1"_s );
        projectionsEnabledElem.appendChild( projectionsEnabledText );
        srsElem.appendChild( proj4Elem );
        srsElem.appendChild( projectCrsElem );
        srsElem.appendChild( projectCrsIdElem );
        srsElem.appendChild( projectionsEnabledElem );

        const QDomNodeList srsNodes = propsElem.elementsByTagName( u"SpatialRefSys"_s );
        for ( int i = srsNodes.count(); i >= 0; --i )
        {
          propsElem.removeChild( srsNodes.at( i ) );
        }
        propsElem.appendChild( srsElem );
      }
    }
  }


  const QDomNodeList mapLayers = pft->dom().elementsByTagName( u"maplayer"_s );

  for ( int mapLayerIndex = 0; mapLayerIndex < mapLayers.count(); ++mapLayerIndex )
  {
    QDomElement layerElem = mapLayers.at( mapLayerIndex ).toElement();

    // The newly added fieldConfiguration element
    QDomElement fieldConfigurationElement = pft->dom().createElement( u"fieldConfiguration"_s );
    layerElem.appendChild( fieldConfigurationElement );

    const QDomNodeList editTypeNodes = layerElem.namedItem( u"edittypes"_s ).childNodes();
    QDomElement constraintExpressionsElem = pft->dom().createElement( u"constraintExpressions"_s );
    layerElem.appendChild( constraintExpressionsElem );

    for ( int i = 0; i < editTypeNodes.size(); ++i )
    {
      const QDomNode editTypeNode = editTypeNodes.at( i );
      const QDomElement editTypeElement = editTypeNode.toElement();

      QDomElement fieldElement = pft->dom().createElement( u"field"_s );
      fieldConfigurationElement.appendChild( fieldElement );

      const QString name = editTypeElement.attribute( u"name"_s );
      fieldElement.setAttribute( u"name"_s, name );
      QDomElement constraintExpressionElem = pft->dom().createElement( u"constraint"_s );
      constraintExpressionElem.setAttribute( u"field"_s, name );
      constraintExpressionsElem.appendChild( constraintExpressionElem );

      QDomElement editWidgetElement = pft->dom().createElement( u"editWidget"_s );
      fieldElement.appendChild( editWidgetElement );

      const QString ewv2Type = editTypeElement.attribute( u"widgetv2type"_s );
      editWidgetElement.setAttribute( u"type"_s, ewv2Type );

      const QDomElement ewv2CfgElem = editTypeElement.namedItem( u"widgetv2config"_s ).toElement();

      if ( !ewv2CfgElem.isNull() )
      {
        QDomElement editWidgetConfigElement = pft->dom().createElement( u"config"_s );
        editWidgetElement.appendChild( editWidgetConfigElement );

        QVariantMap editWidgetConfiguration;

        const QDomNamedNodeMap configAttrs = ewv2CfgElem.attributes();
        for ( int configIndex = 0; configIndex < configAttrs.count(); ++configIndex )
        {
          const QDomAttr configAttr = configAttrs.item( configIndex ).toAttr();
          if ( configAttr.name() == "fieldEditable"_L1 )
          {
            editWidgetConfigElement.setAttribute( u"fieldEditable"_s, configAttr.value() );
          }
          else if ( configAttr.name() == "labelOnTop"_L1 )
          {
            editWidgetConfigElement.setAttribute( u"labelOnTop"_s, configAttr.value() );
          }
          else if ( configAttr.name() == "notNull"_L1 )
          {
            editWidgetConfigElement.setAttribute( u"notNull"_s, configAttr.value() );
          }
          else if ( configAttr.name() == "constraint"_L1 )
          {
            constraintExpressionElem.setAttribute( u"exp"_s, configAttr.value() );
          }
          else if ( configAttr.name() == "constraintDescription"_L1 )
          {
            constraintExpressionElem.setAttribute( u"desc"_s, configAttr.value() );
          }
          else
          {
            editWidgetConfiguration.insert( configAttr.name(), configAttr.value() );
          }
        }

        if ( ewv2Type == "ValueMap"_L1 )
        {
          const QDomNodeList configElements = ewv2CfgElem.childNodes();
          QVariantMap map;
          for ( int configIndex = 0; configIndex < configElements.count(); ++configIndex )
          {
            const QDomElement configElem = configElements.at( configIndex ).toElement();
            map.insert( configElem.attribute( u"key"_s ), configElem.attribute( u"value"_s ) );
          }
          editWidgetConfiguration.insert( u"map"_s, map );
        }
        else if ( ewv2Type == "Photo"_L1 )
        {
          editWidgetElement.setAttribute( u"type"_s, u"ExternalResource"_s );

          editWidgetConfiguration.insert( u"DocumentViewer"_s, 1 );
          editWidgetConfiguration.insert( u"DocumentViewerHeight"_s, editWidgetConfiguration.value( u"Height"_s ) );
          editWidgetConfiguration.insert( u"DocumentViewerWidth"_s, editWidgetConfiguration.value( u"Width"_s ) );
          editWidgetConfiguration.insert( u"RelativeStorage"_s, 1 );
        }
        else if ( ewv2Type == "FileName"_L1 )
        {
          editWidgetElement.setAttribute( u"type"_s, u"ExternalResource"_s );

          editWidgetConfiguration.insert( u"RelativeStorage"_s, 1 );
        }
        else if ( ewv2Type == "WebView"_L1 )
        {
          editWidgetElement.setAttribute( u"type"_s, u"ExternalResource"_s );

          editWidgetConfiguration.insert( u"DocumentViewerHeight"_s, editWidgetConfiguration.value( u"Height"_s ) );
          editWidgetConfiguration.insert( u"DocumentViewerWidth"_s, editWidgetConfiguration.value( u"Width"_s ) );
          editWidgetConfiguration.insert( u"RelativeStorage"_s, 1 );
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
  const QDomNode noDataNode = rasterPropertiesElem.namedItem( u"mNoDataValue"_s );
  const QDomElement noDataElement = noDataNode.toElement();
  if ( !noDataElement.text().isEmpty() )
  {
    QgsDebugMsgLevel( "mNoDataValue = " + noDataElement.text(), 2 );
    QDomElement noDataElem = doc.createElement( u"noData"_s );

    QDomElement noDataRangeList = doc.createElement( u"noDataRangeList"_s );
    noDataRangeList.setAttribute( u"bandNo"_s, 1 );

    QDomElement noDataRange = doc.createElement( u"noDataRange"_s );
    noDataRange.setAttribute( u"min"_s, noDataElement.text() );
    noDataRange.setAttribute( u"max"_s, noDataElement.text() );
    noDataRangeList.appendChild( noDataRange );

    noDataElem.appendChild( noDataRangeList );

    parentNode.appendChild( noDataElem );
  }

  QDomElement rasterRendererElem = doc.createElement( u"rasterrenderer"_s );
  //convert general properties

  //invert color
  rasterRendererElem.setAttribute( u"invertColor"_s, u"0"_s );
  const QDomElement  invertColorElem = rasterPropertiesElem.firstChildElement( u"mInvertColor"_s );
  if ( !invertColorElem.isNull() )
  {
    if ( invertColorElem.text() == "true"_L1 )
    {
      rasterRendererElem.setAttribute( u"invertColor"_s, u"1"_s );
    }
  }

  //opacity
  rasterRendererElem.setAttribute( u"opacity"_s, u"1"_s );
  const QDomElement transparencyElem = parentNode.firstChildElement( u"transparencyLevelInt"_s );
  if ( !transparencyElem.isNull() )
  {
    const double transparency = transparencyElem.text().toInt();
    rasterRendererElem.setAttribute( u"opacity"_s, QString::number( transparency / 255.0 ) );
  }

  //alphaBand was not saved until now (bug)
  rasterRendererElem.setAttribute( u"alphaBand"_s, -1 );

  //gray band is used for several renderers
  const int grayBand = rasterBandNumber( rasterPropertiesElem, u"mGrayBandName"_s, rlayer );

  //convert renderer specific properties
  QString drawingStyle = rasterPropertiesElem.firstChildElement( u"mDrawingStyle"_s ).text();

  // While PalettedColor should normally contain only integer values, usually
  // color palette 0-255, it may happen (Tim, issue #7023) that it contains
  // colormap classification with double values and text labels
  // (which should normally only appear in SingleBandPseudoColor drawingStyle)
  // => we have to check first the values and change drawingStyle if necessary
  if ( drawingStyle == "PalettedColor"_L1 )
  {
    const QDomElement customColorRampElem = rasterPropertiesElem.firstChildElement( u"customColorRamp"_s );
    const QDomNodeList colorRampEntryList = customColorRampElem.elementsByTagName( u"colorRampEntry"_s );

    for ( int i = 0; i < colorRampEntryList.size(); ++i )
    {
      const QDomElement colorRampEntryElem = colorRampEntryList.at( i ).toElement();
      const QString strValue = colorRampEntryElem.attribute( u"value"_s );
      const double value = strValue.toDouble();
      if ( value < 0 || value > 10000 || !qgsDoubleNear( value, static_cast< int >( value ) ) )
      {
        QgsDebugMsgLevel( u"forcing SingleBandPseudoColor value = %1"_s.arg( value ), 2 );
        drawingStyle = u"SingleBandPseudoColor"_s;
        break;
      }
    }
  }

  if ( drawingStyle == "SingleBandGray"_L1 )
  {
    rasterRendererElem.setAttribute( u"type"_s, u"singlebandgray"_s );
    rasterRendererElem.setAttribute( u"grayBand"_s, grayBand );
    transformContrastEnhancement( doc, rasterPropertiesElem, rasterRendererElem );
  }
  else if ( drawingStyle == "SingleBandPseudoColor"_L1 )
  {
    rasterRendererElem.setAttribute( u"type"_s, u"singlebandpseudocolor"_s );
    rasterRendererElem.setAttribute( u"band"_s, grayBand );
    QDomElement newRasterShaderElem = doc.createElement( u"rastershader"_s );
    QDomElement newColorRampShaderElem = doc.createElement( u"colorrampshader"_s );
    newRasterShaderElem.appendChild( newColorRampShaderElem );
    rasterRendererElem.appendChild( newRasterShaderElem );

    //switch depending on mColorShadingAlgorithm
    const QString colorShadingAlgorithm = rasterPropertiesElem.firstChildElement( u"mColorShadingAlgorithm"_s ).text();
    if ( colorShadingAlgorithm == "PseudoColorShader"_L1 || colorShadingAlgorithm == "FreakOutShader"_L1 )
    {
      newColorRampShaderElem.setAttribute( u"colorRampType"_s, u"INTERPOLATED"_s );

      //get minmax from rasterlayer
      const QgsRasterBandStats rasterBandStats = rlayer->dataProvider()->bandStatistics( grayBand );
      const double minValue = rasterBandStats.minimumValue;
      const double maxValue = rasterBandStats.maximumValue;
      const double breakSize = ( maxValue - minValue ) / 3;

      QStringList colorList;
      if ( colorShadingAlgorithm == "FreakOutShader"_L1 )
      {
        colorList << u"#ff00ff"_s << u"#00ffff"_s << u"#ff0000"_s << u"#00ff00"_s;
      }
      else //pseudocolor
      {
        colorList << u"#0000ff"_s << u"#00ffff"_s << u"#ffff00"_s << u"#ff0000"_s;
      }
      QStringList::const_iterator colorIt = colorList.constBegin();
      double boundValue = minValue;
      for ( ; colorIt != colorList.constEnd(); ++colorIt )
      {
        QDomElement newItemElem = doc.createElement( u"item"_s );
        newItemElem.setAttribute( u"value"_s, QString::number( boundValue ) );
        newItemElem.setAttribute( u"label"_s, QString::number( boundValue ) );
        newItemElem.setAttribute( u"color"_s, *colorIt );
        newColorRampShaderElem.appendChild( newItemElem );
        boundValue += breakSize;
      }
    }
    else if ( colorShadingAlgorithm == "ColorRampShader"_L1 )
    {
      const QDomElement customColorRampElem = rasterPropertiesElem.firstChildElement( u"customColorRamp"_s );
      const QString type = customColorRampElem.firstChildElement( u"colorRampType"_s ).text();
      newColorRampShaderElem.setAttribute( u"colorRampType"_s, type );
      const QDomNodeList colorNodeList = customColorRampElem.elementsByTagName( u"colorRampEntry"_s );

      QString value, label;
      QColor newColor;
      int red, green, blue;
      QDomElement currentItemElem;
      for ( int i = 0; i < colorNodeList.size(); ++i )
      {
        currentItemElem = colorNodeList.at( i ).toElement();
        value = currentItemElem.attribute( u"value"_s );
        label = currentItemElem.attribute( u"label"_s );
        red = currentItemElem.attribute( u"red"_s ).toInt();
        green = currentItemElem.attribute( u"green"_s ).toInt();
        blue = currentItemElem.attribute( u"blue"_s ).toInt();
        newColor = QColor( red, green, blue );
        QDomElement newItemElem = doc.createElement( u"item"_s );
        newItemElem.setAttribute( u"value"_s, value );
        newItemElem.setAttribute( u"label"_s, label );
        newItemElem.setAttribute( u"color"_s, newColor.name() );
        newColorRampShaderElem.appendChild( newItemElem );
      }
    }
  }
  else if ( drawingStyle == "PalettedColor"_L1 )
  {
    rasterRendererElem.setAttribute( u"type"_s, u"paletted"_s );
    rasterRendererElem.setAttribute( u"band"_s, grayBand );
    const QDomElement customColorRampElem = rasterPropertiesElem.firstChildElement( u"customColorRamp"_s );
    const QDomNodeList colorRampEntryList = customColorRampElem.elementsByTagName( u"colorRampEntry"_s );
    QDomElement newColorPaletteElem = doc.createElement( u"colorPalette"_s );

    int red = 0;
    int green = 0;
    int blue = 0;
    int value = 0;
    QDomElement colorRampEntryElem;
    for ( int i = 0; i < colorRampEntryList.size(); ++i )
    {
      colorRampEntryElem = colorRampEntryList.at( i ).toElement();
      QDomElement newPaletteElem = doc.createElement( u"paletteEntry"_s );
      value = static_cast< int >( colorRampEntryElem.attribute( u"value"_s ).toDouble() );
      newPaletteElem.setAttribute( u"value"_s, value );
      red = colorRampEntryElem.attribute( u"red"_s ).toInt();
      green = colorRampEntryElem.attribute( u"green"_s ).toInt();
      blue = colorRampEntryElem.attribute( u"blue"_s ).toInt();
      newPaletteElem.setAttribute( u"color"_s, QColor( red, green, blue ).name() );
      const QString label = colorRampEntryElem.attribute( u"label"_s );
      if ( !label.isEmpty() )
      {
        newPaletteElem.setAttribute( u"label"_s, label );
      }
      newColorPaletteElem.appendChild( newPaletteElem );
    }
    rasterRendererElem.appendChild( newColorPaletteElem );
  }
  else if ( drawingStyle == "MultiBandColor"_L1 )
  {
    rasterRendererElem.setAttribute( u"type"_s, u"multibandcolor"_s );

    //red band, green band, blue band
    const int redBand = rasterBandNumber( rasterPropertiesElem, u"mRedBandName"_s, rlayer );
    const int greenBand = rasterBandNumber( rasterPropertiesElem, u"mGreenBandName"_s, rlayer );
    const int blueBand = rasterBandNumber( rasterPropertiesElem, u"mBlueBandName"_s, rlayer );
    rasterRendererElem.setAttribute( u"redBand"_s, redBand );
    rasterRendererElem.setAttribute( u"greenBand"_s, greenBand );
    rasterRendererElem.setAttribute( u"blueBand"_s, blueBand );

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
  const QDomElement contrastMinMaxElem = rasterproperties.firstChildElement( u"contrastEnhancementMinMaxValues"_s );
  if ( contrastMinMaxElem.isNull() )
  {
    return;
  }

  const QDomElement contrastEnhancementAlgorithmElem = rasterproperties.firstChildElement( u"mContrastEnhancementAlgorithm"_s );
  if ( contrastEnhancementAlgorithmElem.isNull() )
  {
    return;
  }

  //convert enhancement name to enumeration
  int algorithmEnum = 0;
  const QString algorithmString = contrastEnhancementAlgorithmElem.text();
  if ( algorithmString == "StretchToMinimumMaximum"_L1 )
  {
    algorithmEnum = 1;
  }
  else if ( algorithmString == "StretchAndClipToMinimumMaximum"_L1 )
  {
    algorithmEnum = 2;
  }
  else if ( algorithmString == "ClipToMinimumMaximum"_L1 )
  {
    algorithmEnum = 3;
  }
  else if ( algorithmString == "UserDefinedEnhancement"_L1 )
  {
    algorithmEnum = 4;
  }

  const QDomNodeList minMaxEntryList = contrastMinMaxElem.elementsByTagName( u"minMaxEntry"_s );
  QStringList enhancementNameList;
  if ( minMaxEntryList.size() == 1 )
  {
    enhancementNameList << u"contrastEnhancement"_s;
  }
  if ( minMaxEntryList.size() == 3 )
  {
    enhancementNameList << u"redContrastEnhancement"_s << u"greenContrastEnhancement"_s << u"blueContrastEnhancement"_s;
  }
  if ( minMaxEntryList.size() > enhancementNameList.size() )
  {
    return;
  }

  QDomElement minMaxEntryElem;
  for ( int i = 0; i < minMaxEntryList.size(); ++i )
  {
    minMaxEntryElem = minMaxEntryList.at( i ).toElement();
    const QDomElement minElem = minMaxEntryElem.firstChildElement( u"min"_s );
    if ( minElem.isNull() )
    {
      return;
    }
    minimumValue = minElem.text().toDouble();

    const QDomElement maxElem = minMaxEntryElem.firstChildElement( u"max"_s );
    if ( maxElem.isNull() )
    {
      return;
    }
    maximumValue = maxElem.text().toDouble();

    QDomElement newContrastEnhancementElem = doc.createElement( enhancementNameList.at( i ) );
    QDomElement newMinValElem = doc.createElement( u"minValue"_s );
    const QDomText minText = doc.createTextNode( QString::number( minimumValue ) );
    newMinValElem.appendChild( minText );
    newContrastEnhancementElem.appendChild( newMinValElem );
    QDomElement newMaxValElem = doc.createElement( u"maxValue"_s );
    const QDomText maxText = doc.createTextNode( QString::number( maximumValue ) );
    newMaxValElem.appendChild( maxText );
    newContrastEnhancementElem.appendChild( newMaxValElem );

    QDomElement newAlgorithmElem = doc.createElement( u"algorithm"_s );
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
