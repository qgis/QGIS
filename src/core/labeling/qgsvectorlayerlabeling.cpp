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

#include "qgspallabeling.h"
#include "qgsrulebasedlabeling.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerutils.h"
#include "qgssymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgis.h"
#include "qgsstyleentityvisitor.h"
#include "qgsmarkersymbol.h"

QgsAbstractVectorLayerLabeling *QgsAbstractVectorLayerLabeling::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString type = element.attribute( QStringLiteral( "type" ) );
  if ( type == QLatin1String( "rule-based" ) )
  {
    return QgsRuleBasedLabeling::create( element, context );
  }
  else if ( type == QLatin1String( "simple" ) )
  {
    return QgsVectorLayerSimpleLabeling::create( element, context );
  }
  else
  {
    return nullptr;
  }
}

bool QgsAbstractVectorLayerLabeling::accept( QgsStyleEntityVisitorInterface * ) const
{
  return true;
}

QgsPalLayerSettings QgsAbstractVectorLayerLabeling::defaultSettingsForLayer( const QgsVectorLayer *layer )
{
  QgsPalLayerSettings settings;
  settings.fieldName = layer->displayField();

  switch ( layer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
      settings.placement = QgsPalLayerSettings::OrderedPositionsAroundPoint;
      settings.offsetType = QgsPalLayerSettings::FromSymbolBounds;
      break;
    case QgsWkbTypes::LineGeometry:
      settings.placement = QgsPalLayerSettings::Line;
      break;
    case QgsWkbTypes::PolygonGeometry:
      settings.placement = QgsPalLayerSettings::AroundPoint;
      break;

    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
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
  return QStringLiteral( "simple" );
}

QgsAbstractVectorLayerLabeling *QgsVectorLayerSimpleLabeling::clone() const
{
  return new QgsVectorLayerSimpleLabeling( *mSettings );
}

QDomElement QgsVectorLayerSimpleLabeling::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "labeling" ) );
  elem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "simple" ) );
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

QgsVectorLayerSimpleLabeling *QgsVectorLayerSimpleLabeling::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement settingsElem = element.firstChildElement( QStringLiteral( "settings" ) );
  if ( !settingsElem.isNull() )
  {
    QgsPalLayerSettings settings;
    settings.readXml( settingsElem, context );
    return new QgsVectorLayerSimpleLabeling( settings );
  }

  return new QgsVectorLayerSimpleLabeling( QgsPalLayerSettings() );
}

QPointF quadOffsetToSldAnchor( QgsPalLayerSettings::QuadrantPosition quadrantPosition )
{
  double quadOffsetX = 0.5, quadOffsetY = 0.5;

  // adjust quadrant offset of labels
  switch ( quadrantPosition )
  {
    case QgsPalLayerSettings::QuadrantAboveLeft:
      quadOffsetX = 1;
      quadOffsetY = 0;
      break;
    case QgsPalLayerSettings::QuadrantAbove:
      quadOffsetX = 0.5;
      quadOffsetY = 0;
      break;
    case QgsPalLayerSettings::QuadrantAboveRight:
      quadOffsetX = 0;
      quadOffsetY = 0;
      break;
    case QgsPalLayerSettings::QuadrantLeft:
      quadOffsetX = 1;
      quadOffsetY = 0.5;
      break;
    case QgsPalLayerSettings::QuadrantRight:
      quadOffsetX = 0;
      quadOffsetY = 0.5;
      break;
    case QgsPalLayerSettings::QuadrantBelowLeft:
      quadOffsetX = 1;
      quadOffsetY = 1;
      break;
    case QgsPalLayerSettings::QuadrantBelow:
      quadOffsetX = 0.5;
      quadOffsetY = 1;
      break;
    case QgsPalLayerSettings::QuadrantBelowRight:
      quadOffsetX = 0;
      quadOffsetY = 1.0;
      break;
    case QgsPalLayerSettings::QuadrantOver:
      break;
  }

  return QPointF( quadOffsetX, quadOffsetY );
}

/*
 * This is not a generic function encoder, just enough to encode the label case control functions
 */
