/***************************************************************************
                            qgsnumericscalebarstyle.cpp
                            ---------------------------
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

#include "qgsnumericscalebarstyle.h"
#include "qgscomposermap.h"
#include "qgscomposerscalebar.h"
#include <QList>
#include <QPainter>

QgsNumericScaleBarStyle::QgsNumericScaleBarStyle( QgsComposerScaleBar* bar ): QgsScaleBarStyle( bar )
{

}

QgsNumericScaleBarStyle::QgsNumericScaleBarStyle(): QgsScaleBarStyle( 0 )
{

}

QgsNumericScaleBarStyle::~QgsNumericScaleBarStyle()
{

}

QString QgsNumericScaleBarStyle::name() const
{
  return "Numeric";
}

void QgsNumericScaleBarStyle::draw( QPainter* p, double xOffset ) const
{
  if ( !p || !mScaleBar )
  {
    return;
  }

  p->save();
  p->setFont( mScaleBar->font() );
  p->setPen( QColor( 0, 0, 0 ) );
  mScaleBar->drawText( p, mScaleBar->pen().widthF() + mScaleBar->boxContentSpace(), mScaleBar->boxContentSpace() + mScaleBar->fontAscentMillimeters( mScaleBar->font() ), scaleText(), mScaleBar->font() );

  p->restore();
}

QRectF QgsNumericScaleBarStyle::calculateBoxSize() const
{
  QRectF rect;
  if ( !mScaleBar )
  {
    return rect;
  }

  double textWidth = mScaleBar->textWidthMillimeters( mScaleBar->font(), scaleText() );
  double textHeight = mScaleBar->fontAscentMillimeters( mScaleBar->font() );

  return QRectF( mScaleBar->transform().dx(), mScaleBar->transform().dy(), 2 * mScaleBar->boxContentSpace()
                 + 2 * mScaleBar->pen().width() + textWidth,
                 textHeight + 2 * mScaleBar->boxContentSpace() );
}

QString QgsNumericScaleBarStyle::scaleText() const
{
  QString scaleBarText;
  if ( mScaleBar )
  {
    //find out scale
    const QgsComposerMap* composerMap = mScaleBar->composerMap();
    if ( composerMap )
    {
      double scaleDenominator = composerMap->scale();
      scaleBarText = "1:" + QString::number( scaleDenominator, 'f', 0 );
    }
  }
  return scaleBarText;
}
