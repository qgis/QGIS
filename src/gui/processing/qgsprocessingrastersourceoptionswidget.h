/***************************************************************************
                qgsprocessingrastersourceoptionswidget.h
                       --------------------------
    begin                : August 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGRASTERSOURCEOPTIONSWIDGET_H
#define QGSPROCESSINGRASTERSOURCEOPTIONSWIDGET_H

#include "ui_qgsprocessingrastersourceoptionsbase.h"

#include "qgis.h"
#include "qgspanelwidget.h"

#define SIP_NO_FILE

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief Widget for configuring advanced settings for a raster layer.
 * \note Not stable API
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsProcessingRasterSourceOptionsWidget : public QgsPanelWidget, private Ui::QgsProcessingRasterSourceOptionsBase
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsProcessingRasterSourceOptionsWidget, with the specified \a parent widget.
     */
    QgsProcessingRasterSourceOptionsWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets a reference scale for the raster.
     *
     * \param scale   Reference scale at which a raster (e.g., a WMS) should be requested or rendered.
     *
     * \see referenceScale()
     */
    void setReferenceScale( double scale );

    /**
     * Sets the resolution of the raster source (e.g., a WMS server).
     *
     * \param dpi   Dots per inch used by the raster source.
     *
     * \see dpi()
     */
    void setDpi( int dpi );

    /**
     * Reference scale at which a raster (e.g., a WMS) should be requested or rendered.
     *
     * \see setReferenceScale()
     */
    double referenceScale() const;

    /**
     * Resolution of the raster source (e.g., a WMS server).
     *
     * \see setDpi()
     */
    int dpi() const;

    /**
     * Sets the supported \a capabilities of the raster layer parameter and updates control visibility based on them.
     *
     * \param capabilities Capabilities to be set to the widget.
     * \since QGIS 4.0
     */
    void setWidgetParameterCapabilities( Qgis::RasterProcessingParameterCapabilities capabilities );

    /**
     * Returns flags containing the supported capabilities of the raster layer parameter.
     * \since QGIS 4.0
     */
    Qgis::RasterProcessingParameterCapabilities widgetParameterCapabilities() const;

  private:
    void updateControlVisibility();

    Qgis::RasterProcessingParameterCapabilities mCapabilities;
};

///@endcond

#endif // QGSPROCESSINGRASTERSOURCEOPTIONSWIDGET_H
