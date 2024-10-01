/***************************************************************************
  qgssettingsregistrycore.h
  --------------------------------------
  Date                 : February 2021
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


#ifndef QGSSETTINGSREGISTRYCORE_H
#define QGSSETTINGSREGISTRYCORE_H

#include "qgis_core.h"
#include "qgssettingsregistry.h"

#include "qgis.h"

class QgsSettingsEntryBool;
class QgsSettingsEntryColor;
class QgsSettingsEntryDouble;
class QgsSettingsEntryInteger;
class QgsSettingsEntryInteger64;
class QgsSettingsEntryString;
class QgsSettingsEntryStringList;
template<class T> class QgsSettingsEntryEnumFlag;

/**
 * \ingroup core
 * \class QgsSettingsRegistryCore
 * \brief QgsSettingsRegistryCore is used for settings introspection and collects all
 * QgsSettingsEntry instances of core.
 *
 * \since QGIS 3.20
 */
Q_NOWARN_DEPRECATED_PUSH
class CORE_EXPORT QgsSettingsRegistryCore : public QgsSettingsRegistry
{
    Q_NOWARN_DEPRECATED_POP
    // TODO QGIS 4 do not inherit QgsSettingsRegistry
  public:

    QgsSettingsRegistryCore();
    virtual ~QgsSettingsRegistryCore();

#ifndef SIP_RUN
    //! Settings entry digitizing stream tolerance
    static const QgsSettingsEntryInteger *settingsDigitizingStreamTolerance;

    //! Settings entry digitizing line width
    static const QgsSettingsEntryInteger *settingsDigitizingLineWidth;

    //! Settings entry digitizing line color
    static const QgsSettingsEntryColor *settingsDigitizingLineColor;

    //! Settings entry digitizing line color alpha scale
    static const QgsSettingsEntryDouble *settingsDigitizingLineColorAlphaScale;

    //! Settings entry digitizing fill color
    static const QgsSettingsEntryColor *settingsDigitizingFillColor;

    //! Settings entry digitizing line ghost
    static const QgsSettingsEntryBool *settingsDigitizingLineGhost;

    //! Settings entry digitizing default z value
    static const QgsSettingsEntryDouble *settingsDigitizingDefaultZValue;

    //! Settings entry digitizing default m value
    static const QgsSettingsEntryDouble *settingsDigitizingDefaultMValue;

    //! Settings entry digitizing default snap enabled
    static const QgsSettingsEntryBool *settingsDigitizingDefaultSnapEnabled;

    //! Settings entry digitizing default snap type
    static const QgsSettingsEntryEnumFlag<Qgis::SnappingMode> *settingsDigitizingDefaultSnapMode;

    //! Settings entry digitizing default snap type
    static const QgsSettingsEntryEnumFlag<Qgis::SnappingType> *settingsDigitizingDefaultSnapType;

    //! Settings entry digitizing default snapping tolerance
    static const QgsSettingsEntryDouble *settingsDigitizingDefaultSnappingTolerance;

    //! Settings entry digitizing default snapping tolerance unit
    static const QgsSettingsEntryEnumFlag<Qgis::MapToolUnit> *settingsDigitizingDefaultSnappingToleranceUnit;

    //! Settings entry digitizing search radius vertex edit
    static const QgsSettingsEntryDouble *settingsDigitizingSearchRadiusVertexEdit;

    //! Settings entry digitizing search radius vertex edit unit
    static const QgsSettingsEntryEnumFlag<Qgis::MapToolUnit> *settingsDigitizingSearchRadiusVertexEditUnit;

    //! Settings entry digitizing snap color
    static const QgsSettingsEntryColor *settingsDigitizingSnapColor;

    //! Settings entry digitizing snap tooltip
    static const QgsSettingsEntryBool *settingsDigitizingSnapTooltip;

    //! Settings entry digitizing snap invisible feature
    static const QgsSettingsEntryBool *settingsDigitizingSnapInvisibleFeature;

    //! Settings entry digitizing marker only for selected
    static const QgsSettingsEntryBool *settingsDigitizingMarkerOnlyForSelected;

    //! Settings entry digitizing marker style
    static const QgsSettingsEntryString *settingsDigitizingMarkerStyle;

    //! Settings entry digitizing marker size mm
    static const QgsSettingsEntryDouble *settingsDigitizingMarkerSizeMm;

    //! Settings entry digitizing reuseLastValues
    static const QgsSettingsEntryBool *settingsDigitizingReuseLastValues;

    //! Settings entry digitizing disable enter attribute values dialog
    static const QgsSettingsEntryBool *settingsDigitizingDisableEnterAttributeValuesDialog;

    //! Settings entry digitizing validate geometries
    static const QgsSettingsEntryInteger *settingsDigitizingValidateGeometries;

    //! Settings entry digitizing offset join style
    static const QgsSettingsEntryEnumFlag<Qgis::JoinStyle> *settingsDigitizingOffsetJoinStyle;

    //! Settings entry digitizing offset quad seg
    static const QgsSettingsEntryInteger *settingsDigitizingOffsetQuadSeg;

    //! Settings entry digitizing offset miter limit
    static const QgsSettingsEntryDouble *settingsDigitizingOffsetMiterLimit;

    //! Settings entry digitizing convert to curve
    static const QgsSettingsEntryBool *settingsDigitizingConvertToCurve;

    //! Settings entry digitizing convert to curve angle tolerance
    static const QgsSettingsEntryDouble *settingsDigitizingConvertToCurveAngleTolerance;

    //! Settings entry digitizing convert to curve distance tolerance
    static const QgsSettingsEntryDouble *settingsDigitizingConvertToCurveDistanceTolerance;

    //! Settings entry digitizing offset cap style
    static const QgsSettingsEntryEnumFlag<Qgis::EndCapStyle> *settingsDigitizingOffsetCapStyle;

    //! Settings entry digitizing offset show advanced
    static const QgsSettingsEntryBool *settingsDigitizingOffsetShowAdvanced;

    //! Settings entry digitizing tracing max feature count
    static const QgsSettingsEntryInteger *settingsDigitizingTracingMaxFeatureCount;

    //! Settings entry path to GPSBabel executable.
    static const QgsSettingsEntryString *settingsGpsBabelPath;

    //! Settings entry show feature counts for newly added layers by default
    static const QgsSettingsEntryBool *settingsLayerTreeShowFeatureCountForNewLayers;

    //! Settings entry enable WMS tile prefetching.
    static const QgsSettingsEntryBool *settingsEnableWMSTilePrefetching;

    static const QgsSettingsEntryStringList *settingsMapScales;

    //! Settings entry maximum thread count used to load layer in parallel
    static const QgsSettingsEntryInteger *settingsLayerParallelLoadingMaxCount;

    //! Settings entry whether layer are loading in parallel
    static const QgsSettingsEntryBool *settingsLayerParallelLoading;

    //! Settings entry network cache directory
    static const QgsSettingsEntryString *settingsNetworkCacheDirectory;

    //! Settings entry network cache directory
    static const QgsSettingsEntryInteger64 *settingsNetworkCacheSize;

    //! Settings entry autosize columns by default when opening attribute table
    static const QgsSettingsEntryBool *settingsAutosizeAttributeTable;

  private:
    friend class QgsApplication;

    void migrateOldSettings();
    void backwardCompatibility();

#endif
};

#endif // QGSSETTINGSREGISTRYCORE_H
