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
#include "qgis_gui.h"


class QgsAdvancedDigitizingDockWidget;

#ifdef SIP_RUN
% ModuleHeaderCode
// For ConvertToSubClassCode.
#include <qgsadvanceddigitizingcanvasitem.h>
% End
#endif

/**
 * \ingroup gui
 * \brief The QgsAdvancedDigitizingCanvasItem class draws the graphical elements of the CAD tools (\see QgsAdvancedDigitizingDockWidget) on the map canvas.
 */
class GUI_EXPORT QgsAdvancedDigitizingCanvasItem : public QgsMapCanvasItem
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsAdvancedDigitizingCanvasItem *>( sipCpp ) )
      sipType = sipType_QgsAdvancedDigitizingCanvasItem;
    else
      sipType = nullptr;
    SIP_END
#endif

  public:
    explicit QgsAdvancedDigitizingCanvasItem( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

    void paint( QPainter *painter ) override;
    void updatePosition() override;

  private:
    QPen mLockedPen;
    QPen mConstruction1Pen;
    QPen mConstruction2Pen;
    QPen mSnapPen;
    QPen mSnapLinePen;
    QPen mCursorPen;
    QgsAdvancedDigitizingDockWidget *mAdvancedDigitizingDockWidget = nullptr;
};

#endif // QGSADVANCEDDIGITIZINGCANVASITEM_H
