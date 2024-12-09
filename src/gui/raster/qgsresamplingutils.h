/***************************************************************************
  qgsresamplingutils.h
  --------------------
      begin                : 12/06/2020
      copyright            : (C) 2020 Even Rouault
      email                : even.rouault at spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRESAMPLINGUTILS_H
#define QGSRESAMPLINGUTILS_H

#include "qgis_gui.h"
#include <QObject>

class QgsRasterLayer;
class QComboBox;
class QDoubleSpinBox;
class QCheckBox;

/// @cond PRIVATE
#define SIP_NO_FILE

/**
 * Utility class shared by QgsRasterLayerProperties and
 * QgsRenderedRasterPropertiesWidget to deal with widgets related to
 * resampling settings.
 */
class GUI_EXPORT QgsResamplingUtils : public QObject
{
    Q_OBJECT

    QgsRasterLayer *mRasterLayer = nullptr;
    QComboBox *mZoomedInResamplingComboBox = nullptr;
    QComboBox *mZoomedOutResamplingComboBox = nullptr;
    QDoubleSpinBox *mMaximumOversamplingSpinBox = nullptr;
    QCheckBox *mCbEarlyResampling = nullptr;

    QgsResamplingUtils( const QgsResamplingUtils & ) = delete;
    QgsResamplingUtils &operator=( const QgsResamplingUtils & ) = delete;

  public:
    //! Constructor
    QgsResamplingUtils();

    //! Setup widgets (initialize combobox, etc.)
    void initWidgets( QgsRasterLayer *rasterLayer, QComboBox *zoomedInResamplingComboBox, QComboBox *zoomedOutResamplingComboBox, QDoubleSpinBox *maximumOversamplingSpinBox, QCheckBox *cbEarlyResampling );

    //! Synchronize widgets with QgsRasterLayer state
    void refreshWidgetsFromLayer();

    //! Synchronize QgsRasterLayer state from widgets
    void refreshLayerFromWidgets();

  private:
    void addExtraEarlyResamplingMethodsToCombos();
    void removeExtraEarlyResamplingMethodsFromCombos();
};

///@endcond PRIVATE

#endif // QGSRESAMPLINGUTILS_H
