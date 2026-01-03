/***************************************************************************
    qgsvectorlayerlabeling.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsvectorlayerlabeling.h"

#include "qgis.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgspallabeling.h"
#include "qgsrulebasedlabeling.h"
#include "qgssldexportcontext.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayer.h"

QgsAbstractVectorLayerLabeling *QgsAbstractVectorLayerLabeling::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString type = element.attribute( u"type"_s );
  if ( type == "rule-based"_L1 )
  {
    return QgsRuleBasedLabeling::create( element, context );
  }
  else if ( type == "simple"_L1 )
  {
    return QgsVectorLayerSimpleLabeling::create( element, context );
  }
  else
  {
    return nullptr;
  }
}

void QgsAbstractVectorLayerLabeling::toSld( QDomNode &parent, const QVariantMap &props ) const
{
  QgsSldExportContext context;
  context.setExtraProperties( props );
  toSld( parent, context );
}

bool QgsAbstractVectorLayerLabeling::toSld( QDomNode &, QgsSldExportContext &context ) const
{
  context.pushError( QObject::tr( "%1 labeling cannot be exported to SLD" ).arg( type() ) );
  return false;
}

bool QgsAbstractVectorLayerLabeling::accept( QgsStyleEntityVisitorInterface * ) const
{
  return true;
}

QgsPalLayerSettings QgsAbstractVectorLayerLabeling::defaultSettingsForLayer( const QgsVectorLayer *layer )
{
  QgsPalLayerSettings settings;
  settings.fieldName = layer->displayField();
  settings.setFormat( QgsStyle::defaultTextFormatForProject( layer->project() ) );

  switch ( layer->geometryType() )
  {
    case Qgis::GeometryType::Point:
      settings.placement = Qgis::LabelPlacement::OrderedPositionsAroundPoint;
      settings.offsetType = Qgis::LabelOffsetType::FromSymbolBounds;
      break;
    case Qgis::GeometryType::Line:
      settings.placement = Qgis::LabelPlacement::Line;
      break;
    case Qgis::GeometryType::Polygon:
      settings.placement = Qgis::LabelPlacement::AroundPoint;
      break;

    case Qgis::GeometryType::Unknown:
    case Qgis::GeometryType::Null:
      break;
  }
  return settings;
}

QgsVectorLayerLabelProvider *QgsVectorLayerSimpleLabeling::provider( QgsVectorLayer *layer ) const
{
  return new QgsVectorLayerLabelProvider( layer, QString(), false, mSettings.get() );
}

QgsVectorLayerSimpleLabeling::QgsVectorLayerSimpleLabeling( const QgsPalLayerSettings &settings )
  : mSettings( new QgsPalLayerSettings( settings ) )
{

}

QString QgsVectorLayerSimpleLabeling::type() const
{
  return u"simple"_s;
}

QgsAbstractVectorLayerLabeling *QgsVectorLayerSimpleLabeling::clone() const
{
  return new QgsVectorLayerSimpleLabeling( *mSettings );
}

QDomElement QgsVectorLayerSimpleLabeling::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( u"labeling"_s );
  elem.setAttribute( u"type"_s, u"simple"_s );
  elem.appendChild( mSettings->writeXml( doc, context ) );
  return elem;
}

QgsPalLayerSettings QgsVectorLayerSimpleLabeling::settings( const QString &providerId ) const
{
  Q_UNUSED( providerId )
  return *mSettings;
}

bool QgsVectorLayerSimpleLabeling::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mSettings )
  {
    QgsStyleLabelSettingsEntity entity( *mSettings );
    if ( !visitor->visit( &entity ) )
      return false;
  }
  return true;
}

bool QgsVectorLayerSimpleLabeling::requiresAdvancedEffects() const
{
  return mSettings->containsAdvancedEffects();
}

bool QgsVectorLayerSimpleLabeling::hasNonDefaultCompositionMode() const
{
  return mSettings->dataDefinedProperties().isActive( QgsPalLayerSettings::Property::FontBlendMode )
         || mSettings->dataDefinedProperties().isActive( QgsPalLayerSettings::Property::ShapeBlendMode )
         || mSettings->dataDefinedProperties().isActive( QgsPalLayerSettings::Property::BufferBlendMode )
         || mSettings->dataDefinedProperties().isActive( QgsPalLayerSettings::Property::ShadowBlendMode )
         || mSettings->format().hasNonDefaultCompositionMode();
}

QgsVectorLayerSimpleLabeling *QgsVectorLayerSimpleLabeling::create( const QDomElement &element, const QgsReadWriteContext &context ) // cppcheck-suppress duplInheritedMember
{
  const QDomElement settingsElem = element.firstChildElement( u"settings"_s );
  if ( !settingsElem.isNull() )
  {
    QgsPalLayerSettings settings;
    settings.readXml( settingsElem, context );
    return new QgsVectorLayerSimpleLabeling( settings );
  }

  return new QgsVectorLayerSimpleLabeling( QgsPalLayerSettings() );
}

QPointF quadOffsetToSldAnchor( Qgis::LabelQuadrantPosition quadrantPosition )
{
  double quadOffsetX = 0.5, quadOffsetY = 0.5;

  // adjust quadrant offset of labels
  switch ( quadrantPosition )
  {
    case Qgis::LabelQuadrantPosition::AboveLeft:
      quadOffsetX = 1;
      quadOffsetY = 0;
      break;
    case Qgis::LabelQuadrantPosition::Above:
      quadOffsetX = 0.5;
      quadOffsetY = 0;
      break;
    case Qgis::LabelQuadrantPosition::AboveRight:
      quadOffsetX = 0;
      quadOffsetY = 0;
      break;
    case Qgis::LabelQuadrantPosition::Left:
      quadOffsetX = 1;
      quadOffsetY = 0.5;
      break;
    case Qgis::LabelQuadrantPosition::Right:
      quadOffsetX = 0;
      quadOffsetY = 0.5;
      break;
    case Qgis::LabelQuadrantPosition::BelowLeft:
      quadOffsetX = 1;
      quadOffsetY = 1;
      break;
    case Qgis::LabelQuadrantPosition::Below:
      quadOffsetX = 0.5;
      quadOffsetY = 1;
      break;
    case Qgis::LabelQuadrantPosition::BelowRight:
      quadOffsetX = 0;
      quadOffsetY = 1.0;
      break;
    case Qgis::LabelQuadrantPosition::Over:
      break;
  }

  return QPointF( quadOffsetX, quadOffsetY );
}

/*
 * This is not a generic function encoder, just enough to encode the label case control functions
 */
