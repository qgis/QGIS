/***************************************************************************
                            qgsnumericscalebarstyle.h
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

#ifndef QGSNUMERICSCALEBARSTYLE_H
#define QGSNUMERICSCALEBARSTYLE_H

#include "qgsscalebarstyle.h"

/** \ingroup MapComposer
 * A scale bar style that draws text in the form of '1:XXXXX'
 */
class CORE_EXPORT QgsNumericScaleBarStyle: public QgsScaleBarStyle
{
  public:
    QgsNumericScaleBarStyle( QgsComposerScaleBar* bar );
    ~QgsNumericScaleBarStyle();

    QString name() const;

    void draw( QPainter* p, double xOffset = 0 ) const;

    //calculation of box size is different compared to segment based scale bars
    QRectF calculateBoxSize() const;

  private:
    QgsNumericScaleBarStyle(); //forbidden
    /**Returns the text for the scale bar or an empty string in case of error*/
    QString scaleText() const;

    /**Store last width (in mm) to keep alignement to left/middle/right side*/
    mutable double mLastScaleBarWidth;
};

#endif
