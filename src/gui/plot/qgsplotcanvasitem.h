/***************************************************************************
                          qgsplotcanvasitem.h
                          ------------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPLOTCANVASITEM_H
#define QGSPLOTCANVASITEM_H

#include <QGraphicsItem>
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsPlotCanvas;

#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgsplotcanvasitem.h"
% End
#endif

/**
 * \ingroup gui
 * \brief An abstract class for items that can be placed on a QgsPlotCanvas.
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPlotCanvasItem : public QGraphicsItem
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsPlotCanvasItem *>( sipCpp ) != nullptr )
      sipType = sipType_QgsPlotCanvasItem;
    else
      sipType = nullptr;
    SIP_END
#endif

  protected:

    /**
     * Constructor for QgsPlotCanvasItem for the specified \a canvas.
     */
    QgsPlotCanvasItem( QgsPlotCanvas *canvas SIP_TRANSFERTHIS );

    ~QgsPlotCanvasItem() override;

    /**
     * Paints the item. Must be implemented by derived classes.
     */
    virtual void paint( QPainter *painter ) = 0;

    void paint( QPainter *painter,
                const QStyleOptionGraphicsItem *option,
                QWidget *widget = nullptr ) override;

  protected:

    //! Associated canvas
    QgsPlotCanvas *mCanvas = nullptr;

};


#endif // QGSPLOTCANVASITEM_H
