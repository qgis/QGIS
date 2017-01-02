/***************************************************************************
    qgsdiagramrenderer.cpp
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
#include "qgsdiagramrenderer.h"
#include "qgsvectorlayer.h"
#include "diagram/qgstextdiagram.h"
#include "diagram/qgspiediagram.h"
#include "diagram/qgshistogramdiagram.h"
#include "qgsrendercontext.h"
#include "qgslayertreemodellegendnode.h"
#include "qgsfontutils.h"
#include "qgssymbollayerutils.h"

#include <QDomElement>
#include <QPainter>

QgsDiagramLayerSettings::QgsDiagramLayerSettings()
    : xPosColumn( -1 )
    , yPosColumn( -1 )
    , showColumn( -1 )
    , mRenderer( nullptr )
{
  init();
}

QgsDiagramLayerSettings::QgsDiagramLayerSettings( const QgsDiagramLayerSettings& rh )
    : xPosColumn( rh.xPosColumn )
    , yPosColumn( rh.yPosColumn )
    , showColumn( rh.showColumn )
    , mCt( rh.mCt )
    , mPlacement( rh.mPlacement )
    , mPlacementFlags( rh.mPlacementFlags )
    , mPriority( rh.mPriority )
    , mZIndex( rh.mZIndex )
    , mObstacle( rh.mObstacle )
    , mDistance( rh.mDistance )
    , mRenderer( rh.mRenderer ? rh.mRenderer->clone() : nullptr )
    , mShowAll( rh.mShowAll )
    , mProperties( rh.mProperties )
{
  init();
}

QgsDiagramLayerSettings&QgsDiagramLayerSettings::operator=( const QgsDiagramLayerSettings & rh )
{
  mPlacement = rh.mPlacement;
  mPlacementFlags = rh.mPlacementFlags;
  mPriority = rh.mPriority;
  mZIndex = rh.mZIndex;
  mObstacle = rh.mObstacle;
  mDistance = rh.mDistance;
  mRenderer = rh.mRenderer ? rh.mRenderer->clone() : nullptr;
  mCt = rh.mCt;
  xPosColumn = rh.xPosColumn;
  yPosColumn = rh.yPosColumn;
  showColumn = rh.showColumn;
  mShowAll = rh.mShowAll;
  mProperties = rh.mProperties;
  return *this;
}

QgsDiagramLayerSettings::~QgsDiagramLayerSettings()
{
  delete mRenderer;
}

void QgsDiagramLayerSettings::setRenderer( QgsDiagramRenderer *diagramRenderer )
{
  if ( diagramRenderer == mRenderer )
    return;

  delete mRenderer;
  mRenderer = diagramRenderer;
}

void QgsDiagramLayerSettings::setCoordinateTransform( const QgsCoordinateTransform& transform )
{
  mCt = transform;
}

void QgsDiagramLayerSettings::readXml( const QDomElement& elem, const QgsVectorLayer* layer )
{
  Q_UNUSED( layer )

  QDomNodeList propertyElems = elem.elementsByTagName( "properties" );
  if ( !propertyElems.isEmpty() )
  {
    ( void )mProperties.readXML( propertyElems.at( 0 ).toElement(), elem.ownerDocument(), sPropertyNameMap );
  }
  else
  {
    mProperties.clear();
  }

  mPlacement = static_cast< Placement >( elem.attribute( QStringLiteral( "placement" ) ).toInt() );
  mPlacementFlags = static_cast< LinePlacementFlag >( elem.attribute( QStringLiteral( "linePlacementFlags" ) ).toInt() );
  mPriority = elem.attribute( QStringLiteral( "priority" ) ).toInt();
  mZIndex = elem.attribute( QStringLiteral( "zIndex" ) ).toDouble();
  mObstacle = elem.attribute( QStringLiteral( "obstacle" ) ).toInt();
  mDistance = elem.attribute( QStringLiteral( "dist" ) ).toDouble();
  xPosColumn = elem.attribute( QStringLiteral( "xPosColumn" ) ).toInt();
  yPosColumn = elem.attribute( QStringLiteral( "yPosColumn" ) ).toInt();
  showColumn = elem.attribute( QStringLiteral( "showColumn" ) ).toInt();
  mShowAll = ( elem.attribute( QStringLiteral( "showAll" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
}

void QgsDiagramLayerSettings::writeXml( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const
{
  Q_UNUSED( layer )

  QDomElement diagramLayerElem = doc.createElement( QStringLiteral( "DiagramLayerSettings" ) );
  QDomElement propertiesElem = doc.createElement( "properties" );
  ( void )mProperties.writeXML( propertiesElem, doc, sPropertyNameMap );
  diagramLayerElem.appendChild( propertiesElem );
  diagramLayerElem.setAttribute( QStringLiteral( "placement" ), mPlacement );
  diagramLayerElem.setAttribute( QStringLiteral( "linePlacementFlags" ), mPlacementFlags );
  diagramLayerElem.setAttribute( QStringLiteral( "priority" ), mPriority );
  diagramLayerElem.setAttribute( QStringLiteral( "zIndex" ), mZIndex );
  diagramLayerElem.setAttribute( QStringLiteral( "obstacle" ), mObstacle );
  diagramLayerElem.setAttribute( QStringLiteral( "dist" ), QString::number( mDistance ) );
  diagramLayerElem.setAttribute( QStringLiteral( "xPosColumn" ), xPosColumn );
  diagramLayerElem.setAttribute( QStringLiteral( "yPosColumn" ), yPosColumn );
  diagramLayerElem.setAttribute( QStringLiteral( "showColumn" ), showColumn );
  diagramLayerElem.setAttribute( QStringLiteral( "showAll" ), mShowAll );
  layerElem.appendChild( diagramLayerElem );
}

void QgsDiagramLayerSettings::init()
{
  if ( sPropertyNameMap.isEmpty() )
  {
    sPropertyNameMap.insert( Size, "diagramSize" );
    sPropertyNameMap.insert( BackgroundColor, "backgroundColor" );
    sPropertyNameMap.insert( OutlineColor, "outlineColor" );
    sPropertyNameMap.insert( OutlineWidth, "outlineWidth" );
    sPropertyNameMap.insert( Opacity, "opacity" );
  }
}

QSet<QString> QgsDiagramLayerSettings::referencedFields( const QgsExpressionContext &context, const QgsFields& fieldsParameter ) const
{
  QSet< QString > referenced;
  if ( mRenderer )
    referenced = mRenderer->referencedFields( context );

  //add the ones needed for data defined settings
  referenced.unite( mProperties.referencedFields( context ) );

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

void QgsDiagramSettings::readXml( const QDomElement& elem, const QgsVectorLayer* layer )
{
  Q_UNUSED( layer );

  enabled = ( elem.attribute( QStringLiteral( "enabled" ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  if ( !QgsFontUtils::setFromXmlChildNode( font, elem, QStringLiteral( "fontProperties" ) ) )
  {
    font.fromString( elem.attribute( QStringLiteral( "font" ) ) );
  }
  backgroundColor.setNamedColor( elem.attribute( QStringLiteral( "backgroundColor" ) ) );
  backgroundColor.setAlpha( elem.attribute( QStringLiteral( "backgroundAlpha" ) ).toInt() );
  size.setWidth( elem.attribute( QStringLiteral( "width" ) ).toDouble() );
  size.setHeight( elem.attribute( QStringLiteral( "height" ) ).toDouble() );
  transparency = elem.attribute( QStringLiteral( "transparency" ), QStringLiteral( "0" ) ).toInt();
  penColor.setNamedColor( elem.attribute( QStringLiteral( "penColor" ) ) );
  int penAlpha = elem.attribute( QStringLiteral( "penAlpha" ), QStringLiteral( "255" ) ).toInt();
  penColor.setAlpha( penAlpha );
  penWidth = elem.attribute( QStringLiteral( "penWidth" ) ).toDouble();

  minScaleDenominator = elem.attribute( QStringLiteral( "minScaleDenominator" ), QStringLiteral( "-1" ) ).toDouble();
  maxScaleDenominator = elem.attribute( QStringLiteral( "maxScaleDenominator" ), QStringLiteral( "-1" ) ).toDouble();
  if ( elem.hasAttribute( QStringLiteral( "scaleBasedVisibility" ) ) )
  {
    scaleBasedVisibility = ( elem.attribute( QStringLiteral( "scaleBasedVisibility" ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  }
  else
  {
    scaleBasedVisibility = minScaleDenominator >= 0 && maxScaleDenominator >= 0;
  }

  //diagram size unit type and scale
  if ( elem.attribute( QStringLiteral( "sizeType" ) ) == QLatin1String( "MapUnits" ) )
  {
    //compatibility with pre-2.16 project files
    sizeType = QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    sizeType = QgsUnitTypes::decodeRenderUnit( elem.attribute( QStringLiteral( "sizeType" ) ) );
  }
  sizeScale = QgsSymbolLayerUtils::decodeMapUnitScale( elem.attribute( QStringLiteral( "sizeScale" ) ) );

  //line width unit type and scale
  lineSizeUnit = QgsUnitTypes::decodeRenderUnit( elem.attribute( QStringLiteral( "lineSizeType" ) ) );
  lineSizeScale = QgsSymbolLayerUtils::decodeMapUnitScale( elem.attribute( QStringLiteral( "lineSizeScale" ) ) );

  //label placement method
  if ( elem.attribute( QStringLiteral( "labelPlacementMethod" ) ) == QLatin1String( "Height" ) )
  {
    labelPlacementMethod = Height;
  }
  else
  {
    labelPlacementMethod = XHeight;
  }

  // orientation
  if ( elem.attribute( QStringLiteral( "diagramOrientation" ) ) == QLatin1String( "Left" ) )
  {
    diagramOrientation = Left;
  }
  else if ( elem.attribute( QStringLiteral( "diagramOrientation" ) ) == QLatin1String( "Right" ) )
  {
    diagramOrientation = Right;
  }
  else if ( elem.attribute( QStringLiteral( "diagramOrientation" ) ) == QLatin1String( "Down" ) )
  {
    diagramOrientation = Down;
  }
  else
  {
    diagramOrientation = Up;
  }

  // scale dependency
  if ( elem.attribute( QStringLiteral( "scaleDependency" ) ) == QLatin1String( "Diameter" ) )
  {
    scaleByArea = false;
  }
  else
  {
    scaleByArea = true;
  }

  barWidth = elem.attribute( QStringLiteral( "barWidth" ) ).toDouble();

  angleOffset = elem.attribute( QStringLiteral( "angleOffset" ) ).toInt();

  minimumSize = elem.attribute( QStringLiteral( "minimumSize" ) ).toDouble();

  //colors
  categoryColors.clear();
  QDomNodeList attributes = elem.elementsByTagName( QStringLiteral( "attribute" ) );

  if ( attributes.length() > 0 )
  {
    for ( int i = 0; i < attributes.size(); i++ )
    {
      QDomElement attrElem = attributes.at( i ).toElement();
      QColor newColor( attrElem.attribute( QStringLiteral( "color" ) ) );
      newColor.setAlpha( 255 - transparency );
      categoryColors.append( newColor );
      categoryAttributes.append( attrElem.attribute( QStringLiteral( "field" ) ) );
      categoryLabels.append( attrElem.attribute( QStringLiteral( "label" ) ) );
      if ( categoryLabels.back().isEmpty() )
      {
        categoryLabels.back() = categoryAttributes.back();
      }
    }
  }
  else
  {
    // Restore old format attributes and colors

    QStringList colorList = elem.attribute( QStringLiteral( "colors" ) ).split( '/' );
    QStringList::const_iterator colorIt = colorList.constBegin();
    for ( ; colorIt != colorList.constEnd(); ++colorIt )
    {
      QColor newColor( *colorIt );
      newColor.setAlpha( 255 - transparency );
      categoryColors.append( QColor( newColor ) );
    }

    //attribute indices
    categoryAttributes.clear();
    QStringList catList = elem.attribute( QStringLiteral( "categories" ) ).split( '/' );
    QStringList::const_iterator catIt = catList.constBegin();
    for ( ; catIt != catList.constEnd(); ++catIt )
    {
      categoryAttributes.append( *catIt );
      categoryLabels.append( *catIt );
    }
  }
}

void QgsDiagramSettings::writeXml( QDomElement& rendererElem, QDomDocument& doc, const QgsVectorLayer* layer ) const
{
  Q_UNUSED( layer );

  QDomElement categoryElem = doc.createElement( QStringLiteral( "DiagramCategory" ) );
  categoryElem.setAttribute( QStringLiteral( "enabled" ), enabled );
  categoryElem.appendChild( QgsFontUtils::toXmlElement( font, doc, QStringLiteral( "fontProperties" ) ) );
  categoryElem.setAttribute( QStringLiteral( "backgroundColor" ), backgroundColor.name() );
  categoryElem.setAttribute( QStringLiteral( "backgroundAlpha" ), backgroundColor.alpha() );
  categoryElem.setAttribute( QStringLiteral( "width" ), QString::number( size.width() ) );
  categoryElem.setAttribute( QStringLiteral( "height" ), QString::number( size.height() ) );
  categoryElem.setAttribute( QStringLiteral( "penColor" ), penColor.name() );
  categoryElem.setAttribute( QStringLiteral( "penAlpha" ), penColor.alpha() );
  categoryElem.setAttribute( QStringLiteral( "penWidth" ), QString::number( penWidth ) );
  categoryElem.setAttribute( QStringLiteral( "scaleBasedVisibility" ), scaleBasedVisibility );
  categoryElem.setAttribute( QStringLiteral( "minScaleDenominator" ), QString::number( minScaleDenominator ) );
  categoryElem.setAttribute( QStringLiteral( "maxScaleDenominator" ), QString::number( maxScaleDenominator ) );
  categoryElem.setAttribute( QStringLiteral( "transparency" ), QString::number( transparency ) );

  //diagram size unit type and scale
  categoryElem.setAttribute( QStringLiteral( "sizeType" ), QgsUnitTypes::encodeUnit( sizeType ) );
  categoryElem.setAttribute( QStringLiteral( "sizeScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( sizeScale ) );

  //line width unit type and scale
  categoryElem.setAttribute( QStringLiteral( "lineSizeType" ), QgsUnitTypes::encodeUnit( lineSizeUnit ) );
  categoryElem.setAttribute( QStringLiteral( "lineSizeScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( lineSizeScale ) );

  // label placement method (text diagram)
  if ( labelPlacementMethod == Height )
  {
    categoryElem.setAttribute( QStringLiteral( "labelPlacementMethod" ), QStringLiteral( "Height" ) );
  }
  else
  {
    categoryElem.setAttribute( QStringLiteral( "labelPlacementMethod" ), QStringLiteral( "XHeight" ) );
  }

  if ( scaleByArea )
  {
    categoryElem.setAttribute( QStringLiteral( "scaleDependency" ), QStringLiteral( "Area" ) );
  }
  else
  {
    categoryElem.setAttribute( QStringLiteral( "scaleDependency" ), QStringLiteral( "Diameter" ) );
  }

  // orientation (histogram)
  switch ( diagramOrientation )
  {
    case Left:
      categoryElem.setAttribute( QStringLiteral( "diagramOrientation" ), QStringLiteral( "Left" ) );
      break;

    case Right:
      categoryElem.setAttribute( QStringLiteral( "diagramOrientation" ), QStringLiteral( "Right" ) );
      break;

    case Down:
      categoryElem.setAttribute( QStringLiteral( "diagramOrientation" ), QStringLiteral( "Down" ) );
      break;

    case Up:
      categoryElem.setAttribute( QStringLiteral( "diagramOrientation" ), QStringLiteral( "Up" ) );
      break;

    default:
      categoryElem.setAttribute( QStringLiteral( "diagramOrientation" ), QStringLiteral( "Up" ) );
      break;
  }

  categoryElem.setAttribute( QStringLiteral( "barWidth" ), QString::number( barWidth ) );
  categoryElem.setAttribute( QStringLiteral( "minimumSize" ), QString::number( minimumSize ) );
  categoryElem.setAttribute( QStringLiteral( "angleOffset" ), QString::number( angleOffset ) );

  int nCats = qMin( categoryColors.size(), categoryAttributes.size() );
  for ( int i = 0; i < nCats; ++i )
  {
    QDomElement attributeElem = doc.createElement( QStringLiteral( "attribute" ) );

    attributeElem.setAttribute( QStringLiteral( "field" ), categoryAttributes.at( i ) );
    attributeElem.setAttribute( QStringLiteral( "color" ), categoryColors.at( i ).name() );
    attributeElem.setAttribute( QStringLiteral( "label" ), categoryLabels.at( i ) );
    categoryElem.appendChild( attributeElem );
  }

  rendererElem.appendChild( categoryElem );
}

QgsDiagramRenderer::QgsDiagramRenderer()
    : mDiagram( nullptr )
    , mShowAttributeLegend( true )
    , mShowSizeLegend( false )
    , mSizeLegendSymbol( QgsMarkerSymbol::createSimple( QgsStringMap() ) )
{
}

QgsDiagramRenderer::~QgsDiagramRenderer()
{
  delete mDiagram;
}

void QgsDiagramRenderer::setDiagram( QgsDiagram* d )
{
  delete mDiagram;
  mDiagram = d;
}

QgsDiagramRenderer::QgsDiagramRenderer( const QgsDiagramRenderer& other )
    : mDiagram( other.mDiagram ? other.mDiagram->clone() : nullptr )
    , mShowAttributeLegend( other.mShowAttributeLegend )
    , mShowSizeLegend( other.mShowSizeLegend )
    , mSizeLegendSymbol( other.mSizeLegendSymbol.data() ? other.mSizeLegendSymbol->clone() : nullptr )
{
}

QgsDiagramRenderer &QgsDiagramRenderer::operator=( const QgsDiagramRenderer & other )
{
  mDiagram = other.mDiagram ? other.mDiagram->clone() : nullptr;
  mShowAttributeLegend = other.mShowAttributeLegend;
  mShowSizeLegend = other.mShowSizeLegend;
  mSizeLegendSymbol.reset( other.mSizeLegendSymbol.data() ? other.mSizeLegendSymbol->clone() : nullptr );
  return *this;
}

void QgsDiagramRenderer::renderDiagram( const QgsFeature& feature, QgsRenderContext& c, QPointF pos, const QgsPropertyCollection &properties ) const
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

  if ( properties.hasActiveProperties() )
  {
    s.transparency = properties.valueAsInt( QgsDiagramLayerSettings::Opacity, c.expressionContext(), s.transparency );
    s.backgroundColor = properties.valueAsColor( QgsDiagramLayerSettings::BackgroundColor, c.expressionContext(), s.backgroundColor );
    s.penColor = properties.valueAsColor( QgsDiagramLayerSettings::OutlineColor, c.expressionContext(), s.penColor );
    s.penWidth = properties.valueAsDouble( QgsDiagramLayerSettings::OutlineWidth, c.expressionContext(), s.penWidth );
  }

  mDiagram->renderDiagram( feature, c, s, pos );
}

QSizeF QgsDiagramRenderer::sizeMapUnits( const QgsFeature& feature, const QgsRenderContext& c ) const
{
  QgsDiagramSettings s;
  if ( !diagramSettings( feature, c, s ) )
  {
    return QSizeF();
  }

  QSizeF size = diagramSize( feature, c );
  if ( size.isValid() )
  {
    double width = c.convertToMapUnits( size.width(), s.sizeType, s.sizeScale );
    size.rheight() *= width / size.width();
    size.setWidth( width );
  }
  return size;
}

QSet<QString> QgsDiagramRenderer::referencedFields( const QgsExpressionContext &context ) const
{
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

void QgsDiagramRenderer::convertSizeToMapUnits( QSizeF& size, const QgsRenderContext& context ) const
{
  if ( !size.isValid() )
  {
    return;
  }

  double pixelToMap = context.scaleFactor() * context.mapToPixel().mapUnitsPerPixel();
  size.rwidth() *= pixelToMap;
  size.rheight() *= pixelToMap;
}

int QgsDiagramRenderer::dpiPaintDevice( const QPainter* painter )
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

void QgsDiagramRenderer::_readXml( const QDomElement& elem, const QgsVectorLayer* layer )
{
  Q_UNUSED( layer )

  delete mDiagram;
  QString diagramType = elem.attribute( QStringLiteral( "diagramType" ) );
  if ( diagramType == QLatin1String( "Pie" ) )
  {
    mDiagram = new QgsPieDiagram();
  }
  else if ( diagramType == QLatin1String( "Text" ) )
  {
    mDiagram = new QgsTextDiagram();
  }
  else if ( diagramType == QLatin1String( "Histogram" ) )
  {
    mDiagram = new QgsHistogramDiagram();
  }
  else
  {
    mDiagram = nullptr;
  }
  mShowAttributeLegend = ( elem.attribute( QStringLiteral( "attributeLegend" ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  mShowSizeLegend = ( elem.attribute( QStringLiteral( "sizeLegend" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
  QDomElement sizeLegendSymbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !sizeLegendSymbolElem.isNull() && sizeLegendSymbolElem.attribute( QStringLiteral( "name" ) ) == QLatin1String( "sizeSymbol" ) )
  {
    mSizeLegendSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( sizeLegendSymbolElem ) );
  }
}

void QgsDiagramRenderer::_writeXml( QDomElement& rendererElem, QDomDocument& doc, const QgsVectorLayer* layer ) const
{
  Q_UNUSED( doc );
  Q_UNUSED( layer )

  if ( mDiagram )
  {
    rendererElem.setAttribute( QStringLiteral( "diagramType" ), mDiagram->diagramName() );
  }
  rendererElem.setAttribute( QStringLiteral( "attributeLegend" ), mShowAttributeLegend );
  rendererElem.setAttribute( QStringLiteral( "sizeLegend" ), mShowSizeLegend );
  QDomElement sizeLegendSymbolElem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "sizeSymbol" ), mSizeLegendSymbol.data(), doc );
  rendererElem.appendChild( sizeLegendSymbolElem );
}

QgsSingleCategoryDiagramRenderer::QgsSingleCategoryDiagramRenderer(): QgsDiagramRenderer()
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

void QgsSingleCategoryDiagramRenderer::readXml( const QDomElement& elem, const QgsVectorLayer* layer )
{
  QDomElement categoryElem = elem.firstChildElement( QStringLiteral( "DiagramCategory" ) );
  if ( categoryElem.isNull() )
  {
    return;
  }

  mSettings.readXml( categoryElem, layer );
  _readXml( elem, layer );
}

void QgsSingleCategoryDiagramRenderer::writeXml( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "SingleCategoryDiagramRenderer" ) );
  mSettings.writeXml( rendererElem, doc, layer );
  _writeXml( rendererElem, doc, layer );
  layerElem.appendChild( rendererElem );
}


QgsLinearlyInterpolatedDiagramRenderer::QgsLinearlyInterpolatedDiagramRenderer(): QgsDiagramRenderer()
{
  mInterpolationSettings.classificationAttributeIsExpression = false;
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

QSet<QString> QgsLinearlyInterpolatedDiagramRenderer::referencedFields( const QgsExpressionContext &context ) const
{
  QSet< QString > referenced = QgsDiagramRenderer::referencedFields( context );
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
    referenced << mInterpolationSettings.classificationField;
  }
  return referenced;
}

QSizeF QgsLinearlyInterpolatedDiagramRenderer::diagramSize( const QgsFeature& feature, const QgsRenderContext& c ) const
{
  return mDiagram->diagramSize( feature, c, mSettings, mInterpolationSettings );
}

void QgsLinearlyInterpolatedDiagramRenderer::readXml( const QDomElement& elem, const QgsVectorLayer* layer )
{
  mInterpolationSettings.lowerValue = elem.attribute( QStringLiteral( "lowerValue" ) ).toDouble();
  mInterpolationSettings.upperValue = elem.attribute( QStringLiteral( "upperValue" ) ).toDouble();
  mInterpolationSettings.lowerSize.setWidth( elem.attribute( QStringLiteral( "lowerWidth" ) ).toDouble() );
  mInterpolationSettings.lowerSize.setHeight( elem.attribute( QStringLiteral( "lowerHeight" ) ).toDouble() );
  mInterpolationSettings.upperSize.setWidth( elem.attribute( QStringLiteral( "upperWidth" ) ).toDouble() );
  mInterpolationSettings.upperSize.setHeight( elem.attribute( QStringLiteral( "upperHeight" ) ).toDouble() );
  mInterpolationSettings.classificationAttributeIsExpression = elem.hasAttribute( QStringLiteral( "classificationAttributeExpression" ) );
  if ( mInterpolationSettings.classificationAttributeIsExpression )
  {
    mInterpolationSettings.classificationAttributeExpression = elem.attribute( QStringLiteral( "classificationAttributeExpression" ) );
  }
  else
  {
    if ( elem.hasAttribute( QStringLiteral( "classificationAttribute" ) ) )
    {
      int idx = elem.attribute( QStringLiteral( "classificationAttribute" ) ).toInt();
      if ( idx >= 0 && idx < layer->fields().count() )
        mInterpolationSettings.classificationField = layer->fields().at( idx ).name();
    }
    else
      mInterpolationSettings.classificationField = elem.attribute( QStringLiteral( "classificationField " ) );
  }
  QDomElement settingsElem = elem.firstChildElement( QStringLiteral( "DiagramCategory" ) );
  if ( !settingsElem.isNull() )
  {
    mSettings.readXml( settingsElem, layer );
  }
  _readXml( elem, layer );
}

void QgsLinearlyInterpolatedDiagramRenderer::writeXml( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "LinearlyInterpolatedDiagramRenderer" ) );
  rendererElem.setAttribute( QStringLiteral( "lowerValue" ), QString::number( mInterpolationSettings.lowerValue ) );
  rendererElem.setAttribute( QStringLiteral( "upperValue" ), QString::number( mInterpolationSettings.upperValue ) );
  rendererElem.setAttribute( QStringLiteral( "lowerWidth" ), QString::number( mInterpolationSettings.lowerSize.width() ) );
  rendererElem.setAttribute( QStringLiteral( "lowerHeight" ), QString::number( mInterpolationSettings.lowerSize.height() ) );
  rendererElem.setAttribute( QStringLiteral( "upperWidth" ), QString::number( mInterpolationSettings.upperSize.width() ) );
  rendererElem.setAttribute( QStringLiteral( "upperHeight" ), QString::number( mInterpolationSettings.upperSize.height() ) );
  if ( mInterpolationSettings.classificationAttributeIsExpression )
  {
    rendererElem.setAttribute( QStringLiteral( "classificationAttributeExpression" ), mInterpolationSettings.classificationAttributeExpression );
  }
  else
  {
    rendererElem.setAttribute( QStringLiteral( "classificationField" ), mInterpolationSettings.classificationField );
  }
  mSettings.writeXml( rendererElem, doc, layer );
  _writeXml( rendererElem, doc, layer );
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
    list << new QgsSimpleLegendNode( nodeLayer, categoryLabels[i], QIcon( pix ), nullptr, QStringLiteral( "diagram_%1" ).arg( QString::number( i ) ) );
  }
  return list;
}

QList< QgsLayerTreeModelLegendNode* > QgsDiagramRenderer::legendItems( QgsLayerTreeLayer* ) const
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
    Q_FOREACH ( double v, QgsSymbolLayerUtils::prettyBreaks( mInterpolationSettings.lowerValue, mInterpolationSettings.upperValue, 4 ) )
    {
      double size = mDiagram->legendSize( v, mSettings, mInterpolationSettings );
      QgsLegendSymbolItem si( mSizeLegendSymbol.data(), QString::number( v ), QString() );
      QgsMarkerSymbol * s = static_cast<QgsMarkerSymbol *>( si.symbol() );
      s->setSize( size );
      s->setSizeUnit( mSettings.sizeType );
      s->setSizeMapUnitScale( mSettings.sizeScale );
      nodes << new QgsSymbolLegendNode( nodeLayer, si );
    }
  }

  return nodes;
}
