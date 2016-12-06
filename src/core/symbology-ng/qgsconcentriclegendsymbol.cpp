/***************************************************************************
    qgsconcentriclegendsymbol.h
    ---------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Stéhane Brunner
    email                : stephane dot brunner at camptocamp dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconcentriclegendsymbol.h"
#include "qgssymbollayerutils.h"

QgsConcentricLegendSymbol::QgsConcentricLegendSymbol( const QgsMarkerSymbol *symbol,
    const QList< QPair< QString, double > > values,
    const QgsUnitTypes::RenderUnit types,
    const double maxSize,
    const QgsDiagramSettings::SizeLegendType type )
  : QgsMarkerSymbol( symbol->cloneLayers() )
  , mValues( values )
  , mMaxSize( maxSize )
  , mType( type )
{
  for ( int i = mValues.size() - 1 ; i >= 0 ; i-- )
  {
    QPair< QString, double > value_size = mValues[i];
    double size = value_size.second;

    QgsMarkerSymbol *s = symbol->clone();
    s->setSize( size );
    s->setSizeUnit( types );
    mSymbols << s;
  }
}

QgsConcentricLegendSymbol::QgsConcentricLegendSymbol( const QgsConcentricLegendSymbol *const original )
  : QgsMarkerSymbol( original->cloneLayers() )
  , mSymbols( original->mSymbols )
  , mValues( original->mValues )
  , mMaxSize( original->mMaxSize )
  , mType( original->mType )
{
}

QgsConcentricLegendSymbol::~QgsConcentricLegendSymbol()
{
}


QgsConcentricLegendSymbol *QgsConcentricLegendSymbol::clone() const
{
  return new QgsConcentricLegendSymbol( this );
}

void QgsConcentricLegendSymbol::drawPreviewIcon( QPainter *painter, QSize size, QgsRenderContext *customContext )
{
  QgsRenderContext context = customContext ? *customContext : QgsRenderContext::fromQPainter( painter );
  context.setForceVectorOutput( true );
  const double factor = context.scaleFactor();
  QPainter *const p = context.painter();
  p->save();

  int max_label_size = 0;
  Q_FOREACH ( LabelValue lv, mValues )
  {
    max_label_size = std::max( max_label_size, p->fontMetrics().width( lv.first ) );
  }
  p->translate( -max_label_size - 3 * factor, 0 );


  Q_FOREACH ( QgsMarkerSymbol *s, mSymbols )
  {
    p->save();
    switch ( mType )
    {
      case QgsDiagramSettings::ConcentricBottom:
        p->translate( 0, ( mMaxSize - s->size() ) * factor / 2 );
        break;
      case QgsDiagramSettings::ConcentricCenter:
        break;
      case QgsDiagramSettings::ConcentricTop:
        p->translate( 0, ( s->size() - mMaxSize ) * factor / 2 );
        break;
      default:
        Q_ASSERT( true );
        p->translate( 0, ( s->size() - mMaxSize ) * factor / 2 );
    }
    s->drawPreviewIcon( p, size, &context );
    p->restore();
  }

  const double center_x = size.width() / 2;
  const double center_y = size.height() / 2;
  const double line_end = center_x + ( mMaxSize / 2 + 2 ) * factor;
  const double text_start = line_end + 1 * factor;
  const double text_dy = p->fontMetrics().xHeight() / 2;
  const double bottom = center_y + mMaxSize / 2 * factor;
  const double top = center_y - mMaxSize / 2 * factor;
  double pos;

  p->setOpacity( 1.0 );
  p->setPen( QPen( Qt::black, 1 ) );

  // pen width 1 opacity 1 is equivalent with width 2 opacity 0.5 :-(
  Q_FOREACH ( LabelValue lv, mValues )
  {
    switch ( mType )
    {
      case QgsDiagramSettings::ConcentricBottom:
        pos = bottom - lv.second * factor;
        break;
      case QgsDiagramSettings::ConcentricTop:
        pos = top + lv.second * factor;
        break;
      case QgsDiagramSettings::ConcentricCenter:
        pos = center_y - lv.second * factor / 2;
        break;
      default:
        Q_ASSERT( true );
        pos = top + lv.second * factor;
    }
    p->drawLine( center_x, pos, line_end, pos );
    p->drawText( text_start, pos + text_dy, lv.first );
  }

  p->restore();
}
