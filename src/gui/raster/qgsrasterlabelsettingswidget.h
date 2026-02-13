/***************************************************************************
    qgsrasterlabelsettingswidget.h
    -------------------------
    begin                : December 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERLABELSETTINGSWIDGET_H
#define QGSRASTERLABELSETTINGSWIDGET_H

#include "qgis_gui.h"
#include "qgslabelinggui.h"

class QgsRasterLayer;
class QgsAbstractRasterLayerLabeling;
class QgsRasterBandComboBox;
class QgsNumericFormat;

// We don't want to expose this in the public API
#define SIP_NO_FILE

/**
 * \class QgsRasterLabelSettingsWidget
 * \ingroup gui
 * \brief A widget for customizing settings for raster layer labeling.
 *
 * \since QGIS 3.42
 */
class GUI_EXPORT QgsRasterLabelSettingsWidget : public QgsLabelingGui
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsRasterLabelSettingsWidget, for configuring a raster \a layer labeling.
     */
    QgsRasterLabelSettingsWidget( QgsRasterLayer *layer, QgsMapCanvas *mapCanvas, QWidget *parent = nullptr );
    ~QgsRasterLabelSettingsWidget() override;

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
    QgsRasterBandComboBox *mBandCombo = nullptr;
    std::unique_ptr<QgsNumericFormat> mNumberFormat;
    int mBlockChangesSignal = 0;

    QgsSpinBox *mResampleOverSpin = nullptr;
    QComboBox *mResampleMethodComboBox = nullptr;
};

#endif // QGSRASTERLABELSETTINGSWIDGET_H
