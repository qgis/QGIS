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
#include "qgslayertreemodellegendnode.h"
#include "qgsfontutils.h"
#include "qgssymbollayerv2utils.h"
#include "qgssymbolv2.h"

#include <QDomElement>
#include <QPainter>

Q_NOWARN_DEPRECATED_PUSH // because of deprecated xform member
QgsDiagramLayerSettings::QgsDiagramLayerSettings()
    : placement( AroundPoint )
    , placementFlags( OnLine )
    , priority( 5 )
    , zIndex( 0.0 )
    , obstacle( false )
    , dist( 0.0 )
    , renderer( nullptr )
    , ct( nullptr )
    , xform( nullptr )
    , xPosColumn( -1 )
    , yPosColumn( -1 )
    , showColumn( -1 )
    , showAll( true )
{
}
Q_NOWARN_DEPRECATED_POP

Q_NOWARN_DEPRECATED_PUSH // because of deprecated xform member
QgsDiagramLayerSettings::QgsDiagramLayerSettings( const QgsDiagramLayerSettings& rh )
    : placement( rh.placement )
    , placementFlags( rh.placementFlags )
    , priority( rh.priority )
    , zIndex( rh.zIndex )
    , obstacle( rh.obstacle )
    , dist( rh.dist )
    , renderer( rh.renderer ? rh.renderer->clone() : nullptr )
    , ct( rh.ct ? rh.ct->clone() : nullptr )
    , xform( rh.xform )
    , fields( rh.fields )
    , xPosColumn( rh.xPosColumn )
    , yPosColumn( rh.yPosColumn )
    , showColumn( rh.showColumn )
    , showAll( rh.showAll )
{
}
Q_NOWARN_DEPRECATED_POP

Q_NOWARN_DEPRECATED_PUSH // because of deprecated xform member
QgsDiagramLayerSettings&QgsDiagramLayerSettings::operator=( const QgsDiagramLayerSettings & rh )
{
  placement = rh.placement;
  placementFlags = rh.placementFlags;
  priority = rh.priority;
  zIndex = rh.zIndex;
  obstacle = rh.obstacle;
  dist = rh.dist;
  renderer = rh.renderer ? rh.renderer->clone() : nullptr;
  ct = rh.ct ? rh.ct->clone() : nullptr;
  xform = rh.xform;
  fields = rh.fields;
  xPosColumn = rh.xPosColumn;
  yPosColumn = rh.yPosColumn;
  showColumn = rh.showColumn;
  showAll = rh.showAll;
  return *this;
}
Q_NOWARN_DEPRECATED_POP

Q_NOWARN_DEPRECATED_PUSH // because of deprecated fields member
QgsDiagramLayerSettings::~QgsDiagramLayerSettings()
{
  delete renderer;
  delete ct;
}
Q_NOWARN_DEPRECATED_POP

void QgsDiagramLayerSettings::setRenderer( QgsDiagramRendererV2 *diagramRenderer )
{
  if ( diagramRenderer == renderer )
    return;

  delete renderer;
  renderer = diagramRenderer;
}

void QgsDiagramLayerSettings::setCoordinateTransform( QgsCoordinateTransform *transform )
{
  delete ct;
  ct = transform;
}

void QgsDiagramLayerSettings::readXML( const QDomElement& elem, const QgsVectorLayer* layer )
{
  Q_UNUSED( layer )

  placement = static_cast< Placement >( elem.attribute( "placement" ).toInt() );
  placementFlags = static_cast< LinePlacementFlags >( elem.attribute( "linePlacementFlags" ).toInt() );
  priority = elem.attribute( "priority" ).toInt();
  zIndex = elem.attribute( "zIndex" ).toDouble();
  obstacle = elem.attribute( "obstacle" ).toInt();
  dist = elem.attribute( "dist" ).toDouble();
  xPosColumn = elem.attribute( "xPosColumn" ).toInt();
  yPosColumn = elem.attribute( "yPosColumn" ).toInt();
  showColumn = elem.attribute( "showColumn" ).toInt();
  showAll = ( elem.attribute( "showAll", "0" ) != "0" );
}

