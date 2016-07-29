/***************************************************************************
                            qgsticksscalebarstyle.h
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

#ifndef QGSTICKSSCALEBARSTYLE_H
#define QGSTICKSSCALEBARSTYLE_H

#include "qgsscalebarstyle.h"

/** \ingroup core
 * A scale bar that draws segments using short ticks.
 */
class CORE_EXPORT QgsTicksScaleBarStyle: public QgsScaleBarStyle
{
  public:
    enum TickPosition
    {
      TicksUp,
      TicksDown,
      TicksMiddle
    };

    QgsTicksScaleBarStyle( const QgsComposerScaleBar* bar );
    ~QgsTicksScaleBarStyle();

    QString name() const override;

    /** Draw method
     @param p painter object
     @param xOffset offset
     */
    void draw( QPainter* p, double xOffset = 0 ) const override;

    void setTickPosition( TickPosition p ) {mTickPosition = p;}

  private:
    QgsTicksScaleBarStyle(); //forbidden

    TickPosition mTickPosition;
};

#endif
