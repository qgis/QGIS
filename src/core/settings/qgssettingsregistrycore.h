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

#ifdef _MSC_VER
#include "qgsunittypes.h"

template class CORE_EXPORT QgsSettingsEntryEnumFlag<Qgis::SnappingType> SIP_SKIP;
template class CORE_EXPORT QgsSettingsEntryEnumFlag<QgsTolerance::UnitType> SIP_SKIP;
template class CORE_EXPORT QgsSettingsEntryEnumFlag<Qgis::JoinStyle> SIP_SKIP;
template class CORE_EXPORT QgsSettingsEntryEnumFlag<Qgis::EndCapStyle> SIP_SKIP;
template class CORE_EXPORT QgsSettingsEntryEnumFlag<QgsUnitTypes::LayoutUnit> SIP_SKIP;
template class CORE_EXPORT QgsSettingsEntryEnumFlag< class QFlags<enum QgsMapLayerProxyModel::Filter> > SIP_SKIP;
#endif

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
    static const inline QgsSettingsEntryInteger settingsDigitizingStreamTolerance = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/stream_tolerance" ), QgsSettings::NoSection, 2 );

    //! Settings entry digitizing line width
    static const inline QgsSettingsEntryInteger settingsDigitizingLineWidth = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/line_width" ), QgsSettings::NoSection, 1 );

    //! Settings entry digitizing line color red
    static const inline QgsSettingsEntryInteger settingsDigitizingLineColorRed = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/line_color_red" ), QgsSettings::NoSection, 255 );

    //! Settings entry digitizing line color green
    static const inline QgsSettingsEntryInteger settingsDigitizingLineColorGreen = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/line_color_green" ), QgsSettings::NoSection, 0 );

    //! Settings entry digitizing line color blue
    static const inline QgsSettingsEntryInteger settingsDigitizingLineColorBlue = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/line_color_blue" ), QgsSettings::NoSection, 0 );

    //! Settings entry digitizing line color alpha
    static const inline QgsSettingsEntryInteger settingsDigitizingLineColorAlpha = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/line_color_alpha" ), QgsSettings::NoSection, 200 );

    //! Settings entry digitizing line color alpha scale
    static const inline QgsSettingsEntryDouble settingsDigitizingLineColorAlphaScale = QgsSettingsEntryDouble( QStringLiteral( "/qgis/digitizing/line_color_alpha_scale" ), QgsSettings::NoSection, 0.75 );

    //! Settings entry digitizing fill color red
    static const inline QgsSettingsEntryInteger settingsDigitizingFillColorRed = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/fill_color_red" ), QgsSettings::NoSection, 255 );

    //! Settings entry digitizing fill color green
    static const inline QgsSettingsEntryInteger settingsDigitizingFillColorGreen = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/fill_color_green" ), QgsSettings::NoSection, 0 );

    //! Settings entry digitizing fill color blue
    static const inline QgsSettingsEntryInteger settingsDigitizingFillColorBlue = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/fill_color_blue" ), QgsSettings::NoSection, 0 );

    //! Settings entry digitizing fill color alpha
    static const inline QgsSettingsEntryInteger settingsDigitizingFillColorAlpha = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/fill_color_alpha" ), QgsSettings::NoSection, 30 );

    //! Settings entry digitizing line ghost
    static const inline QgsSettingsEntryBool settingsDigitizingLineGhost = QgsSettingsEntryBool( QStringLiteral( "/qgis/digitizing/line_ghost" ), QgsSettings::NoSection, false );

    //! Settings entry digitizing default z value
    static const inline QgsSettingsEntryDouble settingsDigitizingDefaultZValue = QgsSettingsEntryDouble( QStringLiteral( "/qgis/digitizing/default_z_value" ), QgsSettings::NoSection, Qgis::DEFAULT_Z_COORDINATE );

    //! Settings entry digitizing default m value
    static const inline QgsSettingsEntryDouble settingsDigitizingDefaultMValue = QgsSettingsEntryDouble( QStringLiteral( "/qgis/digitizing/default_m_value" ), QgsSettings::NoSection, Qgis::DEFAULT_M_COORDINATE );

    //! Settings entry digitizing default snap enabled
    static const inline QgsSettingsEntryBool settingsDigitizingDefaultSnapEnabled = QgsSettingsEntryBool( QStringLiteral( "/qgis/digitizing/default_snap_enabled" ), QgsSettings::NoSection,  false );

    //! Settings entry digitizing default snap type
    static const inline QgsSettingsEntryEnumFlag<Qgis::SnappingMode> settingsDigitizingDefaultSnapMode = QgsSettingsEntryEnumFlag<Qgis::SnappingMode>( QStringLiteral( "/qgis/digitizing/default_snap_mode" ), QgsSettings::NoSection, Qgis::SnappingMode::AllLayers );

    //! Settings entry digitizing default snap type
    static const inline QgsSettingsEntryEnumFlag<Qgis::SnappingType> settingsDigitizingDefaultSnapType = QgsSettingsEntryEnumFlag<Qgis::SnappingType>( QStringLiteral( "/qgis/digitizing/default_snap_type" ), QgsSettings::NoSection, Qgis::SnappingType::Vertex );

    //! Settings entry digitizing default snapping tolerance
    static const inline QgsSettingsEntryDouble settingsDigitizingDefaultSnappingTolerance = QgsSettingsEntryDouble( QStringLiteral( "/qgis/digitizing/default_snapping_tolerance" ), QgsSettings::NoSection, Qgis::DEFAULT_SNAP_TOLERANCE );

    //! Settings entry digitizing default snapping tolerance unit
    static const inline QgsSettingsEntryEnumFlag<QgsTolerance::UnitType> settingsDigitizingDefaultSnappingToleranceUnit = QgsSettingsEntryEnumFlag<QgsTolerance::UnitType>( QStringLiteral( "/qgis/digitizing/default_snapping_tolerance_unit" ), QgsSettings::NoSection, Qgis::DEFAULT_SNAP_UNITS );

    //! Settings entry digitizing search radius vertex edit
    static const inline QgsSettingsEntryDouble settingsDigitizingSearchRadiusVertexEdit = QgsSettingsEntryDouble( QStringLiteral( "/qgis/digitizing/search_radius_vertex_edit" ), QgsSettings::NoSection, 10 );

    //! Settings entry digitizing search radius vertex edit unit
    static const inline QgsSettingsEntryEnumFlag<QgsTolerance::UnitType> settingsDigitizingSearchRadiusVertexEditUnit = QgsSettingsEntryEnumFlag<QgsTolerance::UnitType>( QStringLiteral( "/qgis/digitizing/search_radius_vertex_edit_unit" ), QgsSettings::NoSection, QgsTolerance::Pixels );

    //! Settings entry digitizing snap color
    static const inline QgsSettingsEntryColor settingsDigitizingSnapColor = QgsSettingsEntryColor( QStringLiteral( "/qgis/digitizing/snap_color" ), QgsSettings::NoSection, QColor( Qt::magenta ) );

    //! Settings entry digitizing snap tooltip
    static const inline QgsSettingsEntryBool settingsDigitizingSnapTooltip = QgsSettingsEntryBool( QStringLiteral( "/qgis/digitizing/snap_tooltip" ), QgsSettings::NoSection, false );

    //! Settings entry digitizing snap invisible feature
    static const inline QgsSettingsEntryBool settingsDigitizingSnapInvisibleFeature = QgsSettingsEntryBool( QStringLiteral( "/qgis/digitizing/snap_invisible_feature" ), QgsSettings::NoSection, false );

    //! Settings entry digitizing marker only for selected
    static const inline QgsSettingsEntryBool settingsDigitizingMarkerOnlyForSelected = QgsSettingsEntryBool( QStringLiteral( "/qgis/digitizing/marker_only_for_selected" ), QgsSettings::NoSection, true );

    //! Settings entry digitizing marker style
    static const inline QgsSettingsEntryString settingsDigitizingMarkerStyle = QgsSettingsEntryString( QStringLiteral( "/qgis/digitizing/marker_style" ), QgsSettings::NoSection, "Cross" );

    //! Settings entry digitizing marker size mm
    static const inline QgsSettingsEntryDouble settingsDigitizingMarkerSizeMm = QgsSettingsEntryDouble( QStringLiteral( "/qgis/digitizing/marker_size_mm" ), QgsSettings::NoSection, 2.0 );

    //! Settings entry digitizing reuseLastValues
    static const inline QgsSettingsEntryBool settingsDigitizingReuseLastValues = QgsSettingsEntryBool( QStringLiteral( "/qgis/digitizing/reuseLastValues" ), QgsSettings::NoSection, false );

    //! Settings entry digitizing disable enter attribute values dialog
    static const inline QgsSettingsEntryBool settingsDigitizingDisableEnterAttributeValuesDialog = QgsSettingsEntryBool( QStringLiteral( "/qgis/digitizing/disable_enter_attribute_values_dialog" ), QgsSettings::NoSection, false );

    //! Settings entry digitizing validate geometries
    static const inline QgsSettingsEntryInteger settingsDigitizingValidateGeometries = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/validate_geometries" ), QgsSettings::NoSection, 1 );

    //! Settings entry digitizing offset join style
    static const inline QgsSettingsEntryEnumFlag<Qgis::JoinStyle> settingsDigitizingOffsetJoinStyle = QgsSettingsEntryEnumFlag<Qgis::JoinStyle>( QStringLiteral( "/qgis/digitizing/offset_join_style" ), QgsSettings::NoSection, Qgis::JoinStyle::Round );

    //! Settings entry digitizing offset quad seg
    static const inline QgsSettingsEntryInteger settingsDigitizingOffsetQuadSeg = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/offset_quad_seg" ), QgsSettings::NoSection, 8 );

    //! Settings entry digitizing offset miter limit
    static const inline QgsSettingsEntryDouble settingsDigitizingOffsetMiterLimit = QgsSettingsEntryDouble( QStringLiteral( "/qgis/digitizing/offset_miter_limit" ), QgsSettings::NoSection, 5.0 );

    //! Settings entry digitizing convert to curve
    static const inline QgsSettingsEntryBool settingsDigitizingConvertToCurve = QgsSettingsEntryBool( QStringLiteral( "/qgis/digitizing/convert_to_curve" ), QgsSettings::NoSection, false );

    //! Settings entry digitizing convert to curve angle tolerance
    static const inline QgsSettingsEntryDouble settingsDigitizingConvertToCurveAngleTolerance = QgsSettingsEntryDouble( QStringLiteral( "/qgis/digitizing/convert_to_curve_angle_tolerance" ), QgsSettings::NoSection, 1e-6 );

    //! Settings entry digitizing convert to curve distance tolerance
    static const inline QgsSettingsEntryDouble settingsDigitizingConvertToCurveDistanceTolerance = QgsSettingsEntryDouble( QStringLiteral( "/qgis/digitizing/convert_to_curve_distance_tolerance" ), QgsSettings::NoSection, 1e-6 );

    //! Settings entry digitizing offset cap style
    static const inline QgsSettingsEntryEnumFlag<Qgis::EndCapStyle> settingsDigitizingOffsetCapStyle = QgsSettingsEntryEnumFlag<Qgis::EndCapStyle>( QStringLiteral( "/qgis/digitizing/offset_cap_style" ), QgsSettings::NoSection,  Qgis::EndCapStyle::Round );

    //! Settings entry digitizing offset show advanced
    static const inline QgsSettingsEntryBool settingsDigitizingOffsetShowAdvanced = QgsSettingsEntryBool( QStringLiteral( "/qgis/digitizing/offset_show_advanced" ), QgsSettings::NoSection, false );

    //! Settings entry digitizing tracing max feature count
    static const inline QgsSettingsEntryInteger settingsDigitizingTracingMaxFeatureCount = QgsSettingsEntryInteger( QStringLiteral( "/qgis/digitizing/tracing_max_feature_count" ), QgsSettings::NoSection, 10000 );

    //! Settings entry path to GPSBabel executable.
    static const inline QgsSettingsEntryString settingsGpsBabelPath = QgsSettingsEntryString( QStringLiteral( "gpsbabelPath" ), QgsSettings::Gps, QStringLiteral( "gpsbabel" ) );
#endif

};

#endif // QGSSETTINGSREGISTRYCORE_H
