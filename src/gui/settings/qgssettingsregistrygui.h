/***************************************************************************
  qgssettingsregistrygui.h
  --------------------------------------
  Date                 : July 2021
  Copyright            : (C) 2021 by Damiano Lombardi
  Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSETTINGSREGISTRYGUI_H
#define QGSSETTINGSREGISTRYGUI_H

#include "qgis_gui.h"
#include "qgsabstractgeometry.h"
#include "qgssettingsregistry.h"

class QgsSettingsEntryBool;
class QgsSettingsEntryColor;
class QgsSettingsEntryDouble;
class QgsSettingsEntryInteger;
class QgsSettingsEntryString;
template<class T> class QgsSettingsEntryEnumFlag;

/**
 * \ingroup gui
 * \class QgsSettingsRegistryGui
 * \brief Used for settings introspection and collects all QgsSettingsEntry instances of GUI.
 *
 * \since QGIS 3.22
 */
Q_NOWARN_DEPRECATED_PUSH
class GUI_EXPORT QgsSettingsRegistryGui : public QgsSettingsRegistry
{
    Q_NOWARN_UNREACHABLE_POP
    // TODO QGIS 5 do not inherit QgsSettingsRegistry
  public:
    QgsSettingsRegistryGui();
    ~QgsSettingsRegistryGui() override;

#ifndef SIP_RUN
    //! Settings entry respect screen dpi
    static const QgsSettingsEntryBool *settingsRespectScreenDPI;

    //! Settings entry CAD floater active state
    static const QgsSettingsEntryBool *settingsCadFloaterActive;

    //! Settings entry raster histogram show markers
    static const QgsSettingsEntryBool *settingsRasterHistogramShowMarkers;

    //! Settings entry raster histogram zoom to min/max
    static const QgsSettingsEntryBool *settingsRasterHistogramZoomToMinMax;

    //! Settings entry raster histogram update style to min/max
    static const QgsSettingsEntryBool *settingsRasterHistogramUpdateStyleToMinMax;

    //! Settings entry raster histogram draw lines
    static const QgsSettingsEntryBool *settingsRasterHistogramDrawLines;

    //! Settings entry zoom factor
    static const QgsSettingsEntryDouble *settingsZoomFactor;

    //! Settings entry reverse wheel zoom
    static const QgsSettingsEntryBool *settingsReverseWheelZoom;

    //! Settings entry whether newly added layers are visible
    static const QgsSettingsEntryBool *settingsNewLayersVisible;

    //! Settings entry default raster color ramp palette name
    static const QgsSettingsEntryString *settingsRasterDefaultPalette;

    //! Settings entry message timeout in seconds
    static const QgsSettingsEntryInteger *settingsMessageTimeout;

    //! Settings entry whether anti-aliasing is enabled for rendering
    static const QgsSettingsEntryBool *settingsEnableAntiAliasing;

    //! Settings entry whether to use native color dialogs
    static const QgsSettingsEntryBool *settingsNativeColorDialogs;

    //! Settings entry whether to format layer names
    static const QgsSettingsEntryBool *settingsFormatLayerName;

    //! Settings entry whether to open sublayers in a group
    static const QgsSettingsEntryBool *settingsOpenSublayersInGroup;

    //! Settings entry map update interval in milliseconds
    static const QgsSettingsEntryInteger *settingsMapUpdateInterval;

    //! Settings entry default magnifier factor
    static const QgsSettingsEntryDouble *settingsMagnifierFactorDefault;

    //! Settings entry segmentation tolerance for curved geometries
    static const QgsSettingsEntryDouble *settingsSegmentationTolerance;

    //! Settings entry default measure tool color
    static const QgsSettingsEntryColor *settingsDefaultMeasureColor;

    //! Settings entry segmentation tolerance type for curved geometries
    static const QgsSettingsEntryEnumFlag<QgsAbstractGeometry::SegmentationToleranceType> *settingsSegmentationToleranceType;

#endif
};

#endif // QGSSETTINGSREGISTRYGUI_H