void appendSimpleFunction( QDomDocument &doc, QDomElement &parent, const QString &name, const QString &attribute )
{
  QDomElement function = doc.createElement( QStringLiteral( "ogc:Function" ) );
  function.setAttribute( QStringLiteral( "name" ), name );
  parent.appendChild( function );
  QDomElement property = doc.createElement( QStringLiteral( "ogc:PropertyName" ) );
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
      FALLTHROUGH // not set, just go with the default
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
  QDomDocument doc = parent.ownerDocument();

  // text symbolizer
  QDomElement textSymbolizerElement = doc.createElement( QStringLiteral( "se:TextSymbolizer" ) );
  parent.appendChild( textSymbolizerElement );

  // label
  QgsTextFormat format = settings.format();
  const QFont font = format.font();
  QDomElement labelElement = doc.createElement( QStringLiteral( "se:Label" ) );
  textSymbolizerElement.appendChild( labelElement );
  if ( settings.isExpression )
  {
    labelElement.appendChild( doc.createComment( QStringLiteral( "SE Export for %1 not implemented yet" ).arg( settings.getLabelExpression()->dump() ) ) );
    labelElement.appendChild( doc.createTextNode( "Placeholder" ) );
  }
  else
  {
    Qgis::Capitalization capitalization = format.capitalization();
    if ( capitalization == Qgis::Capitalization::MixedCase && font.capitalization() != QFont::MixedCase )
      capitalization = static_cast< Qgis::Capitalization >( font.capitalization() );
    if ( capitalization == Qgis::Capitalization::AllUppercase )
    {
      appendSimpleFunction( doc, labelElement, QStringLiteral( "strToUpperCase" ), settings.fieldName );
    }
    else if ( capitalization == Qgis::Capitalization::AllLowercase )
    {
      appendSimpleFunction( doc, labelElement, QStringLiteral( "strToLowerCase" ), settings.fieldName );
    }
    else if ( capitalization == Qgis::Capitalization::ForceFirstLetterToCapital )
    {
      appendSimpleFunction( doc, labelElement, QStringLiteral( "strCapitalize" ), settings.fieldName );
    }
    else
    {
      QDomElement propertyNameElement = doc.createElement( QStringLiteral( "ogc:PropertyName" ) );
      propertyNameElement.appendChild( doc.createTextNode( settings.fieldName ) );
      labelElement.appendChild( propertyNameElement );
    }
  }

  // font
  QDomElement fontElement = doc.createElement( QStringLiteral( "se:Font" ) );
  textSymbolizerElement.appendChild( fontElement );
  fontElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, QStringLiteral( "font-family" ), font.family() ) );
  const double fontSize = QgsSymbolLayerUtils::rescaleUom( format.size(), format.sizeUnit(), props );
  fontElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, QStringLiteral( "font-size" ), QString::number( fontSize ) ) );
  if ( format.font().italic() )
  {
    fontElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, QStringLiteral( "font-style" ), QStringLiteral( "italic" ) ) );
  }
  if ( format.font().bold() )
  {
    fontElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, QStringLiteral( "font-weight" ), QStringLiteral( "bold" ) ) );
  }

  // label placement
  QDomElement labelPlacement = doc.createElement( QStringLiteral( "se:LabelPlacement" ) );
  textSymbolizerElement.appendChild( labelPlacement );
  double maxDisplacement = 0;
  double repeatDistance = 0;
  switch ( settings.placement )
  {
    case QgsPalLayerSettings::OverPoint:
    {
      QDomElement pointPlacement = doc.createElement( "se:PointPlacement" );
      labelPlacement.appendChild( pointPlacement );
      // anchor point
      const QPointF anchor = quadOffsetToSldAnchor( settings.quadOffset );
      QgsSymbolLayerUtils::createAnchorPointElement( doc, pointPlacement, anchor );
      // displacement
      if ( settings.xOffset > 0 || settings.yOffset > 0 )
      {
        const QgsUnitTypes::RenderUnit offsetUnit =  settings.offsetUnits;
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
    case QgsPalLayerSettings::AroundPoint:
    case QgsPalLayerSettings::OrderedPositionsAroundPoint:
    {
      QDomElement pointPlacement = doc.createElement( "se:PointPlacement" );
      labelPlacement.appendChild( pointPlacement );

      // SLD cannot do either, but let's do a best effort setting the distance using
      // anchor point and displacement
      QgsSymbolLayerUtils::createAnchorPointElement( doc, pointPlacement, QPointF( 0, 0.5 ) );
      const QgsUnitTypes::RenderUnit distUnit = settings.distUnits;
      const double radius = QgsSymbolLayerUtils::rescaleUom( settings.dist, distUnit, props );
      const double offset = std::sqrt( radius * radius / 2 ); // make it start top/right
      maxDisplacement = radius + 1; // lock the distance
      QgsSymbolLayerUtils::createDisplacementElement( doc, pointPlacement, QPointF( offset, offset ) );
    }
    break;
    case QgsPalLayerSettings::Horizontal:
    case QgsPalLayerSettings::Free:
    case QgsPalLayerSettings::OutsidePolygons:
    {
      // still a point placement (for "free" it's a fallback, there is no SLD equivalent)
      QDomElement pointPlacement = doc.createElement( "se:PointPlacement" );
      labelPlacement.appendChild( pointPlacement );
      QgsSymbolLayerUtils::createAnchorPointElement( doc, pointPlacement, QPointF( 0.5, 0.5 ) );
      const QgsUnitTypes::RenderUnit distUnit = settings.distUnits;
      const double dist = QgsSymbolLayerUtils::rescaleUom( settings.dist, distUnit, props );
      QgsSymbolLayerUtils::createDisplacementElement( doc, pointPlacement, QPointF( 0, dist ) );
      break;
    }
    case QgsPalLayerSettings::Line:
    case QgsPalLayerSettings::Curved:
    case QgsPalLayerSettings::PerimeterCurved:
    {
      QDomElement linePlacement = doc.createElement( "se:LinePlacement" );
      labelPlacement.appendChild( linePlacement );

      // perpendicular distance if required
      if ( settings.dist > 0 )
      {
        const QgsUnitTypes::RenderUnit distUnit = settings.distUnits;
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
        repeat.appendChild( doc.createTextNode( QStringLiteral( "true" ) ) );
        QDomElement gap = doc.createElement( "se:Gap" );
        linePlacement.appendChild( gap );
        repeatDistance = QgsSymbolLayerUtils::rescaleUom( settings.repeatDistance, settings.repeatDistanceUnit, props );
        gap.appendChild( doc.createTextNode( qgsDoubleToString( repeatDistance, 2 ) ) );
      }

      // always generalized
      QDomElement generalize = doc.createElement( "se:GeneralizeLine" );
      linePlacement.appendChild( generalize );
      generalize.appendChild( doc.createTextNode( QStringLiteral( "true" ) ) );
    }
    break;
  }

  // halo
  const QgsTextBufferSettings buffer = format.buffer();
  if ( buffer.enabled() )
  {
    QDomElement haloElement = doc.createElement( QStringLiteral( "se:Halo" ) );
    textSymbolizerElement.appendChild( haloElement );

    QDomElement radiusElement = doc.createElement( QStringLiteral( "se:Radius" ) );
    haloElement.appendChild( radiusElement );
    // the SLD uses a radius, which is actually half of the link thickness the buffer size specifies
    const double radius = QgsSymbolLayerUtils::rescaleUom( buffer.size(), buffer.sizeUnit(), props ) / 2;
    radiusElement.appendChild( doc.createTextNode( qgsDoubleToString( radius ) ) );

    QDomElement fillElement = doc.createElement( QStringLiteral( "se:Fill" ) );
    haloElement.appendChild( fillElement );
    fillElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, QStringLiteral( "fill" ), buffer.color().name() ) );
    if ( buffer.opacity() != 1 )
    {
      fillElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, QStringLiteral( "fill-opacity" ), QString::number( buffer.opacity() ) ) );
    }
  }

  // fill
  QDomElement fillElement = doc.createElement( QStringLiteral( "se:Fill" ) );
  textSymbolizerElement.appendChild( fillElement );
  fillElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, QStringLiteral( "fill" ), format.color().name() ) );
  if ( format.opacity() != 1 )
  {
    fillElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, QStringLiteral( "fill-opacity" ), QString::number( format.opacity() ) ) );
  }

  // background graphic (not supported by SE 1.1, but supported by the GeoTools ecosystem as an extension)
  const QgsTextBackgroundSettings background = format.background();
  if ( background.enabled() )
  {
    std::unique_ptr<QgsMarkerSymbolLayer> layer = backgroundToMarkerLayer( background );
    layer->writeSldMarker( doc, textSymbolizerElement, props );
  }

  // priority and zIndex, the default values are 0 and 5 in qgis (and between 0 and 10),
  // in the GeoTools ecosystem there is a single priority value set at 1000 by default
  if ( settings.priority != 5 || settings.zIndex > 0 )
  {
    QDomElement priorityElement = doc.createElement( QStringLiteral( "se:Priority" ) );
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
    const QDomElement vo = QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "underlineText" ), QStringLiteral( "true" ) );
    textSymbolizerElement.appendChild( vo );
  }
  if ( font.strikeOut() )
  {
    const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "strikethroughText" ), QStringLiteral( "true" ) );
    textSymbolizerElement.appendChild( vo );
  }
  // vendor options for text positioning
  if ( maxDisplacement > 0 )
  {
    const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "maxDisplacement" ), qgsDoubleToString( maxDisplacement, 2 ) );
    textSymbolizerElement.appendChild( vo );
  }
  if ( settings.placement == QgsPalLayerSettings::Curved || settings.placement == QgsPalLayerSettings::PerimeterCurved )
  {
    const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "followLine" ), QStringLiteral( "true" ) );
    textSymbolizerElement.appendChild( vo );
    if ( settings.maxCurvedCharAngleIn > 0 || settings.maxCurvedCharAngleOut > 0 )
    {
      // SLD has no notion for this, the GeoTools ecosystem can only do a single angle
      const double angle = std::min( std::fabs( settings.maxCurvedCharAngleIn ), std::fabs( settings.maxCurvedCharAngleOut ) );
      const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "maxAngleDelta" ), qgsDoubleToString( angle ) );
      textSymbolizerElement.appendChild( vo );
    }
  }
  if ( repeatDistance > 0 )
  {
    const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "repeat" ), qgsDoubleToString( repeatDistance, 2 ) );
    textSymbolizerElement.appendChild( vo );
  }
  // miscellaneous options
  if ( settings.displayAll )
  {
    const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "conflictResolution" ), QStringLiteral( "false" ) );
    textSymbolizerElement.appendChild( vo );
  }
  if ( settings.upsidedownLabels == QgsPalLayerSettings::ShowAll )
  {
    const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "forceLeftToRight" ), QStringLiteral( "false" ) );
    textSymbolizerElement.appendChild( vo );
  }
  if ( settings.lineSettings().mergeLines() )
  {
    const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "group" ), QStringLiteral( "yes" ) );
    textSymbolizerElement.appendChild( vo );
    if ( settings.labelPerPart )
    {
      const QDomElement vo =  QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "labelAllGroup" ), QStringLiteral( "true" ) );
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
          resizeType = QStringLiteral( "stretch" );
        }
        else
        {
          resizeType = QStringLiteral( "proportional" );
        }
        const QDomElement voResize =  QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "graphic-resize" ), resizeType );
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
          const QDomElement voMargin =  QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "graphic-margin" ), resizeSpec );
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
}


void QgsVectorLayerSimpleLabeling::toSld( QDomNode &parent, const QVariantMap &props ) const
{

  if ( mSettings->drawLabels )
  {
    QDomDocument doc = parent.ownerDocument();

    QDomElement ruleElement = doc.createElement( QStringLiteral( "se:Rule" ) );
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

    writeTextSymbolizer( ruleElement, *mSettings, props );
  }


}

void QgsVectorLayerSimpleLabeling::setSettings( QgsPalLayerSettings *settings, const QString &providerId )
{
  Q_UNUSED( providerId )

  if ( mSettings.get() == settings )
    return;

  mSettings.reset( settings );
}
