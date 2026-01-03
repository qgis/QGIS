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

#include <memory>

#include "diagram/qgshistogramdiagram.h"
#include "diagram/qgspiediagram.h"
#include "diagram/qgsstackedbardiagram.h"
#include "diagram/qgsstackeddiagram.h"
#include "diagram/qgstextdiagram.h"
#include "qgsapplication.h"
#include "qgscolorutils.h"
#include "qgsdatadefinedsizelegend.h"
#include "qgsfontutils.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsrendercontext.h"
#include "qgsscaleutils.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"

#include <QDomElement>
#include <QPainter>

QgsPropertiesDefinition QgsDiagramLayerSettings::sPropertyDefinitions;

void QgsDiagramLayerSettings::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  const QString origin = u"diagram"_s;

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { static_cast< int >( QgsDiagramLayerSettings::Property::BackgroundColor ), QgsPropertyDefinition( "backgroundColor", QObject::tr( "Background color" ), QgsPropertyDefinition::ColorWithAlpha, origin ) },
    { static_cast< int >( QgsDiagramLayerSettings::Property::StrokeColor ), QgsPropertyDefinition( "strokeColor", QObject::tr( "Stroke color" ), QgsPropertyDefinition::ColorWithAlpha, origin ) },
    { static_cast< int >( QgsDiagramLayerSettings::Property::StrokeWidth ), QgsPropertyDefinition( "strokeWidth", QObject::tr( "Stroke width" ), QgsPropertyDefinition::StrokeWidth, origin ) },
    { static_cast< int >( QgsDiagramLayerSettings::Property::PositionX ), QgsPropertyDefinition( "positionX", QObject::tr( "Position (X)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsDiagramLayerSettings::Property::PositionY ), QgsPropertyDefinition( "positionY", QObject::tr( "Position (Y)" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsDiagramLayerSettings::Property::Distance ), QgsPropertyDefinition( "distance", QObject::tr( "Placement distance" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsDiagramLayerSettings::Property::Priority ), QgsPropertyDefinition( "priority", QObject::tr( "Placement priority" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsDiagramLayerSettings::Property::ZIndex ), QgsPropertyDefinition( "zIndex", QObject::tr( "Placement z-index" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsDiagramLayerSettings::Property::IsObstacle ), QgsPropertyDefinition( "isObstacle", QObject::tr( "Diagram is an obstacle" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsDiagramLayerSettings::Property::Show ), QgsPropertyDefinition( "show", QObject::tr( "Show diagram" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsDiagramLayerSettings::Property::AlwaysShow ), QgsPropertyDefinition( "alwaysShow", QObject::tr( "Always show diagram" ), QgsPropertyDefinition::Boolean, origin ) },
    { static_cast< int >( QgsDiagramLayerSettings::Property::StartAngle ), QgsPropertyDefinition( "startAngle", QObject::tr( "Pie chart start angle" ), QgsPropertyDefinition::Rotation, origin ) },
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
//****** IMPORTANT! editing this? make sure you update the move constructor too! *****
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
    //****** IMPORTANT! editing this? make sure you update the move constructor too! *****
{
  initPropertyDefinitions();
}

QgsDiagramLayerSettings::QgsDiagramLayerSettings( QgsDiagramLayerSettings &&rh )
  : mCt( std::move( rh.mCt ) )
  , mPlacement( rh.mPlacement )
  , mPlacementFlags( rh.mPlacementFlags )
  , mPriority( rh.mPriority )
  , mZIndex( rh.mZIndex )
  , mObstacle( rh.mObstacle )
  , mDistance( rh.mDistance )
  , mRenderer( std::move( rh.mRenderer ) )
  , mShowAll( rh.mShowAll )
  , mDataDefinedProperties( std::move( rh.mDataDefinedProperties ) )
{
}

QgsDiagramLayerSettings &QgsDiagramLayerSettings::operator=( const QgsDiagramLayerSettings &rh )
{
  if ( &rh == this )
    return *this;

  //****** IMPORTANT! editing this? make sure you update the move assignment operator too! *****
  mPlacement = rh.mPlacement;
  mPlacementFlags = rh.mPlacementFlags;
  mPriority = rh.mPriority;
  mZIndex = rh.mZIndex;
  mObstacle = rh.mObstacle;
  mDistance = rh.mDistance;
  mRenderer.reset( rh.mRenderer ? rh.mRenderer->clone() : nullptr );
  mCt = rh.mCt;
  mShowAll = rh.mShowAll;
  mDataDefinedProperties = rh.mDataDefinedProperties;
  //****** IMPORTANT! editing this? make sure you update the move assignment operator too! *****
  return *this;
}

QgsDiagramLayerSettings &QgsDiagramLayerSettings::operator=( QgsDiagramLayerSettings &&rh )
{
  if ( &rh == this )
    return *this;

  mPlacement = rh.mPlacement;
  mPlacementFlags = rh.mPlacementFlags;
  mPriority = rh.mPriority;
  mZIndex = rh.mZIndex;
  mObstacle = rh.mObstacle;
  mDistance = rh.mDistance;
  mRenderer = std::move( rh.mRenderer );
  mCt = std::move( rh.mCt );
  mShowAll = rh.mShowAll;
  mDataDefinedProperties = std::move( rh.mDataDefinedProperties );
  return *this;
}

QgsDiagramLayerSettings::~QgsDiagramLayerSettings()
{

}

void QgsDiagramLayerSettings::setRenderer( QgsDiagramRenderer *diagramRenderer )
{
  if ( diagramRenderer == mRenderer.get() )
    return;

  mRenderer.reset( diagramRenderer );

}

void QgsDiagramLayerSettings::setCoordinateTransform( const QgsCoordinateTransform &transform )
{
  mCt = transform;
}

void QgsDiagramLayerSettings::readXml( const QDomElement &elem )
{
  const QDomNodeList propertyElems = elem.elementsByTagName( u"properties"_s );
  if ( !propertyElems.isEmpty() )
  {
    ( void )mDataDefinedProperties.readXml( propertyElems.at( 0 ).toElement(), sPropertyDefinitions );
  }
  else
  {
    mDataDefinedProperties.clear();
  }

  mPlacement = static_cast< Placement >( elem.attribute( u"placement"_s ).toInt() );
  mPlacementFlags = static_cast< LinePlacementFlag >( elem.attribute( u"linePlacementFlags"_s ).toInt() );
  mPriority = elem.attribute( u"priority"_s ).toInt();
  mZIndex = elem.attribute( u"zIndex"_s ).toDouble();
  mObstacle = elem.attribute( u"obstacle"_s ).toInt();
  mDistance = elem.attribute( u"dist"_s ).toDouble();
  mShowAll = ( elem.attribute( u"showAll"_s, u"0"_s ) != "0"_L1 );
}

void QgsDiagramLayerSettings::writeXml( QDomElement &layerElem, QDomDocument &doc ) const
{
  QDomElement diagramLayerElem = doc.createElement( u"DiagramLayerSettings"_s );
  QDomElement propertiesElem = doc.createElement( u"properties"_s );
  ( void )mDataDefinedProperties.writeXml( propertiesElem, sPropertyDefinitions );
  diagramLayerElem.appendChild( propertiesElem );
  diagramLayerElem.setAttribute( u"placement"_s, mPlacement );
  diagramLayerElem.setAttribute( u"linePlacementFlags"_s, mPlacementFlags );
  diagramLayerElem.setAttribute( u"priority"_s, mPriority );
  diagramLayerElem.setAttribute( u"zIndex"_s, mZIndex );
  diagramLayerElem.setAttribute( u"obstacle"_s, mObstacle );
  diagramLayerElem.setAttribute( u"dist"_s, QString::number( mDistance ) );
  diagramLayerElem.setAttribute( u"showAll"_s, mShowAll );
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
  enabled = ( elem.attribute( u"enabled"_s, u"1"_s ) != "0"_L1 );
  if ( !QgsFontUtils::setFromXmlChildNode( font, elem, u"fontProperties"_s ) )
  {
    font.fromString( elem.attribute( u"font"_s ) );
  }
  backgroundColor.setNamedColor( elem.attribute( u"backgroundColor"_s ) );
  backgroundColor.setAlpha( elem.attribute( u"backgroundAlpha"_s ).toInt() );
  size.setWidth( elem.attribute( u"width"_s ).toDouble() );
  size.setHeight( elem.attribute( u"height"_s ).toDouble() );
  if ( elem.hasAttribute( u"transparency"_s ) )
  {
    opacity = 1 - elem.attribute( u"transparency"_s, u"0"_s ).toInt() / 255.0;
  }
  else
  {
    opacity = elem.attribute( u"opacity"_s, u"1.00"_s ).toDouble();
  }

  penColor.setNamedColor( elem.attribute( u"penColor"_s ) );
  const int penAlpha = elem.attribute( u"penAlpha"_s, u"255"_s ).toInt();
  penColor.setAlpha( penAlpha );
  penWidth = elem.attribute( u"penWidth"_s ).toDouble();

  mDirection = static_cast< Direction >( elem.attribute( u"direction"_s, u"1"_s ).toInt() );

  maximumScale = elem.attribute( u"minScaleDenominator"_s, u"-1"_s ).toDouble();
  minimumScale = elem.attribute( u"maxScaleDenominator"_s, u"-1"_s ).toDouble();
  if ( elem.hasAttribute( u"scaleBasedVisibility"_s ) )
  {
    scaleBasedVisibility = ( elem.attribute( u"scaleBasedVisibility"_s, u"1"_s ) != "0"_L1 );
  }
  else
  {
    scaleBasedVisibility = maximumScale >= 0 && minimumScale >= 0;
  }

  //diagram size unit type and scale
  if ( elem.attribute( u"sizeType"_s ) == "MapUnits"_L1 )
  {
    //compatibility with pre-2.16 project files
    sizeType = Qgis::RenderUnit::MapUnits;
  }
  else
  {
    sizeType = QgsUnitTypes::decodeRenderUnit( elem.attribute( u"sizeType"_s ) );
  }
  sizeScale = QgsSymbolLayerUtils::decodeMapUnitScale( elem.attribute( u"sizeScale"_s ) );

  //line width unit type and scale
  lineSizeUnit = QgsUnitTypes::decodeRenderUnit( elem.attribute( u"lineSizeType"_s ) );
  lineSizeScale = QgsSymbolLayerUtils::decodeMapUnitScale( elem.attribute( u"lineSizeScale"_s ) );

  mSpacing = elem.attribute( u"spacing"_s ).toDouble();
  mSpacingUnit = QgsUnitTypes::decodeRenderUnit( elem.attribute( u"spacingUnit"_s ) );
  mSpacingMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( elem.attribute( u"spacingUnitScale"_s ) );

  mStackedDiagramSpacing = elem.attribute( u"stackedDiagramSpacing"_s ).toDouble();
  mStackedDiagramSpacingUnit = QgsUnitTypes::decodeRenderUnit( elem.attribute( u"stackedDiagramSpacingUnit"_s ) );
  mStackedDiagramSpacingMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( elem.attribute( u"stackedDiagramSpacingUnitScale"_s ) );

  //label placement method
  if ( elem.attribute( u"labelPlacementMethod"_s ) == "Height"_L1 )
  {
    labelPlacementMethod = Height;
  }
  else
  {
    labelPlacementMethod = XHeight;
  }

  // orientation
  if ( elem.attribute( u"diagramOrientation"_s ) == "Left"_L1 )
  {
    diagramOrientation = Left;
  }
  else if ( elem.attribute( u"diagramOrientation"_s ) == "Right"_L1 )
  {
    diagramOrientation = Right;
  }
  else if ( elem.attribute( u"diagramOrientation"_s ) == "Down"_L1 )
  {
    diagramOrientation = Down;
  }
  else
  {
    diagramOrientation = Up;
  }

  // stacked mode
  if ( elem.attribute( u"stackedDiagramMode"_s ) == "Horizontal"_L1 )
  {
    stackedDiagramMode = Horizontal;
  }
  else if ( elem.attribute( u"stackedDiagramMode"_s ) == "Vertical"_L1 )
  {
    stackedDiagramMode = Vertical;
  }

  // scale dependency
  if ( elem.attribute( u"scaleDependency"_s ) == "Diameter"_L1 )
  {
    scaleByArea = false;
  }
  else
  {
    scaleByArea = true;
  }

  barWidth = elem.attribute( u"barWidth"_s ).toDouble();

  if ( elem.hasAttribute( u"angleOffset"_s ) )
    rotationOffset = std::fmod( 360.0 - elem.attribute( u"angleOffset"_s ).toInt() / 16.0, 360.0 );
  else
    rotationOffset = elem.attribute( u"rotationOffset"_s ).toDouble();

  minimumSize = elem.attribute( u"minimumSize"_s ).toDouble();

  const QDomNodeList axisSymbolNodes = elem.elementsByTagName( u"axisSymbol"_s );
  if ( axisSymbolNodes.count() > 0 )
  {
    const QDomElement axisSymbolElem = axisSymbolNodes.at( 0 ).toElement().firstChildElement();
    mAxisLineSymbol = QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( axisSymbolElem, context );
  }
  else
  {
    mAxisLineSymbol = std::make_unique< QgsLineSymbol >();
  }

  mShowAxis = elem.attribute( u"showAxis"_s, u"0"_s ).toInt();

  //colors
  categoryColors.clear();
  const QDomNodeList attributes = elem.elementsByTagName( u"attribute"_s );


  if ( attributes.length() > 0 )
  {
    for ( int i = 0; i < attributes.size(); i++ )
    {
      const QDomElement attrElem = attributes.at( i ).toElement();
      QColor newColor( attrElem.attribute( u"color"_s ) );
      newColor.setAlphaF( attrElem.attribute( u"colorOpacity"_s, u"1.0"_s ).toDouble() );
      categoryColors.append( newColor );
      categoryAttributes.append( attrElem.attribute( u"field"_s ) );
      categoryLabels.append( attrElem.attribute( u"label"_s ) );
      if ( categoryLabels.constLast().isEmpty() )
      {
        categoryLabels.back() = categoryAttributes.back();
      }
    }
  }
  else
  {
    // Restore old format attributes and colors

    const QStringList colorList = elem.attribute( u"colors"_s ).split( '/' );
    QStringList::const_iterator colorIt = colorList.constBegin();
    for ( ; colorIt != colorList.constEnd(); ++colorIt )
    {
      QColor newColor( *colorIt );
      categoryColors.append( QColor( newColor ) );
    }

    //attribute indices
    categoryAttributes.clear();
    const QStringList catList = elem.attribute( u"categories"_s ).split( '/' );
    QStringList::const_iterator catIt = catList.constBegin();
    for ( ; catIt != catList.constEnd(); ++catIt )
    {
      categoryAttributes.append( *catIt );
      categoryLabels.append( *catIt );
    }
  }

  const QDomElement effectElem = elem.firstChildElement( u"effect"_s );
  if ( !effectElem.isNull() )
    setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
  else
    setPaintEffect( QgsPaintEffectRegistry::defaultStack() );
}

void QgsDiagramSettings::writeXml( QDomElement &rendererElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement categoryElem = doc.createElement( u"DiagramCategory"_s );
  categoryElem.setAttribute( u"enabled"_s, enabled );
  categoryElem.appendChild( QgsFontUtils::toXmlElement( font, doc, u"fontProperties"_s ) );
  categoryElem.setAttribute( u"backgroundColor"_s, backgroundColor.name() );
  categoryElem.setAttribute( u"backgroundAlpha"_s, backgroundColor.alpha() );
  categoryElem.setAttribute( u"width"_s, QString::number( size.width() ) );
  categoryElem.setAttribute( u"height"_s, QString::number( size.height() ) );
  categoryElem.setAttribute( u"penColor"_s, penColor.name() );
  categoryElem.setAttribute( u"penAlpha"_s, penColor.alpha() );
  categoryElem.setAttribute( u"penWidth"_s, QString::number( penWidth ) );
  categoryElem.setAttribute( u"scaleBasedVisibility"_s, scaleBasedVisibility );
  categoryElem.setAttribute( u"minScaleDenominator"_s, QString::number( maximumScale ) );
  categoryElem.setAttribute( u"maxScaleDenominator"_s, QString::number( minimumScale ) );
  categoryElem.setAttribute( u"opacity"_s, QString::number( opacity ) );
  categoryElem.setAttribute( u"spacing"_s, QString::number( mSpacing ) );
  categoryElem.setAttribute( u"spacingUnit"_s, QgsUnitTypes::encodeUnit( mSpacingUnit ) );
  categoryElem.setAttribute( u"spacingUnitScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( mSpacingMapUnitScale ) );
  categoryElem.setAttribute( u"stackedDiagramSpacing"_s, QString::number( mStackedDiagramSpacing ) );
  categoryElem.setAttribute( u"stackedDiagramSpacingUnit"_s, QgsUnitTypes::encodeUnit( mStackedDiagramSpacingUnit ) );
  categoryElem.setAttribute( u"stackedDiagramSpacingUnitScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( mStackedDiagramSpacingMapUnitScale ) );
  categoryElem.setAttribute( u"direction"_s, QString::number( mDirection ) );

  //diagram size unit type and scale
  categoryElem.setAttribute( u"sizeType"_s, QgsUnitTypes::encodeUnit( sizeType ) );
  categoryElem.setAttribute( u"sizeScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( sizeScale ) );

  //line width unit type and scale
  categoryElem.setAttribute( u"lineSizeType"_s, QgsUnitTypes::encodeUnit( lineSizeUnit ) );
  categoryElem.setAttribute( u"lineSizeScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( lineSizeScale ) );

  // label placement method (text diagram)
  if ( labelPlacementMethod == Height )
  {
    categoryElem.setAttribute( u"labelPlacementMethod"_s, u"Height"_s );
  }
  else
  {
    categoryElem.setAttribute( u"labelPlacementMethod"_s, u"XHeight"_s );
  }

  if ( scaleByArea )
  {
    categoryElem.setAttribute( u"scaleDependency"_s, u"Area"_s );
  }
  else
  {
    categoryElem.setAttribute( u"scaleDependency"_s, u"Diameter"_s );
  }

  // orientation (histogram)
  switch ( diagramOrientation )
  {
    case Left:
      categoryElem.setAttribute( u"diagramOrientation"_s, u"Left"_s );
      break;

    case Right:
      categoryElem.setAttribute( u"diagramOrientation"_s, u"Right"_s );
      break;

    case Down:
      categoryElem.setAttribute( u"diagramOrientation"_s, u"Down"_s );
      break;

    case Up:
      categoryElem.setAttribute( u"diagramOrientation"_s, u"Up"_s );
      break;
  }

  // stacked mode
  switch ( stackedDiagramMode )
  {
    case Horizontal:
      categoryElem.setAttribute( u"stackedDiagramMode"_s, u"Horizontal"_s );
      break;

    case Vertical:
      categoryElem.setAttribute( u"stackedDiagramMode"_s, u"Vertical"_s );
      break;
  }

  categoryElem.setAttribute( u"barWidth"_s, QString::number( barWidth ) );
  categoryElem.setAttribute( u"minimumSize"_s, QString::number( minimumSize ) );
  categoryElem.setAttribute( u"rotationOffset"_s, QString::number( rotationOffset ) );

  const int nCats = std::min( categoryColors.size(), categoryAttributes.size() );
  for ( int i = 0; i < nCats; ++i )
  {
    QDomElement attributeElem = doc.createElement( u"attribute"_s );

    attributeElem.setAttribute( u"field"_s, categoryAttributes.at( i ) );
    attributeElem.setAttribute( u"color"_s, categoryColors.at( i ).name() );
    attributeElem.setAttribute( u"colorOpacity"_s, QString::number( categoryColors.at( i ).alphaF() ) );
    attributeElem.setAttribute( u"label"_s, categoryLabels.at( i ) );
    categoryElem.appendChild( attributeElem );
  }

  categoryElem.setAttribute( u"showAxis"_s, mShowAxis ? u"1"_s : u"0"_s );
  QDomElement axisSymbolElem = doc.createElement( u"axisSymbol"_s );
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
  if ( &other == this )
    return *this;

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
    c.expressionContext().setOriginalValueVariable( QgsColorUtils::colorToString( s.backgroundColor ) );
    s.backgroundColor = properties.valueAsColor( QgsDiagramLayerSettings::Property::BackgroundColor, c.expressionContext(), s.backgroundColor );
    c.expressionContext().setOriginalValueVariable( QgsColorUtils::colorToString( s.penColor ) );
    s.penColor = properties.valueAsColor( QgsDiagramLayerSettings::Property::StrokeColor, c.expressionContext(), s.penColor );
    c.expressionContext().setOriginalValueVariable( s.penWidth );
    s.penWidth = properties.valueAsDouble( QgsDiagramLayerSettings::Property::StrokeWidth, c.expressionContext(), s.penWidth );
    c.expressionContext().setOriginalValueVariable( s.rotationOffset );
    s.rotationOffset = properties.valueAsDouble( QgsDiagramLayerSettings::Property::StartAngle, c.expressionContext(), s.rotationOffset );
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
  if ( !diagramSettings( feature, c, s ) || !s.enabled )
  {
    return QSizeF();
  }

  if ( s.scaleBasedVisibility )
  {
    // Note: scale might be a non-round number, so compare with qgsDoubleNear
    const double rendererScale = c.rendererScale();

    // maxScale is inclusive ( < --> no size )
    double maxScale = s.maximumScale;
    if ( maxScale > 0 && QgsScaleUtils::lessThanMaximumScale( rendererScale, maxScale ) )
    {
      return QSizeF();
    }

    // minScale is exclusive ( >= --> no size)
    double minScale = s.minimumScale;
    if ( minScale > 0 && QgsScaleUtils::equalToOrGreaterThanMinimumScale( rendererScale, minScale ) )
    {
      return QSizeF();
    }
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
  const QString diagramType = elem.attribute( u"diagramType"_s );
  if ( diagramType == QgsPieDiagram::DIAGRAM_NAME_PIE )
  {
    mDiagram = std::make_unique<QgsPieDiagram>( );
  }
  else if ( diagramType == QgsTextDiagram::DIAGRAM_NAME_TEXT )
  {
    mDiagram = std::make_unique<QgsTextDiagram>( );
  }
  else if ( diagramType == QgsHistogramDiagram::DIAGRAM_NAME_HISTOGRAM )
  {
    mDiagram = std::make_unique<QgsHistogramDiagram>( );
  }
  else if ( diagramType == QgsStackedBarDiagram::DIAGRAM_NAME_STACKED_BAR )
  {
    mDiagram = std::make_unique<QgsStackedBarDiagram>( );
  }
  else if ( diagramType == QgsStackedDiagram::DIAGRAM_NAME_STACKED )
  {
    mDiagram = std::make_unique<QgsStackedDiagram>( );
  }
  else
  {
    // unknown diagram type -- default to histograms
    mDiagram = std::make_unique<QgsHistogramDiagram>( );
  }
  mShowAttributeLegend = ( elem.attribute( u"attributeLegend"_s, u"1"_s ) != "0"_L1 );
}

void QgsDiagramRenderer::_writeXml( QDomElement &rendererElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( doc )
  Q_UNUSED( context )

  if ( mDiagram )
  {
    rendererElem.setAttribute( u"diagramType"_s, mDiagram->diagramName() );
  }
  rendererElem.setAttribute( u"attributeLegend"_s, mShowAttributeLegend );
}

const QString QgsSingleCategoryDiagramRenderer::DIAGRAM_RENDERER_NAME_SINGLE_CATEGORY = u"SingleCategory"_s;

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
  const QDomElement categoryElem = elem.firstChildElement( u"DiagramCategory"_s );
  if ( categoryElem.isNull() )
  {
    return;
  }

  mSettings.readXml( categoryElem, context );
  _readXml( elem, context );
}

void QgsSingleCategoryDiagramRenderer::writeXml( QDomElement &layerElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( u"SingleCategoryDiagramRenderer"_s );
  mSettings.writeXml( rendererElem, doc, context );
  _writeXml( rendererElem, doc, context );
  layerElem.appendChild( rendererElem );
}

const QString QgsLinearlyInterpolatedDiagramRenderer::DIAGRAM_RENDERER_NAME_LINEARLY_INTERPOLATED = "LinearlyInterpolated"_L1;

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

}

QgsLinearlyInterpolatedDiagramRenderer &QgsLinearlyInterpolatedDiagramRenderer::operator=( const QgsLinearlyInterpolatedDiagramRenderer &other )
{
  if ( &other == this )
  {
    return *this;
  }
  mSettings = other.mSettings;
  mInterpolationSettings = other.mInterpolationSettings;
  mDataDefinedSizeLegend = std::make_unique<QgsDataDefinedSizeLegend>( *other.mDataDefinedSizeLegend );

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
  mInterpolationSettings.lowerValue = elem.attribute( u"lowerValue"_s ).toDouble();
  mInterpolationSettings.upperValue = elem.attribute( u"upperValue"_s ).toDouble();
  mInterpolationSettings.lowerSize.setWidth( elem.attribute( u"lowerWidth"_s ).toDouble() );
  mInterpolationSettings.lowerSize.setHeight( elem.attribute( u"lowerHeight"_s ).toDouble() );
  mInterpolationSettings.upperSize.setWidth( elem.attribute( u"upperWidth"_s ).toDouble() );
  mInterpolationSettings.upperSize.setHeight( elem.attribute( u"upperHeight"_s ).toDouble() );
  mInterpolationSettings.classificationAttributeIsExpression = elem.hasAttribute( u"classificationAttributeExpression"_s );
  if ( mInterpolationSettings.classificationAttributeIsExpression )
  {
    mInterpolationSettings.classificationAttributeExpression = elem.attribute( u"classificationAttributeExpression"_s );
  }
  else
  {
    mInterpolationSettings.classificationField = elem.attribute( u"classificationField"_s );
  }
  const QDomElement settingsElem = elem.firstChildElement( u"DiagramCategory"_s );
  if ( !settingsElem.isNull() )
  {
    mSettings.readXml( settingsElem );
  }

  mDataDefinedSizeLegend.reset( );

  const QDomElement ddsLegendSizeElem = elem.firstChildElement( u"data-defined-size-legend"_s );
  if ( !ddsLegendSizeElem.isNull() )
  {
    mDataDefinedSizeLegend.reset( QgsDataDefinedSizeLegend::readXml( ddsLegendSizeElem, context ) );
  }
  else
  {
    // pre-3.0 projects
    if ( elem.attribute( u"sizeLegend"_s, u"0"_s ) != "0"_L1 )
    {
      mDataDefinedSizeLegend = std::make_unique<QgsDataDefinedSizeLegend>();
      const QDomElement sizeLegendSymbolElem = elem.firstChildElement( u"symbol"_s );
      if ( !sizeLegendSymbolElem.isNull() && sizeLegendSymbolElem.attribute( u"name"_s ) == "sizeSymbol"_L1 )
      {
        mDataDefinedSizeLegend->setSymbol( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( sizeLegendSymbolElem, context ).release() );
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
  QDomElement rendererElem = doc.createElement( u"LinearlyInterpolatedDiagramRenderer"_s );
  rendererElem.setAttribute( u"lowerValue"_s, QString::number( mInterpolationSettings.lowerValue ) );
  rendererElem.setAttribute( u"upperValue"_s, QString::number( mInterpolationSettings.upperValue ) );
  rendererElem.setAttribute( u"lowerWidth"_s, QString::number( mInterpolationSettings.lowerSize.width() ) );
  rendererElem.setAttribute( u"lowerHeight"_s, QString::number( mInterpolationSettings.lowerSize.height() ) );
  rendererElem.setAttribute( u"upperWidth"_s, QString::number( mInterpolationSettings.upperSize.width() ) );
  rendererElem.setAttribute( u"upperHeight"_s, QString::number( mInterpolationSettings.upperSize.height() ) );
  if ( mInterpolationSettings.classificationAttributeIsExpression )
  {
    rendererElem.setAttribute( u"classificationAttributeExpression"_s, mInterpolationSettings.classificationAttributeExpression );
  }
  else
  {
    rendererElem.setAttribute( u"classificationField"_s, mInterpolationSettings.classificationField );
  }
  mSettings.writeXml( rendererElem, doc );

  if ( mDataDefinedSizeLegend )
  {
    QDomElement ddsLegendElem = doc.createElement( u"data-defined-size-legend"_s );
    mDataDefinedSizeLegend->writeXml( ddsLegendElem, context );
    rendererElem.appendChild( ddsLegendElem );
  }

  _writeXml( rendererElem, doc, context );
  layerElem.appendChild( rendererElem );
}

const QString QgsStackedDiagramRenderer::DIAGRAM_RENDERER_NAME_STACKED = u"Stacked"_s;

QgsStackedDiagramRenderer::QgsStackedDiagramRenderer( const QgsStackedDiagramRenderer &other )
  : QgsDiagramRenderer( other )
  , mSettings( other.mSettings )
  , mDiagramRenderers()
{
  for ( QgsDiagramRenderer *renderer : std::as_const( other.mDiagramRenderers ) )
  {
    if ( renderer )
      mDiagramRenderers << renderer->clone();
  }
}

QgsStackedDiagramRenderer &QgsStackedDiagramRenderer::operator=( const QgsStackedDiagramRenderer &other )
{
  if ( &other == this )
    return *this;

  mSettings = other.mSettings;
  qDeleteAll( mDiagramRenderers );
  mDiagramRenderers.clear();
  for ( QgsDiagramRenderer *renderer : std::as_const( other.mDiagramRenderers ) )
  {
    if ( renderer )
      mDiagramRenderers << renderer->clone();
  }

  return *this;
}

QgsStackedDiagramRenderer::~QgsStackedDiagramRenderer()
{
  qDeleteAll( mDiagramRenderers );
}

QgsStackedDiagramRenderer *QgsStackedDiagramRenderer::clone() const
{
  return new QgsStackedDiagramRenderer( *this );
}

QSizeF QgsStackedDiagramRenderer::sizeMapUnits( const QgsFeature &feature, const QgsRenderContext &c ) const
{
  QSizeF stackedSize( 0, 0 );
  int enabledDiagramCount = 0;  // We'll add spacing only for enabled subDiagrams

  // Iterate renderers. For each renderer, get the diagram
  // size for the feature and add it to the total size
  // accounting for stacked diagram defined spacing
  for ( const QgsDiagramRenderer *subRenderer : std::as_const( mDiagramRenderers ) )
  {
    QSizeF size = subRenderer->sizeMapUnits( feature, c );

    if ( size.isValid() )
    {
      enabledDiagramCount++;
      switch ( mSettings.stackedDiagramMode )
      {
        case QgsDiagramSettings::Horizontal:
          stackedSize.setWidth( stackedSize.width() + size.width() );
          stackedSize.setHeight( std::max( stackedSize.height(), size.height() ) );
          break;

        case QgsDiagramSettings::Vertical:
          stackedSize.setWidth( std::max( stackedSize.width(), size.width() ) );
          stackedSize.setHeight( stackedSize.height() + size.height() );
          break;
      }
    }
  }

  if ( stackedSize.isValid() )
  {
    const double spacing = c.convertToMapUnits( mSettings.stackedDiagramSpacing(), mSettings.stackedDiagramSpacingUnit(), mSettings.stackedDiagramSpacingMapUnitScale() );

    switch ( mSettings.stackedDiagramMode )
    {
      case QgsDiagramSettings::Horizontal:
        stackedSize.scale( stackedSize.width() + spacing * ( enabledDiagramCount - 1 ), stackedSize.height(), Qt::IgnoreAspectRatio );
        break;

      case QgsDiagramSettings::Vertical:
        stackedSize.scale( stackedSize.width(), stackedSize.height() + spacing * ( enabledDiagramCount - 1 ), Qt::IgnoreAspectRatio );
        break;
    }
  }
  return stackedSize;
}

void QgsStackedDiagramRenderer::renderDiagram( const QgsFeature &feature, QgsRenderContext &c, QPointF pos, const QgsPropertyCollection &properties ) const
{
  if ( !mDiagram )
  {
    return;
  }

  QPointF newPos = pos; // Each subdiagram will have its own newPos

  // Get subrenderers sorted by mode (vertical diagrams are returned backwards)
  const QList< QgsDiagramRenderer * > stackedRenderers = renderers( true );

  for ( const QgsDiagramRenderer *stackedRenderer : stackedRenderers )
  {
    if ( stackedRenderer->rendererName() == QgsStackedDiagramRenderer::DIAGRAM_RENDERER_NAME_STACKED )
    {
      // Nested stacked diagrams will use this recursion
      stackedRenderer->renderDiagram( feature, c, newPos, properties );
      continue;
    }

    QgsDiagramSettings s;
    if ( !stackedRenderer->diagramSettings( feature, c, s ) )
    {
      continue;
    }

    if ( !s.enabled )
    {
      continue;
    }

    if ( s.scaleBasedVisibility )
    {
      // Note: scale might be a non-round number, so compare with qgsDoubleNear
      const double rendererScale = c.rendererScale();

      // maxScale is inclusive ( < --> no diagram )
      double maxScale = s.maximumScale;
      if ( maxScale > 0 && QgsScaleUtils::lessThanMaximumScale( rendererScale, maxScale ) )
      {
        continue;
      }

      // minScale is exclusive ( >= --> no diagram)
      double minScale = s.minimumScale;
      if ( minScale > 0 && QgsScaleUtils::equalToOrGreaterThanMinimumScale( rendererScale, minScale ) )
      {
        continue;
      }
    }

    if ( properties.hasActiveProperties() )
    {
      c.expressionContext().setOriginalValueVariable( QgsColorUtils::colorToString( s.backgroundColor ) );
      s.backgroundColor = properties.valueAsColor( QgsDiagramLayerSettings::Property::BackgroundColor, c.expressionContext(), s.backgroundColor );
      c.expressionContext().setOriginalValueVariable( QgsColorUtils::colorToString( s.penColor ) );
      s.penColor = properties.valueAsColor( QgsDiagramLayerSettings::Property::StrokeColor, c.expressionContext(), s.penColor );
      c.expressionContext().setOriginalValueVariable( s.penWidth );
      s.penWidth = properties.valueAsDouble( QgsDiagramLayerSettings::Property::StrokeWidth, c.expressionContext(), s.penWidth );
      c.expressionContext().setOriginalValueVariable( s.rotationOffset );
      s.rotationOffset = properties.valueAsDouble( QgsDiagramLayerSettings::Property::StartAngle, c.expressionContext(), s.rotationOffset );
    }

    QgsPaintEffect *effect = s.paintEffect();
    std::unique_ptr< QgsEffectPainter > effectPainter;
    if ( effect && effect->enabled() )
    {
      effectPainter = std::make_unique< QgsEffectPainter >( c, effect );
    }

    stackedRenderer->diagram()->renderDiagram( feature, c, s, newPos );
    QgsStackedDiagram *stackedDiagram = dynamic_cast< QgsStackedDiagram *>( mDiagram.get() );
    stackedDiagram->subDiagramPosition( newPos, c, mSettings, s );
  }
}

bool QgsStackedDiagramRenderer::diagramSettings( const QgsFeature &feature, const QgsRenderContext &c, QgsDiagramSettings &s ) const
{
  Q_UNUSED( feature )
  Q_UNUSED( c )
  Q_UNUSED( s )
  return false;
}

QSizeF QgsStackedDiagramRenderer::diagramSize( const QgsFeature &feature, const QgsRenderContext &c ) const
{
  Q_UNUSED( feature )
  Q_UNUSED( c )
  return QSizeF( 0, 0 );
}

QList<QgsDiagramSettings> QgsStackedDiagramRenderer::diagramSettings() const
{
  QList<QgsDiagramSettings> settingsList;
  settingsList.push_back( mSettings );
  return settingsList;
}

QList<QString> QgsStackedDiagramRenderer::diagramAttributes() const
{
  return mSettings.categoryAttributes;
}

QList< QgsLayerTreeModelLegendNode * > QgsStackedDiagramRenderer::legendItems( QgsLayerTreeLayer *nodeLayer ) const
{
  QList< QgsLayerTreeModelLegendNode * > nodes;
  for ( const QgsDiagramRenderer *renderer : std::as_const( mDiagramRenderers ) )
  {
    nodes << renderer->legendItems( nodeLayer );
  }

  return nodes;
}

QList< QgsDiagramRenderer * > QgsStackedDiagramRenderer::renderers( bool sortByDiagramMode ) const
{
  QList< QgsDiagramRenderer * > renderers = mDiagramRenderers;

  if ( sortByDiagramMode && mSettings.stackedDiagramMode == QgsDiagramSettings::Vertical )
  {
    // We draw vertical diagrams backwards, so
    // we return the subrenderers in reverse order
    std::reverse( renderers.begin(), renderers.end() );
  }
  return renderers;
}

void QgsStackedDiagramRenderer::addRenderer( QgsDiagramRenderer *renderer )
{
  if ( renderer )
  {
    mDiagramRenderers.append( renderer );
  }
}

const QgsDiagramRenderer *QgsStackedDiagramRenderer::renderer( const int index ) const
{
  return mDiagramRenderers.value( index );
}

int QgsStackedDiagramRenderer::rendererCount() const
{
  return mDiagramRenderers.size();
}

void QgsStackedDiagramRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  const QDomElement categoryElem = elem.firstChildElement( u"DiagramCategory"_s );
  if ( categoryElem.isNull() )
  {
    return;
  }

  mSettings.readXml( categoryElem, context );
  _readXml( elem, context );
  _readXmlSubRenderers( elem, context );
}

void QgsStackedDiagramRenderer::_readXmlSubRenderers( const QDomElement &elem, const QgsReadWriteContext &context )
{
  qDeleteAll( mDiagramRenderers );
  mDiagramRenderers.clear();

  const QDomElement subRenderersElem = elem.firstChildElement( u"DiagramRenderers"_s );

  if ( !subRenderersElem.isNull() )
  {
    const QDomNodeList childRendererList = subRenderersElem.childNodes();

    for ( int i = 0; i < childRendererList.size(); i++ )
    {
      const QDomElement subRendererElem = childRendererList.at( i ).toElement();

      if ( subRendererElem.nodeName() == "SingleCategoryDiagramRenderer"_L1 )
      {
        auto singleCatDiagramRenderer = std::make_unique< QgsSingleCategoryDiagramRenderer >();
        singleCatDiagramRenderer->readXml( subRendererElem, context );
        addRenderer( singleCatDiagramRenderer.release() );
      }
      else if ( subRendererElem.nodeName() == "LinearlyInterpolatedDiagramRenderer"_L1 )
      {
        auto linearDiagramRenderer = std::make_unique< QgsLinearlyInterpolatedDiagramRenderer >();
        linearDiagramRenderer->readXml( subRendererElem, context );
        addRenderer( linearDiagramRenderer.release() );
      }
      else if ( subRendererElem.nodeName() == "StackedDiagramRenderer"_L1 )
      {
        auto stackedDiagramRenderer = std::make_unique< QgsStackedDiagramRenderer >();
        stackedDiagramRenderer->readXml( subRendererElem, context );
        addRenderer( stackedDiagramRenderer.release() );
      }
    }
  }
}

void QgsStackedDiagramRenderer::writeXml( QDomElement &layerElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( u"StackedDiagramRenderer"_s );
  mSettings.writeXml( rendererElem, doc, context );
  _writeXml( rendererElem, doc, context );
  _writeXmlSubRenderers( rendererElem, doc, context );
  layerElem.appendChild( rendererElem );
}

void QgsStackedDiagramRenderer::_writeXmlSubRenderers( QDomElement &rendererElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement renderersElem = doc.createElement( u"DiagramRenderers"_s );

  // Iterate sub renderers and write their settings to a DOM object
  for ( int i = 0; i < mDiagramRenderers.count(); i++ )
  {
    mDiagramRenderers.at( i )->writeXml( renderersElem, doc, context );
  }
  rendererElem.appendChild( renderersElem );
}

QList< QgsLayerTreeModelLegendNode * > QgsDiagramSettings::legendItems( QgsLayerTreeLayer *nodeLayer ) const
{
  QList< QgsLayerTreeModelLegendNode * > list;
  list.reserve( categoryLabels.size() );
  for ( int i = 0; i < categoryLabels.size(); ++i )
  {
    QPixmap pix( 16, 16 );
    pix.fill( categoryColors[i] );
    list << new QgsSimpleLegendNode( nodeLayer, categoryLabels[i], QIcon( pix ), nullptr, u"diagram_%1"_s.arg( QString::number( i ) ) );
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
  , stackedDiagramMode( other.stackedDiagramMode )
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
  , mStackedDiagramSpacing( other.mStackedDiagramSpacing )
  , mStackedDiagramSpacingUnit( other.mStackedDiagramSpacingUnit )
  , mStackedDiagramSpacingMapUnitScale( other.mStackedDiagramSpacingMapUnitScale )
  , mDirection( other.mDirection )
  , mShowAxis( other.mShowAxis )
  , mAxisLineSymbol( other.mAxisLineSymbol ? other.mAxisLineSymbol->clone() : nullptr )
  , mPaintEffect( other.mPaintEffect ? other.mPaintEffect->clone() : nullptr )
{

}

QgsDiagramSettings &QgsDiagramSettings::operator=( const QgsDiagramSettings &other )
{
  if ( &other == this )
    return *this;

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
  stackedDiagramMode = other.stackedDiagramMode;
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
  mStackedDiagramSpacing = other.mStackedDiagramSpacing;
  mStackedDiagramSpacingUnit = other.mStackedDiagramSpacingUnit;
  mStackedDiagramSpacingMapUnitScale = other.mStackedDiagramSpacingMapUnitScale;
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
  if ( mShowAttributeLegend && mSettings.enabled )
    nodes = mSettings.legendItems( nodeLayer );

  return nodes;
}

QList< QgsLayerTreeModelLegendNode * > QgsLinearlyInterpolatedDiagramRenderer::legendItems( QgsLayerTreeLayer *nodeLayer ) const
{
  QList< QgsLayerTreeModelLegendNode * > nodes;
  if ( !mSettings.enabled )
  {
    return nodes;
  }

  if ( mShowAttributeLegend )
    nodes = mSettings.legendItems( nodeLayer );

  if ( mDataDefinedSizeLegend && mDiagram )
  {
    // add size legend
    QgsMarkerSymbol *legendSymbol = mDataDefinedSizeLegend->symbol() ? mDataDefinedSizeLegend->symbol()->clone() : QgsMarkerSymbol::createSimple( QVariantMap() ).release();
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
  mDataDefinedSizeLegend.reset( settings );

}

QgsDataDefinedSizeLegend *QgsLinearlyInterpolatedDiagramRenderer::dataDefinedSizeLegend() const
{
  return mDataDefinedSizeLegend.get();
}
