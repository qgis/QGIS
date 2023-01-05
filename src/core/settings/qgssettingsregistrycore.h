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
#include "qgssettingsentry.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettings.h"

#include "qgis.h"


/**
 * \ingroup core
 * \class QgsSettingsRegistryCore
 * \brief QgsSettingsRegistryCore is used for settings introspection and collects all
 * QgsSettingsEntry instances of core.
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsRegistryCore : public QgsSettingsRegistry
{
  public:

    /**
      * Constructor for QgsSettingsRegistryCore.
      */
    QgsSettingsRegistryCore();

    /**
     * Destructor for QgsSettingsRegistryCore.
     */
    virtual ~QgsSettingsRegistryCore();

#ifndef SIP_RUN
    //! Settings entry digitizing stream tolerance
    static inline QgsSettingsEntryInteger *settingsDigitizingStreamTolerance = new QgsSettingsEntryInteger( QStringLiteral( "stream_tolerance" ), QgsSettings::sTreeDigitizing, 2 );

    //! Settings entry digitizing line width
    static inline QgsSettingsEntryInteger *settingsDigitizingLineWidth = new QgsSettingsEntryInteger( QStringLiteral( "line_width" ), QgsSettings::sTreeDigitizing, 1 );

    //! Settings entry digitizing line color
    static inline QgsSettingsEntryColor *settingsDigitizingLineColor = new QgsSettingsEntryColor( QStringLiteral( "line_color" ), QgsSettings::sTreeDigitizing, QColor( 255, 0, 0, 200 ) );

    //! Settings entry digitizing line color alpha scale
    static inline QgsSettingsEntryDouble *settingsDigitizingLineColorAlphaScale = new QgsSettingsEntryDouble( QStringLiteral( "line_color_alpha_scale" ), QgsSettings::sTreeDigitizing, 0.75 );

    //! Settings entry digitizing fill color red
    static inline QgsSettingsEntryInteger *settingsDigitizingFillColorRed = new QgsSettingsEntryInteger( QStringLiteral( "fill_color_red" ), QgsSettings::sTreeDigitizing, 255 );

    //! Settings entry digitizing fill color green
    static inline QgsSettingsEntryInteger *settingsDigitizingFillColorGreen = new QgsSettingsEntryInteger( QStringLiteral( "fill_color_green" ), QgsSettings::sTreeDigitizing, 0 );

    //! Settings entry digitizing fill color blue
    static inline QgsSettingsEntryInteger *settingsDigitizingFillColorBlue = new QgsSettingsEntryInteger( QStringLiteral( "fill_color_blue" ), QgsSettings::sTreeDigitizing, 0 );

    //! Settings entry digitizing fill color alpha
    static inline QgsSettingsEntryInteger *settingsDigitizingFillColorAlpha = new QgsSettingsEntryInteger( QStringLiteral( "fill_color_alpha" ), QgsSettings::sTreeDigitizing, 30 );

    //! Settings entry digitizing line ghost
    static inline QgsSettingsEntryBool *settingsDigitizingLineGhost = new QgsSettingsEntryBool( QStringLiteral( "line_ghost" ), QgsSettings::sTreeDigitizing, false );

    //! Settings entry digitizing default z value
    static inline QgsSettingsEntryDouble *settingsDigitizingDefaultZValue = new QgsSettingsEntryDouble( QStringLiteral( "default_z_value" ), QgsSettings::sTreeDigitizing, Qgis::DEFAULT_Z_COORDINATE );

    //! Settings entry digitizing default m value
    static inline QgsSettingsEntryDouble *settingsDigitizingDefaultMValue = new QgsSettingsEntryDouble( QStringLiteral( "default_m_value" ), QgsSettings::sTreeDigitizing, Qgis::DEFAULT_M_COORDINATE );

    //! Settings entry digitizing default snap enabled
    static inline QgsSettingsEntryBool *settingsDigitizingDefaultSnapEnabled = new QgsSettingsEntryBool( QStringLiteral( "default_snap_enabled" ), QgsSettings::sTreeDigitizing,  false );

    //! Settings entry digitizing default snap type
    static const inline QgsSettingsEntryEnumFlag<Qgis::SnappingMode> settingsDigitizingDefaultSnapMode = QgsSettingsEntryEnumFlag<Qgis::SnappingMode>( QStringLiteral( "default_snap_mode" ), QgsSettings::sTreeDigitizing, Qgis::SnappingMode::AllLayers );

    //! Settings entry digitizing default snap type
    static const inline QgsSettingsEntryEnumFlag<Qgis::SnappingType> settingsDigitizingDefaultSnapType = QgsSettingsEntryEnumFlag<Qgis::SnappingType>( QStringLiteral( "default_snap_type" ), QgsSettings::sTreeDigitizing, Qgis::SnappingType::Vertex );

    //! Settings entry digitizing default snapping tolerance
    static inline QgsSettingsEntryDouble *settingsDigitizingDefaultSnappingTolerance = new QgsSettingsEntryDouble( QStringLiteral( "default_snapping_tolerance" ), QgsSettings::sTreeDigitizing, Qgis::DEFAULT_SNAP_TOLERANCE );

    //! Settings entry digitizing default snapping tolerance unit
    static const inline QgsSettingsEntryEnumFlag<QgsTolerance::UnitType> settingsDigitizingDefaultSnappingToleranceUnit = QgsSettingsEntryEnumFlag<QgsTolerance::UnitType>( QStringLiteral( "default_snapping_tolerance_unit" ), QgsSettings::sTreeDigitizing, Qgis::DEFAULT_SNAP_UNITS );

    //! Settings entry digitizing search radius vertex edit
    static inline QgsSettingsEntryDouble *settingsDigitizingSearchRadiusVertexEdit = new QgsSettingsEntryDouble( QStringLiteral( "search_radius_vertex_edit" ), QgsSettings::sTreeDigitizing, 10 );

    //! Settings entry digitizing search radius vertex edit unit
    static const inline QgsSettingsEntryEnumFlag<QgsTolerance::UnitType> settingsDigitizingSearchRadiusVertexEditUnit = QgsSettingsEntryEnumFlag<QgsTolerance::UnitType>( QStringLiteral( "search_radius_vertex_edit_unit" ), QgsSettings::sTreeDigitizing, QgsTolerance::Pixels );

    //! Settings entry digitizing snap color
    static inline QgsSettingsEntryColor *settingsDigitizingSnapColor = new QgsSettingsEntryColor( QStringLiteral( "snap_color" ), QgsSettings::sTreeDigitizing, QColor( Qt::magenta ) );

    //! Settings entry digitizing snap tooltip
    static inline QgsSettingsEntryBool *settingsDigitizingSnapTooltip = new QgsSettingsEntryBool( QStringLiteral( "snap_tooltip" ), QgsSettings::sTreeDigitizing, false );

    //! Settings entry digitizing snap invisible feature
    static inline QgsSettingsEntryBool *settingsDigitizingSnapInvisibleFeature = new QgsSettingsEntryBool( QStringLiteral( "snap_invisible_feature" ), QgsSettings::sTreeDigitizing, false );

    //! Settings entry digitizing marker only for selected
    static inline QgsSettingsEntryBool *settingsDigitizingMarkerOnlyForSelected = new QgsSettingsEntryBool( QStringLiteral( "marker_only_for_selected" ), QgsSettings::sTreeDigitizing, true );

    //! Settings entry digitizing marker style
    static inline QgsSettingsEntryString *settingsDigitizingMarkerStyle = new QgsSettingsEntryString( QStringLiteral( "marker_style" ), QgsSettings::sTreeDigitizing, "Cross" );

    //! Settings entry digitizing marker size mm
    static inline QgsSettingsEntryDouble *settingsDigitizingMarkerSizeMm = new QgsSettingsEntryDouble( QStringLiteral( "marker_size_mm" ), QgsSettings::sTreeDigitizing, 2.0 );

    //! Settings entry digitizing reuseLastValues
    static inline QgsSettingsEntryBool *settingsDigitizingReuseLastValues = new QgsSettingsEntryBool( QStringLiteral( "reuseLastValues" ), QgsSettings::sTreeDigitizing, false );

    //! Settings entry digitizing disable enter attribute values dialog
    static inline QgsSettingsEntryBool *settingsDigitizingDisableEnterAttributeValuesDialog = new QgsSettingsEntryBool( QStringLiteral( "disable_enter_attribute_values_dialog" ), QgsSettings::sTreeDigitizing, false );

    //! Settings entry digitizing validate geometries
    static inline QgsSettingsEntryInteger *settingsDigitizingValidateGeometries = new QgsSettingsEntryInteger( QStringLiteral( "validate_geometries" ), QgsSettings::sTreeDigitizing, 1 );

    //! Settings entry digitizing offset join style
    static const inline QgsSettingsEntryEnumFlag<Qgis::JoinStyle> settingsDigitizingOffsetJoinStyle = QgsSettingsEntryEnumFlag<Qgis::JoinStyle>( QStringLiteral( "offset_join_style" ), QgsSettings::sTreeDigitizing, Qgis::JoinStyle::Round );

    //! Settings entry digitizing offset quad seg
    static inline QgsSettingsEntryInteger *settingsDigitizingOffsetQuadSeg = new QgsSettingsEntryInteger( QStringLiteral( "offset_quad_seg" ), QgsSettings::sTreeDigitizing, 8 );

    //! Settings entry digitizing offset miter limit
    static inline QgsSettingsEntryDouble *settingsDigitizingOffsetMiterLimit = new QgsSettingsEntryDouble( QStringLiteral( "offset_miter_limit" ), QgsSettings::sTreeDigitizing, 5.0 );

    //! Settings entry digitizing convert to curve
    static inline QgsSettingsEntryBool *settingsDigitizingConvertToCurve = new QgsSettingsEntryBool( QStringLiteral( "convert_to_curve" ), QgsSettings::sTreeDigitizing, false );

    //! Settings entry digitizing convert to curve angle tolerance
    static inline QgsSettingsEntryDouble *settingsDigitizingConvertToCurveAngleTolerance = new QgsSettingsEntryDouble( QStringLiteral( "convert_to_curve_angle_tolerance" ), QgsSettings::sTreeDigitizing, 1e-6 );

    //! Settings entry digitizing convert to curve distance tolerance
    static inline QgsSettingsEntryDouble *settingsDigitizingConvertToCurveDistanceTolerance = new QgsSettingsEntryDouble( QStringLiteral( "convert_to_curve_distance_tolerance" ), QgsSettings::sTreeDigitizing, 1e-6 );

    //! Settings entry digitizing offset cap style
    static const inline QgsSettingsEntryEnumFlag<Qgis::EndCapStyle> settingsDigitizingOffsetCapStyle = QgsSettingsEntryEnumFlag<Qgis::EndCapStyle>( QStringLiteral( "offset_cap_style" ), QgsSettings::sTreeDigitizing,  Qgis::EndCapStyle::Round );

    //! Settings entry digitizing offset show advanced
    static inline QgsSettingsEntryBool *settingsDigitizingOffsetShowAdvanced = new QgsSettingsEntryBool( QStringLiteral( "offset_show_advanced" ), QgsSettings::sTreeDigitizing, false );

    //! Settings entry digitizing tracing max feature count
    static inline QgsSettingsEntryInteger *settingsDigitizingTracingMaxFeatureCount = new QgsSettingsEntryInteger( QStringLiteral( "tracing_max_feature_count" ), QgsSettings::sTreeDigitizing, 10000 );

    //! Settings entry path to GPSBabel executable.
    static inline QgsSettingsEntryString *settingsGpsBabelPath = new QgsSettingsEntryString( QStringLiteral( "gpsbabelPath" ), QgsSettings::sTreeGps, QStringLiteral( "gpsbabel" ) );

    //! Settings entry show feature counts for newly added layers by default
    static inline QgsSettingsEntryBool *settingsLayerTreeShowFeatureCountForNewLayers = new QgsSettingsEntryBool( QStringLiteral( "show-feature-count-for-new-layers" ), QgsSettings::sTreeLayerTree, false, QStringLiteral( "If true, feature counts will be shown in the layer tree for all newly added layers." ) );

    //! Settings entry enable WMS tile prefetching.
    static inline QgsSettingsEntryBool *settingsEnableWMSTilePrefetching = new QgsSettingsEntryBool( QStringLiteral( "enable_wms_tile_prefetch" ), QgsSettings::sTreeWms, false, QStringLiteral( "Whether to include WMS layers when rendering tiles adjacent to the visible map area" ) );

  private:
    void migrateOldSettings();
    void backwardCompatibility();

#endif
};

#endif // QGSSETTINGSREGISTRYCORE_H
