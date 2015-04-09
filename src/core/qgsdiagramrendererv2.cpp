/***************************************************************************
    qgsdiagramrendererv2.cpp
    ---------------------
    begin                : March 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsdiagramrendererv2.h"
#include "qgsvectorlayer.h"
#include "diagram/qgstextdiagram.h"
#include "diagram/qgspiediagram.h"
#include "diagram/qgshistogramdiagram.h"
#include "qgsrendercontext.h"

#include <QDomElement>
#include <QPainter>

QgsDiagramLayerSettings::QgsDiagramLayerSettings()
    : placement( AroundPoint )
    , placementFlags( OnLine )
    , priority( 5 )
    , obstacle( false )
    , dist( 0.0 )
    , renderer( 0 )
    , palLayer( 0 )
    , ct( 0 )
    , xform( 0 )
    , xPosColumn( -1 )
    , yPosColumn( -1 )
    , showAll( true )
{
}

QgsDiagramLayerSettings::~QgsDiagramLayerSettings()
{
  delete renderer;
}

void QgsDiagramLayerSettings::readXML( const QDomElement& elem, const QgsVectorLayer* layer )
{
  Q_UNUSED( layer )

  placement = ( Placement )elem.attribute( "placement" ).toInt();
  placementFlags = ( LinePlacementFlags )elem.attribute( "linePlacementFlags" ).toInt();
  priority = elem.attribute( "priority" ).toInt();
  obstacle = elem.attribute( "obstacle" ).toInt();
  dist = elem.attribute( "dist" ).toDouble();
  xPosColumn = elem.attribute( "xPosColumn" ).toInt();
  yPosColumn = elem.attribute( "yPosColumn" ).toInt();
  showAll = ( elem.attribute( "showAll", "0" ) != "0" );
}

void QgsDiagramLayerSettings::writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const
{
  Q_UNUSED( layer )

  QDomElement diagramLayerElem = doc.createElement( "DiagramLayerSettings" );
  diagramLayerElem.setAttribute( "placement", placement );
  diagramLayerElem.setAttribute( "linePlacementFlags", placementFlags );
  diagramLayerElem.setAttribute( "priority", priority );
  diagramLayerElem.setAttribute( "obstacle", obstacle );
  diagramLayerElem.setAttribute( "dist", QString::number( dist ) );
  diagramLayerElem.setAttribute( "xPosColumn", xPosColumn );
  diagramLayerElem.setAttribute( "yPosColumn", yPosColumn );
  diagramLayerElem.setAttribute( "showAll", showAll );
  layerElem.appendChild( diagramLayerElem );
}

void QgsDiagramSettings::readXML( const QDomElement& elem, const QgsVectorLayer* layer )
{
  Q_UNUSED( layer );

  enabled = ( elem.attribute( "enabled", "1" ) != "0" );
  font.fromString( elem.attribute( "font" ) );
  backgroundColor.setNamedColor( elem.attribute( "backgroundColor" ) );
  backgroundColor.setAlpha( elem.attribute( "backgroundAlpha" ).toInt() );
  size.setWidth( elem.attribute( "width" ).toDouble() );
  size.setHeight( elem.attribute( "height" ).toDouble() );
  transparency = elem.attribute( "transparency", "0" ).toInt();
  penColor.setNamedColor( elem.attribute( "penColor" ) );
  int penAlpha = elem.attribute( "penAlpha", "255" ).toInt();
  penColor.setAlpha( penAlpha );
  penWidth = elem.attribute( "penWidth" ).toDouble();

  minScaleDenominator = elem.attribute( "minScaleDenominator", "-1" ).toDouble();
  maxScaleDenominator = elem.attribute( "maxScaleDenominator", "-1" ).toDouble();

  //mm vs map units
  if ( elem.attribute( "sizeType" ) == "MM" )
  {
    sizeType = MM;
  }
  else
  {
    sizeType = MapUnits;
  }

  //label placement method
  if ( elem.attribute( "labelPlacementMethod" ) == "Height" )
  {
    labelPlacementMethod = Height;
  }
  else
  {
    labelPlacementMethod = XHeight;
  }

  // orientation
  if ( elem.attribute( "diagramOrientation" ) == "Left" )
  {
    diagramOrientation = Left;
  }
  else if ( elem.attribute( "diagramOrientation" ) == "Right" )
  {
    diagramOrientation = Right;
  }
  else if ( elem.attribute( "diagramOrientation" ) == "Down" )
  {
    diagramOrientation = Down;
  }
  else
  {
    diagramOrientation = Up;
  }

  // scale dependency
  if ( elem.attribute( "scaleDependency" ) == "Diameter" )
  {
    scaleByArea = false;
  }
  else
  {
    scaleByArea = true;
  }

  barWidth = elem.attribute( "barWidth" ).toDouble();

  angleOffset = elem.attribute( "angleOffset" ).toInt();

  minimumSize = elem.attribute( "minimumSize" ).toDouble();

  //colors
  categoryColors.clear();
  QDomNodeList attributes = elem.elementsByTagName( "attribute" );

  if ( attributes.length() > 0 )
  {
    for ( uint i = 0; i < attributes.length(); i++ )
    {
      QDomElement attrElem = attributes.at( i ).toElement();
      QColor newColor( attrElem.attribute( "color" ) );
      newColor.setAlpha( 255 - transparency );
      categoryColors.append( newColor );
      categoryAttributes.append( attrElem.attribute( "field" ) );
    }
  }
  else
  {
    // Restore old format attributes and colors

    QStringList colorList = elem.attribute( "colors" ).split( "/" );
    QStringList::const_iterator colorIt = colorList.constBegin();
    for ( ; colorIt != colorList.constEnd(); ++colorIt )
    {
      QColor newColor( *colorIt );
      newColor.setAlpha( 255 - transparency );
      categoryColors.append( QColor( newColor ) );
    }

    //attribute indices
    categoryAttributes.clear();
    QStringList catList = elem.attribute( "categories" ).split( "/" );
    QStringList::const_iterator catIt = catList.constBegin();
    for ( ; catIt != catList.constEnd(); ++catIt )
    {
      categoryAttributes.append( *catIt );
    }
  }
}

void QgsDiagramSettings::writeXML( QDomElement& rendererElem, QDomDocument& doc, const QgsVectorLayer* layer ) const
{
  Q_UNUSED( layer );

  QDomElement categoryElem = doc.createElement( "DiagramCategory" );
  categoryElem.setAttribute( "enabled", enabled );
  categoryElem.setAttribute( "font", font.toString() );
  categoryElem.setAttribute( "backgroundColor", backgroundColor.name() );
  categoryElem.setAttribute( "backgroundAlpha", backgroundColor.alpha() );
  categoryElem.setAttribute( "width", QString::number( size.width() ) );
  categoryElem.setAttribute( "height", QString::number( size.height() ) );
  categoryElem.setAttribute( "penColor", penColor.name() );
  categoryElem.setAttribute( "penAlpha", penColor.alpha() );
  categoryElem.setAttribute( "penWidth", QString::number( penWidth ) );
  categoryElem.setAttribute( "minScaleDenominator", QString::number( minScaleDenominator ) );
  categoryElem.setAttribute( "maxScaleDenominator", QString::number( maxScaleDenominator ) );
  categoryElem.setAttribute( "transparency", QString::number( transparency ) );

  // site type (mm vs. map units)
  if ( sizeType == MM )
  {
    categoryElem.setAttribute( "sizeType", "MM" );
  }
  else
  {
    categoryElem.setAttribute( "sizeType", "MapUnits" );
  }

  // label placement method (text diagram)
  if ( labelPlacementMethod == Height )
  {
    categoryElem.setAttribute( "labelPlacementMethod", "Height" );
  }
  else
  {
    categoryElem.setAttribute( "labelPlacementMethod", "XHeight" );
  }

  if ( scaleByArea )
  {
    categoryElem.setAttribute( "scaleDependency", "Area" );
  }
  else
  {
    categoryElem.setAttribute( "scaleDependency", "Diameter" );
  }

  // orientation (histogram)
  switch ( diagramOrientation )
  {
    case Left:
      categoryElem.setAttribute( "diagramOrientation", "Left" );
      break;

    case Right:
      categoryElem.setAttribute( "diagramOrientation", "Right" );
      break;

    case Down:
      categoryElem.setAttribute( "diagramOrientation", "Down" );
      break;

    case Up:
      categoryElem.setAttribute( "diagramOrientation", "Up" );
      break;

    default:
      categoryElem.setAttribute( "diagramOrientation", "Up" );
      break;
  }

  categoryElem.setAttribute( "barWidth", QString::number( barWidth ) );
  categoryElem.setAttribute( "minimumSize", QString::number( minimumSize ) );
  categoryElem.setAttribute( "angleOffset", QString::number( angleOffset ) );

  QString colors;
  int nCats = qMin( categoryColors.size(), categoryAttributes.size() );
  for ( int i = 0; i < nCats; ++i )
  {
    QDomElement attributeElem = doc.createElement( "attribute" );

    attributeElem.setAttribute( "field", categoryAttributes.at( i ) );
    attributeElem.setAttribute( "color", categoryColors.at( i ).name() );
    categoryElem.appendChild( attributeElem );
  }

  rendererElem.appendChild( categoryElem );
}

QgsDiagramRendererV2::QgsDiagramRendererV2()
    : mDiagram( 0 )
{
}

QgsDiagramRendererV2::~QgsDiagramRendererV2()
{
  delete mDiagram;
}

void QgsDiagramRendererV2::setDiagram( QgsDiagram* d )
{
  delete mDiagram;
  mDiagram = d;
}

QgsDiagramRendererV2::QgsDiagramRendererV2( const QgsDiagramRendererV2& other )
    : mDiagram( other.mDiagram ? other.mDiagram->clone() : 0 )
{
}

void QgsDiagramRendererV2::renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QPointF& pos )
{
  if ( !mDiagram )
  {
    return;
  }

  QgsDiagramSettings s;
  if ( !diagramSettings( feature, c, s ) )
  {
    return;
  }

  mDiagram->renderDiagram( feature, c, s, pos );
}

QSizeF QgsDiagramRendererV2::sizeMapUnits( const QgsFeature& feature, const QgsRenderContext& c )
{
  QgsDiagramSettings s;
  if ( !diagramSettings( feature, c, s ) )
  {
    return QSizeF();
  }

  QSizeF size = diagramSize( feature, c );
  if ( s.sizeType == QgsDiagramSettings::MM )
  {
    convertSizeToMapUnits( size, c );
  }
  return size;
}

void QgsDiagramRendererV2::convertSizeToMapUnits( QSizeF& size, const QgsRenderContext& context ) const
{
  if ( !size.isValid() )
  {
    return;
  }

  double pixelToMap = context.scaleFactor() * context.mapToPixel().mapUnitsPerPixel();
  size.rwidth() *= pixelToMap;
  size.rheight() *= pixelToMap;
}

int QgsDiagramRendererV2::dpiPaintDevice( const QPainter* painter )
{
  if ( painter )
  {
    QPaintDevice* device = painter->device();
    if ( device )
    {
      return device->logicalDpiX();
    }
  }
  return -1;
}

void QgsDiagramRendererV2::_readXML( const QDomElement& elem, const QgsVectorLayer* layer )
{
  Q_UNUSED( layer )

  delete mDiagram;
  QString diagramType = elem.attribute( "diagramType" );
  if ( diagramType == "Pie" )
  {
    mDiagram = new QgsPieDiagram();
  }
  else if ( diagramType == "Text" )
  {
    mDiagram = new QgsTextDiagram();
  }
  else if ( diagramType == "Histogram" )
  {
    mDiagram = new QgsHistogramDiagram();
  }
  else
  {
    mDiagram = 0;
  }
}

void QgsDiagramRendererV2::_writeXML( QDomElement& rendererElem, QDomDocument& doc, const QgsVectorLayer* layer ) const
{
  Q_UNUSED( doc );
  Q_UNUSED( layer )

  if ( mDiagram )
  {
    rendererElem.setAttribute( "diagramType", mDiagram->diagramName() );
  }
}

QgsSingleCategoryDiagramRenderer::QgsSingleCategoryDiagramRenderer(): QgsDiagramRendererV2()
{
}

QgsSingleCategoryDiagramRenderer::~QgsSingleCategoryDiagramRenderer()
{
}

QgsDiagramRendererV2* QgsSingleCategoryDiagramRenderer::clone() const
{
  return new QgsSingleCategoryDiagramRenderer( *this );
}

bool QgsSingleCategoryDiagramRenderer::diagramSettings( const QgsFeature&, const QgsRenderContext& c, QgsDiagramSettings& s )
{
  Q_UNUSED( c );
  s = mSettings;
  return true;
}

QSizeF QgsSingleCategoryDiagramRenderer::diagramSize( const QgsFeature &feature, const QgsRenderContext &c )
{
  return mDiagram->diagramSize( feature.attributes(), c, mSettings );
}

QList<QgsDiagramSettings> QgsSingleCategoryDiagramRenderer::diagramSettings() const
{
  QList<QgsDiagramSettings> settingsList;
  settingsList.push_back( mSettings );
  return settingsList;
}

void QgsSingleCategoryDiagramRenderer::readXML( const QDomElement& elem, const QgsVectorLayer* layer )
{
  QDomElement categoryElem = elem.firstChildElement( "DiagramCategory" );
  if ( categoryElem.isNull() )
  {
    return;
  }

  mSettings.readXML( categoryElem, layer );
  _readXML( elem, layer );
}

void QgsSingleCategoryDiagramRenderer::writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const
{
  QDomElement rendererElem = doc.createElement( "SingleCategoryDiagramRenderer" );
  mSettings.writeXML( rendererElem, doc, layer );
  _writeXML( rendererElem, doc, layer );
  layerElem.appendChild( rendererElem );
}


QgsLinearlyInterpolatedDiagramRenderer::QgsLinearlyInterpolatedDiagramRenderer(): QgsDiagramRendererV2()
{
  mInterpolationSettings.classificationAttributeIsExpression = false;
}

QgsLinearlyInterpolatedDiagramRenderer::~QgsLinearlyInterpolatedDiagramRenderer()
{
}

QgsDiagramRendererV2 *QgsLinearlyInterpolatedDiagramRenderer::clone() const
{
  return new QgsLinearlyInterpolatedDiagramRenderer( *this );
}

QList<QgsDiagramSettings> QgsLinearlyInterpolatedDiagramRenderer::diagramSettings() const
{
  QList<QgsDiagramSettings> settingsList;
  settingsList.push_back( mSettings );
  return settingsList;
}

bool QgsLinearlyInterpolatedDiagramRenderer::diagramSettings( const QgsFeature& feature, const QgsRenderContext& c, QgsDiagramSettings& s )
{
  s = mSettings;
  s.size = diagramSize( feature, c );
  return true;
}

QList<QString> QgsLinearlyInterpolatedDiagramRenderer::diagramAttributes() const
{
  return mSettings.categoryAttributes;
}

QSizeF QgsLinearlyInterpolatedDiagramRenderer::diagramSize( const QgsFeature& feature, const QgsRenderContext& c )
{
  return mDiagram->diagramSize( feature, c, mSettings, mInterpolationSettings );
}

void QgsLinearlyInterpolatedDiagramRenderer::readXML( const QDomElement& elem, const QgsVectorLayer* layer )
{
  mInterpolationSettings.lowerValue = elem.attribute( "lowerValue" ).toDouble();
  mInterpolationSettings.upperValue = elem.attribute( "upperValue" ).toDouble();
  mInterpolationSettings.lowerSize.setWidth( elem.attribute( "lowerWidth" ).toDouble() );
  mInterpolationSettings.lowerSize.setHeight( elem.attribute( "lowerHeight" ).toDouble() );
  mInterpolationSettings.upperSize.setWidth( elem.attribute( "upperWidth" ).toDouble() );
  mInterpolationSettings.upperSize.setHeight( elem.attribute( "upperHeight" ).toDouble() );
  mInterpolationSettings.classificationAttributeIsExpression = elem.hasAttribute( "classificationAttributeExpression" );
  if ( mInterpolationSettings.classificationAttributeIsExpression )
  {
    mInterpolationSettings.classificationAttributeExpression = elem.attribute( "classificationAttributeExpression" );
  }
  else
  {
    mInterpolationSettings.classificationAttribute = elem.attribute( "classificationAttribute" ).toInt();
  }
  QDomElement settingsElem = elem.firstChildElement( "DiagramCategory" );
  if ( !settingsElem.isNull() )
  {
    mSettings.readXML( settingsElem, layer );
  }
  _readXML( elem, layer );
}

void QgsLinearlyInterpolatedDiagramRenderer::writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const
{
  QDomElement rendererElem = doc.createElement( "LinearlyInterpolatedDiagramRenderer" );
  rendererElem.setAttribute( "lowerValue", QString::number( mInterpolationSettings.lowerValue ) );
  rendererElem.setAttribute( "upperValue", QString::number( mInterpolationSettings.upperValue ) );
  rendererElem.setAttribute( "lowerWidth", QString::number( mInterpolationSettings.lowerSize.width() ) );
  rendererElem.setAttribute( "lowerHeight", QString::number( mInterpolationSettings.lowerSize.height() ) );
  rendererElem.setAttribute( "upperWidth", QString::number( mInterpolationSettings.upperSize.width() ) );
  rendererElem.setAttribute( "upperHeight", QString::number( mInterpolationSettings.upperSize.height() ) );
  if ( mInterpolationSettings.classificationAttributeIsExpression )
  {
    rendererElem.setAttribute( "classificationAttributeExpression", mInterpolationSettings.classificationAttributeExpression );
  }
  else
  {
    rendererElem.setAttribute( "classificationAttribute", mInterpolationSettings.classificationAttribute );
  }
  mSettings.writeXML( rendererElem, doc, layer );
  _writeXML( rendererElem, doc, layer );
  layerElem.appendChild( rendererElem );
}
