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

#include "ui_qgshillshaderendererwidget.h"
#include "qgis_sip.h"

#include <QDoubleSpinBox>

#include "qgsrasterrendererwidget.h"
#include "qgis_gui.h"

/**
 * \brief Renderer widget for the hill shade renderer.
 * \ingroup gui
 * \since QGIS 2.16
 */
class GUI_EXPORT QgsHillshadeRendererWidget: public QgsRasterRendererWidget, private Ui::QgsHillShadeWidget
{
    Q_OBJECT
  public:

    /**
     * \brief Renderer widget for the hill shade renderer.
     * \param layer The layer attached for this widget.
     * \param extent The current extent.
     */
    QgsHillshadeRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent = QgsRectangle() );

    /**
      * Factory method to create the renderer for this type.
      */
    static QgsRasterRendererWidget *create( QgsRasterLayer *layer, const QgsRectangle &extent ) SIP_FACTORY { return new QgsHillshadeRendererWidget( layer, extent ); }

    QgsRasterRenderer *renderer() override SIP_FACTORY;

    /**
     * \brief Set the widget state from the given renderer.
     * \param renderer The renderer to take the state from.
     */
    void setFromRenderer( const QgsRasterRenderer *renderer );

    /**
     * Returns the direction of the light over the raster between 0-360.
     * \see setAzimuth()
     */
    double azimuth() const;

    /**
     * Returns the angle of the light source over the raster.
     * \see setAltitude()
     */
    double altitude()  const;

    /**
     * Returns the Z scaling factor.
     * \see setZFactor()
     */
    double zFactor()  const;

    /**
     * Returns TRUE if the renderer should use the multi-directional hillshade algorithm.
     * \see setMultiDirectional()
     */
    bool multiDirectional() const;

  public slots:

    /**
     * \brief Set the altitude of the light source
     * \param altitude the altitude
     * \see altitude()
     */
    void setAltitude( double altitude );

    /**
     * \brief Set the azimuth of the light source.
     * \param azimuth The azimuth of the light source, between 0 and 360.0
     * \see azimuth()
     */
    void setAzimuth( double azimuth );

    /**
     * \brief Set the Z scaling factor of the result image.
     * \param zfactor The z factor
     * \see zFactor()
     */
    void setZFactor( double zfactor );

    /**
     * Sets whether to render using a multi-directional hillshade algorithm.
     * \param isMultiDirectional set to TRUE to use multi directional rendering
     * \see multiDirectional()
     */
    void setMultiDirectional( bool isMultiDirectional );

  private slots:
    void mLightAzimuth_updated( double value );
    void mLightAzimuthDial_updated( int value );
};

#endif // QGSSINGLEBANDGRAYRENDERERWIDGET_H


