/***************************************************************************
                            qgssingleboxscalebarstyle.h
                            ------------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco.hugentobler@karto.baug.ethz.ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssingleboxscalebarstyle.h"
#include "qgscomposerscalebar.h"
#include <QList>
#include <QPainter>

QgsSingleBoxScaleBarStyle::QgsSingleBoxScaleBarStyle( const QgsComposerScaleBar* bar ): QgsScaleBarStyle( bar )
{

}

QgsSingleBoxScaleBarStyle::QgsSingleBoxScaleBarStyle(): QgsScaleBarStyle( 0 )
{

}

QgsSingleBoxScaleBarStyle::~QgsSingleBoxScaleBarStyle()
{
  //nothing to do...
}

void QgsSingleBoxScaleBarStyle::draw( QPainter* p, double xOffset ) const
{
  if ( !mScaleBar )
  {
    return;
  }
  double barTopPosition = mScaleBar->fontAscentMillimeters( mScaleBar->font() ) + mScaleBar->labelBarSpace() + mScaleBar->boxContentSpace();

  p->save();
  p->setPen( p->pen() );

  QList<QPair<double, double> > segmentInfo;
  mScaleBar->segmentPositions( segmentInfo );

  bool useColor = true; //alternate brush color/white

  QList<QPair<double, double> >::const_iterator segmentIt = segmentInfo.constBegin();
  for ( ; segmentIt != segmentInfo.constEnd(); ++segmentIt )
  {
    if ( useColor ) //alternating colors
    {
      p->setBrush( mScaleBar->brush() );
    }
    else //white
    {
      p->setBrush( QColor( 255, 255, 255 ) );
    }

    QRectF segmentRect( segmentIt->first + xOffset, barTopPosition, segmentIt->second, mScaleBar->height() );
    p->drawRect( segmentRect );
    useColor = !useColor;
  }

  p->restore();

  //draw labels using the default method
  drawLabels( p );
}

QString QgsSingleBoxScaleBarStyle::name() const
{
  return "Single Box";
}

