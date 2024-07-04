/***************************************************************************
    qgsstackediagram.cpp
    ---------------------
    begin                : June 2024
    copyright            : (C) 2024 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsstackeddiagram.h"
#include "qgsdiagramrenderer.h"
#include "qgsrendercontext.h"
#include "qgsexpression.h"
#include "qgssymbollayerutils.h"
#include "qgslinesymbol.h"

#include <QPainter>

QgsStackedDiagram::QgsStackedDiagram()
{
  mCategoryBrush.setStyle( Qt::SolidPattern );
  mPen.setStyle( Qt::SolidLine );
  mScaleFactor = 0;
}

QgsStackedDiagram *QgsStackedDiagram::clone() const
{
  return new QgsStackedDiagram( *this );
}

void QgsStackedDiagram::addSubDiagram( QgsDiagram *diagram, QgsDiagramSettings *s )
{
  mSubDiagrams.append( DiagramData{diagram, s} );
}

QList< QgsDiagram * > QgsStackedDiagram::subDiagrams() const
{
  QList< QgsDiagram * > diagrams;
  for ( const auto &item : std::as_const( mSubDiagrams ) )
  {
    diagrams.append( item.diagram );
  }
  return diagrams;
}

QgsDiagramSettings *QgsStackedDiagram::subDiagramSettings( const QgsDiagram *diagram ) const
{
  for ( const auto &item : std::as_const( mSubDiagrams ) )
  {
    if ( item.diagram == diagram )
    {
      return item.settings;
    }
  }
  return nullptr;
}

void QgsStackedDiagram::subDiagramPosition( QPointF &newPos, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramSettings &subSettings )
{
  QSizeF size = sizePainterUnits( subSettings.size, subSettings, c );
  const double spacing = c.convertToPainterUnits( s.stackedDiagramSpacing(), s.stackedDiagramSpacingUnit(), s.stackedDiagramSpacingMapUnitScale() );

  if ( s.stackedDiagramMode == QgsDiagramSettings::Horizontal )
  {
    newPos += QPointF( size.width() + spacing, 0 );
  }
  else
  {
    newPos += QPointF( 0, size.height() + spacing );
  }
}

QSizeF QgsStackedDiagram::diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is )
{
  QSizeF size( 0, 0 );
  if ( feature.attributeCount() == 0 )
  {
    return size; //zero size if no attributes
  }

  // Iterate subdiagramas and sum their individual sizes
  for ( const auto &item : std::as_const( mSubDiagrams ) )
  {
    size += item.diagram->diagramSize( feature, c, *item.settings, is );
  }

  // eh - this method returns size in unknown units ...! We'll have to fake it and use a rough estimation of
  // a conversion factor to painter units...
  // TODO QGIS 4.0 -- these methods should all use painter units, dependent on the render context scaling...
  double painterUnitConversionScale = c.convertToPainterUnits( 1, s.sizeType );

  const double spacing = c.convertToPainterUnits( s.stackedDiagramSpacing(), s.stackedDiagramSpacingUnit(), s.stackedDiagramSpacingMapUnitScale() ) / painterUnitConversionScale;

  switch ( s.stackedDiagramMode )
  {
    case QgsDiagramSettings::Horizontal:
      size.scale( size.width() + spacing * std::max( 0, static_cast<int>( mSubDiagrams.length() ) - 1 ), size.height(), Qt::IgnoreAspectRatio );
      break;

    case QgsDiagramSettings::Vertical:
      size.scale( size.width(), size.height() + spacing * std::max( 0, static_cast<int>( mSubDiagrams.length() ) - 1 ), Qt::IgnoreAspectRatio );
      break;
  }

  if ( s.showAxis() && s.axisLineSymbol() )
  {
    const double maxBleed = QgsSymbolLayerUtils::estimateMaxSymbolBleed( s.axisLineSymbol(), c ) / painterUnitConversionScale;
    size.setWidth( size.width() + 2 * maxBleed );
    size.setHeight( size.height() + 2 * maxBleed );
  }

  return size;
}

double QgsStackedDiagram::legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const
{
  if ( qgsDoubleNear( is.upperValue, is.lowerValue ) )
    return s.minimumSize; // invalid value range => zero size

  // Scale, if extension is smaller than the specified minimum
  if ( value < s.minimumSize )
  {
    value = s.minimumSize;
  }

  double scaleFactor = ( ( is.upperSize.width() - is.lowerSize.width() ) / ( is.upperValue - is.lowerValue ) );
  return value * scaleFactor;
}

QString QgsStackedDiagram::diagramName() const
{
  return DIAGRAM_NAME_STACKED;
}

QSizeF QgsStackedDiagram::diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s )
{
  QSizeF size( 0, 0 );

  if ( attributes.isEmpty() )
  {
    return QSizeF(); //zero size if no attributes
  }

  // Iterate subdiagramas and sum their individual sizes
  // accounting for stacked diagram defined spacing
  for ( const auto &item : std::as_const( mSubDiagrams ) )
  {
    size += item.diagram->diagramSize( attributes, c, *item.settings );
  }

  // eh - this method returns size in unknown units ...! We'll have to fake it and use a rough estimation of
  // a conversion factor to painter units...
  // TODO QGIS 4.0 -- these methods should all use painter units, dependent on the render context scaling...
  double painterUnitConversionScale = c.convertToPainterUnits( 1, s.sizeType );

  const double spacing = c.convertToPainterUnits( s.stackedDiagramSpacing(), s.stackedDiagramSpacingUnit(), s.stackedDiagramSpacingMapUnitScale() ) / painterUnitConversionScale;

  switch ( s.stackedDiagramMode )
  {
    case QgsDiagramSettings::Horizontal:
      size.scale( size.width() + spacing * std::max( 0, static_cast<int>( mSubDiagrams.length() ) - 1 ), s.size.height(), Qt::IgnoreAspectRatio );
      break;

    case QgsDiagramSettings::Vertical:
      size.scale( s.size.width(), size.height() + spacing * std::max( 0, static_cast<int>( mSubDiagrams.length() ) - 1 ), Qt::IgnoreAspectRatio );
      break;
  }

  if ( s.showAxis() && s.axisLineSymbol() )
  {
    const double maxBleed = QgsSymbolLayerUtils::estimateMaxSymbolBleed( s.axisLineSymbol(), c ) / painterUnitConversionScale;
    size.setWidth( size.width() + 2 * maxBleed );
    size.setHeight( size.height() + 2 * maxBleed );
  }

  return size;
}

void QgsStackedDiagram::renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position )
{
  Q_UNUSED( feature )
  Q_UNUSED( c )
  Q_UNUSED( s )
  Q_UNUSED( position )
}