void appendSimpleFunction( QDomDocument &doc, QDomElement &parent, const QString &name, const QString &attribute )
{
  QDomElement function = doc.createElement( u"ogc:Function"_s );
  function.setAttribute( u"name"_s, name );
  parent.appendChild( function );
  QDomElement property = doc.createElement( u"ogc:PropertyName"_s );
  property.appendChild( doc.createTextNode( attribute ) );
  function.appendChild( property );
}

std::unique_ptr<QgsMarkerSymbolLayer> backgroundToMarkerLayer( const QgsTextBackgroundSettings &settings )
{
  std::unique_ptr<QgsMarkerSymbolLayer> layer;
  switch ( settings.type() )
  {
    case QgsTextBackgroundSettings::ShapeSVG:
    {
      QgsSvgMarkerSymbolLayer *svg = new QgsSvgMarkerSymbolLayer( settings.svgFile() );
      svg->setStrokeWidth( settings.strokeWidth() );
      svg->setStrokeWidthUnit( settings.strokeWidthUnit() );
      layer.reset( svg );
      break;
    }
    case QgsTextBackgroundSettings::ShapeMarkerSymbol:
    {
      // just grab the first layer and hope for the best
      if ( settings.markerSymbol() && settings.markerSymbol()->symbolLayerCount() > 0 )
      {
        layer.reset( static_cast< QgsMarkerSymbolLayer * >( settings.markerSymbol()->symbolLayer( 0 )->clone() ) );
        break;
      }
      [[fallthrough]]; // not set, just go with the default
    }
    case QgsTextBackgroundSettings::ShapeCircle:
    case QgsTextBackgroundSettings::ShapeEllipse:
    case QgsTextBackgroundSettings::ShapeRectangle:
    case QgsTextBackgroundSettings::ShapeSquare:
    {
      QgsSimpleMarkerSymbolLayer *marker = new QgsSimpleMarkerSymbolLayer();
      // default value
      Qgis::MarkerShape shape = Qgis::MarkerShape::Diamond;
      switch ( settings.type() )
      {
        case QgsTextBackgroundSettings::ShapeCircle:
        case QgsTextBackgroundSettings::ShapeEllipse:
          shape = Qgis::MarkerShape::Circle;
          break;
        case QgsTextBackgroundSettings::ShapeRectangle:
        case QgsTextBackgroundSettings::ShapeSquare:
          shape = Qgis::MarkerShape::Square;
          break;
        case QgsTextBackgroundSettings::ShapeSVG:
        case QgsTextBackgroundSettings::ShapeMarkerSymbol:
          break;
      }

      marker->setShape( shape );
      marker->setStrokeWidth( settings.strokeWidth() );
      marker->setStrokeWidthUnit( settings.strokeWidthUnit() );
      layer.reset( marker );
    }
  }
  layer->setEnabled( true );
  // a marker does not have a size x and y, just a size (and it should be at least one)
  const QSizeF size = settings.size();
  layer->setSize( std::max( 1., std::max( size.width(), size.height() ) ) );
  layer->setSizeUnit( settings.sizeUnit() );
  // fill and stroke
  QColor fillColor = settings.fillColor();
  QColor strokeColor = settings.strokeColor();
  if ( settings.opacity() < 1 )
  {
    const int alpha = std::round( settings.opacity() * 255 );
    fillColor.setAlpha( alpha );
    strokeColor.setAlpha( alpha );
  }
  layer->setFillColor( fillColor );
  layer->setStrokeColor( strokeColor );
  // rotation
  if ( settings.rotationType() == QgsTextBackgroundSettings::RotationFixed )
  {
    layer->setAngle( settings.rotation() );
  }
  // offset
  layer->setOffset( settings.offset() );
  layer->setOffsetUnit( settings.offsetUnit() );

  return layer;
}

