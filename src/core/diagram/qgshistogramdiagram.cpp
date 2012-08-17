/***************************************************************************
    qgshistogramdiagram.cpp
    ---------------------
    begin                : August 2012
    copyright            : (C) 2012 by Matthias Kuhn
    email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgshistogramdiagram.h"
#include "qgsdiagramrendererv2.h"
#include "qgsrendercontext.h"

#include <QPainter>

QgsHistogramDiagram::QgsHistogramDiagram()
{
  mCategoryBrush.setStyle( Qt::SolidPattern );
  mPen.setStyle( Qt::SolidLine );
  mScaleFactor = 0;
}

QgsHistogramDiagram::~QgsHistogramDiagram()
{
}

QSizeF QgsHistogramDiagram::diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is )
{
  Q_UNUSED( c );
  QgsAttributeMap::const_iterator attIt = attributes.constBegin();
  if ( attIt == attributes.constEnd() )
  {
    return QSizeF(); //zero size if no attributes
  }

  double maxValue = attIt.value().toDouble();

  for ( ++attIt; attIt != attributes.constEnd(); ++attIt )
  {
    maxValue = qMax( attIt.value().toDouble(), maxValue );
  }

  // Scale, if extension is smaller than the specified minimum
  if ( maxValue < s.minimumSize )
  {
    maxValue = s.minimumSize;
  }

  switch ( s.diagramOrientation )
  {
    case QgsDiagramSettings::Up:
    case QgsDiagramSettings::Down:
      mScaleFactor = ( ( is.upperSize.width() - is.lowerSize.height() ) / ( is.upperValue - is.lowerValue ) );
      size.scale( s.barWidth * attributes.size(), maxValue * mScaleFactor, Qt::IgnoreAspectRatio );
      break;

    case QgsDiagramSettings::Right:
    case QgsDiagramSettings::Left:
      mScaleFactor = ( ( is.upperSize.width() - is.lowerSize.width() ) / ( is.upperValue - is.lowerValue ) );
      size.scale( maxValue * mScaleFactor, s.barWidth * attributes.size(), Qt::IgnoreAspectRatio );
      break;
  }

  QSizeF scaledSize = sizeForPAL( size, s, c );
  return scaledSize;
}

QSizeF QgsHistogramDiagram::diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s )
{
  QSizeF size;

  QgsAttributeMap::const_iterator attIt = attributes.constBegin();
  if ( attIt == attributes.constEnd() )
  {
    return QSizeF(); //zero size if no attributes
  }

  double maxValue = attIt.value().toDouble();

  for ( ; attIt != attributes.constEnd(); ++attIt )
  {
    maxValue = qMax( attIt.value().toDouble(), maxValue );
  }

  switch ( s.diagramOrientation )
  {
    case QgsDiagramSettings::Up:
    case QgsDiagramSettings::Down:
      mScaleFactor = maxValue / s.size.height();
      size.scale( s.barWidth * attributes.size(), s.size.height(), Qt::IgnoreAspectRatio );
      break;

    case QgsDiagramSettings::Right:
    case QgsDiagramSettings::Left:
    default: // just in case...
      mScaleFactor = maxValue / s.size.width();
      size.scale( s.size.width(), s.barWidth * attributes.size(), Qt::IgnoreAspectRatio );
      break;
  }

  QSizeF scaledSize = sizeForPAL( size, s, c );
  return scaledSize;
}

void QgsHistogramDiagram::renderDiagram( const QgsAttributeMap& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position )
{
  QPainter* p = c.painter();
  if ( !p )
  {
    return;
  }

  QList<double> values;

  QList<int>::const_iterator catIt = s.categoryIndices.constBegin();
  for ( ; catIt != s.categoryIndices.constEnd(); ++catIt )
  {
    double currentVal = att[*catIt].toDouble();
    values.push_back( currentVal );
  }

  double currentOffset = 0 - ( values.size() * s.barWidth ) / 2;
  double scaledWidth = sizePainterUnits( s.barWidth, s, c );

  double baseX = position.x();
  double baseY = position.y();

  mPen.setColor( s.penColor );
  setPenWidth( mPen, s, c );
  p->setPen( mPen );

  QList<double>::const_iterator valIt = values.constBegin();
  QList< QColor >::const_iterator colIt = s.categoryColors.constBegin();
  for ( ; valIt != values.constEnd(); ++valIt, ++colIt )
  {
    double length = sizePainterUnits( *valIt * mScaleFactor, s, c );
    
    mCategoryBrush.setColor( *colIt );
    p->setBrush( mCategoryBrush );

    switch ( s.diagramOrientation )
    {
      case QgsDiagramSettings::Up:
        p->drawRect( baseX + currentOffset, baseY, scaledWidth, 0 - length );
        break;

      case QgsDiagramSettings::Down:
        p->drawRect( baseX + currentOffset, baseY, scaledWidth, length );
        break;

      case QgsDiagramSettings::Right:
        p->drawRect( baseX, baseY + currentOffset, length, scaledWidth );
        break;

      case QgsDiagramSettings::Left:
        p->drawRect( baseX, baseY + currentOffset, 0 - length, scaledWidth );
        break;
    }

    currentOffset += scaledWidth;
  }
}
