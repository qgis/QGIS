/***************************************************************************
                             qgslegendpatchshapewidget.h
                             ---------------------------
    Date                 : April 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLEGENDPATCHSHAPEWIDGET_H
#define QGSLEGENDPATCHSHAPEWIDGET_H

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgslegendpatchshapewidgetbase.h"
#include "qgslegendpatchshape.h"

/**
 * \ingroup gui
 * \brief Widget for configuring a custom legend patch shape.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsLegendPatchShapeWidget : public QgsPanelWidget, private Ui::QgsLegendPatchShapeWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLegendPatchShapeWidget, with the specified \a parent widget.
     */
    QgsLegendPatchShapeWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, const QgsLegendPatchShape &shape = QgsLegendPatchShape() );

    /**
     * Returns the legend patch shape as currently defined by the widget.
     *
     * \see setShape()
     */
    QgsLegendPatchShape shape() const;

    /**
     * Sets the shape to show in the widget.
     *
     * \see shape()
     */
    void setShape( const QgsLegendPatchShape &shape );

  signals:

    /**
     * Emitted whenever the patch shape defined by the widget is changed.
     */
    void changed();

  private:

    QgsSymbol::SymbolType mType = QgsSymbol::Fill;

};

#endif // QGSLEGENDPATCHSHAPEWIDGET_H