void QgsDiagramLayerSettings::writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const
{
  Q_UNUSED( layer )

  QDomElement diagramLayerElem = doc.createElement( "DiagramLayerSettings" );
  diagramLayerElem.setAttribute( "placement", placement );
  diagramLayerElem.setAttribute( "linePlacementFlags", placementFlags );
  diagramLayerElem.setAttribute( "priority", priority );
  diagramLayerElem.setAttribute( "zIndex", zIndex );
  diagramLayerElem.setAttribute( "obstacle", obstacle );
  diagramLayerElem.setAttribute( "dist", QString::number( dist ) );
  diagramLayerElem.setAttribute( "xPosColumn", xPosColumn );
  diagramLayerElem.setAttribute( "yPosColumn", yPosColumn );
  diagramLayerElem.setAttribute( "showColumn", showColumn );
  diagramLayerElem.setAttribute( "showAll", showAll );
  layerElem.appendChild( diagramLayerElem );
}

QSet<QString> QgsDiagramLayerSettings::referencedFields( const QgsExpressionContext &context, const QgsFields& fieldsParameter ) const
{
  QSet< QString > referenced;
  if ( renderer )
    referenced = renderer->referencedFields( context, fieldsParameter );

  //and the ones needed for data defined diagram positions
  if ( xPosColumn >= 0 && xPosColumn < fieldsParameter.count() )
    referenced << fieldsParameter.at( xPosColumn ).name();
  if ( yPosColumn >= 0 && yPosColumn < fieldsParameter.count() )
    referenced << fieldsParameter.at( yPosColumn ).name();

  // and the ones needed for data defined diagram visibility
  if ( showColumn >= 0 && showColumn < fieldsParameter.count() )
    referenced << fieldsParameter.at( showColumn ).name();

  return referenced;
}

void QgsDiagramSettings::readXML( const QDomElement& elem, const QgsVectorLayer* layer )
{
  Q_UNUSED( layer );

  enabled = ( elem.attribute( "enabled", "1" ) != "0" );
  if ( !QgsFontUtils::setFromXmlChildNode( font, elem, "fontProperties" ) )
  {
    font.fromString( elem.attribute( "font" ) );
  }
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
  if ( elem.hasAttribute( "scaleBasedVisibility" ) )
  {
    scaleBasedVisibility = ( elem.attribute( "scaleBasedVisibility", "1" ) != "0" );
  }
  else
  {
    scaleBasedVisibility = minScaleDenominator >= 0 && maxScaleDenominator >= 0;
  }

  //diagram size unit type and scale
  if ( elem.attribute( "sizeType" ) == "MapUnits" )
  {
    //compatibility with pre-2.16 project files
    sizeType = QgsSymbolV2::MapUnit;
  }
  else
  {
    sizeType = QgsSymbolLayerV2Utils::decodeOutputUnit( elem.attribute( "sizeType" ) );
  }
  sizeScale = QgsSymbolLayerV2Utils::decodeMapUnitScale( elem.attribute( "sizeScale" ) );

  //line width unit type and scale
  lineSizeUnit = QgsSymbolLayerV2Utils::decodeOutputUnit( elem.attribute( "lineSizeType" ) );
  lineSizeScale = QgsSymbolLayerV2Utils::decodeMapUnitScale( elem.attribute( "lineSizeScale" ) );

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
    for ( int i = 0; i < attributes.size(); i++ )
    {
      QDomElement attrElem = attributes.at( i ).toElement();
      QColor newColor( attrElem.attribute( "color" ) );
      newColor.setAlpha( 255 - transparency );
      categoryColors.append( newColor );
      categoryAttributes.append( attrElem.attribute( "field" ) );
      categoryLabels.append( attrElem.attribute( "label" ) );
      if ( categoryLabels.back().isEmpty() )
      {
        categoryLabels.back() = categoryAttributes.back();
      }
    }
  }
  else
  {
    // Restore old format attributes and colors

    QStringList colorList = elem.attribute( "colors" ).split( '/' );
    QStringList::const_iterator colorIt = colorList.constBegin();
    for ( ; colorIt != colorList.constEnd(); ++colorIt )
    {
      QColor newColor( *colorIt );
      newColor.setAlpha( 255 - transparency );
      categoryColors.append( QColor( newColor ) );
    }

    //attribute indices
    categoryAttributes.clear();
    QStringList catList = elem.attribute( "categories" ).split( '/' );
    QStringList::const_iterator catIt = catList.constBegin();
    for ( ; catIt != catList.constEnd(); ++catIt )
    {
      categoryAttributes.append( *catIt );
      categoryLabels.append( *catIt );
    }
  }
}

