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
#include "qgsexpression.h"

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

QgsDiagram* QgsHistogramDiagram::clone() const
{
  return new QgsHistogramDiagram( *this );
}

QSizeF QgsHistogramDiagram::diagramSize( const QgsFeature& feature, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is )
{
  Q_UNUSED( c );
  QSizeF size;
  if ( feature.attributes().count() == 0 )
  {
    return size; //zero size if no attributes
  }

  if ( is.upperValue - is.lowerValue == 0 )
    return size; // invalid value range => zero size

  double maxValue = 0;

  foreach ( QString cat, s.categoryAttributes )
  {
    QgsExpression* expression = getExpression( cat, feature.fields() );
    maxValue = qMax( expression->evaluate( feature ).toDouble(), maxValue );
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
      mScaleFactor = (( is.upperSize.width() - is.lowerSize.height() ) / ( is.upperValue - is.lowerValue ) );
      size.scale( s.barWidth * s.categoryAttributes.size(), maxValue * mScaleFactor, Qt::IgnoreAspectRatio );
      break;

    case QgsDiagramSettings::Right:
    case QgsDiagramSettings::Left:
      mScaleFactor = (( is.upperSize.width() - is.lowerSize.width() ) / ( is.upperValue - is.lowerValue ) );
      size.scale( maxValue * mScaleFactor, s.barWidth * s.categoryAttributes.size(), Qt::IgnoreAspectRatio );
      break;
  }

  return size;
}

QSizeF QgsHistogramDiagram::diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s )
{
  Q_UNUSED( c );
  QSizeF size;

  if ( attributes.count() == 0 )
  {
    return QSizeF(); //zero size if no attributes
  }

  double maxValue = attributes[0].toDouble();

  for ( int i = 0; i < attributes.count(); ++i )
  {
    maxValue = qMax( attributes[i].toDouble(), maxValue );
  }

  switch ( s.diagramOrientation )
  {
    case QgsDiagramSettings::Up:
    case QgsDiagramSettings::Down:
      mScaleFactor = maxValue / s.size.height();
      size.scale( s.barWidth * s.categoryColors.size(), s.size.height(), Qt::IgnoreAspectRatio );
      break;

    case QgsDiagramSettings::Right:
    case QgsDiagramSettings::Left:
    default: // just in case...
      mScaleFactor = maxValue / s.size.width();
      size.scale( s.size.width(), s.barWidth * s.categoryColors.size(), Qt::IgnoreAspectRatio );
      break;
  }

  return size;
}

void QgsHistogramDiagram::renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position )
{
  QPainter* p = c.painter();
  if ( !p )
  {
    return;
  }

  QList<double> values;
  double maxValue = 0;

  foreach ( QString cat, s.categoryAttributes )
  {
    QgsExpression* expression = getExpression( cat, feature.fields() );
    double currentVal = expression->evaluate( feature ).toDouble();
    values.push_back( currentVal );
    maxValue = qMax( currentVal, maxValue );
  }

  double scaledMaxVal = sizePainterUnits( maxValue * mScaleFactor, s, c );

  double currentOffset = 0;
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
        p->drawRect( baseX + currentOffset, baseY, scaledWidth, length * -1 );
        break;

      case QgsDiagramSettings::Down:
        p->drawRect( baseX + currentOffset, baseY - scaledMaxVal, scaledWidth, length );
        break;

      case QgsDiagramSettings::Right:
        p->drawRect( baseX, baseY - currentOffset, length, scaledWidth * -1 );
        break;

      case QgsDiagramSettings::Left:
        p->drawRect( baseX + scaledMaxVal, baseY - currentOffset, 0 - length, scaledWidth * -1 );
        break;
    }

    currentOffset += scaledWidth;
  }
}
