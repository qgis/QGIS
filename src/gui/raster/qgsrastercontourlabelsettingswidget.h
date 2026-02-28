/***************************************************************************
    qgsrastercontourlabelsettingswidget.h
    -------------------------
    begin                : February 2026
    copyright            : (C) 2026 by the QGIS project
    email                : info at qgis dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERCONTOURLABELSETTINGSWIDGET_H
#define QGSRASTERCONTOURLABELSETTINGSWIDGET_H

#include "qgis_gui.h"
#include "qgslabelinggui.h"

#define SIP_NO_FILE

class QgsRasterLayer;
class QgsAbstractRasterLayerLabeling;
class QgsNumericFormat;
class QCheckBox;

// We don't want to expose this in the public API

/**
 * \class QgsRasterContourLabelSettingsWidget
 * \ingroup gui
 * \brief A widget for customizing settings for raster contour line labeling.
 *
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsRasterContourLabelSettingsWidget : public QgsLabelingGui
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsRasterContourLabelSettingsWidget, for configuring a raster \a layer contour labeling.
     */
    QgsRasterContourLabelSettingsWidget( QgsRasterLayer *layer, QgsMapCanvas *mapCanvas, QWidget *parent = nullptr );
    ~QgsRasterContourLabelSettingsWidget() override;

    /**
     * Sets the \a labeling settings to show in the widget.
     */
    void setLabeling( QgsAbstractRasterLayerLabeling *labeling );

    /**
     * Updates \a labeling by setting properties to match the current state of the widget.
     */
    void updateLabeling( QgsAbstractRasterLayerLabeling *labeling );

    void setLayer( QgsMapLayer *layer ) final;

  private slots:
    void changeNumberFormat();

  private:
    QCheckBox *mLabelIndexOnlyCheck = nullptr;
    std::unique_ptr<QgsNumericFormat> mNumberFormat;
    int mBlockChangesSignal = 0;
};

#endif // QGSRASTERCONTOURLABELSETTINGSWIDGET_H
