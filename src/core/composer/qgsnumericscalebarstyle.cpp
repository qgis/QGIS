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

QgsNumericScaleBarStyle::QgsNumericScaleBarStyle( QgsComposerScaleBar* bar ): QgsScaleBarStyle( bar ), mLastScaleBarWidth( 0 )
{

}

QgsNumericScaleBarStyle::QgsNumericScaleBarStyle(): QgsScaleBarStyle( 0 ), mLastScaleBarWidth( 0 )
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
  Q_UNUSED( xOffset );
  if ( !p || !mScaleBar )
  {
    return;
  }

  p->save();
  p->setFont( mScaleBar->font() );
  p->setPen( mScaleBar->fontColor() );

  //call QgsComposerItem's pen() function, since that refers to the frame pen
  //and QgsComposerScalebar's pen() function refers to the scale bar line width,
  //which is not used for numeric scale bars. Divide the pen width by 2 since
  //half the width of the frame is drawn outside the item.
  double penWidth = mScaleBar->QgsComposerItem::pen().widthF() / 2.0;
  double margin = mScaleBar->boxContentSpace();
  //map scalebar alignment to Qt::AlignmentFlag type
  Qt::AlignmentFlag hAlign;
  switch ( mScaleBar->alignment() )
  {
    case QgsComposerScaleBar::Left:
      hAlign = Qt::AlignLeft;
      break;
    case QgsComposerScaleBar::Middle:
      hAlign = Qt::AlignHCenter;
      break;
    case QgsComposerScaleBar::Right:
      hAlign = Qt::AlignRight;
      break;
    default:
      hAlign = Qt::AlignLeft;
      break;
  }

  //text destination is item's rect, excluding the margin and frame
  QRectF painterRect( penWidth + margin, penWidth + margin, mScaleBar->rect().width() - 2 * penWidth - 2 * margin, mScaleBar->rect().height() - 2 * penWidth - 2 * margin );
  mScaleBar->drawText( p, painterRect, scaleText(), mScaleBar->font(), hAlign, Qt::AlignTop );

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

  rect = QRectF( mScaleBar->transform().dx(), mScaleBar->transform().dy(), 2 * mScaleBar->boxContentSpace()
                 + 2 * mScaleBar->pen().width() + textWidth,
                 textHeight + 2 * mScaleBar->boxContentSpace() );

  if ( mLastScaleBarWidth != rect.width() && mLastScaleBarWidth > 0 && rect.width() > 0 )
  {
    //hack to move scale bar the left / right in order to keep the bar alignment
    const_cast<QgsComposerScaleBar*>( mScaleBar )->correctXPositionAlignment( mLastScaleBarWidth, rect.width() );
  }
  mLastScaleBarWidth = rect.width();
  return rect;
}

QString QgsNumericScaleBarStyle::scaleText() const
{
  QString scaleBarText;
  if ( mScaleBar )
  {
    //find out scale
    double scaleDenominator = 1;
    const QgsComposerMap* composerMap = mScaleBar->composerMap();
    if ( composerMap )
    {
      scaleDenominator = composerMap->scale();
      scaleBarText = "1:" + QString( "%L1" ).arg( scaleDenominator, 0, 'f', 0 );
    }
    scaleBarText = "1:" + QString( "%L1" ).arg( scaleDenominator, 0, 'f', 0 );
  }
  return scaleBarText;
}
