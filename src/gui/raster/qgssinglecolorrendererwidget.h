/***************************************************************************
                         qgssinglecolorrendererwidget.h
                         ---------------------------------
    begin                : April 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSINGLECOLORRENDERERWIDGET_H
#define QGSSINGLECOLORRENDERERWIDGET_H

#include "ui_qgssinglecolorrendererwidgetbase.h"

#include "qgsrasterrendererwidget.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

/**
 * \brief Renderer widget for the single color renderer.
 * \ingroup gui
 * \since QGIS 3.38
 */
class GUI_EXPORT QgsSingleColorRendererWidget: public QgsRasterRendererWidget, private Ui::QgsSingleColorRendererWidgetBase
{
    Q_OBJECT
  public:
    //! Constructs the widget
    QgsSingleColorRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent = QgsRectangle() );

    //! Widget creation function (use by the renderer registry)
    static QgsRasterRendererWidget *create( QgsRasterLayer *layer, const QgsRectangle &extent ) SIP_FACTORY { return new QgsSingleColorRendererWidget( layer, extent ); }

    QgsRasterRenderer *renderer() SIP_FACTORY override;

    /**
     * Sets the widget state from the specified renderer.
     */
    void setFromRenderer( const QgsRasterRenderer *r );

  private slots:
    void colorChanged( const QColor &color );

};

#endif // QGSSINGLECOLORRENDERERWIDGET_H