void QgsDiagramSettings::writeXML( QDomElement& rendererElem, QDomDocument& doc, const QgsVectorLayer* layer ) const
{
  Q_UNUSED( layer );

  QDomElement categoryElem = doc.createElement( "DiagramCategory" );
  categoryElem.setAttribute( "enabled", enabled );
  categoryElem.appendChild( QgsFontUtils::toXmlElement( font, doc, "fontProperties" ) );
  categoryElem.setAttribute( "backgroundColor", backgroundColor.name() );
  categoryElem.setAttribute( "backgroundAlpha", backgroundColor.alpha() );
  categoryElem.setAttribute( "width", QString::number( size.width() ) );
  categoryElem.setAttribute( "height", QString::number( size.height() ) );
  categoryElem.setAttribute( "penColor", penColor.name() );
  categoryElem.setAttribute( "penAlpha", penColor.alpha() );
  categoryElem.setAttribute( "penWidth", QString::number( penWidth ) );
  categoryElem.setAttribute( "scaleBasedVisibility", scaleBasedVisibility );
  categoryElem.setAttribute( "minScaleDenominator", QString::number( minScaleDenominator ) );
  categoryElem.setAttribute( "maxScaleDenominator", QString::number( maxScaleDenominator ) );
  categoryElem.setAttribute( "transparency", QString::number( transparency ) );

  //diagram size unit type and scale
  categoryElem.setAttribute( "sizeType", QgsSymbolLayerV2Utils::encodeOutputUnit( sizeType ) );
  categoryElem.setAttribute( "sizeScale", QgsSymbolLayerV2Utils::encodeMapUnitScale( sizeScale ) );

  //line width unit type and scale
  categoryElem.setAttribute( "lineSizeType", QgsSymbolLayerV2Utils::encodeOutputUnit( lineSizeUnit ) );
  categoryElem.setAttribute( "lineSizeScale", QgsSymbolLayerV2Utils::encodeMapUnitScale( lineSizeScale ) );

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

  int nCats = qMin( categoryColors.size(), categoryAttributes.size() );
  for ( int i = 0; i < nCats; ++i )
  {
    QDomElement attributeElem = doc.createElement( "attribute" );

    attributeElem.setAttribute( "field", categoryAttributes.at( i ) );
    attributeElem.setAttribute( "color", categoryColors.at( i ).name() );
    attributeElem.setAttribute( "label", categoryLabels.at( i ) );
    categoryElem.appendChild( attributeElem );
  }

  rendererElem.appendChild( categoryElem );
}

QgsDiagramRendererV2::QgsDiagramRendererV2()
    : mDiagram( nullptr )
    , mShowAttributeLegend( true )
    , mShowSizeLegend( false )
    , mSizeLegendSymbol( QgsMarkerSymbolV2::createSimple( QgsStringMap() ) )
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
    : mDiagram( other.mDiagram ? other.mDiagram->clone() : nullptr )
    , mShowAttributeLegend( other.mShowAttributeLegend )
    , mShowSizeLegend( other.mShowSizeLegend )
    , mSizeLegendSymbol( other.mSizeLegendSymbol.data() ? other.mSizeLegendSymbol->clone() : nullptr )
{
}

