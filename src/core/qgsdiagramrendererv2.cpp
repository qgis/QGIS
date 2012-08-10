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
#include "diagram/qgstextdiagram.h"
#include "diagram/qgspiediagram.h"
#include "diagram/qgshistogramdiagram.h"
#include "qgsrendercontext.h"
#include <QDomElement>
#include <QPainter>


void QgsDiagramLayerSettings::readXML( const QDomElement& elem )
{
  placement = ( Placement )elem.attribute( "placement" ).toInt();
  placementFlags = ( LinePlacementFlags )elem.attribute( "linePlacementFlags" ).toInt();
  priority = elem.attribute( "priority" ).toInt();
  obstacle = elem.attribute( "obstacle" ).toInt();
  dist = elem.attribute( "dist" ).toDouble();
  xPosColumn = elem.attribute( "xPosColumn" ).toInt();
  yPosColumn = elem.attribute( "yPosColumn" ).toInt();
}

void QgsDiagramLayerSettings::writeXML( QDomElement& layerElem, QDomDocument& doc ) const
{
  QDomElement diagramLayerElem = doc.createElement( "DiagramLayerSettings" );
  diagramLayerElem.setAttribute( "placement", placement );
  diagramLayerElem.setAttribute( "linePlacementFlags", placementFlags );
  diagramLayerElem.setAttribute( "priority", priority );
  diagramLayerElem.setAttribute( "obstacle", obstacle );
  diagramLayerElem.setAttribute( "dist", QString::number( dist ) );
  diagramLayerElem.setAttribute( "xPosColumn", xPosColumn );
  diagramLayerElem.setAttribute( "yPosColumn", yPosColumn );
  layerElem.appendChild( diagramLayerElem );
}

void QgsDiagramSettings::readXML( const QDomElement& elem )
{
  font.fromString( elem.attribute( "font" ) );
  backgroundColor.setNamedColor( elem.attribute( "backgroundColor" ) );
  backgroundColor.setAlpha( elem.attribute( "backgroundAlpha" ).toInt() );
  size.setWidth( elem.attribute( "width" ).toDouble() );
  size.setHeight( elem.attribute( "height" ).toDouble() );
  penColor.setNamedColor( elem.attribute( "penColor" ) );
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

  barWidth = elem.attribute( "barWidth" ).toDouble();

  minimumSize = elem.attribute( "minimumSize" ).toDouble();

  //colors
  categoryColors.clear();
  QStringList colorList = elem.attribute( "colors" ).split( "/" );
  QStringList::const_iterator colorIt = colorList.constBegin();
  for ( ; colorIt != colorList.constEnd(); ++colorIt )
  {
    categoryColors.append( QColor( *colorIt ) );
  }

  //attribute indices
  categoryIndices.clear();
  QStringList catList = elem.attribute( "categories" ).split( "/" );
  QStringList::const_iterator catIt = catList.constBegin();
  for ( ; catIt != catList.constEnd(); ++catIt )
  {
    categoryIndices.append( catIt->toInt() );
  }
}

