/***************************************************************************
                         qgsrastervectorfieldrendererwidget.h
                         ------------------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERVECTORFIELDRENDERERWIDGET_H
#define QGSRASTERVECTORFIELDRENDERERWIDGET_H

#include "ui_qgsrastervectorfieldrendererwidget.h"

#include <limits>

#include "qgis.h"
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsrasterrendererwidget.h"

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief Configuration widget for QgsRasterVectorFieldRenderer.
 *
 * Holds the pair of band selectors which are specific to a raster layer, and a
 * QgsVectorFieldSettingsWidget for the symbology itself, which is shared with the mesh layer.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsRasterVectorFieldRendererWidget : public QgsRasterRendererWidget, private Ui::QgsRasterVectorFieldRendererWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsRasterVectorFieldRendererWidget.
     * \param layer associated raster layer
     * \param extent current canvas extent
     */
    QgsRasterVectorFieldRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent = QgsRectangle() );

    //! Widget creation function (mainly for the use by the renderer registry)
    static QgsRasterRendererWidget *create( QgsRasterLayer *layer, const QgsRectangle &extent ) SIP_FACTORY { return new QgsRasterVectorFieldRendererWidget( layer, extent ); }

    QgsRasterRenderer *renderer() SIP_FACTORY override;

  private slots:
    void onBandChanged();
    void onSourceModeChanged();

  private:
    //! Returns the way the vectors are currently read from the bands
    Qgis::RasterVectorFieldSourceMode currentSourceMode() const;

    //! Shows the band selectors of the current source mode, and tells the symbology widget what it supports
    void updateSourceMode();

    /**
     * Estimates the magnitude range of the vectors from the statistics of the selected bands over
     * the current extent, and hands it to the symbology widget.
     *
     * The encoded directions carry no magnitude, and always use a range of 0 to 1.
     */
    void updateMagnitudeRange();

    double mMinimumMagnitude = std::numeric_limits<double>::quiet_NaN();
    double mMaximumMagnitude = std::numeric_limits<double>::quiet_NaN();
};

#endif // QGSRASTERVECTORFIELDRENDERERWIDGET_H
