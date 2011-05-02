/***************************************************************************
                            qgsdoubleboxscalebarstyle.cpp
                            -----------------------------
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

#include "qgsdoubleboxscalebarstyle.h"
#include "qgscomposerscalebar.h"
#include <QList>
#include <QPainter>

QgsDoubleBoxScaleBarStyle::QgsDoubleBoxScaleBarStyle( const QgsComposerScaleBar* bar ): QgsScaleBarStyle( bar )
{

}

QgsDoubleBoxScaleBarStyle::QgsDoubleBoxScaleBarStyle(): QgsScaleBarStyle( 0 )
{

}

QgsDoubleBoxScaleBarStyle::~QgsDoubleBoxScaleBarStyle()
{

}

QString QgsDoubleBoxScaleBarStyle::name() const
{
  return "Double Box";
}

void QgsDoubleBoxScaleBarStyle::draw( QPainter* p, double xOffset ) const
{
  if ( !mScaleBar )
  {
    return;
  }
  double barTopPosition = mScaleBar->fontAscentMillimeters( mScaleBar->font() ) + mScaleBar->labelBarSpace() + mScaleBar->boxContentSpace();
  double segmentHeight = mScaleBar->height() / 2;

  p->save();
  p->setPen( p->pen() );

  QList<QPair<double, double> > segmentInfo;
  mScaleBar->segmentPositions( segmentInfo );

  bool useColor = true; //alternate brush color/white



  QList<QPair<double, double> >::const_iterator segmentIt = segmentInfo.constBegin();
  for ( ; segmentIt != segmentInfo.constEnd(); ++segmentIt )
  {
    //draw top half
    if ( useColor )
    {
      p->setBrush( mScaleBar->brush() );
    }
    else //white
    {
      p->setBrush( QColor( 255, 255, 255 ) );
    }

    QRectF segmentRectTop( segmentIt->first + xOffset, barTopPosition, segmentIt->second, segmentHeight );
    p->drawRect( segmentRectTop );

    //draw bottom half
    if ( useColor )
    {
      p->setBrush( QColor( 255, 255, 255 ) );
    }
    else //white
    {
      p->setBrush( mScaleBar->brush() );
    }

    QRectF segmentRectBottom( segmentIt->first + xOffset, barTopPosition + segmentHeight, segmentIt->second, segmentHeight );
    p->drawRect( segmentRectBottom );
    useColor = !useColor;
  }

  p->restore();

  //draw labels using the default method
  drawLabels( p );
}
