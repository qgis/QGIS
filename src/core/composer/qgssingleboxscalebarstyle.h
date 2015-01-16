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

#ifndef QGSSINGLEBOXSCALEBARSTYLE_H
#define QGSSINGLEBOXSCALEBARSTYLE_H

#include "qgsscalebarstyle.h"

/** \ingroup MapComposer
 * Scalebar style that draws a single box with alternating
 * color for the segments.
 */
class CORE_EXPORT QgsSingleBoxScaleBarStyle: public QgsScaleBarStyle
{
  public:
    QgsSingleBoxScaleBarStyle( const QgsComposerScaleBar* bar );
    ~QgsSingleBoxScaleBarStyle();

    QString name() const override;

    /*! draw method
     @param p painter object
     @param xOffset x offset
     */
    void draw( QPainter* p, double xOffset = 0 ) const override;

  private:
    QgsSingleBoxScaleBarStyle(); //forbidden
};

#endif
