/***************************************************************************
    qgsadvanceddigitizingcanvasitem.h  -  map canvas item for CAD tools
    ----------------------
    begin                : October 2014
    copyright            : (C) Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSADVANCEDDIGITIZINGCANVASITEM_H
#define QGSADVANCEDDIGITIZINGCANVASITEM_H

#include <QPen>

#include "qgsmapcanvasitem.h"

class QgsAdvancedDigitizingDockWidget;

/**
 * @brief The QgsAdvancedDigitizingCanvasItem class draws the graphical elements of the CAD tools (@see QgsAdvancedDigitizingDock) on the map canvas.
 */
class APP_EXPORT QgsAdvancedDigitizingCanvasItem : public QgsMapCanvasItem
{
  public:
    explicit QgsAdvancedDigitizingCanvasItem( QgsMapCanvas* canvas, QgsAdvancedDigitizingDockWidget* cadDockWidget );
    ~QgsAdvancedDigitizingCanvasItem();

    void paint( QPainter *painter ) override;

  protected:
    QPen mLockedPen;
    QPen mConstruction1Pen;
    QPen mConstruction2Pen;
    QPen mSnapPen;
    QPen mSnapLinePen;
    QPen mCursorPen;

  private:
    QgsAdvancedDigitizingDockWidget* mAdvancedDigitizingDockWidget;
};

#endif // QGSADVANCEDDIGITIZINGCANVASITEM_H
