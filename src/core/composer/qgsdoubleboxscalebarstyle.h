/***************************************************************************
                            qgsdoubleboxscalebarstyle.h
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

#ifndef QGSDOUBLEBOXSCALEBARSTYLE_H
#define QGSDOUBLEBOXSCALEBARSTYLE_H

#include "qgsscalebarstyle.h"

/** \ingroup MapComposer
  * Double box with alternating colors
  */
class CORE_EXPORT QgsDoubleBoxScaleBarStyle: public QgsScaleBarStyle
{
  public:
    QgsDoubleBoxScaleBarStyle( const QgsComposerScaleBar* bar );
    ~QgsDoubleBoxScaleBarStyle();

    QString name() const;

    void draw( QPainter* p, double xOffset = 0 ) const;

  private:
    QgsDoubleBoxScaleBarStyle(); //forbidden
};

#endif
