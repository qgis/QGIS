/***************************************************************************
                         qgshillshaderendererwidget.h
                         ---------------------------------
    begin                : May 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHILLSHADERENDERERWIDGET_H
#define QGSHILLSHADERENDERERWIDGET_H

#include "ui_qghillshaderendererwidget.h"

#include <QDoubleSpinBox>

#include "qgsrasterminmaxwidget.h"
#include "qgsrasterrendererwidget.h"

/**
 * @brief Renderer widget for the hill shade renderer.
 */
class GUI_EXPORT QgsHillshadeRendererWidget: public QgsRasterRendererWidget, private Ui::QgsHillShadeWidget
{
    Q_OBJECT
  public:

    /**
     * @brief Renderer widget for the hill shade renderer.
     * @param layer The layer attached for this widget.
     * @param extent The current extent.
     */
    QgsHillshadeRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent = QgsRectangle() );

    ~QgsHillshadeRendererWidget();

    /**
      * Factory method to create the renderer for this type.
      */
    static QgsRasterRendererWidget* create( QgsRasterLayer* layer, const QgsRectangle &theExtent ) { return new QgsHillshadeRendererWidget( layer, theExtent ); }

    /**
     * @brief The renderer for the widget.
     * @return A new renderer for the the config in the widget
     */
    QgsRasterRenderer* renderer() override;

    /**
     * @brief Set the widget state from the given renderer.
     * @param r The renderer to take the state from.
     */
    void setFromRenderer( const QgsRasterRenderer* renderer );

  public slots:
    /**
     * @brief Set the altitude of the light source
     * @param altitude The altitude
     */
    void setAltitude( double altitude );

    /**
     * @brief Set the azimith of the light source.
     * @param azimuth The azimuth of the light source.
     */
    void setAzimuth( double azimuth );

    /**
     * @brief Set the Z factor of the result image.
     * @param zfactor The z factor.
     */
    void setZFactor( double zfactor );
};

#endif // QGSSINGLEBANDGRAYRENDERERWIDGET_H


