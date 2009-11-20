/***************************************************************************
                            qgsscalebarstyle.cpp
                            --------------------
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

#include "qgsscalebarstyle.h"
#include "qgscomposerscalebar.h"
#include <QFontMetricsF>
#include <QPainter>

QgsScaleBarStyle::QgsScaleBarStyle( const QgsComposerScaleBar* bar ):  mScaleBar( bar )
{

}

QgsScaleBarStyle::QgsScaleBarStyle(): mScaleBar( 0 )
{

}

QgsScaleBarStyle::~QgsScaleBarStyle()
{

}

void QgsScaleBarStyle::drawLabels( QPainter* p ) const
{
  if ( !p || !mScaleBar )
  {
    return;
  }

  p->save();

  p->setFont( mScaleBar->font() );

  QString firstLabel = mScaleBar->firstLabelString();
  double xOffset =  mScaleBar->textWidthMillimeters( mScaleBar->font(), firstLabel ) / 2;

  //double mCurrentXCoord = mScaleBar->pen().widthF() + mScaleBar->boxContentSpace();
  QList<QPair<double, double> > segmentInfo;
  mScaleBar->segmentPositions( segmentInfo );

  double currentLabelNumber = 0.0;

  int nSegmentsLeft = mScaleBar->numSegmentsLeft();
  int segmentCounter = 0;
  QString currentNumericLabel;

  QList<QPair<double, double> >::const_iterator segmentIt = segmentInfo.constBegin();
  for ( ; segmentIt != segmentInfo.constEnd(); ++segmentIt )
  {
    if ( segmentCounter == 0 && nSegmentsLeft > 0 )
    {
      //label first left segment
      currentNumericLabel = firstLabel;
    }
    else if ( segmentCounter != 0 && segmentCounter == nSegmentsLeft ) //reset label number to 0 if there are left segments
    {
      currentLabelNumber = 0;
    }

    if ( segmentCounter >= nSegmentsLeft )
    {
      currentNumericLabel = QString::number( currentLabelNumber / mScaleBar->numMapUnitsPerScaleBarUnit() );
    }

    if ( segmentCounter == 0 || segmentCounter >= nSegmentsLeft ) //don't draw label for intermediate left segments
    {
      p->setPen( QColor( 0, 0, 0 ) );
      mScaleBar->drawText( p, segmentIt->first - mScaleBar->textWidthMillimeters( mScaleBar->font(), currentNumericLabel ) / 2 + xOffset, mScaleBar->fontAscentMillimeters( mScaleBar->font() ) + mScaleBar->boxContentSpace(), currentNumericLabel, mScaleBar->font() );
    }

    if ( segmentCounter >= nSegmentsLeft )
    {
      currentLabelNumber += mScaleBar->numUnitsPerSegment();
    }
    ++segmentCounter;
  }

  //also draw the last label
  if ( !segmentInfo.isEmpty() )
  {
    currentNumericLabel = QString::number( currentLabelNumber / mScaleBar->numMapUnitsPerScaleBarUnit() );
    p->setPen( QColor( 0, 0, 0 ) );
    mScaleBar->drawText( p, segmentInfo.last().first + mScaleBar->segmentMillimeters() - mScaleBar->textWidthMillimeters( mScaleBar->font(), currentNumericLabel ) / 2 + xOffset, mScaleBar->fontAscentMillimeters( mScaleBar->font() ) + mScaleBar->boxContentSpace(), currentNumericLabel + " " + mScaleBar->unitLabeling(), mScaleBar->font() );
  }

  p->restore();
}

QRectF QgsScaleBarStyle::calculateBoxSize() const
{
  if ( !mScaleBar )
  {
    return QRectF();
  }

  //consider centered first label
  double firstLabelLeft = mScaleBar->textWidthMillimeters( mScaleBar->font(), mScaleBar->firstLabelString() ) / 2;

  //consider last number and label

  double largestLabelNumber = mScaleBar->numSegments() * mScaleBar->numUnitsPerSegment() / mScaleBar->numMapUnitsPerScaleBarUnit();
  QString largestNumberLabel = QString::number( largestLabelNumber );
  QString largestLabel = QString::number( largestLabelNumber ) + " " + mScaleBar->unitLabeling();
  double largestLabelWidth = mScaleBar->textWidthMillimeters( mScaleBar->font(), largestLabel ) - mScaleBar->textWidthMillimeters( mScaleBar->font(), largestNumberLabel ) / 2;

  double totalBarLength = 0.0;

  QList< QPair<double, double> > segmentList;
  mScaleBar->segmentPositions( segmentList );

  QList< QPair<double, double> >::const_iterator segmentIt = segmentList.constBegin();
  for ( ; segmentIt != segmentList.constEnd(); ++segmentIt )
  {
    totalBarLength += segmentIt->second;
  }

  double width =  firstLabelLeft + totalBarLength + 2 * mScaleBar->pen().widthF() + largestLabelWidth + 2 * mScaleBar->boxContentSpace();
  double height = mScaleBar->height() + mScaleBar->labelBarSpace() + 2 * mScaleBar->boxContentSpace() + mScaleBar->fontAscentMillimeters( mScaleBar->font() );

  return QRectF( mScaleBar->transform().dx(), mScaleBar->transform().dy(), width, height );
}