void QgsAbstractVectorLayerLabeling::writeTextSymbolizer( QDomNode &parent, QgsPalLayerSettings &settings, const QVariantMap &props ) const
{
  QgsSldExportContext context;
  context.setExtraProperties( props );
  writeTextSymbolizer( parent, settings, context );
}

bool QgsAbstractVectorLayerLabeling::writeTextSymbolizer( QDomNode &parent, QgsPalLayerSettings &settings, QgsSldExportContext &context ) const
{
  QDomDocument doc = parent.ownerDocument();

  // text symbolizer
  QDomElement textSymbolizerElement = doc.createElement( u"se:TextSymbolizer"_s );
  parent.appendChild( textSymbolizerElement );

  // label
  QgsTextFormat format = settings.format();
  const QFont font = format.font();
  QDomElement labelElement = doc.createElement( u"se:Label"_s );
  textSymbolizerElement.appendChild( labelElement );
  if ( settings.isExpression )
  {
    context.pushError( QObject::tr( "Labels containing expressions cannot be exported to SLD. Skipping label '%1'" ).arg( settings.getLabelExpression()->dump() ) );
    labelElement.appendChild( doc.createTextNode( "Placeholder" ) );
  }
  else
  {
    Qgis::Capitalization capitalization = format.capitalization();
    if ( capitalization == Qgis::Capitalization::MixedCase && font.capitalization() != QFont::MixedCase )
      capitalization = static_cast< Qgis::Capitalization >( font.capitalization() );
    if ( capitalization == Qgis::Capitalization::AllUppercase )
    {
      appendSimpleFunction( doc, labelElement, u"strToUpperCase"_s, settings.fieldName );
    }
    else if ( capitalization == Qgis::Capitalization::AllLowercase )
    {
      appendSimpleFunction( doc, labelElement, u"strToLowerCase"_s, settings.fieldName );
    }
    else if ( capitalization == Qgis::Capitalization::ForceFirstLetterToCapital )
    {
      appendSimpleFunction( doc, labelElement, u"strCapitalize"_s, settings.fieldName );
    }
    else
    {
      QDomElement propertyNameElement = doc.createElement( u"ogc:PropertyName"_s );
      propertyNameElement.appendChild( doc.createTextNode( settings.fieldName ) );
      labelElement.appendChild( propertyNameElement );
    }
  }

  // font
  QDomElement fontElement = doc.createElement( u"se:Font"_s );
  textSymbolizerElement.appendChild( fontElement );
  fontElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, u"font-family"_s, font.family() ) );
  const QVariantMap props = context.extraProperties();
  const double fontSize = QgsSymbolLayerUtils::rescaleUom( format.size(), format.sizeUnit(), props );
  fontElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, u"font-size"_s, QString::number( fontSize ) ) );
  if ( format.font().italic() )
  {
    fontElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, u"font-style"_s, u"italic"_s ) );
  }
  if ( format.font().bold() )
  {
    fontElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, u"font-weight"_s, u"bold"_s ) );
  }

  // label placement
  QDomElement labelPlacement = doc.createElement( u"se:LabelPlacement"_s );
  textSymbolizerElement.appendChild( labelPlacement );
  double maxDisplacement = 0;
  double repeatDistance = 0;
  switch ( settings.placement )
  {
    case Qgis::LabelPlacement::OverPoint:
    {
      QDomElement pointPlacement = doc.createElement( "se:PointPlacement" );
      labelPlacement.appendChild( pointPlacement );
      // anchor point
      const QPointF anchor = quadOffsetToSldAnchor( settings.pointSettings().quadrant() );
      QgsSymbolLayerUtils::createAnchorPointElement( doc, pointPlacement, anchor );
      // displacement
      if ( settings.xOffset != 0 || settings.yOffset != 0 )
      {
        const Qgis::RenderUnit offsetUnit =  settings.offsetUnits;
        const double dx = QgsSymbolLayerUtils::rescaleUom( settings.xOffset, offsetUnit, props );
        const double dy = QgsSymbolLayerUtils::rescaleUom( settings.yOffset, offsetUnit, props );
        QgsSymbolLayerUtils::createDisplacementElement( doc, pointPlacement, QPointF( dx, dy ) );
      }
      // rotation
      if ( settings.angleOffset != 0 )
      {
        QDomElement rotation = doc.createElement( "se:Rotation" );
        pointPlacement.appendChild( rotation );
        rotation.appendChild( doc.createTextNode( QString::number( settings.angleOffset ) ) );
      }
    }
    break;
    case Qgis::LabelPlacement::AroundPoint:
    case Qgis::LabelPlacement::OrderedPositionsAroundPoint:
    {
      QDomElement pointPlacement = doc.createElement( "se:PointPlacement" );
      labelPlacement.appendChild( pointPlacement );

      // SLD cannot do either, but let's do a best effort setting the distance using
      // anchor point and displacement
      QgsSymbolLayerUtils::createAnchorPointElement( doc, pointPlacement, QPointF( 0, 0.5 ) );
      const Qgis::RenderUnit distUnit = settings.distUnits;
      const double radius = QgsSymbolLayerUtils::rescaleUom( settings.dist, distUnit, props );
      const double offset = std::sqrt( radius * radius / 2 ); // make it start top/right
      maxDisplacement = radius + 1; // lock the distance
      QgsSymbolLayerUtils::createDisplacementElement( doc, pointPlacement, QPointF( offset, offset ) );
    }
    break;
    case Qgis::LabelPlacement::Horizontal:
    case Qgis::LabelPlacement::Free:
    case Qgis::LabelPlacement::OutsidePolygons:
    {
      // still a point placement (for "free" it's a fallback, there is no SLD equivalent)
      QDomElement pointPlacement = doc.createElement( "se:PointPlacement" );
      labelPlacement.appendChild( pointPlacement );
      QgsSymbolLayerUtils::createAnchorPointElement( doc, pointPlacement, QPointF( 0.5, 0.5 ) );
      const Qgis::RenderUnit distUnit = settings.distUnits;
      const double dist = QgsSymbolLayerUtils::rescaleUom( settings.dist, distUnit, props );
      QgsSymbolLayerUtils::createDisplacementElement( doc, pointPlacement, QPointF( 0, dist ) );
      break;
    }
    case Qgis::LabelPlacement::Line:
    case Qgis::LabelPlacement::Curved:
    case Qgis::LabelPlacement::PerimeterCurved:
    {
      QDomElement linePlacement = doc.createElement( "se:LinePlacement" );
      labelPlacement.appendChild( linePlacement );

      // perpendicular distance if required
      if ( settings.dist > 0 )
      {
        const Qgis::RenderUnit distUnit = settings.distUnits;
        const double dist = QgsSymbolLayerUtils::rescaleUom( settings.dist, distUnit, props );
        QDomElement perpendicular = doc.createElement( "se:PerpendicularOffset" );
        linePlacement.appendChild( perpendicular );
        perpendicular.appendChild( doc.createTextNode( qgsDoubleToString( dist, 2 ) ) );
      }

      // repeat distance if required
      if ( settings.repeatDistance > 0 )
      {
        QDomElement repeat = doc.createElement( "se:Repeat" );
        linePlacement.appendChild( repeat );
        repeat.appendChild( doc.createTextNode( u"true"_s ) );
        QDomElement gap = doc.createElement( "se:Gap" );
        linePlacement.appendChild( gap );
        repeatDistance = QgsSymbolLayerUtils::rescaleUom( settings.repeatDistance, settings.repeatDistanceUnit, props );
        gap.appendChild( doc.createTextNode( qgsDoubleToString( repeatDistance, 2 ) ) );
      }

      // always generalized
      QDomElement generalize = doc.createElement( "se:GeneralizeLine" );
      linePlacement.appendChild( generalize );
      generalize.appendChild( doc.createTextNode( u"true"_s ) );
    }
    break;
  }

  // halo
  const QgsTextBufferSettings buffer = format.buffer();
  if ( buffer.enabled() )
  {
    QDomElement haloElement = doc.createElement( u"se:Halo"_s );
    textSymbolizerElement.appendChild( haloElement );

    QDomElement radiusElement = doc.createElement( u"se:Radius"_s );
    haloElement.appendChild( radiusElement );
    // the SLD uses a radius, which is actually half of the link thickness the buffer size specifies
    const double radius = QgsSymbolLayerUtils::rescaleUom( buffer.size(), buffer.sizeUnit(), props ) / 2;
    radiusElement.appendChild( doc.createTextNode( qgsDoubleToString( radius ) ) );

    QDomElement fillElement = doc.createElement( u"se:Fill"_s );
    haloElement.appendChild( fillElement );
    fillElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, u"fill"_s, buffer.color().name() ) );
    if ( buffer.opacity() != 1 )
    {
      fillElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, u"fill-opacity"_s, QString::number( buffer.opacity() ) ) );
    }
  }

  // fill
  QDomElement fillElement = doc.createElement( u"se:Fill"_s );
  textSymbolizerElement.appendChild( fillElement );
  fillElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, u"fill"_s, format.color().name() ) );
  if ( format.opacity() != 1 )
  {
    fillElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, u"fill-opacity"_s, QString::number( format.opacity() ) ) );
  }

  // background graphic (not supported by SE 1.1, but supported by the GeoTools ecosystem as an extension)
  const QgsTextBackgroundSettings background = format.background();
  if ( background.enabled() )
  {
    std::unique_ptr<QgsMarkerSymbolLayer> layer = backgroundToMarkerLayer( background );
    layer->writeSldMarker( doc, textSymbolizerElement, context );
  }

  // priority and zIndex, the default values are 0 and 5 in qgis (and between 0 and 10),
  // in the GeoTools ecosystem there is a single priority value set at 1000 by default
  if ( settings.priority != 5 || settings.zIndex > 0 )
  {
    QDomElement priorityElement = doc.createElement( u"se:Priority"_s );
    textSymbolizerElement.appendChild( priorityElement );
    int priority = 500 + 1000 * settings.zIndex + ( settings.priority - 5 ) * 100;
    if ( settings.priority == 0 && settings.zIndex > 0 )
    {
      // small adjustment to make sure labels in z index n+1 are all above level n despite the priority value
      priority += 1;
    }
    priorityElement.appendChild( doc.createTextNode( QString::number( priority ) ) );
  }

  // vendor options for text appearance
  if ( font.underline() )
  {
    const QDomElement vo = QgsSymbolLayerUtils::createVendorOptionElement( doc, u"underlineText"_s, u"true"_s );
    textSymbolizerElement.appendChild( vo );
  }
  if ( font.strikeOut() )
  {
    const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, u"strikethroughText"_s, u"true"_s );
    textSymbolizerElement.appendChild( vo );
  }
  // vendor options for text positioning
  if ( maxDisplacement > 0 )
  {
    const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, u"maxDisplacement"_s, qgsDoubleToString( maxDisplacement, 2 ) );
    textSymbolizerElement.appendChild( vo );
  }

  switch ( settings.placement )
  {
    case Qgis::LabelPlacement::Curved:
    case Qgis::LabelPlacement::PerimeterCurved:
    {
      const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, u"followLine"_s, u"true"_s );
      textSymbolizerElement.appendChild( vo );
      if ( settings.maxCurvedCharAngleIn > 0 || settings.maxCurvedCharAngleOut > 0 )
      {
        // SLD has no notion for this, the GeoTools ecosystem can only do a single angle
        const double angle = std::min( std::fabs( settings.maxCurvedCharAngleIn ), std::fabs( settings.maxCurvedCharAngleOut ) );
        const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, u"maxAngleDelta"_s, qgsDoubleToString( angle ) );
        textSymbolizerElement.appendChild( vo );
      }
      break;
    }

    case Qgis::LabelPlacement::AroundPoint:
    case Qgis::LabelPlacement::OverPoint:
    case Qgis::LabelPlacement::Line:
    case Qgis::LabelPlacement::Horizontal:
    case Qgis::LabelPlacement::Free:
    case Qgis::LabelPlacement::OrderedPositionsAroundPoint:
    case Qgis::LabelPlacement::OutsidePolygons:
      break;
  }

  if ( repeatDistance > 0 )
  {
    const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, u"repeat"_s, qgsDoubleToString( repeatDistance, 2 ) );
    textSymbolizerElement.appendChild( vo );
  }
  // miscellaneous options
  switch ( settings.placementSettings().overlapHandling() )
  {
    case Qgis::LabelOverlapHandling::PreventOverlap:
      break;
    case Qgis::LabelOverlapHandling::AllowOverlapIfRequired:
    case Qgis::LabelOverlapHandling::AllowOverlapAtNoCost:
      const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, u"conflictResolution"_s, u"false"_s );
      textSymbolizerElement.appendChild( vo );
      break;
  }
  if ( settings.upsidedownLabels == Qgis::UpsideDownLabelHandling::AlwaysAllowUpsideDown )
  {
    const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, u"forceLeftToRight"_s, u"false"_s );
    textSymbolizerElement.appendChild( vo );
  }
  if ( settings.lineSettings().mergeLines() )
  {
    const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, u"group"_s, u"yes"_s );
    textSymbolizerElement.appendChild( vo );
    if ( settings.labelPerPart )
    {
      const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, u"labelAllGroup"_s, u"true"_s );
      textSymbolizerElement.appendChild( vo );
    }
  }
  // background symbol resize handling
  if ( background.enabled() )
  {
    // enable resizing if needed
    switch ( background.sizeType() )
    {
      case QgsTextBackgroundSettings::SizeBuffer:
      {
        QString resizeType;
        if ( background.type() == QgsTextBackgroundSettings::ShapeRectangle || background.type() == QgsTextBackgroundSettings::ShapeEllipse )
        {
          resizeType = u"stretch"_s;
        }
        else
        {
          resizeType = u"proportional"_s;
        }
        const QDomElement voResize =  QgsSymbolLayerUtils::createVendorOptionElement( doc, u"graphic-resize"_s, resizeType );
        textSymbolizerElement.appendChild( voResize );

        // now hadle margin
        const QSizeF size = background.size();
        if ( size.width() > 0 || size.height() > 0 )
        {
          double x = QgsSymbolLayerUtils::rescaleUom( size.width(), background.sizeUnit(), props );
          double y = QgsSymbolLayerUtils::rescaleUom( size.height(), background.sizeUnit(), props );
          // in case of ellipse qgis pads the size generously to make sure the text is inside the ellipse
          // the following seems to do the trick and keep visual output similar
          if ( background.type() == QgsTextBackgroundSettings::ShapeEllipse )
          {
            x += fontSize / 2;
            y += fontSize;
          }
          const QString resizeSpec = QString( "%1 %2" ).arg( qgsDoubleToString( x, 2 ), qgsDoubleToString( y, 2 ) );
          const QDomElement voMargin =  QgsSymbolLayerUtils::createVendorOptionElement( doc, u"graphic-margin"_s, resizeSpec );
          textSymbolizerElement.appendChild( voMargin );
        }
        break;
      }
      case QgsTextBackgroundSettings::SizeFixed:
      case QgsTextBackgroundSettings::SizePercent:
        // nothing to do here
        break;
    }
  }
  return true;
}

