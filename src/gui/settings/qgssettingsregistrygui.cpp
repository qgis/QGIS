/***************************************************************************
  qgssettingsregistrygui.cpp
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

#include "qgssettingsregistrygui.h"

#include "qgsabstractdbsourceselect.h"
#include "qgsaddtaborgroup.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsapplication.h"
#include "qgscodeeditor.h"
#include "qgscolorwidgets.h"
#include "qgsdualview.h"
#include "qgsfeaturefiltermodel.h"
#include "qgsgradientcolorrampdialog.h"
#include "qgshistogramwidget.h"
#include "qgsmapcanvas.h"
#include "qgsmaptool.h"
#include "qgsmaptoolidentify.h"
#include "qgsrenderermeshpropertieswidget.h"
#include "qgssettings.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"
#include "qgsstylemanagerdialog.h"

#include <QString>

using namespace Qt::StringLiterals;

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsRespectScreenDPI = new QgsSettingsEntryBool( u"respect-screen-dpi"_s, QgsSettingsTree::sTreeGui, false );

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsCadFloaterActive
  = new QgsSettingsEntryBool( u"floater-active"_s, QgsSettingsTree::sTreeCad, false, u"Whether the CAD floater widget is active"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsRasterHistogramShowMarkers
  = new QgsSettingsEntryBool( u"show-markers"_s, QgsSettingsTree::sTreeRasterHistogram, false, u"Whether to show markers on the raster histogram"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsRasterHistogramZoomToMinMax
  = new QgsSettingsEntryBool( u"zoom-to-min-max"_s, QgsSettingsTree::sTreeRasterHistogram, false, u"Whether to zoom the raster histogram to min/max values"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsRasterHistogramUpdateStyleToMinMax
  = new QgsSettingsEntryBool( u"update-style-to-min-max"_s, QgsSettingsTree::sTreeRasterHistogram, true, u"Whether to update the style when histogram changes to min/max values"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsRasterHistogramDrawLines
  = new QgsSettingsEntryBool( u"draw-lines"_s, QgsSettingsTree::sTreeRasterHistogram, true, u"Whether to draw the raster histogram as lines"_s );

const QgsSettingsEntryDouble *QgsSettingsRegistryGui::settingsZoomFactor = new QgsSettingsEntryDouble( u"zoom-factor"_s, QgsSettingsTree::sTreeQgis, 2.0, u"Zoom factor for map canvas and other views"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsReverseWheelZoom
  = new QgsSettingsEntryBool( u"reverse-wheel-zoom"_s, QgsSettingsTree::sTreeQgis, false, u"Whether to reverse the direction of wheel zoom"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsNewLayersVisible
  = new QgsSettingsEntryBool( u"new-layers-visible"_s, QgsSettingsTree::sTreeQgis, true, u"Whether newly added layers are visible by default"_s );

const QgsSettingsEntryString *QgsSettingsRegistryGui::settingsRasterDefaultPalette
  = new QgsSettingsEntryString( u"default-palette"_s, QgsSettingsTree::sTreeRaster, QString(), u"Default color ramp palette name for raster layers"_s );

const QgsSettingsEntryInteger *QgsSettingsRegistryGui::settingsMessageTimeout
  = new QgsSettingsEntryInteger( u"message-timeout"_s, QgsSettingsTree::sTreeQgis, 5, u"Timeout in seconds for message bar messages"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsEnableAntiAliasing
  = new QgsSettingsEntryBool( u"enable-anti-aliasing"_s, QgsSettingsTree::sTreeQgis, true, u"Whether anti-aliasing is enabled for rendering"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsNativeColorDialogs
  = new QgsSettingsEntryBool( u"native-color-dialogs"_s, QgsSettingsTree::sTreeQgis, false, u"Whether to use native color dialogs"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsFormatLayerName
  = new QgsSettingsEntryBool( u"format-layer-name"_s, QgsSettingsTree::sTreeQgis, false, u"Whether to format layer names for better readability"_s );

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsOpenSublayersInGroup
  = new QgsSettingsEntryBool( u"open-sublayers-in-group"_s, QgsSettingsTree::sTreeQgis, false, u"Whether to open sublayers in a group"_s );

const QgsSettingsEntryInteger *QgsSettingsRegistryGui::settingsMapUpdateInterval
  = new QgsSettingsEntryInteger( u"map-update-interval"_s, QgsSettingsTree::sTreeQgis, 250, u"Map update interval in milliseconds"_s );

const QgsSettingsEntryDouble *QgsSettingsRegistryGui::settingsMagnifierFactorDefault
  = new QgsSettingsEntryDouble( u"magnifier-factor-default"_s, QgsSettingsTree::sTreeQgis, 1.0, u"Default magnifier factor"_s );

const QgsSettingsEntryDouble *QgsSettingsRegistryGui::settingsSegmentationTolerance
  = new QgsSettingsEntryDouble( u"segmentation-tolerance"_s, QgsSettingsTree::sTreeQgis, 0.01745, u"Segmentation tolerance for curved geometries"_s );

const QgsSettingsEntryColor *QgsSettingsRegistryGui::settingsDefaultMeasureColor
  = new QgsSettingsEntryColor( u"default-measure-color"_s, QgsSettingsTree::sTreeQgis, QColor( 222, 155, 67 ), u"Default measure tool color"_s );

const QgsSettingsEntryEnumFlag<QgsAbstractGeometry::SegmentationToleranceType> *QgsSettingsRegistryGui::settingsSegmentationToleranceType = new QgsSettingsEntryEnumFlag<
  QgsAbstractGeometry::SegmentationToleranceType>( u"segmentation-tolerance-type"_s, QgsSettingsTree::sTreeQgis, QgsAbstractGeometry::MaximumAngle, u"Segmentation tolerance type for curved geometries"_s );

QgsSettingsRegistryGui::QgsSettingsRegistryGui()
  : QgsSettingsRegistry()
{
  // copy values from old keys to new keys and delete the old ones
  // for backward compatibility, old keys are recreated when the registry gets deleted

  // TODO: remove in QGIS 4.4 (after LTR 4.2)

  QgsSettings::holdFlush();

  // single settings - added in 3.30
  settingsRespectScreenDPI->copyValueFromKey( u"gui/qgis/respect_screen_dpi"_s, {}, true );

  // single settings - added in 4.2
  settingsCadFloaterActive->copyValueFromKey( u"/Cad/Floater"_s, true );
  QgsAdvancedDigitizingDockWidget::settingsCadCommonAngle->copyValueFromKey( u"/Cad/CommonAngle"_s, true );
  QgsMapCanvas::settingsCustomCoordinateCrs->copyValueFromKey( u"qgis/custom_coordinate_crs"_s, true );
  QgsGradientColorRampDialog::settingsPlotHue->copyValueFromKey( u"GradientEditor/plotHue"_s, true );
  QgsGradientColorRampDialog::settingsPlotLightness->copyValueFromKey( u"GradientEditor/plotLightness"_s, true );
  QgsGradientColorRampDialog::settingsPlotSaturation->copyValueFromKey( u"GradientEditor/plotSaturation"_s, true );
  QgsGradientColorRampDialog::settingsPlotAlpha->copyValueFromKey( u"GradientEditor/plotAlpha"_s, true );
  QgsColorTextWidget::settingsTextFormat->copyValueFromKey( u"ColorWidgets/textWidgetFormat"_s, true );
  QgsRendererMeshPropertiesWidget::settingsTab->copyValueFromKey( u"/Windows/RendererMeshProperties/tab"_s, true );
  QgsMapToolIdentify::settingIdentifyMode->copyValueFromKey( u"Map/identifyMode"_s, true );
  QgsMapToolIdentify::settingIdentifyMode->copyValueFromKey( u"/Map/identifyMode"_s, true );
  QgsHistogramWidget::settingsHistogramShowMean->copyValueFromKey( u"HistogramWidget/showMean"_s, true );
  QgsHistogramWidget::settingsHistogramShowStdev->copyValueFromKey( u"HistogramWidget/showStdev"_s, true );
  settingsRasterHistogramShowMarkers->copyValueFromKey( u"Raster/histogram/showMarkers"_s, true );
  settingsRasterHistogramZoomToMinMax->copyValueFromKey( u"Raster/histogram/zoomToMinMax"_s, true );
  settingsRasterHistogramUpdateStyleToMinMax->copyValueFromKey( u"Raster/histogram/updateStyleToMinMax"_s, true );
  settingsRasterHistogramDrawLines->copyValueFromKey( u"Raster/histogram/drawLines"_s, true );
  settingsZoomFactor->copyValueFromKey( u"qgis/zoom_factor"_s, true );
  settingsZoomFactor->copyValueFromKey( u"/qgis/zoom_factor"_s, true );
  settingsReverseWheelZoom->copyValueFromKey( u"qgis/reverse_wheel_zoom"_s, true );
  settingsReverseWheelZoom->copyValueFromKey( u"/qgis/reverse_wheel_zoom"_s, true );
  settingsNewLayersVisible->copyValueFromKey( u"qgis/new_layers_visible"_s, true );
  settingsNewLayersVisible->copyValueFromKey( u"/qgis/new_layers_visible"_s, true );
  settingsRasterDefaultPalette->copyValueFromKey( u"Raster/defaultPalette"_s, true );
  settingsRasterDefaultPalette->copyValueFromKey( u"/Raster/defaultPalette"_s, true );
  settingsMessageTimeout->copyValueFromKey( u"qgis/messageTimeout"_s, true );
  settingsMessageTimeout->copyValueFromKey( u"/qgis/messageTimeout"_s, true );
  settingsEnableAntiAliasing->copyValueFromKey( u"qgis/enable_anti_aliasing"_s, true );
  settingsEnableAntiAliasing->copyValueFromKey( u"/qgis/enable_anti_aliasing"_s, true );
  settingsNativeColorDialogs->copyValueFromKey( u"qgis/native_color_dialogs"_s, true );
  settingsNativeColorDialogs->copyValueFromKey( u"/qgis/native_color_dialogs"_s, true );
  settingsFormatLayerName->copyValueFromKey( u"qgis/formatLayerName"_s, true );
  settingsFormatLayerName->copyValueFromKey( u"/qgis/formatLayerName"_s, true );
  settingsOpenSublayersInGroup->copyValueFromKey( u"qgis/openSublayersInGroup"_s, true );
  settingsOpenSublayersInGroup->copyValueFromKey( u"/qgis/openSublayersInGroup"_s, true );
  settingsMapUpdateInterval->copyValueFromKey( u"qgis/map_update_interval"_s, true );
  settingsMapUpdateInterval->copyValueFromKey( u"/qgis/map_update_interval"_s, true );
  settingsMagnifierFactorDefault->copyValueFromKey( u"qgis/magnifier_factor_default"_s, true );
  settingsMagnifierFactorDefault->copyValueFromKey( u"/qgis/magnifier_factor_default"_s, true );
  settingsSegmentationTolerance->copyValueFromKey( u"qgis/segmentationTolerance"_s, true );
  settingsSegmentationTolerance->copyValueFromKey( u"/qgis/segmentationTolerance"_s, true );
  settingsDefaultMeasureColor->copyValueFromKeys( u"qgis/default_measure_color_red"_s, u"qgis/default_measure_color_green"_s, u"qgis/default_measure_color_blue"_s, QString(), true );
  settingsDefaultMeasureColor->copyValueFromKeys( u"/qgis/default_measure_color_red"_s, u"/qgis/default_measure_color_green"_s, u"/qgis/default_measure_color_blue"_s, QString(), true );
  settingsSegmentationToleranceType->copyValueFromKey( u"qgis/segmentationToleranceType"_s, true );
  settingsSegmentationToleranceType->copyValueFromKey( u"/qgis/segmentationToleranceType"_s, true );
  QgsFeatureFilterModel::settingsMaxEntriesRelationWidget->copyValueFromKey( u"gui/maxEntriesRelationWidget"_s, true );
  QgsCodeEditor::settingFontFamily->copyValueFromKey( u"gui/codeEditor/fontfamily"_s, true );
  QgsCodeEditor::settingFontSize->copyValueFromKey( u"gui/codeEditor/fontsize"_s, true );
  QgsMapTool::settingSearchRadiusMM->copyValueFromKey( u"Map/searchRadiusMM"_s, true );
  QgsMapTool::settingSearchRadiusMM->copyValueFromKey( u"/Map/searchRadiusMM"_s, true );
  QgsAddAttributeFormContainerDialog::settingsDefaultTabColumnCount->copyValueFromKey( u"qgis/attributeForm/defaultTabColumnCount"_s, true );
  QgsAddAttributeFormContainerDialog::settingsDefaultTabColumnCount->copyValueFromKey( u"/qgis/attributeForm/defaultTabColumnCount"_s, true );
  QgsAddAttributeFormContainerDialog::settingsDefaultGroupColumnCount->copyValueFromKey( u"qgis/attributeForm/defaultGroupColumnCount"_s, true );
  QgsAddAttributeFormContainerDialog::settingsDefaultGroupColumnCount->copyValueFromKey( u"/qgis/attributeForm/defaultGroupColumnCount"_s, true );
  QgsDualView::settingsFeatureListHighlightFeature->copyValueFromKey( u"qgis/attributeTable/featureListHighlightFeature"_s, true );
  QgsDualView::settingsFeatureListHighlightFeature->copyValueFromKey( u"/qgis/attributeTable/featureListHighlightFeature"_s, true );

  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( u"ogr/GPKGSourceSelect/HoldDialogOpen"_s, { u"ogr/GPKGSourceSelect"_s }, true );
  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( u"ogr/SQLiteSourceSelect/HoldDialogOpen"_s, { u"ogr/SQLiteSourceSelect"_s }, true );
  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( u"Windows/MSSQLSourceSelect/HoldDialogOpen"_s, { u"MSSQLSourceSelect"_s }, true );
  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( u"Windows/PgSourceSelect/HoldDialogOpen"_s, { u"PgSourceSelect"_s }, true );
  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( u"Windows/SpatiaLiteSourceSelect/HoldDialogOpen"_s, { u"SpatiaLiteSourceSelect"_s }, true );

  QgsSettings::releaseFlush();
}

QgsSettingsRegistryGui::~QgsSettingsRegistryGui()
{}