QgsDiagramRendererV2 &QgsDiagramRendererV2::operator=( const QgsDiagramRendererV2 & other )
{
  mDiagram = other.mDiagram ? other.mDiagram->clone() : nullptr;
  mShowAttributeLegend = other.mShowAttributeLegend;
  mShowSizeLegend = other.mShowSizeLegend;
  mSizeLegendSymbol.reset( other.mSizeLegendSymbol.data() ? other.mSizeLegendSymbol->clone() : nullptr );
  return *this;
}

void QgsDiagramRendererV2::renderDiagram( const QgsFeature& feature, QgsRenderContext& c, QPointF pos ) const
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

QSizeF QgsDiagramRendererV2::sizeMapUnits( const QgsFeature& feature, const QgsRenderContext& c ) const
{
  QgsDiagramSettings s;
  if ( !diagramSettings( feature, c, s ) )
  {
    return QSizeF();
  }

  QSizeF size = diagramSize( feature, c );
  if ( size.isValid() )
  {
    double width = QgsSymbolLayerV2Utils::convertToMapUnits( c, size.width(), s.sizeType, s.sizeScale );
    size.rheight() *= width / size.width();
    size.setWidth( width );
  }
  return size;
}

QSet<QString> QgsDiagramRendererV2::referencedFields( const QgsExpressionContext &context, const QgsFields &fields ) const
{
  Q_UNUSED( fields );

  QSet< QString > referenced;

  if ( !mDiagram )
    return referenced;

  Q_FOREACH ( const QString& att, diagramAttributes() )
  {
    QgsExpression* expression = mDiagram->getExpression( att, context );
    Q_FOREACH ( const QString& field, expression->referencedColumns() )
    {
      referenced << field;
    }
  }
  return referenced;
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
    mDiagram = nullptr;
  }
  mShowAttributeLegend = ( elem.attribute( "attributeLegend", "1" ) != "0" );
  mShowSizeLegend = ( elem.attribute( "sizeLegend", "0" ) != "0" );
  QDomElement sizeLegendSymbolElem = elem.firstChildElement( "symbol" );
  if ( !sizeLegendSymbolElem.isNull() && sizeLegendSymbolElem.attribute( "name" ) == "sizeSymbol" )
  {
    mSizeLegendSymbol.reset( QgsSymbolLayerV2Utils::loadSymbol<QgsMarkerSymbolV2>( sizeLegendSymbolElem ) );
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
  rendererElem.setAttribute( "attributeLegend", mShowAttributeLegend );
  rendererElem.setAttribute( "sizeLegend", mShowSizeLegend );
  QDomElement sizeLegendSymbolElem = QgsSymbolLayerV2Utils::saveSymbol( "sizeSymbol", mSizeLegendSymbol.data(), doc );
  rendererElem.appendChild( sizeLegendSymbolElem );
}

QgsSingleCategoryDiagramRenderer::QgsSingleCategoryDiagramRenderer(): QgsDiagramRendererV2()
{
}

QgsSingleCategoryDiagramRenderer::~QgsSingleCategoryDiagramRenderer()
{
}

QgsSingleCategoryDiagramRenderer* QgsSingleCategoryDiagramRenderer::clone() const
{
  return new QgsSingleCategoryDiagramRenderer( *this );
}

bool QgsSingleCategoryDiagramRenderer::diagramSettings( const QgsFeature&, const QgsRenderContext& c, QgsDiagramSettings& s ) const
{
  Q_UNUSED( c );
  s = mSettings;
  return true;
}

QSizeF QgsSingleCategoryDiagramRenderer::diagramSize( const QgsFeature &feature, const QgsRenderContext &c ) const
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

QgsLinearlyInterpolatedDiagramRenderer* QgsLinearlyInterpolatedDiagramRenderer::clone() const
{
  return new QgsLinearlyInterpolatedDiagramRenderer( *this );
}