void QgsVectorLayerSimpleLabeling::toSld( QDomNode &parent, const QVariantMap &props ) const
{
  QgsSldExportContext context;
  context.setExtraProperties( props );
  toSld( parent, context );
}

bool QgsVectorLayerSimpleLabeling::toSld( QDomNode &parent, QgsSldExportContext &context ) const
{
  if ( mSettings->drawLabels )
  {
    QDomDocument doc = parent.ownerDocument();

    QDomElement ruleElement = doc.createElement( u"se:Rule"_s );
    parent.appendChild( ruleElement );

    // scale dependencies
    if ( mSettings->scaleVisibility )
    {
      QVariantMap scaleProps = QVariantMap();
      // tricky here, the max scale is expressed as its denominator, but it's still the max scale
      // in other words, the smallest scale denominator....
      scaleProps.insert( "scaleMinDenom", qgsDoubleToString( mSettings->maximumScale ) );
      scaleProps.insert( "scaleMaxDenom", qgsDoubleToString( mSettings->minimumScale ) );
      QgsSymbolLayerUtils::applyScaleDependency( doc, ruleElement, scaleProps );
    }

    writeTextSymbolizer( ruleElement, *mSettings, context );
  }
  return true;
}

void QgsVectorLayerSimpleLabeling::multiplyOpacity( double opacityFactor )
{
  QgsTextFormat format { mSettings->format() };
  format.multiplyOpacity( opacityFactor );
  mSettings->setFormat( format );
}

void QgsVectorLayerSimpleLabeling::setSettings( QgsPalLayerSettings *settings, const QString &providerId )
{
  Q_UNUSED( providerId )

  if ( mSettings.get() == settings )
    return;

  mSettings.reset( settings );
}
