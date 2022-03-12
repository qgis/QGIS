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
#include "qgis_sip.h"
#include "qgssettingsregistry.h"
#include "qgssettingsentry.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"

#include "qgis.h"
#include "qgsgeometry.h"
#include "qgsmaplayerproxymodel.h"

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
    static const inline QgsSettingsEntryInteger settingsDigitizingStreamTolerance = QgsSettingsEntryInteger( QStringLiteral( "stream_tolerance" ), QgsSettings::Prefix::QGIS_DIGITIZING, 2 );

    //! Settings entry digitizing line width
    static const inline QgsSettingsEntryInteger settingsDigitizingLineWidth = QgsSettingsEntryInteger( QStringLiteral( "line_width" ), QgsSettings::Prefix::QGIS_DIGITIZING, 1 );

    //! Settings entry digitizing line color red
    static const inline QgsSettingsEntryInteger settingsDigitizingLineColorRed = QgsSettingsEntryInteger( QStringLiteral( "line_color_red" ), QgsSettings::Prefix::QGIS_DIGITIZING, 255 );

    //! Settings entry digitizing line color green
    static const inline QgsSettingsEntryInteger settingsDigitizingLineColorGreen = QgsSettingsEntryInteger( QStringLiteral( "line_color_green" ), QgsSettings::Prefix::QGIS_DIGITIZING, 0 );

    //! Settings entry digitizing line color blue
    static const inline QgsSettingsEntryInteger settingsDigitizingLineColorBlue = QgsSettingsEntryInteger( QStringLiteral( "line_color_blue" ), QgsSettings::Prefix::QGIS_DIGITIZING, 0 );

    //! Settings entry digitizing line color alpha
    static const inline QgsSettingsEntryInteger settingsDigitizingLineColorAlpha = QgsSettingsEntryInteger( QStringLiteral( "line_color_alpha" ), QgsSettings::Prefix::QGIS_DIGITIZING, 200 );

    //! Settings entry digitizing line color alpha scale
    static const inline QgsSettingsEntryDouble settingsDigitizingLineColorAlphaScale = QgsSettingsEntryDouble( QStringLiteral( "line_color_alpha_scale" ), QgsSettings::Prefix::QGIS_DIGITIZING, 0.75 );

    //! Settings entry digitizing fill color red
    static const inline QgsSettingsEntryInteger settingsDigitizingFillColorRed = QgsSettingsEntryInteger( QStringLiteral( "fill_color_red" ), QgsSettings::Prefix::QGIS_DIGITIZING, 255 );

    //! Settings entry digitizing fill color green
    static const inline QgsSettingsEntryInteger settingsDigitizingFillColorGreen = QgsSettingsEntryInteger( QStringLiteral( "fill_color_green" ), QgsSettings::Prefix::QGIS_DIGITIZING, 0 );

    //! Settings entry digitizing fill color blue
    static const inline QgsSettingsEntryInteger settingsDigitizingFillColorBlue = QgsSettingsEntryInteger( QStringLiteral( "fill_color_blue" ), QgsSettings::Prefix::QGIS_DIGITIZING, 0 );

    //! Settings entry digitizing fill color alpha
    static const inline QgsSettingsEntryInteger settingsDigitizingFillColorAlpha = QgsSettingsEntryInteger( QStringLiteral( "fill_color_alpha" ), QgsSettings::Prefix::QGIS_DIGITIZING, 30 );

    //! Settings entry digitizing line ghost
    static const inline QgsSettingsEntryBool settingsDigitizingLineGhost = QgsSettingsEntryBool( QStringLiteral( "line_ghost" ), QgsSettings::Prefix::QGIS_DIGITIZING, false );

    //! Settings entry digitizing default z value
    static const inline QgsSettingsEntryDouble settingsDigitizingDefaultZValue = QgsSettingsEntryDouble( QStringLiteral( "default_z_value" ), QgsSettings::Prefix::QGIS_DIGITIZING, Qgis::DEFAULT_Z_COORDINATE );

    //! Settings entry digitizing default m value
    static const inline QgsSettingsEntryDouble settingsDigitizingDefaultMValue = QgsSettingsEntryDouble( QStringLiteral( "default_m_value" ), QgsSettings::Prefix::QGIS_DIGITIZING, Qgis::DEFAULT_M_COORDINATE );

    //! Settings entry digitizing default snap enabled
    static const inline QgsSettingsEntryBool settingsDigitizingDefaultSnapEnabled = QgsSettingsEntryBool( QStringLiteral( "default_snap_enabled" ), QgsSettings::Prefix::QGIS_DIGITIZING,  false );

    //! Settings entry digitizing default snap type
    static const inline QgsSettingsEntryEnumFlag<Qgis::SnappingMode> settingsDigitizingDefaultSnapMode = QgsSettingsEntryEnumFlag<Qgis::SnappingMode>( QStringLiteral( "default_snap_mode" ), QgsSettings::Prefix::QGIS_DIGITIZING, Qgis::SnappingMode::AllLayers );

    //! Settings entry digitizing default snap type
    static const inline QgsSettingsEntryEnumFlag<Qgis::SnappingType> settingsDigitizingDefaultSnapType = QgsSettingsEntryEnumFlag<Qgis::SnappingType>( QStringLiteral( "default_snap_type" ), QgsSettings::Prefix::QGIS_DIGITIZING, Qgis::SnappingType::Vertex );

    //! Settings entry digitizing default snapping tolerance
    static const inline QgsSettingsEntryDouble settingsDigitizingDefaultSnappingTolerance = QgsSettingsEntryDouble( QStringLiteral( "default_snapping_tolerance" ), QgsSettings::Prefix::QGIS_DIGITIZING, Qgis::DEFAULT_SNAP_TOLERANCE );

    //! Settings entry digitizing default snapping tolerance unit
    static const inline QgsSettingsEntryEnumFlag<QgsTolerance::UnitType> settingsDigitizingDefaultSnappingToleranceUnit = QgsSettingsEntryEnumFlag<QgsTolerance::UnitType>( QStringLiteral( "default_snapping_tolerance_unit" ), QgsSettings::Prefix::QGIS_DIGITIZING, Qgis::DEFAULT_SNAP_UNITS );

    //! Settings entry digitizing search radius vertex edit
    static const inline QgsSettingsEntryDouble settingsDigitizingSearchRadiusVertexEdit = QgsSettingsEntryDouble( QStringLiteral( "search_radius_vertex_edit" ), QgsSettings::Prefix::QGIS_DIGITIZING, 10 );

    //! Settings entry digitizing search radius vertex edit unit
    static const inline QgsSettingsEntryEnumFlag<QgsTolerance::UnitType> settingsDigitizingSearchRadiusVertexEditUnit = QgsSettingsEntryEnumFlag<QgsTolerance::UnitType>( QStringLiteral( "search_radius_vertex_edit_unit" ), QgsSettings::Prefix::QGIS_DIGITIZING, QgsTolerance::Pixels );

    //! Settings entry digitizing snap color
    static const inline QgsSettingsEntryColor settingsDigitizingSnapColor = QgsSettingsEntryColor( QStringLiteral( "snap_color" ), QgsSettings::Prefix::QGIS_DIGITIZING, QColor( Qt::magenta ) );

    //! Settings entry digitizing snap tooltip
    static const inline QgsSettingsEntryBool settingsDigitizingSnapTooltip = QgsSettingsEntryBool( QStringLiteral( "snap_tooltip" ), QgsSettings::Prefix::QGIS_DIGITIZING, false );

    //! Settings entry digitizing snap invisible feature
    static const inline QgsSettingsEntryBool settingsDigitizingSnapInvisibleFeature = QgsSettingsEntryBool( QStringLiteral( "snap_invisible_feature" ), QgsSettings::Prefix::QGIS_DIGITIZING, false );

    //! Settings entry digitizing marker only for selected
    static const inline QgsSettingsEntryBool settingsDigitizingMarkerOnlyForSelected = QgsSettingsEntryBool( QStringLiteral( "marker_only_for_selected" ), QgsSettings::Prefix::QGIS_DIGITIZING, true );

    //! Settings entry digitizing marker style
    static const inline QgsSettingsEntryString settingsDigitizingMarkerStyle = QgsSettingsEntryString( QStringLiteral( "marker_style" ), QgsSettings::Prefix::QGIS_DIGITIZING, "Cross" );

    //! Settings entry digitizing marker size mm
    static const inline QgsSettingsEntryDouble settingsDigitizingMarkerSizeMm = QgsSettingsEntryDouble( QStringLiteral( "marker_size_mm" ), QgsSettings::Prefix::QGIS_DIGITIZING, 2.0 );

    //! Settings entry digitizing reuseLastValues
    static const inline QgsSettingsEntryBool settingsDigitizingReuseLastValues = QgsSettingsEntryBool( QStringLiteral( "reuseLastValues" ), QgsSettings::Prefix::QGIS_DIGITIZING, false );

    //! Settings entry digitizing disable enter attribute values dialog
    static const inline QgsSettingsEntryBool settingsDigitizingDisableEnterAttributeValuesDialog = QgsSettingsEntryBool( QStringLiteral( "disable_enter_attribute_values_dialog" ), QgsSettings::Prefix::QGIS_DIGITIZING, false );

    //! Settings entry digitizing validate geometries
    static const inline QgsSettingsEntryInteger settingsDigitizingValidateGeometries = QgsSettingsEntryInteger( QStringLiteral( "validate_geometries" ), QgsSettings::Prefix::QGIS_DIGITIZING, 1 );

    //! Settings entry digitizing offset join style
    static const inline QgsSettingsEntryEnumFlag<Qgis::JoinStyle> settingsDigitizingOffsetJoinStyle = QgsSettingsEntryEnumFlag<Qgis::JoinStyle>( QStringLiteral( "offset_join_style" ), QgsSettings::Prefix::QGIS_DIGITIZING, Qgis::JoinStyle::Round );

    //! Settings entry digitizing offset quad seg
    static const inline QgsSettingsEntryInteger settingsDigitizingOffsetQuadSeg = QgsSettingsEntryInteger( QStringLiteral( "offset_quad_seg" ), QgsSettings::Prefix::QGIS_DIGITIZING, 8 );

    //! Settings entry digitizing offset miter limit
    static const inline QgsSettingsEntryDouble settingsDigitizingOffsetMiterLimit = QgsSettingsEntryDouble( QStringLiteral( "offset_miter_limit" ), QgsSettings::Prefix::QGIS_DIGITIZING, 5.0 );

    //! Settings entry digitizing convert to curve
    static const inline QgsSettingsEntryBool settingsDigitizingConvertToCurve = QgsSettingsEntryBool( QStringLiteral( "convert_to_curve" ), QgsSettings::Prefix::QGIS_DIGITIZING, false );

    //! Settings entry digitizing convert to curve angle tolerance
    static const inline QgsSettingsEntryDouble settingsDigitizingConvertToCurveAngleTolerance = QgsSettingsEntryDouble( QStringLiteral( "convert_to_curve_angle_tolerance" ), QgsSettings::Prefix::QGIS_DIGITIZING, 1e-6 );

    //! Settings entry digitizing convert to curve distance tolerance
    static const inline QgsSettingsEntryDouble settingsDigitizingConvertToCurveDistanceTolerance = QgsSettingsEntryDouble( QStringLiteral( "convert_to_curve_distance_tolerance" ), QgsSettings::Prefix::QGIS_DIGITIZING, 1e-6 );

    //! Settings entry digitizing offset cap style
    static const inline QgsSettingsEntryEnumFlag<Qgis::EndCapStyle> settingsDigitizingOffsetCapStyle = QgsSettingsEntryEnumFlag<Qgis::EndCapStyle>( QStringLiteral( "offset_cap_style" ), QgsSettings::Prefix::QGIS_DIGITIZING,  Qgis::EndCapStyle::Round );

    //! Settings entry digitizing offset show advanced
    static const inline QgsSettingsEntryBool settingsDigitizingOffsetShowAdvanced = QgsSettingsEntryBool( QStringLiteral( "offset_show_advanced" ), QgsSettings::Prefix::QGIS_DIGITIZING, false );

    //! Settings entry digitizing tracing max feature count
    static const inline QgsSettingsEntryInteger settingsDigitizingTracingMaxFeatureCount = QgsSettingsEntryInteger( QStringLiteral( "tracing_max_feature_count" ), QgsSettings::Prefix::QGIS_DIGITIZING, 10000 );

    //! Settings entry path to GPSBabel executable.
    static const inline QgsSettingsEntryString settingsGpsBabelPath = QgsSettingsEntryString( QStringLiteral( "gpsbabelPath" ), QgsSettings::Prefix::GPS, QStringLiteral( "gpsbabel" ) );
#endif

};

#endif // QGSSETTINGSREGISTRYCORE_H
