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

const QString QgsStackedDiagram::DIAGRAM_NAME_STACKED = QStringLiteral( "StackedDiagram" );

QgsStackedDiagram::QgsStackedDiagram()
{
}

QgsStackedDiagram *QgsStackedDiagram::clone() const
{
  return new QgsStackedDiagram( *this );
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
    newPos -= QPointF( 0, size.height() + spacing );
  }
}

QSizeF QgsStackedDiagram::diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is )
{
  Q_UNUSED( feature )
  Q_UNUSED( c )
  Q_UNUSED( s )
  Q_UNUSED( is )
  return QSize( 0, 0 );
}

double QgsStackedDiagram::legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const
{
  Q_UNUSED( value )
  Q_UNUSED( s )
  Q_UNUSED( is )
  return 0;
}

QString QgsStackedDiagram::diagramName() const
{
  return QgsStackedDiagram::DIAGRAM_NAME_STACKED;
}

QSizeF QgsStackedDiagram::diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s )
{
  Q_UNUSED( attributes )
  Q_UNUSED( c )
  Q_UNUSED( s )
  return QSizeF( 0, 0 );
}

void QgsStackedDiagram::renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position )
{
  Q_UNUSED( feature )
  Q_UNUSED( c )
  Q_UNUSED( s )
  Q_UNUSED( position )
}