void QgsDiagramSettings::writeXML( QDomElement& rendererElem, QDomDocument& doc ) const
{
  QDomElement categoryElem = doc.createElement( "DiagramCategory" );
  categoryElem.setAttribute( "font", font.toString() );
  categoryElem.setAttribute( "backgroundColor", backgroundColor.name() );
  categoryElem.setAttribute( "backgroundAlpha", backgroundColor.alpha() );
  categoryElem.setAttribute( "width", QString::number( size.width() ) );
  categoryElem.setAttribute( "height", QString::number( size.height() ) );
  categoryElem.setAttribute( "penColor", penColor.name() );
  categoryElem.setAttribute( "penWidth", QString::number( penWidth ) );
  categoryElem.setAttribute( "minScaleDenominator", QString::number( minScaleDenominator ) );
  categoryElem.setAttribute( "maxScaleDenominator", QString::number( maxScaleDenominator ) );

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

  // orientation (histogram)
  switch ( diagramOrientation ) {
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

  QString colors;
  for ( int i = 0; i < categoryColors.size(); ++i )
  {
    if ( i > 0 )
    {
      colors.append( "/" );
    }
    colors.append( categoryColors.at( i ).name() );
  }
  categoryElem.setAttribute( "colors", colors );

  QString categories;
  for ( int i = 0; i < categoryIndices.size(); ++i )
  {
    if ( i > 0 )
    {
      categories.append( "/" );
    }
    categories.append( QString::number( categoryIndices.at( i ) ) );
  }
  categoryElem.setAttribute( "categories", categories );

  rendererElem.appendChild( categoryElem );
}

QgsDiagramRendererV2::QgsDiagramRendererV2(): mDiagram( 0 )
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

void QgsDiagramRendererV2::renderDiagram( const QgsAttributeMap& att, QgsRenderContext& c, const QPointF& pos )
{
  if ( !mDiagram )
  {
    return;
  }

  QgsDiagramSettings s;
  if ( !diagramSettings( att, c, s ) )
  {
    return;
  }

  mDiagram->renderDiagram( att, c, s, pos );
}

QSizeF QgsDiagramRendererV2::sizeMapUnits( const QgsAttributeMap& attributes, const QgsRenderContext& c )
{
  QgsDiagramSettings s;
  if ( !diagramSettings( attributes, c, s ) )
  {
    return QSizeF();
  }

  QSizeF size = diagramSize( attributes, c );
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

  int dpi = dpiPaintDevice( context.constPainter() );
  if ( dpi < 0 )
  {
    return;
  }

  double pixelToMap = dpi / 25.4 * context.mapToPixel().mapUnitsPerPixel();
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

void QgsDiagramRendererV2::_readXML( const QDomElement& elem )
{
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

void QgsDiagramRendererV2::_writeXML( QDomElement& rendererElem, QDomDocument& doc ) const
{
  Q_UNUSED( doc );
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

bool QgsSingleCategoryDiagramRenderer::diagramSettings( const QgsAttributeMap&, const QgsRenderContext& c, QgsDiagramSettings& s )
{
  Q_UNUSED( c );
  s = mSettings;
  return true;
}

QSizeF QgsSingleCategoryDiagramRenderer::diagramSize(const QgsAttributeMap &attributes, const QgsRenderContext &c)
{
  return mDiagram->diagramSize( attributes, c, mSettings );
}

QList<QgsDiagramSettings> QgsSingleCategoryDiagramRenderer::diagramSettings() const
{
  QList<QgsDiagramSettings> settingsList;
  settingsList.push_back( mSettings );
  return settingsList;
}

void QgsSingleCategoryDiagramRenderer::readXML( const QDomElement& elem )
{
  QDomElement categoryElem = elem.firstChildElement( "DiagramCategory" );
  if ( categoryElem.isNull() )
  {
    return;
  }

  mSettings.readXML( categoryElem );
  _readXML( elem );
}

void QgsSingleCategoryDiagramRenderer::writeXML( QDomElement& layerElem, QDomDocument& doc ) const
{
  QDomElement rendererElem = doc.createElement( "SingleCategoryDiagramRenderer" );
  mSettings.writeXML( rendererElem, doc );
  _writeXML( rendererElem, doc );
  layerElem.appendChild( rendererElem );
}


QgsLinearlyInterpolatedDiagramRenderer::QgsLinearlyInterpolatedDiagramRenderer(): QgsDiagramRendererV2()
{
}

QgsLinearlyInterpolatedDiagramRenderer::~QgsLinearlyInterpolatedDiagramRenderer()
{
}

QList<QgsDiagramSettings> QgsLinearlyInterpolatedDiagramRenderer::diagramSettings() const
{
  QList<QgsDiagramSettings> settingsList;
  settingsList.push_back( mSettings );
  return settingsList;
}

bool QgsLinearlyInterpolatedDiagramRenderer::diagramSettings( const QgsAttributeMap& attributes, const QgsRenderContext& c, QgsDiagramSettings& s )
{
  s = mSettings;
  s.size = diagramSize( attributes, c );
  return true;
}

QList<int> QgsLinearlyInterpolatedDiagramRenderer::diagramAttributes() const
{
  QList<int> attributes = mSettings.categoryIndices;
  if ( !attributes.contains( mInterpolationSettings.classificationAttribute ) )
  {
    attributes.push_back( mInterpolationSettings.classificationAttribute );
  }
  return attributes;
}

QSizeF QgsLinearlyInterpolatedDiagramRenderer::diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c )
{
  return mDiagram->diagramSize( attributes, c, mSettings, mInterpolationSettings );
}

void QgsLinearlyInterpolatedDiagramRenderer::readXML( const QDomElement& elem )
{
  mInterpolationSettings.lowerValue = elem.attribute( "lowerValue" ).toDouble();
  mInterpolationSettings.upperValue = elem.attribute( "upperValue" ).toDouble();
  mInterpolationSettings.lowerSize.setWidth( elem.attribute( "lowerWidth" ).toDouble() );
  mInterpolationSettings.lowerSize.setHeight( elem.attribute( "lowerHeight" ).toDouble() );
  mInterpolationSettings.upperSize.setWidth( elem.attribute( "upperWidth" ).toDouble() );
  mInterpolationSettings.upperSize.setHeight( elem.attribute( "upperHeight" ).toDouble() );
  mInterpolationSettings.classificationAttribute = elem.attribute( "classificationAttribute" ).toInt();
  QDomElement settingsElem = elem.firstChildElement( "DiagramCategory" );
  if ( !settingsElem.isNull() )
  {
    mSettings.readXML( settingsElem );
  }
  _readXML( elem );
}

void QgsLinearlyInterpolatedDiagramRenderer::writeXML( QDomElement& layerElem, QDomDocument& doc ) const
{
  QDomElement rendererElem = doc.createElement( "LinearlyInterpolatedDiagramRenderer" );
  rendererElem.setAttribute( "lowerValue", QString::number( mInterpolationSettings.lowerValue ) );
  rendererElem.setAttribute( "upperValue", QString::number( mInterpolationSettings.upperValue ) );
  rendererElem.setAttribute( "lowerWidth", QString::number( mInterpolationSettings.lowerSize.width() ) );
  rendererElem.setAttribute( "lowerHeight", QString::number( mInterpolationSettings.lowerSize.height() ) );
  rendererElem.setAttribute( "upperWidth", QString::number( mInterpolationSettings.upperSize.width() ) );
  rendererElem.setAttribute( "upperHeight", QString::number( mInterpolationSettings.upperSize.height() ) );
  rendererElem.setAttribute( "classificationAttribute", mInterpolationSettings.classificationAttribute );
  mSettings.writeXML( rendererElem, doc );
  _writeXML( rendererElem, doc );
  layerElem.appendChild( rendererElem );
}
