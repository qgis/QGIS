/***************************************************************************
  qgsrastercontourrendererwidget.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERCONTOURRENDERERWIDGET_H
#define QGSRASTERCONTOURRENDERERWIDGET_H

#include "qgsrasterrendererwidget.h"
#include "qgis_sip.h"
#include "ui_qgsrastercontourrendererwidget.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * Configuration widget for QgsRasterContourRenderer
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsRasterContourRendererWidget : public QgsRasterRendererWidget, private Ui::QgsRasterContourRendererWidget
{
    Q_OBJECT
  public:
    //! Constructs the widget
    QgsRasterContourRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent = QgsRectangle() );

    //! Widget creation function (mainly for the use by the renderer registry)
    static QgsRasterRendererWidget *create( QgsRasterLayer *layer, const QgsRectangle &extent ) SIP_FACTORY { return new QgsRasterContourRendererWidget( layer, extent ); }

    QgsRasterRenderer *renderer() override;
};

#endif // QGSRASTERCONTOURRENDERERWIDGET_H
