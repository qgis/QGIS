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

#include "qgsdatadefinedsizelegend.h"
#include "qgsvectorlayer.h"
#include "diagram/qgstextdiagram.h"
#include "diagram/qgspiediagram.h"
#include "diagram/qgshistogramdiagram.h"
#include "diagram/qgsstackedbardiagram.h"
#include "qgsrendercontext.h"
#include "qgslayertreemodellegendnode.h"
#include "qgsfontutils.h"
#include "qgssymbollayerutils.h"
#include "qgspainteffectregistry.h"
#include "qgspainteffect.h"
#include "qgsapplication.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"

#include <QDomElement>
#include <QPainter>

QgsPropertiesDefinition QgsDiagramLayerSettings::sPropertyDefinitions;

void QgsDiagramLayerSettings::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  const QString origin = QStringLiteral( "diagram" );

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { QgsDiagramLayerSettings::BackgroundColor, QgsPropertyDefinition( "backgroundColor", QObject::tr( "Background color" ), QgsPropertyDefinition::ColorWithAlpha, origin ) },
    { QgsDiagramLayerSettings::StrokeColor, QgsPropertyDefinition( "strokeColor", QObject::tr( "Stroke color" ), QgsPropertyDefinition::ColorWithAlpha, origin ) },
    { QgsDiagramLayerSettings::StrokeWidth, QgsPropertyDefinition( "strokeWidth", QObject::tr( "Stroke width" ), QgsPropertyDefinition::StrokeWidth, origin ) },
    { QgsDiagramLayerSettings::PositionX, QgsPropertyDefinition( "positionX", QObject::tr( "Position (X)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsDiagramLayerSettings::PositionY, QgsPropertyDefinition( "positionY", QObject::tr( "Position (Y)" ), QgsPropertyDefinition::Double, origin ) },
    { QgsDiagramLayerSettings::Distance, QgsPropertyDefinition( "distance", QObject::tr( "Placement distance" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsDiagramLayerSettings::Priority, QgsPropertyDefinition( "priority", QObject::tr( "Placement priority" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { QgsDiagramLayerSettings::ZIndex, QgsPropertyDefinition( "zIndex", QObject::tr( "Placement z-index" ), QgsPropertyDefinition::Double, origin ) },
    { QgsDiagramLayerSettings::IsObstacle, QgsPropertyDefinition( "isObstacle", QObject::tr( "Diagram is an obstacle" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsDiagramLayerSettings::Show, QgsPropertyDefinition( "show", QObject::tr( "Show diagram" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsDiagramLayerSettings::AlwaysShow, QgsPropertyDefinition( "alwaysShow", QObject::tr( "Always show diagram" ), QgsPropertyDefinition::Boolean, origin ) },
    { QgsDiagramLayerSettings::StartAngle, QgsPropertyDefinition( "startAngle", QObject::tr( "Pie chart start angle" ), QgsPropertyDefinition::Rotation, origin ) },
  };
}

const QgsPropertiesDefinition &QgsDiagramLayerSettings::propertyDefinitions()
{
  initPropertyDefinitions();
  return sPropertyDefinitions;
}

QgsDiagramLayerSettings::QgsDiagramLayerSettings()
{
  initPropertyDefinitions();
}

QgsDiagramLayerSettings::QgsDiagramLayerSettings( const QgsDiagramLayerSettings &rh )
  : mCt( rh.mCt )
  , mPlacement( rh.mPlacement )
  , mPlacementFlags( rh.mPlacementFlags )
  , mPriority( rh.mPriority )
  , mZIndex( rh.mZIndex )
  , mObstacle( rh.mObstacle )
  , mDistance( rh.mDistance )
  , mRenderer( rh.mRenderer ? rh.mRenderer->clone() : nullptr )
  , mShowAll( rh.mShowAll )
  , mDataDefinedProperties( rh.mDataDefinedProperties )
{
  initPropertyDefinitions();
}

QgsDiagramLayerSettings &QgsDiagramLayerSettings::operator=( const QgsDiagramLayerSettings &rh )
{
  mPlacement = rh.mPlacement;
  mPlacementFlags = rh.mPlacementFlags;
  mPriority = rh.mPriority;
  mZIndex = rh.mZIndex;
  mObstacle = rh.mObstacle;
  mDistance = rh.mDistance;
  mRenderer = rh.mRenderer ? rh.mRenderer->clone() : nullptr;
  mCt = rh.mCt;
  mShowAll = rh.mShowAll;
  mDataDefinedProperties = rh.mDataDefinedProperties;
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

void QgsDiagramLayerSettings::setCoordinateTransform( const QgsCoordinateTransform &transform )
{
  mCt = transform;
}

void QgsDiagramLayerSettings::readXml( const QDomElement &elem )
{
  const QDomNodeList propertyElems = elem.elementsByTagName( QStringLiteral( "properties" ) );
  if ( !propertyElems.isEmpty() )
  {
    ( void )mDataDefinedProperties.readXml( propertyElems.at( 0 ).toElement(), sPropertyDefinitions );
  }
  else
  {
    mDataDefinedProperties.clear();
  }

  mPlacement = static_cast< Placement >( elem.attribute( QStringLiteral( "placement" ) ).toInt() );
  mPlacementFlags = static_cast< LinePlacementFlag >( elem.attribute( QStringLiteral( "linePlacementFlags" ) ).toInt() );
  mPriority = elem.attribute( QStringLiteral( "priority" ) ).toInt();
  mZIndex = elem.attribute( QStringLiteral( "zIndex" ) ).toDouble();
  mObstacle = elem.attribute( QStringLiteral( "obstacle" ) ).toInt();
  mDistance = elem.attribute( QStringLiteral( "dist" ) ).toDouble();
  mShowAll = ( elem.attribute( QStringLiteral( "showAll" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
}

void QgsDiagramLayerSettings::writeXml( QDomElement &layerElem, QDomDocument &doc ) const
{
  QDomElement diagramLayerElem = doc.createElement( QStringLiteral( "DiagramLayerSettings" ) );
  QDomElement propertiesElem = doc.createElement( QStringLiteral( "properties" ) );
  ( void )mDataDefinedProperties.writeXml( propertiesElem, sPropertyDefinitions );
  diagramLayerElem.appendChild( propertiesElem );
  diagramLayerElem.setAttribute( QStringLiteral( "placement" ), mPlacement );
  diagramLayerElem.setAttribute( QStringLiteral( "linePlacementFlags" ), mPlacementFlags );
  diagramLayerElem.setAttribute( QStringLiteral( "priority" ), mPriority );
  diagramLayerElem.setAttribute( QStringLiteral( "zIndex" ), mZIndex );
  diagramLayerElem.setAttribute( QStringLiteral( "obstacle" ), mObstacle );
  diagramLayerElem.setAttribute( QStringLiteral( "dist" ), QString::number( mDistance ) );
  diagramLayerElem.setAttribute( QStringLiteral( "showAll" ), mShowAll );
  layerElem.appendChild( diagramLayerElem );
}

bool QgsDiagramLayerSettings::prepare( const QgsExpressionContext &context ) const
{
  return mDataDefinedProperties.prepare( context );
}

QSet<QString> QgsDiagramLayerSettings::referencedFields( const QgsExpressionContext &context ) const
{
  QSet< QString > referenced;
  if ( mRenderer )
    referenced = mRenderer->referencedFields( context );

  //add the ones needed for data defined settings
  referenced.unite( mDataDefinedProperties.referencedFields( context ) );

  return referenced;
}

void QgsDiagramSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  enabled = ( elem.attribute( QStringLiteral( "enabled" ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  if ( !QgsFontUtils::setFromXmlChildNode( font, elem, QStringLiteral( "fontProperties" ) ) )
  {
    font.fromString( elem.attribute( QStringLiteral( "font" ) ) );
  }
  backgroundColor.setNamedColor( elem.attribute( QStringLiteral( "backgroundColor" ) ) );
  backgroundColor.setAlpha( elem.attribute( QStringLiteral( "backgroundAlpha" ) ).toInt() );
  size.setWidth( elem.attribute( QStringLiteral( "width" ) ).toDouble() );
  size.setHeight( elem.attribute( QStringLiteral( "height" ) ).toDouble() );
  if ( elem.hasAttribute( QStringLiteral( "transparency" ) ) )
  {
    opacity = 1 - elem.attribute( QStringLiteral( "transparency" ), QStringLiteral( "0" ) ).toInt() / 255.0;
  }
  else
  {
    opacity = elem.attribute( QStringLiteral( "opacity" ), QStringLiteral( "1.00" ) ).toDouble();
  }

  penColor.setNamedColor( elem.attribute( QStringLiteral( "penColor" ) ) );
  const int penAlpha = elem.attribute( QStringLiteral( "penAlpha" ), QStringLiteral( "255" ) ).toInt();
  penColor.setAlpha( penAlpha );
  penWidth = elem.attribute( QStringLiteral( "penWidth" ) ).toDouble();

  mDirection = static_cast< Direction >( elem.attribute( QStringLiteral( "direction" ), QStringLiteral( "1" ) ).toInt() );

  maximumScale = elem.attribute( QStringLiteral( "minScaleDenominator" ), QStringLiteral( "-1" ) ).toDouble();
  minimumScale = elem.attribute( QStringLiteral( "maxScaleDenominator" ), QStringLiteral( "-1" ) ).toDouble();
  if ( elem.hasAttribute( QStringLiteral( "scaleBasedVisibility" ) ) )
  {
    scaleBasedVisibility = ( elem.attribute( QStringLiteral( "scaleBasedVisibility" ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  }
  else
  {
    scaleBasedVisibility = maximumScale >= 0 && minimumScale >= 0;
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

  mSpacing = elem.attribute( QStringLiteral( "spacing" ) ).toDouble();
  mSpacingUnit = QgsUnitTypes::decodeRenderUnit( elem.attribute( QStringLiteral( "spacingUnit" ) ) );
  mSpacingMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( elem.attribute( QStringLiteral( "spacingUnitScale" ) ) );

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

  if ( elem.hasAttribute( QStringLiteral( "angleOffset" ) ) )
    rotationOffset = std::fmod( 360.0 - elem.attribute( QStringLiteral( "angleOffset" ) ).toInt() / 16.0, 360.0 );
  else
    rotationOffset = elem.attribute( QStringLiteral( "rotationOffset" ) ).toDouble();

  minimumSize = elem.attribute( QStringLiteral( "minimumSize" ) ).toDouble();

  const QDomNodeList axisSymbolNodes = elem.elementsByTagName( QStringLiteral( "axisSymbol" ) );
  if ( axisSymbolNodes.count() > 0 )
  {
    const QDomElement axisSymbolElem = axisSymbolNodes.at( 0 ).toElement().firstChildElement();
    mAxisLineSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( axisSymbolElem, context ) );
  }
  else
  {
    mAxisLineSymbol = std::make_unique< QgsLineSymbol >();
  }

  mShowAxis = elem.attribute( QStringLiteral( "showAxis" ), QStringLiteral( "0" ) ).toInt();

  //colors
  categoryColors.clear();
  const QDomNodeList attributes = elem.elementsByTagName( QStringLiteral( "attribute" ) );


  if ( attributes.length() > 0 )
  {
    for ( int i = 0; i < attributes.size(); i++ )
    {
      const QDomElement attrElem = attributes.at( i ).toElement();
      QColor newColor( attrElem.attribute( QStringLiteral( "color" ) ) );
      newColor.setAlphaF( opacity );
      categoryColors.append( newColor );
      categoryAttributes.append( attrElem.attribute( QStringLiteral( "field" ) ) );
      categoryLabels.append( attrElem.attribute( QStringLiteral( "label" ) ) );
      if ( categoryLabels.constLast().isEmpty() )
      {
        categoryLabels.back() = categoryAttributes.back();
      }
    }
  }
  else
  {
    // Restore old format attributes and colors

    const QStringList colorList = elem.attribute( QStringLiteral( "colors" ) ).split( '/' );
    QStringList::const_iterator colorIt = colorList.constBegin();
    for ( ; colorIt != colorList.constEnd(); ++colorIt )
    {
      QColor newColor( *colorIt );
      newColor.setAlphaF( opacity );
      categoryColors.append( QColor( newColor ) );
    }

    //attribute indices
    categoryAttributes.clear();
    const QStringList catList = elem.attribute( QStringLiteral( "categories" ) ).split( '/' );
    QStringList::const_iterator catIt = catList.constBegin();
    for ( ; catIt != catList.constEnd(); ++catIt )
    {
      categoryAttributes.append( *catIt );
      categoryLabels.append( *catIt );
    }
  }

  const QDomElement effectElem = elem.firstChildElement( QStringLiteral( "effect" ) );
  if ( !effectElem.isNull() )
    setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
  else
    setPaintEffect( QgsPaintEffectRegistry::defaultStack() );
}

void QgsDiagramSettings::writeXml( QDomElement &rendererElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
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
  categoryElem.setAttribute( QStringLiteral( "minScaleDenominator" ), QString::number( maximumScale ) );
  categoryElem.setAttribute( QStringLiteral( "maxScaleDenominator" ), QString::number( minimumScale ) );
  categoryElem.setAttribute( QStringLiteral( "opacity" ), QString::number( opacity ) );
  categoryElem.setAttribute( QStringLiteral( "spacing" ), QString::number( mSpacing ) );
  categoryElem.setAttribute( QStringLiteral( "spacingUnit" ), QgsUnitTypes::encodeUnit( mSpacingUnit ) );
  categoryElem.setAttribute( QStringLiteral( "spacingUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mSpacingMapUnitScale ) );
  categoryElem.setAttribute( QStringLiteral( "direction" ), QString::number( mDirection ) );

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
  }

  categoryElem.setAttribute( QStringLiteral( "barWidth" ), QString::number( barWidth ) );
  categoryElem.setAttribute( QStringLiteral( "minimumSize" ), QString::number( minimumSize ) );
  categoryElem.setAttribute( QStringLiteral( "rotationOffset" ), QString::number( rotationOffset ) );

  const int nCats = std::min( categoryColors.size(), categoryAttributes.size() );
  for ( int i = 0; i < nCats; ++i )
  {
    QDomElement attributeElem = doc.createElement( QStringLiteral( "attribute" ) );

    attributeElem.setAttribute( QStringLiteral( "field" ), categoryAttributes.at( i ) );
    attributeElem.setAttribute( QStringLiteral( "color" ), categoryColors.at( i ).name() );
    attributeElem.setAttribute( QStringLiteral( "label" ), categoryLabels.at( i ) );
    categoryElem.appendChild( attributeElem );
  }

  categoryElem.setAttribute( QStringLiteral( "showAxis" ), mShowAxis ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  QDomElement axisSymbolElem = doc.createElement( QStringLiteral( "axisSymbol" ) );
  const QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( QString(), mAxisLineSymbol.get(), doc, context );
  axisSymbolElem.appendChild( symbolElem );
  categoryElem.appendChild( axisSymbolElem );

  if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect.get() ) )
    mPaintEffect->saveProperties( doc, categoryElem );

  rendererElem.appendChild( categoryElem );
}

void QgsDiagramRenderer::setDiagram( QgsDiagram *d )
{
  if ( mDiagram.get() == d )
    return;

  mDiagram.reset( d );
}

QgsDiagramRenderer::QgsDiagramRenderer( const QgsDiagramRenderer &other )
  : mDiagram( other.mDiagram ? other.mDiagram->clone() : nullptr )
  , mShowAttributeLegend( other.mShowAttributeLegend )
{
}

QgsDiagramRenderer &QgsDiagramRenderer::operator=( const QgsDiagramRenderer &other )
{
  mDiagram.reset( other.mDiagram ? other.mDiagram->clone() : nullptr );
  mShowAttributeLegend = other.mShowAttributeLegend;
  return *this;
}

void QgsDiagramRenderer::renderDiagram( const QgsFeature &feature, QgsRenderContext &c, QPointF pos, const QgsPropertyCollection &properties ) const
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
    c.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( s.backgroundColor ) );
    s.backgroundColor = properties.valueAsColor( QgsDiagramLayerSettings::BackgroundColor, c.expressionContext(), s.backgroundColor );
    c.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( s.penColor ) );
    s.penColor = properties.valueAsColor( QgsDiagramLayerSettings::StrokeColor, c.expressionContext(), s.penColor );
    c.expressionContext().setOriginalValueVariable( s.penWidth );
    s.penWidth = properties.valueAsDouble( QgsDiagramLayerSettings::StrokeWidth, c.expressionContext(), s.penWidth );
    c.expressionContext().setOriginalValueVariable( s.rotationOffset );
    s.rotationOffset = properties.valueAsDouble( QgsDiagramLayerSettings::StartAngle, c.expressionContext(), s.rotationOffset );
  }

  QgsPaintEffect *effect = s.paintEffect();
  std::unique_ptr< QgsEffectPainter > effectPainter;
  if ( effect && effect->enabled() )
  {
    effectPainter = std::make_unique< QgsEffectPainter >( c, effect );
  }

  mDiagram->renderDiagram( feature, c, s, pos );
}

QSizeF QgsDiagramRenderer::sizeMapUnits( const QgsFeature &feature, const QgsRenderContext &c ) const
{
  QgsDiagramSettings s;
  if ( !diagramSettings( feature, c, s ) )
  {
    return QSizeF();
  }

  QSizeF size = diagramSize( feature, c );
  if ( size.isValid() )
  {
    const double width = c.convertToMapUnits( size.width(), s.sizeType, s.sizeScale );
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

  const auto constDiagramAttributes = diagramAttributes();
  for ( const QString &att : constDiagramAttributes )
  {
    QgsExpression *expression = mDiagram->getExpression( att, context );
    const auto constReferencedColumns = expression->referencedColumns();
    for ( const QString &field : constReferencedColumns )
    {
      referenced << field;
    }
  }
  return referenced;
}

void QgsDiagramRenderer::convertSizeToMapUnits( QSizeF &size, const QgsRenderContext &context ) const
{
  if ( !size.isValid() )
  {
    return;
  }

  const double pixelToMap = context.scaleFactor() * context.mapToPixel().mapUnitsPerPixel();
  size.rwidth() *= pixelToMap;
  size.rheight() *= pixelToMap;
}

int QgsDiagramRenderer::dpiPaintDevice( const QPainter *painter )
{
  if ( painter )
  {
    QPaintDevice *device = painter->device();
    if ( device )
    {
      return device->logicalDpiX();
    }
  }
  return -1;
}

void QgsDiagramRenderer::_readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  mDiagram.reset();
  const QString diagramType = elem.attribute( QStringLiteral( "diagramType" ) );
  if ( diagramType == QLatin1String( "Pie" ) )
  {
    mDiagram.reset( new QgsPieDiagram() );
  }
  else if ( diagramType == QLatin1String( "Text" ) )
  {
    mDiagram.reset( new QgsTextDiagram() );
  }
  else if ( diagramType == QLatin1String( "Histogram" ) )
  {
    mDiagram.reset( new QgsHistogramDiagram() );
  }
  else if ( diagramType == QLatin1String( "Stacked" ) )
  {
    mDiagram.reset( new QgsStackedBarDiagram() );
  }
  else
  {
    // unknown diagram type -- default to histograms
    mDiagram.reset( new QgsHistogramDiagram() );
  }
  mShowAttributeLegend = ( elem.attribute( QStringLiteral( "attributeLegend" ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
}

void QgsDiagramRenderer::_writeXml( QDomElement &rendererElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( doc )
  Q_UNUSED( context )

  if ( mDiagram )
  {
    rendererElem.setAttribute( QStringLiteral( "diagramType" ), mDiagram->diagramName() );
  }
  rendererElem.setAttribute( QStringLiteral( "attributeLegend" ), mShowAttributeLegend );
}

QgsSingleCategoryDiagramRenderer *QgsSingleCategoryDiagramRenderer::clone() const
{
  return new QgsSingleCategoryDiagramRenderer( *this );
}

bool QgsSingleCategoryDiagramRenderer::diagramSettings( const QgsFeature &, const QgsRenderContext &c, QgsDiagramSettings &s ) const
{
  Q_UNUSED( c )
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

void QgsSingleCategoryDiagramRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  const QDomElement categoryElem = elem.firstChildElement( QStringLiteral( "DiagramCategory" ) );
  if ( categoryElem.isNull() )
  {
    return;
  }

  mSettings.readXml( categoryElem, context );
  _readXml( elem, context );
}

void QgsSingleCategoryDiagramRenderer::writeXml( QDomElement &layerElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "SingleCategoryDiagramRenderer" ) );
  mSettings.writeXml( rendererElem, doc, context );
  _writeXml( rendererElem, doc, context );
  layerElem.appendChild( rendererElem );
}


QgsLinearlyInterpolatedDiagramRenderer::QgsLinearlyInterpolatedDiagramRenderer()
{
  mInterpolationSettings.classificationAttributeIsExpression = false;
}

QgsLinearlyInterpolatedDiagramRenderer::QgsLinearlyInterpolatedDiagramRenderer( const QgsLinearlyInterpolatedDiagramRenderer &other )
  : QgsDiagramRenderer( other )
  , mSettings( other.mSettings )
  , mInterpolationSettings( other.mInterpolationSettings )
  , mDataDefinedSizeLegend( other.mDataDefinedSizeLegend ? new QgsDataDefinedSizeLegend( *other.mDataDefinedSizeLegend ) : nullptr )
{
}

QgsLinearlyInterpolatedDiagramRenderer::~QgsLinearlyInterpolatedDiagramRenderer()
{
  delete mDataDefinedSizeLegend;
}

QgsLinearlyInterpolatedDiagramRenderer &QgsLinearlyInterpolatedDiagramRenderer::operator=( const QgsLinearlyInterpolatedDiagramRenderer &other )
{
  if ( &other == this )
  {
    return *this;
  }
  mSettings = other.mSettings;
  mInterpolationSettings = other.mInterpolationSettings;
  delete mDataDefinedSizeLegend;
  mDataDefinedSizeLegend = new QgsDataDefinedSizeLegend( *other.mDataDefinedSizeLegend );
  return *this;
}

QgsLinearlyInterpolatedDiagramRenderer *QgsLinearlyInterpolatedDiagramRenderer::clone() const
{
  return new QgsLinearlyInterpolatedDiagramRenderer( *this );
}

QList<QgsDiagramSettings> QgsLinearlyInterpolatedDiagramRenderer::diagramSettings() const
{
  QList<QgsDiagramSettings> settingsList;
  settingsList.push_back( mSettings );
  return settingsList;
}

bool QgsLinearlyInterpolatedDiagramRenderer::diagramSettings( const QgsFeature &feature, const QgsRenderContext &c, QgsDiagramSettings &s ) const
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
    QgsExpression *expression = mDiagram->getExpression( mInterpolationSettings.classificationAttributeExpression, context );
    const auto constReferencedColumns = expression->referencedColumns();
    for ( const QString &field : constReferencedColumns )
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

QSizeF QgsLinearlyInterpolatedDiagramRenderer::diagramSize( const QgsFeature &feature, const QgsRenderContext &c ) const
{
  return mDiagram->diagramSize( feature, c, mSettings, mInterpolationSettings );
}

void QgsLinearlyInterpolatedDiagramRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
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
    mInterpolationSettings.classificationField = elem.attribute( QStringLiteral( "classificationField" ) );
  }
  const QDomElement settingsElem = elem.firstChildElement( QStringLiteral( "DiagramCategory" ) );
  if ( !settingsElem.isNull() )
  {
    mSettings.readXml( settingsElem );
  }

  delete mDataDefinedSizeLegend;

  const QDomElement ddsLegendSizeElem = elem.firstChildElement( QStringLiteral( "data-defined-size-legend" ) );
  if ( !ddsLegendSizeElem.isNull() )
  {
    mDataDefinedSizeLegend = QgsDataDefinedSizeLegend::readXml( ddsLegendSizeElem, context );
  }
  else
  {
    // pre-3.0 projects
    if ( elem.attribute( QStringLiteral( "sizeLegend" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) )
    {
      mDataDefinedSizeLegend = new QgsDataDefinedSizeLegend();
      const QDomElement sizeLegendSymbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
      if ( !sizeLegendSymbolElem.isNull() && sizeLegendSymbolElem.attribute( QStringLiteral( "name" ) ) == QLatin1String( "sizeSymbol" ) )
      {
        mDataDefinedSizeLegend->setSymbol( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( sizeLegendSymbolElem, context ) );
      }
    }
    else
    {
      mDataDefinedSizeLegend = nullptr;
    }
  }

  _readXml( elem, context );
}

void QgsLinearlyInterpolatedDiagramRenderer::writeXml( QDomElement &layerElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
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
  mSettings.writeXml( rendererElem, doc );

  if ( mDataDefinedSizeLegend )
  {
    QDomElement ddsLegendElem = doc.createElement( QStringLiteral( "data-defined-size-legend" ) );
    mDataDefinedSizeLegend->writeXml( ddsLegendElem, context );
    rendererElem.appendChild( ddsLegendElem );
  }

  _writeXml( rendererElem, doc, context );
  layerElem.appendChild( rendererElem );
}

QList< QgsLayerTreeModelLegendNode * > QgsDiagramSettings::legendItems( QgsLayerTreeLayer *nodeLayer ) const
{
  QList< QgsLayerTreeModelLegendNode * > list;
  list.reserve( categoryLabels.size() );
  for ( int i = 0; i < categoryLabels.size(); ++i )
  {
    QPixmap pix( 16, 16 );
    pix.fill( categoryColors[i] );
    list << new QgsSimpleLegendNode( nodeLayer, categoryLabels[i], QIcon( pix ), nullptr, QStringLiteral( "diagram_%1" ).arg( QString::number( i ) ) );
  }
  return list;
}

QgsLineSymbol *QgsDiagramSettings::axisLineSymbol() const
{
  return mAxisLineSymbol.get();
}

void QgsDiagramSettings::setAxisLineSymbol( QgsLineSymbol *axisLineSymbol )
{
  if ( axisLineSymbol != mAxisLineSymbol.get() )
    mAxisLineSymbol.reset( axisLineSymbol );
}

bool QgsDiagramSettings::showAxis() const
{
  return mShowAxis;
}

void QgsDiagramSettings::setShowAxis( bool showAxis )
{
  mShowAxis = showAxis;
}

QgsPaintEffect *QgsDiagramSettings::paintEffect() const
{
  return mPaintEffect.get();
}

void QgsDiagramSettings::setPaintEffect( QgsPaintEffect *effect )
{
  if ( effect != mPaintEffect.get() )
    mPaintEffect.reset( effect );
}

QgsDiagramSettings::QgsDiagramSettings()
  : mAxisLineSymbol( std::make_unique< QgsLineSymbol >() )
{
}

QgsDiagramSettings::~QgsDiagramSettings() = default;

QgsDiagramSettings::QgsDiagramSettings( const QgsDiagramSettings &other )
  : enabled( other.enabled )
  , font( other.font )
  , categoryColors( other.categoryColors )
  , categoryAttributes( other.categoryAttributes )
  , categoryLabels( other.categoryLabels )
  , size( other.size )
  , sizeType( other.sizeType )
  , sizeScale( other.sizeScale )
  , lineSizeUnit( other.lineSizeUnit )
  , lineSizeScale( other.lineSizeScale )
  , backgroundColor( other.backgroundColor )
  , penColor( other.penColor )
  , penWidth( other.penWidth )
  , labelPlacementMethod( other.labelPlacementMethod )
  , diagramOrientation( other.diagramOrientation )
  , barWidth( other.barWidth )
  , opacity( other.opacity )
  , scaleByArea( other.scaleByArea )
  , rotationOffset( other.rotationOffset )
  , scaleBasedVisibility( other.scaleBasedVisibility )
  , maximumScale( other.maximumScale )
  , minimumScale( other.minimumScale )
  , minimumSize( other.minimumSize )
  , mSpacing( other.mSpacing )
  , mSpacingUnit( other.mSpacingUnit )
  , mSpacingMapUnitScale( other.mSpacingMapUnitScale )
  , mDirection( other.mDirection )
  , mShowAxis( other.mShowAxis )
  , mAxisLineSymbol( other.mAxisLineSymbol ? other.mAxisLineSymbol->clone() : nullptr )
  , mPaintEffect( other.mPaintEffect ? other.mPaintEffect->clone() : nullptr )
{

}

QgsDiagramSettings &QgsDiagramSettings::operator=( const QgsDiagramSettings &other )
{
  enabled = other.enabled;
  font = other.font;
  categoryColors = other.categoryColors;
  categoryAttributes = other.categoryAttributes;
  categoryLabels = other.categoryLabels;
  size = other.size;
  sizeType = other.sizeType;
  sizeScale = other.sizeScale;
  lineSizeUnit = other.lineSizeUnit;
  lineSizeScale = other.lineSizeScale;
  backgroundColor = other.backgroundColor;
  penColor = other.penColor;
  penWidth = other.penWidth;
  labelPlacementMethod = other.labelPlacementMethod;
  diagramOrientation = other.diagramOrientation;
  barWidth = other.barWidth;
  opacity = other.opacity;
  scaleByArea = other.scaleByArea;
  rotationOffset = other.rotationOffset;
  scaleBasedVisibility = other.scaleBasedVisibility;
  maximumScale = other.maximumScale;
  minimumScale = other.minimumScale;
  minimumSize = other.minimumSize;
  mSpacing = other.mSpacing;
  mSpacingUnit = other.mSpacingUnit;
  mSpacingMapUnitScale = other.mSpacingMapUnitScale;
  mDirection = other.mDirection;
  mAxisLineSymbol.reset( other.mAxisLineSymbol ? other.mAxisLineSymbol->clone() : nullptr );
  mShowAxis = other.mShowAxis;
  mPaintEffect.reset( other.mPaintEffect ? other.mPaintEffect->clone() : nullptr );
  return *this;
}

QgsDiagramSettings::Direction QgsDiagramSettings::direction() const
{
  return mDirection;
}

void QgsDiagramSettings::setDirection( Direction direction )
{
  mDirection = direction;
}

QList< QgsLayerTreeModelLegendNode * > QgsDiagramRenderer::legendItems( QgsLayerTreeLayer * ) const
{
  return QList< QgsLayerTreeModelLegendNode * >();
}

QList< QgsLayerTreeModelLegendNode * > QgsSingleCategoryDiagramRenderer::legendItems( QgsLayerTreeLayer *nodeLayer ) const
{
  QList< QgsLayerTreeModelLegendNode * > nodes;
  if ( mShowAttributeLegend )
    nodes = mSettings.legendItems( nodeLayer );

  return nodes;
}

QList< QgsLayerTreeModelLegendNode * > QgsLinearlyInterpolatedDiagramRenderer::legendItems( QgsLayerTreeLayer *nodeLayer ) const
{
  QList< QgsLayerTreeModelLegendNode * > nodes;
  if ( mShowAttributeLegend )
    nodes = mSettings.legendItems( nodeLayer );

  if ( mDataDefinedSizeLegend && mDiagram )
  {
    // add size legend
    QgsMarkerSymbol *legendSymbol = mDataDefinedSizeLegend->symbol() ? mDataDefinedSizeLegend->symbol()->clone() : QgsMarkerSymbol::createSimple( QVariantMap() );
    legendSymbol->setSizeUnit( mSettings.sizeType );
    legendSymbol->setSizeMapUnitScale( mSettings.sizeScale );

    QgsDataDefinedSizeLegend ddSizeLegend( *mDataDefinedSizeLegend );
    ddSizeLegend.setSymbol( legendSymbol );  // transfers ownership

    QList<QgsDataDefinedSizeLegend::SizeClass> sizeClasses;
    if ( ddSizeLegend.classes().isEmpty() )
    {
      // automatic class creation if the classes are not defined manually
      const auto prettyBreaks { QgsSymbolLayerUtils::prettyBreaks( mInterpolationSettings.lowerValue, mInterpolationSettings.upperValue, 4 ) };
      for ( const double v : prettyBreaks )
      {
        const double size = mDiagram->legendSize( v, mSettings, mInterpolationSettings );
        sizeClasses << QgsDataDefinedSizeLegend::SizeClass( size, QString::number( v ) );
      }
    }
    else
    {
      // manual classes need to get size scaled because the QgsSizeScaleTransformer is not used in diagrams :-(
      const auto constClasses = ddSizeLegend.classes();
      for ( const QgsDataDefinedSizeLegend::SizeClass &sc : constClasses )
      {
        const double size = mDiagram->legendSize( sc.size, mSettings, mInterpolationSettings );
        sizeClasses << QgsDataDefinedSizeLegend::SizeClass( size, sc.label );
      }
    }
    ddSizeLegend.setClasses( sizeClasses );

    const auto constLegendSymbolList = ddSizeLegend.legendSymbolList();
    for ( const QgsLegendSymbolItem &si : constLegendSymbolList )
    {
      if ( auto *lDataDefinedSizeLegendSettings = si.dataDefinedSizeLegendSettings() )
        nodes << new QgsDataDefinedSizeLegendNode( nodeLayer, *lDataDefinedSizeLegendSettings );
      else
        nodes << new QgsSymbolLegendNode( nodeLayer, si );
    }
  }

  return nodes;
}

void QgsLinearlyInterpolatedDiagramRenderer::setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings )
{
  delete mDataDefinedSizeLegend;
  mDataDefinedSizeLegend = settings;
}

QgsDataDefinedSizeLegend *QgsLinearlyInterpolatedDiagramRenderer::dataDefinedSizeLegend() const
{
  return mDataDefinedSizeLegend;
}