QList<QgsDiagramSettings> QgsLinearlyInterpolatedDiagramRenderer::diagramSettings() const
{
  QList<QgsDiagramSettings> settingsList;
  settingsList.push_back( mSettings );
  return settingsList;
}

bool QgsLinearlyInterpolatedDiagramRenderer::diagramSettings( const QgsFeature& feature, const QgsRenderContext& c, QgsDiagramSettings& s ) const
{
  s = mSettings;
  s.size = diagramSize( feature, c );
  return true;
}

QList<QString> QgsLinearlyInterpolatedDiagramRenderer::diagramAttributes() const
{
  return mSettings.categoryAttributes;
}

QSet<QString> QgsLinearlyInterpolatedDiagramRenderer::referencedFields( const QgsExpressionContext &context, const QgsFields& fields ) const
{
  QSet< QString > referenced = QgsDiagramRendererV2::referencedFields( context, fields );
  if ( mInterpolationSettings.classificationAttributeIsExpression )
  {
    QgsExpression* expression = mDiagram->getExpression( mInterpolationSettings.classificationAttributeExpression, context );
    Q_FOREACH ( const QString& field, expression->referencedColumns() )
    {
      referenced << field;
    }
  }
  else
  {
    referenced << fields.at( mInterpolationSettings.classificationAttribute ).name();
  }
  return referenced;
}

QSizeF QgsLinearlyInterpolatedDiagramRenderer::diagramSize( const QgsFeature& feature, const QgsRenderContext& c ) const
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

QList< QgsLayerTreeModelLegendNode* > QgsDiagramSettings::legendItems( QgsLayerTreeLayer* nodeLayer ) const
{
  QList< QgsLayerTreeModelLegendNode * > list;
  list.reserve( categoryLabels.size() );
  for ( int i = 0 ; i < categoryLabels.size(); ++i )
  {
    QPixmap pix( 16, 16 );
    pix.fill( categoryColors[i] );
    list << new QgsSimpleLegendNode( nodeLayer, categoryLabels[i], QIcon( pix ), nullptr, QString( "diagram_%1" ).arg( QString::number( i ) ) );
  }
  return list;
}

QList< QgsLayerTreeModelLegendNode* > QgsDiagramRendererV2::legendItems( QgsLayerTreeLayer* ) const
{
  return QList< QgsLayerTreeModelLegendNode * >();
}

QList< QgsLayerTreeModelLegendNode* > QgsSingleCategoryDiagramRenderer::legendItems( QgsLayerTreeLayer* nodeLayer ) const
{
  QList< QgsLayerTreeModelLegendNode* > nodes;
  if ( mShowAttributeLegend )
    nodes = mSettings.legendItems( nodeLayer );

  return nodes;
}

QList< QgsLayerTreeModelLegendNode* > QgsLinearlyInterpolatedDiagramRenderer::legendItems( QgsLayerTreeLayer* nodeLayer ) const
{
  QList< QgsLayerTreeModelLegendNode* > nodes;
  if ( mShowAttributeLegend )
    nodes = mSettings.legendItems( nodeLayer );

  if ( mShowSizeLegend && mDiagram && mSizeLegendSymbol.data() )
  {
    // add size legend
    Q_FOREACH ( double v, QgsSymbolLayerV2Utils::prettyBreaks( mInterpolationSettings.lowerValue, mInterpolationSettings.upperValue, 4 ) )
    {
      double size = mDiagram->legendSize( v, mSettings, mInterpolationSettings );
      QgsLegendSymbolItemV2 si( mSizeLegendSymbol.data(), QString::number( v ), nullptr );
      QgsMarkerSymbolV2 * s = static_cast<QgsMarkerSymbolV2 *>( si.symbol() );
      s->setSize( size );
      s->setSizeUnit( mSettings.sizeType );
      s->setSizeMapUnitScale( mSettings.sizeScale );
      nodes << new QgsSymbolV2LegendNode( nodeLayer, si );
    }
  }

  return nodes;
}
