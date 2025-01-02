#!/usr/bin/env python3


import argparse
import os
import re
import sys

from collections import defaultdict
from enum import Enum, auto
from typing import Any, Dict, List, Optional, Tuple

import yaml


class Visibility(Enum):
    Private = auto()
    Protected = auto()
    Public = auto()
    Signals = auto()


class CodeSnippetType(Enum):
    NotCodeSnippet = auto()
    NotSpecified = auto()
    Cpp = auto()


class PrependType(Enum):
    NoPrepend = auto()
    Virtual = auto()
    MakePrivate = auto()


class MultiLineType(Enum):
    NotMultiline = auto()
    Method = auto()
    ConditionalStatement = auto()


# Parse command-line arguments
parser = argparse.ArgumentParser(description="Convert header file to SIP and Python")
parser.add_argument("-debug", action="store_true", help="Enable debug mode")
parser.add_argument("-qt6", action="store_true", help="Enable Qt6 mode")
parser.add_argument("-sip_output", help="SIP output file")
parser.add_argument("-python_output", help="Python output file")
parser.add_argument("-class_map", help="Class map file")
parser.add_argument("headerfile", help="Input header file")
args = parser.parse_args()

# Read the input file
try:
    with open(args.headerfile) as f:
        input_lines = f.read().splitlines()
except OSError as e:
    print(
        f"Couldn't open '{args.headerfile}' for reading because: {e}", file=sys.stderr
    )
    sys.exit(1)

# Read configuration
cfg_file = os.path.join(os.path.dirname(__file__), "../python/sipify.yaml")
try:
    with open(cfg_file) as f:
        sip_config = yaml.safe_load(f)
except OSError as e:
    print(
        f"Couldn't open configuration file '{cfg_file}' because: {e}", file=sys.stderr
    )
    sys.exit(1)


# Initialize contexts


class Context:

    def __init__(self):
        self.debug: bool = False
        self.is_qt6: bool = False
        self.header_file: str = ""
        self.current_line: str = ""
        self.sip_run: bool = False
        self.header_code: bool = False
        self.access: list[Visibility] = [Visibility.Public]
        self.multiline_definition: MultiLineType = MultiLineType.NotMultiline
        self.classname: list[str] = []
        self.class_and_struct: list[str] = []
        self.declared_classes: list[str] = []
        self.all_fully_qualified_class_names: list[str] = []
        self.exported: list[int] = [0]
        self.actual_class: str = ""
        self.python_signature: str = ""
        self.enum_int_types: list[str] = []
        self.enum_intflag_types: list[str] = []
        self.enum_class_non_int_types: list[str] = []
        self.enum_monkey_patched_types: list = []
        self.indent: str = ""
        self.prev_indent: str = ""
        self.comment: str = ""
        self.comment_param_list: bool = False
        self.comment_last_line_note_warning: bool = False
        self.comment_code_snippet: CodeSnippetType = CodeSnippetType.NotCodeSnippet
        self.comment_template_docstring: bool = False
        self.skipped_params_out: list[str] = []
        self.skipped_params_remove: list[str] = []
        self.ifdef_nesting_idx: int = 0
        self.bracket_nesting_idx: list[int] = [0]
        self.private_section_line: str = ""
        self.last_access_section_line: str = ""
        self.return_type: str = ""
        self.is_override_or_make_private: PrependType = PrependType.NoPrepend
        self.if_feature_condition: str = ""
        self.found_since: bool = False
        self.qflag_hash: dict[str, Any] = {}
        self.input_lines: list[str] = []
        self.line_count: int = len(input_lines)
        self.line_idx: int = 0
        self.output: list[str] = []
        self.output_python: list[str] = []
        self.doxy_inside_sip_run: int = 0
        self.has_pushed_force_int: bool = False
        self.attribute_docstrings = defaultdict(dict)
        self.struct_docstrings = defaultdict(dict)
        self.current_method_name: str = ""
        self.static_methods = defaultdict(dict)
        self.current_signal_args = []
        self.signal_arguments = defaultdict(dict)
        self.deprecated_message = None

    def current_fully_qualified_class_name(self) -> str:
        return ".".join(
            _c
            for _c in (
                [c for c in self.classname if c != self.actual_class]
                + [self.actual_class]
            )
            if _c
        )

    def current_fully_qualified_struct_name(self) -> str:
        return ".".join(self.class_and_struct)


CONTEXT = Context()
CONTEXT.debug = args.debug
CONTEXT.is_qt6 = args.qt6
CONTEXT.header_file = args.headerfile
CONTEXT.input_lines = input_lines
CONTEXT.line_count = len(input_lines)

ALLOWED_NON_CLASS_ENUMS = [
    "QgsSipifyHeader::MyEnum",
    "QgsSipifyHeader::OneLiner",
    "CadConstraint::LockMode",
    "ColorrampTable",
    "QgsMesh::ElementType",
    "LabelSettingsTable",
    "Qgis::MessageLevel",
    "Qgs3DMapScene::SceneState",
    "Qgs3DTypes::CullingMode",
    "Qgs3DTypes::Flag3DRenderer",
    "QgsAbstractDatabaseProviderConnection::Capability",
    "QgsAbstractDatabaseProviderConnection::GeometryColumnCapability",
    "QgsAbstractFeatureIterator::CompileStatus",
    "QgsAbstractGeometry::AxisOrder",
    "QgsAbstractGeometry::SegmentationToleranceType",
    "QgsAbstractGeometry::WkbFlag",
    "QgsAbstractReportSection::SubSection",
    "QgsAdvancedDigitizingDockWidget::CadCapacity",
    "QgsAdvancedDigitizingDockWidget::WidgetSetMode",
    "QgsApplication::Cursor",
    "QgsApplication::StyleSheetType",
    "QgsApplication::endian_t",
    "QgsArrowSymbolLayer::ArrowType",
    "QgsArrowSymbolLayer::HeadType",
    "QgsAttributeEditorContext::FormMode",
    "QgsAttributeEditorContext::Mode",
    "QgsAttributeEditorContext::RelationMode",
    "QgsAttributeEditorRelation::Button",
    "QgsAttributeForm::FilterType",
    "QgsAttributeForm::Mode",
    "QgsAttributeFormWidget::Mode",
    "QgsAttributeTableConfig::ActionWidgetStyle",
    "QgsAttributeTableConfig::Type",
    "QgsAttributeTableFilterModel::ColumnType",
    "QgsAttributeTableFilterModel::FilterMode",
    "QgsAuthCertUtils::CaCertSource",
    "QgsAuthCertUtils::CertTrustPolicy",
    "QgsAuthCertUtils::CertUsageType",
    "QgsAuthCertUtils::ConstraintGroup",
    "QgsAuthImportCertDialog::CertFilter",
    "QgsAuthImportCertDialog::CertInput",
    "QgsAuthImportIdentityDialog::BundleTypes",
    "QgsAuthImportIdentityDialog::IdentityType",
    "QgsAuthImportIdentityDialog::Validity",
    "QgsAuthManager::MessageLevel",
    "QgsAuthMethod::Expansion",
    "QgsAuthSettingsWidget::WarningType",
    "QgsBasicNumericFormat::RoundingType",
    "QgsBearingNumericFormat::FormatDirectionOption",
    "QgsBlockingNetworkRequest::ErrorCode",
    "QgsBlurEffect::BlurMethod",
    "QgsBookmarkManagerModel::Columns",
    "QgsBrowserProxyModel::FilterSyntax",
    "QgsCallout::AnchorPoint",
    "QgsCallout::DrawOrder",
    "QgsCallout::LabelAnchorPoint",
    "QgsCheckBoxFieldFormatter::TextDisplayMethod",
    "QgsClassificationLogarithmic::NegativeValueHandling",
    "QgsClassificationMethod::ClassPosition",
    "QgsClassificationMethod::MethodProperty",
    "QgsClipper::Boundary",
    "QgsColorButton::Behavior",
    "QgsColorRampLegendNodeSettings::Direction",
    "QgsColorRampShader::ClassificationMode",
    "QgsColorRampShader::Type",
    "QgsColorRampWidget::Orientation",
    "QgsColorScheme::SchemeFlag",
    "QgsColorTextWidget::ColorTextFormat",
    "QgsColorWidget::ColorComponent",
    "QgsCompoundColorWidget::Layout",
    "QgsContrastEnhancement::ContrastEnhancementAlgorithm",
    "QgsCoordinateFormatter::Format",
    "QgsCoordinateFormatter::FormatFlag",
    "QgsCoordinateReferenceSystem::CrsType",
    "QgsCoordinateReferenceSystemProxyModel::Filter",
    "QgsCptCityBrowserModel::ViewType",
    "QgsCptCityDataItem::Type",
    "QgsCurvedLineCallout::Orientation",
    "QgsDartMeasurement::Type",
    "QgsDataDefinedSizeLegend::LegendType",
    "QgsDataDefinedSizeLegend::VerticalAlignment",
    "QgsDataProvider::DataCapability",
    "QgsDataProvider::ProviderProperty",
    "QgsDataProvider::ReadFlag",
    "QgsDataSourceUri::SslMode",
    "QgsDiagramLayerSettings::LinePlacementFlag",
    "QgsDiagramLayerSettings::Placement",
    "QgsDiagramLayerSettings::DiagramType",
    "QgsDiagramSettings::DiagramOrientation",
    "QgsDiagramSettings::Direction",
    "QgsDiagramSettings::LabelPlacementMethod",
    "QgsDiagramSettings::StackedDiagramMode",
    "QgsDoubleSpinBox::ClearValueMode",
    "QgsDualView::FeatureListBrowsingAction",
    "QgsDualView::ViewMode",
    "QgsDxfExport::DxfPolylineFlag",
    "QgsDxfExport::Flag",
    "QgsEditorWidgetWrapper::ConstraintResult",
    "QgsEllipseSymbolLayer::Shape",
    "QgsErrorMessage::Format",
    "QgsExpression::ParserErrorType",
    "QgsExpression::SpatialOperator",
    "QgsExpressionBuilderWidget::Flag",
    "QgsExpressionItem::ItemType",
    "QgsExpressionNode::NodeType",
    "QgsExpressionNodeBinaryOperator::BinaryOperator",
    "QgsExpressionNodeUnaryOperator::UnaryOperator",
    "QgsExtentGroupBox::ExtentState",
    "QgsExtentWidget::ExtentState",
    "QgsExtentWidget::WidgetStyle",
    "QgsExternalResourceWidget::DocumentViewerContent",
    "QgsFeatureListModel::Role",
    "QgsFeatureListViewDelegate::Element",
    "QgsFeatureRenderer::Capability",
    "QgsFeatureSink::Flag",
    "QgsFeatureSink::SinkFlag",
    "QgsFetchedContent::ContentStatus",
    "QgsFieldConstraints::Constraint",
    "QgsFieldConstraints::ConstraintOrigin",
    "QgsFieldConstraints::ConstraintStrength",
    "QgsFieldFormatter::Flag",
    "QgsFieldProxyModel::Filter",
    "QgsFields::FieldOrigin",
    "QgsFileWidget::RelativeStorage",
    "QgsFileWidget::StorageMode",
    "QgsFilterLineEdit::ClearMode",
    "QgsFloatingWidget::AnchorPoint",
    "QgsFontButton::Mode",
    "QgsGeometryCheck::ChangeType",
    "QgsGeometryCheck::ChangeWhat",
    "QgsGeometryCheck::CheckType",
    "QgsGeometryCheck::Flag",
    "QgsGeometryCheckError::Status",
    "QgsGeometryCheckError::ValueType",
    "QgsGeometryEngine::EngineOperationResult",
    "QgsGeometryRubberBand::IconType",
    "QgsGeometrySnapper::SnapMode",
    "QgsGlowEffect::GlowColorType",
    "QgsGpsConnection::Status",
    "QgsGraduatedSymbolRenderer::Mode",
    "QgsGui::HigFlag",
    "QgsGui::ProjectCrsBehavior",
    "QgsHueSaturationFilter::GrayscaleMode",
    "QgsIdentifyMenu::MenuLevel",
    "QgsImageOperation::FlipType",
    "QgsImageOperation::GrayscaleMode",
    "QgsInterpolatedLineColor::ColoringMethod",
    "QgsInterpolator::Result",
    "QgsInterpolator::SourceType",
    "QgsInterpolator::ValueSource",
    "QgsKernelDensityEstimation::KernelShape",
    "QgsKernelDensityEstimation::OutputValues",
    "QgsKernelDensityEstimation::Result",
    "QgsLabelingEngineSettings::Search",
    "QgsLayerMetadataResultsModel::Roles",
    "QgsLayerMetadataResultsModel::Sections",
    "QgsLayerTreeLayer::LegendNodesSplitBehavior",
    "QgsLayerTreeModel::Flag",
    "QgsLayerTreeModelLegendNode::NodeTypes",
    "QgsLayerTreeNode::NodeType",
    "QgsLayout::UndoCommand",
    "QgsLayout::ZValues",
    "QgsLayoutAligner::Alignment",
    "QgsLayoutAligner::Distribution",
    "QgsLayoutAligner::Resize",
    "QgsLayoutDesignerInterface::StandardTool",
    "QgsLayoutExporter::ExportResult",
    "QgsLayoutGridSettings::Style",
    "QgsLayoutItem::ExportLayerBehavior",
    "QgsLayoutItem::Flag",
    "QgsLayoutItem::ReferencePoint",
    "QgsLayoutItem::UndoCommand",
    "QgsLayoutItemAbstractGuiMetadata::Flag",
    "QgsLayoutItemAttributeTable::ContentSource",
    "QgsLayoutItemHtml::ContentMode",
    "QgsLayoutItemLabel::Mode",
    "QgsLayoutItemMap::AtlasScalingMode",
    "QgsLayoutItemMap::MapItemFlag",
    "QgsLayoutItemMapGrid::AnnotationCoordinate",
    "QgsLayoutItemMapGrid::AnnotationDirection",
    "QgsLayoutItemMapGrid::AnnotationFormat",
    "QgsLayoutItemMapGrid::AnnotationPosition",
    "QgsLayoutItemMapGrid::BorderSide",
    "QgsLayoutItemMapGrid::DisplayMode",
    "QgsLayoutItemMapGrid::FrameSideFlag",
    "QgsLayoutItemMapGrid::FrameStyle",
    "QgsLayoutItemMapGrid::GridStyle",
    "QgsLayoutItemMapGrid::GridUnit",
    "QgsLayoutItemMapGrid::TickLengthMode",
    "QgsLayoutItemMapItem::StackingPosition",
    "QgsLayoutItemPage::Orientation",
    "QgsLayoutItemPage::UndoCommand",
    "QgsLayoutItemPicture::Format",
    "QgsLayoutItemPicture::NorthMode",
    "QgsLayoutItemPicture::ResizeMode",
    "QgsLayoutItemPolyline::MarkerMode",
    "QgsLayoutItemRegistry::ItemType",
    "QgsLayoutItemShape::Shape",
    "QgsLayoutManagerProxyModel::Filter",
    "QgsLayoutModel::Columns",
    "QgsLayoutMultiFrame::ResizeMode",
    "QgsLayoutMultiFrame::UndoCommand",
    "QgsLayoutNorthArrowHandler::NorthMode",
    "QgsLayoutObject::PropertyValueType",
    "QgsLayoutRenderContext::Flag",
    "QgsLayoutTable::CellStyleGroup",
    "QgsLayoutTable::EmptyTableMode",
    "QgsLayoutTable::HeaderHAlignment",
    "QgsLayoutTable::HeaderMode",
    "QgsLayoutTable::WrapBehavior",
    "QgsLayoutView::ClipboardOperation",
    "QgsLayoutView::PasteMode",
    "QgsLayoutViewTool::Flag",
    "QgsLegendStyle::Side",
    "QgsLegendStyle::Style",
    "QgsLineSymbolLayer::RenderRingFilter",
    "QgsLocatorFilter::Flag",
    "QgsLocatorFilter::Priority",
    "QgsManageConnectionsDialog::Mode",
    "QgsManageConnectionsDialog::Type",
    "QgsMapBoxGlStyleConverter::Result",
    "QgsMapCanvasAnnotationItem::MouseMoveAction",
    "QgsMapLayer::LayerFlag",
    "QgsMapLayer::PropertyType",
    "QgsMapLayer::ReadFlag",
    "QgsMapLayer::StyleCategory",
    "QgsMapLayerDependency::Origin",
    "QgsMapLayerDependency::Type",
    "QgsMapLayerElevationProperties::Flag",
    "QgsMapRendererTask::ErrorType",
    "QgsMapToPixelSimplifier::SimplifyAlgorithm",
    "QgsMapToPixelSimplifier::SimplifyFlag",
    "QgsMapTool::Flag",
    "QgsMapToolCapture::Capability",
    "QgsMapToolCapture::CaptureMode",
    "QgsMapToolEdit::TopologicalResult",
    "QgsMapToolIdentify::IdentifyMode",
    "QgsMapToolIdentify::Type",
    "QgsMarkerSymbolLayer::HorizontalAnchorPoint",
    "QgsMarkerSymbolLayer::VerticalAnchorPoint",
    "QgsMasterLayoutInterface::Type",
    "QgsMediaWidget::Mode",
    "QgsMergedFeatureRenderer::GeometryOperation",
    "QgsMesh3DAveragingMethod::Method",
    "QgsMeshCalculator::Result",
    "QgsMeshDataBlock::DataType",
    "QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod",
    "QgsMeshDatasetGroup::Type",
    "QgsMeshDatasetGroupMetadata::DataType",
    "QgsMeshDriverMetadata::MeshDriverCapability",
    "QgsMeshRendererScalarSettings::DataResamplingMethod",
    "QgsMeshRendererVectorArrowSettings::ArrowScalingMethod",
    "QgsMeshRendererVectorSettings::Symbology",
    "QgsMeshRendererVectorStreamlineSettings::SeedingStartPointsMethod",
    "QgsMeshTimeSettings::TimeUnit",
    "QgsMessageOutput::MessageType",
    "QgsMetadataWidget::Mode",
    "QgsModelArrowItem::Marker",
    "QgsModelComponentGraphicItem::Flag",
    "QgsModelComponentGraphicItem::State",
    "QgsModelGraphicsScene::Flag",
    "QgsModelGraphicsScene::ZValues",
    "QgsModelGraphicsView::ClipboardOperation",
    "QgsModelGraphicsView::PasteMode",
    "QgsMultiEditToolButton::State",
    "QgsNetworkRequestParameters::RequestAttributes",
    "QgsNewGeoPackageLayerDialog::OverwriteBehavior",
    "QgsNewHttpConnection::ConnectionType",
    "QgsNewHttpConnection::Flag",
    "QgsNewHttpConnection::WfsVersionIndex",
    "QgsOfflineEditing::ContainerType",
    "QgsOfflineEditing::ProgressMode",
    "QgsOgcUtils::FilterVersion",
    "QgsOgcUtils::GMLVersion",
    "QgsPaintEffect::DrawMode",
    "QgsPercentageNumericFormat::InputValues",
    "QgsPictureSourceLineEditBase::Format",
    "QgsPointCloud3DSymbol::RenderingStyle",
    "QgsPointCloudAttribute::DataType",
    "QgsPointCloudAttributeProxyModel::Filter",
    "QgsPointCloudDataProvider::Capability",
    "QgsPointCloudDataProvider::PointCloudIndexGenerationState",
    "QgsPointDisplacementRenderer::Placement",
    "QgsPointLocator::Type",
    "QgsPreviewEffect::PreviewMode",
    "QgsProcessing::SourceType",
    "QgsProcessingAlgorithm::Flag",
    "QgsProcessingAlgorithm::PropertyAvailability",
    "QgsProcessingAlgorithmDialogBase::LogFormat",
    "QgsProcessingContext::Flag",
    "QgsProcessingContext::LogLevel",
    "QgsProcessingFeatureSource::Flag",
    "QgsProcessingGui::WidgetType",
    "QgsProcessingParameterDateTime::Type",
    "QgsProcessingParameterDefinition::Flag",
    "QgsProcessingParameterField::DataType",
    "QgsProcessingParameterFile::Behavior",
    "QgsProcessingParameterNumber::Type",
    "QgsProcessingParameterTinInputLayers::Type",
    "QgsProcessingParameterType::ParameterFlag",
    "QgsProcessingProvider::Flag",
    "QgsProcessingToolboxModelNode::NodeType",
    "QgsProcessingToolboxProxyModel::Filter",
    "QgsProjectBadLayerHandler::DataType",
    "QgsProjectBadLayerHandler::ProviderType",
    "QgsProjectServerValidator::ValidationError",
    "QgsProjectionSelectionWidget::CrsOption",
    "QgsPropertyDefinition::DataType",
    "QgsPropertyDefinition::StandardPropertyTemplate",
    "QgsPropertyTransformer::Type",
    "QgsProviderMetadata::ProviderCapability",
    "QgsProviderMetadata::ProviderMetadataCapability",
    "QgsProviderRegistry::WidgetMode",
    "QgsQuadrilateral::ConstructionOption",
    "QgsQuadrilateral::Point",
    "QgsRasterCalcNode::Operator",
    "QgsRasterCalcNode::Type",
    "QgsRasterCalculator::Result",
    "QgsRasterDataProvider::ProviderCapability",
    "QgsRasterDataProvider::TransformType",
    "QgsRasterFileWriter::RasterFormatOption",
    "QgsRasterFormatSaveOptionsWidget::Type",
    "QgsRasterInterface::Capability",
    "QgsRasterLayerSaveAsDialog::CrsState",
    "QgsRasterLayerSaveAsDialog::Mode",
    "QgsRasterLayerSaveAsDialog::ResolutionState",
    "QgsRasterMatrix::OneArgOperator",
    "QgsRasterMatrix::TwoArgOperator",
    "QgsRasterMinMaxOrigin::Extent",
    "QgsRasterMinMaxOrigin::Limits",
    "QgsRasterMinMaxOrigin::StatAccuracy",
    "QgsRasterProjector::Precision",
    "QgsRasterRange::BoundsType",
    "QgsReadWriteLocker::Mode",
    "QgsRegularPolygon::ConstructionOption",
    "QgsRelationEditorWidget::Button",
    "QgsRelationReferenceWidget::CanvasExtent",
    "QgsRendererAbstractMetadata::LayerType",
    "QgsReportSectionFieldGroup::SectionVisibility",
    "QgsRubberBand::IconType",
    "QgsRuleBasedRenderer::FeatureFlags",
    "QgsSQLStatement::BinaryOperator",
    "QgsSQLStatement::JoinType",
    "QgsSQLStatement::NodeType",
    "QgsSQLStatement::UnaryOperator",
    "QgsScaleBarSettings::Alignment",
    "QgsScaleBarSettings::LabelHorizontalPlacement",
    "QgsScaleBarSettings::LabelVerticalPlacement",
    "QgsScaleBarSettings::SegmentSizeMode",
    "QgsSearchWidgetWrapper::FilterFlag",
    "QgsServerOgcApi::ContentType",
    "QgsServerOgcApi::Rel",
    "QgsServerParameter::Name",
    "QgsServerRequest::Method",
    "QgsServerRequest::RequestHeader",
    "QgsServerSettingsEnv::EnvVar",
    "QgsServerSettingsEnv::Source",
    "QgsServerWmsDimensionProperties::DefaultDisplay",
    "QgsServerWmsDimensionProperties::PredefinedWmsDimensionName",
    "QgsSettings::Section",
    "QgsSimplifyMethod::MethodType",
    "QgsSingleBandGrayRenderer::Gradient",
    "QgsSizeScaleTransformer::ScaleType",
    "QgsSnappingConfig::ScaleDependencyMode",
    "QgsSnappingConfig::SnappingType",
    "QgsSnappingUtils::IndexingStrategy",
    "QgsSourceSelectProvider::Ordering",
    "QgsSpatialIndex::Flag",
    "QgsSpinBox::ClearValueMode",
    "QgsStatusBar::Anchor",
    "QgsStoredExpression::Category",
    "QgsStyle::StyleEntity",
    "QgsStyleExportImportDialog::Mode",
    "QgsStyleModel::Column",
    "QgsSublayersDialog::PromptMode",
    "QgsSublayersDialog::ProviderType",
    "QgsTask::Flag",
    "QgsTask::SubTaskDependency",
    "QgsTask::TaskStatus",
    "QgsTemporalProperty::Flag",
    "QgsTextBackgroundSettings::RotationType",
    "QgsTextBackgroundSettings::ShapeType",
    "QgsTextBackgroundSettings::SizeType",
    "QgsTextDiagram::Orientation",
    "QgsTextDiagram::Shape",
    "QgsTextFormatWidget::Mode",
    "QgsTextMaskSettings::MaskType",
    "QgsTextShadowSettings::ShadowPlacement",
    "QgsTicksScaleBarRenderer::TickPosition",
    "QgsTinInterpolator::TinInterpolation",
    "QgsTracer::PathError",
    "QgsValidityCheckContext::ContextType",
    "QgsValidityCheckResult::Type",
    "QgsVectorDataProvider::Capability",
    "QgsVectorFieldSymbolLayer::AngleOrientation",
    "QgsVectorFieldSymbolLayer::AngleUnits",
    "QgsVectorFieldSymbolLayer::VectorFieldType",
    "QgsVectorFileWriter::ActionOnExistingFile",
    "QgsVectorFileWriter::EditionCapability",
    "QgsVectorFileWriter::FieldNameSource",
    "QgsVectorFileWriter::OptionType",
    "QgsVectorFileWriter::VectorFormatOption",
    "QgsVectorFileWriter::WriterError",
    "QgsVectorLayerDirector::Direction",
    "QgsVectorLayerUtils::CascadedFeatureFlag",
    "QgsVectorSimplifyMethod::SimplifyAlgorithm",
    "QgsVectorSimplifyMethod::SimplifyHint",
    "QgsVertexMarker::IconType",
    "QgsWeakRelation::WeakRelationType",
    "QgsWindowManagerInterface::StandardDialog",
    "Rule::RegisterResult",
    "Rule::RenderResult",
    "SmartgroupTable",
    "SymbolTable",
    "TagTable",
    "TagmapTable",
    "TextFormatTable",
]


def replace_macros(line):
    global CONTEXT

    line = re.sub(r"\bTRUE\b", "``True``", line)
    line = re.sub(r"\bFALSE\b", "``False``", line)
    line = re.sub(r"\bNULLPTR\b", "``None``", line)

    if CONTEXT.is_qt6:
        # sip for Qt6 chokes on QList/QVector<QVariantMap>, but is happy if you expand out the map explicitly
        line = re.sub(
            r"(QList<\s*|QVector<\s*)QVariantMap", r"\1QMap<QString, QVariant>", line
        )

    return line


def read_line():
    global CONTEXT

    new_line = CONTEXT.input_lines[CONTEXT.line_idx]
    CONTEXT.line_idx += 1

    if CONTEXT.debug:
        print(
            f"LIN:{CONTEXT.line_idx} DEPTH:{len(CONTEXT.access)} ACC:{CONTEXT.access[-1]} "
            f"BRCK:{CONTEXT.bracket_nesting_idx[-1]} SIP:{CONTEXT.sip_run} MLT:{CONTEXT.multiline_definition} "
            f"OVR: {CONTEXT.is_override_or_make_private} CLSS: {CONTEXT.actual_class}/{len(CONTEXT.classname)} :: {new_line}"
        )

    new_line = replace_macros(new_line)
    return new_line


def write_output(dbg_code, out, prepend="no"):
    global CONTEXT

    if CONTEXT.debug:
        dbg_code = f"{CONTEXT.line_idx} {dbg_code:<4} :: "
    else:
        dbg_code = ""

    if prepend == "prepend":
        CONTEXT.output.insert(0, dbg_code + out)
    else:
        if CONTEXT.if_feature_condition != "":
            CONTEXT.output.append(f"%If ({CONTEXT.if_feature_condition})\n")
        CONTEXT.output.append(dbg_code + out)
        if CONTEXT.if_feature_condition != "":
            CONTEXT.output.append("%End\n")

    CONTEXT.if_feature_condition = ""


def dbg_info(info):
    global CONTEXT

    if CONTEXT.debug:
        CONTEXT.output.append(f"{info}\n")
        print(
            f"{CONTEXT.line_idx} {len(CONTEXT.access)} {CONTEXT.sip_run} {CONTEXT.multiline_definition} {info}"
        )


def exit_with_error(message):
    global CONTEXT
    sys.exit(
        f"! Sipify error in {CONTEXT.header_file} at line :: {CONTEXT.line_idx}\n! {message}"
    )


def sip_header_footer():
    global CONTEXT
    header_footer = []
    # small hack to turn files src/core/3d/X.h to src/core/./3d/X.h
    # otherwise "sip up to date" test fails. This is because the test uses %Include entries
    # and over there we have to use ./3d/X.h entries because SIP parser does not allow a number
    # as the first letter of a relative path
    headerfile_x = re.sub(r"src/core/3d", r"src/core/./3d", CONTEXT.header_file)
    header_footer.append(
        "/************************************************************************\n"
    )
    header_footer.append(
        " * This file has been generated automatically from                      *\n"
    )
    header_footer.append(
        " *                                                                      *\n"
    )
    header_footer.append(f" * {headerfile_x:<68} *\n")
    header_footer.append(
        " *                                                                      *\n"
    )
    header_footer.append(
        " * Do not edit manually ! Edit header and run scripts/sipify.py again   *\n"
    )
    header_footer.append(
        " ************************************************************************/\n"
    )
    return header_footer


def python_header():
    global CONTEXT
    header = []
    headerfile_x = re.sub(r"src/core/3d", r"src/core/./3d", CONTEXT.header_file)
    header.append("# The following has been generated automatically from ")
    header.append(f"{headerfile_x}\n")
    return header


def create_class_links(line):
    global CONTEXT

    # Replace Qgs classes (but not the current class) with :py:class: links
    class_link_match = re.search(r"\b(Qgs[A-Z]\w+|Qgis)\b(\.?$|\W{2})", line)
    if class_link_match:
        if CONTEXT.actual_class and class_link_match.group(1) != CONTEXT.actual_class:
            line = re.sub(r"\b(Qgs[A-Z]\w+)\b(\.?$|\W{2})", r":py:class:`\1`\2", line)

    # Replace Qgs class methods with :py:func: links
    line = re.sub(r"\b((Qgs[A-Z]\w+|Qgis)\.[a-z]\w+\(\))(?!\w)", r":py:func:`\1`", line)

    # Replace other methods with :py:func: links
    if CONTEXT.actual_class:
        line = re.sub(
            r"(?<!\.)\b([a-z]\w+)\(\)(?!\w)",
            rf":py:func:`~{CONTEXT.actual_class}.\1`",
            line,
        )
    else:
        line = re.sub(r"(?<!\.)\b([a-z]\w+)\(\)(?!\w)", r":py:func:`~\1`", line)

    # Replace Qgs classes (but not the current class) with :py:class: links
    class_link_match = re.search(r"\b(?<![`~])(Qgs[A-Z]\w+|Qgis)\b(?!\()", line)
    if class_link_match:
        if (
            not CONTEXT.actual_class
            or class_link_match.group(1) != CONTEXT.actual_class
        ):
            line = re.sub(
                r"\b(?<![`~])(Qgs[A-Z]\w+|Qgis)\b(?!\()", r":py:class:`\1`", line
            )

    return line


def process_deprecated_message(message: str) -> str:
    """
    Remove all doxygen specific command from deprecated message
    """
    # SIP issue with ':' , see https://github.com/Python-SIP/sip/issues/59
    return message.replace("\\see", "").replace(":", "")


def process_doxygen_line(line: str) -> str:
    global CONTEXT

    # Handle SIP_RUN preprocessor directives
    if re.search(r"\s*#ifdef SIP_RUN", line):
        CONTEXT.doxy_inside_sip_run = 1
        return ""
    elif re.search(r"\s*#ifndef SIP_RUN", line):
        CONTEXT.doxy_inside_sip_run = 2
        return ""
    elif CONTEXT.doxy_inside_sip_run != 0 and re.search(r"\s*#else", line):
        CONTEXT.doxy_inside_sip_run = 2 if CONTEXT.doxy_inside_sip_run == 1 else 1
        return ""
    elif CONTEXT.doxy_inside_sip_run != 0 and re.search(r"\s*#endif", line):
        CONTEXT.doxy_inside_sip_run = 0
        return ""

    if CONTEXT.doxy_inside_sip_run == 2:
        return ""

    if r"\copydoc" in line:
        exit_with_error(
            "\\copydoc doxygen command cannot be used for methods exposed to Python"
        )

    if re.search(r"<(?:dl|dt|dd>)", line):
        exit_with_error(
            "Don't use raw html <dl>, <dt> or <dd> tags in documentation. "
            "Use markdown headings instead"
        )
    if re.search(r"<h\d>", line):
        exit_with_error(
            "Don't use raw html heading tags in documentation. "
            "Use markdown headings instead"
        )
    if re.search(r"<li>", line):
        exit_with_error(
            "Don't use raw html lists in documentation. " "Use markdown lists instead"
        )
    if re.search(r"<[ib]>", line):
        exit_with_error(
            "Don't use raw <i> or <b> tags in documentation. " "Use markdown instead"
        )

    # Detect code snippet
    code_match = re.search(r"\\code(\{\.?(\w+)})?", line)
    if code_match:
        codelang = f" {code_match.group(2)}" if code_match.group(2) else ""
        if not re.search(r"(cpp|py|unparsed)", codelang):
            exit_with_error(f"invalid code snippet format: {codelang}")
        CONTEXT.comment_code_snippet = CodeSnippetType.NotSpecified
        if re.search(r"cpp", codelang):
            CONTEXT.comment_code_snippet = CodeSnippetType.Cpp
        codelang = codelang.replace("py", "python").replace("unparsed", "text")
        return (
            "\n"
            if CONTEXT.comment_code_snippet == CodeSnippetType.Cpp
            else f"\n.. code-block::{codelang}\n\n"
        )

    if re.search(r"\\endcode", line):
        CONTEXT.comment_code_snippet = CodeSnippetType.NotCodeSnippet
        return "\n"

    if CONTEXT.comment_code_snippet != CodeSnippetType.NotCodeSnippet:
        if CONTEXT.comment_code_snippet == CodeSnippetType.Cpp:
            return ""
        else:
            return f"    {line}\n" if line != "" else "\n"

    # Remove prepending spaces and apply various replacements
    line = re.sub(r"^\s+", "", line)
    line = re.sub(r"\\a (.+?)\b", r"``\1``", line)
    line = line.replace("::", ".")
    line = re.sub(r"\bnullptr\b", "None", line)

    # Handle section and subsection
    section_match = re.match(r"^\\(?P<SUB>sub)?section", line)
    if section_match:
        sep = "^" if section_match.group("SUB") else "-"
        line = re.sub(r"^\\(sub)?section \w+ ", "", line)
        sep_line = re.sub(r"[\w ()]", sep, line)
        line += f"\n{sep_line}"

    # Convert ### style headings
    heading_match = re.match(r"^###\s+(.*)$", line)
    if heading_match:
        line = f"{heading_match.group(1)}\n{'-' * (len(heading_match.group(1)) + 30)}"
    heading_match = re.match(r"^##\s+(.*)$", line)
    if heading_match:
        line = f"{heading_match.group(1)}\n{'=' * (len(heading_match.group(1)) + 30)}"

    if line == "*":
        line = ""

    # Handle multi-line parameters/returns/lists
    if line != "":
        if re.match(r"^\s*[\-#]", line):
            line = f"{CONTEXT.prev_indent}{line}"
            CONTEXT.indent = f"{CONTEXT.prev_indent}  "
        elif not re.match(
            r"^\s*[\\:]+(param|note|since|return|deprecated|warning|throws)", line
        ):
            line = f"{CONTEXT.indent}{line}"
    else:
        CONTEXT.prev_indent = CONTEXT.indent
        CONTEXT.indent = ""

    # Replace \returns with :return:
    if re.search(r"\\return(s)?", line):
        line = re.sub(r"\s*\\return(s)?\s*", "\n:return: ", line)
        line = re.sub(r"\s*$", "", line)
        CONTEXT.indent = " " * (line.index(":", 4) + 1)

    # Handle params
    if re.search(r"\\param(?:\[(?:out|in|,)+])? ", line):
        line = re.sub(
            r"\s*\\param(?:\[(?:out|in|,)+])?\s+(\w+)\b\s*", r":param \1: ", line
        )
        line = re.sub(r"\s*$", "", line)
        CONTEXT.indent = " " * (line.index(":", 2) + 2)
        if line.startswith(":param"):
            if not CONTEXT.comment_param_list:
                line = f"\n{line}"
            CONTEXT.comment_param_list = True
            CONTEXT.comment_last_line_note_warning = False

    # Handle brief
    if re.match(r"^\s*[\\@]brief", line):
        line = re.sub(r"[\\@]brief\s*", "", line)
        if CONTEXT.found_since:
            exit_with_error(
                f"{CONTEXT.header_file}::{CONTEXT.line_idx} Since annotation must come after brief"
            )
        CONTEXT.found_since = False
        if re.match(r"^\s*$", line):
            return ""

    # Handle ingroup and class
    if re.search(r"[\\@](ingroup|class)", line):
        CONTEXT.prev_indent = CONTEXT.indent
        CONTEXT.indent = ""
        return ""

    # Handle since
    since_match = re.search(r"\\since .*?([\d.]+)", line, re.IGNORECASE)
    if since_match:
        CONTEXT.prev_indent = CONTEXT.indent
        CONTEXT.indent = ""
        CONTEXT.found_since = True
        return f"\n.. versionadded:: {since_match.group(1)}\n"

    # Handle deprecated
    if deprecated_match := re.search(
        r"\\deprecated QGIS (?P<DEPR_VERSION>[0-9.]+)\s*(?P<DEPR_MESSAGE>.*)?",
        line,
        re.IGNORECASE,
    ):
        CONTEXT.prev_indent = CONTEXT.indent
        CONTEXT.indent = ""
        version = deprecated_match.group("DEPR_VERSION")
        if version.endswith("."):
            version = version[:-1]
        depr_line = f"\n.. deprecated:: {version}"
        message = deprecated_match.group("DEPR_MESSAGE")
        CONTEXT.deprecated_message = (
            f"Since {version}. {process_deprecated_message(message)}"
        )
        if message:
            depr_line += "\n"
            depr_line += "\n".join(f"\n   {_m}" for _m in message.split("\n"))
        return create_class_links(depr_line)

    # Handle see also
    see_matches = list(
        re.finditer(r"\\see +([\w:/.#-]+(\.\w+)*)(\([^()]*\))?(\.?)", line)
    )
    if see_matches:
        for see_match in reversed(see_matches):
            seealso = see_match.group(1)
            seealso_suffix = see_match.group(4)

            seeline = ""
            dbg_info(f"see also: `{seealso}`")
            if re.match(r"^http", seealso):
                seeline = f"{seealso}"
            elif seealso_match := re.match(
                r"^(Qgs[A-Z]\w+(\([^()]*\))?)(\.)?$", seealso
            ):
                dbg_info(f"\\see :py:class:`{seealso_match.group(1)}`")
                seeline = f":py:class:`{seealso_match.group(1)}`{seealso_match.group(3) or ''}"
            elif seealso_match := re.match(
                r"^((Qgs[A-Z]\w+)\.(\w+)(\([^()]*\))?)(\.)?$", seealso
            ):
                dbg_info(
                    f"\\see py:func with param: :py:func:`{seealso_match.group(1)}`"
                )
                seeline = (
                    f":py:func:`{seealso_match.group(1)}`{seealso_match.group(5) or ''}"
                )
            elif seealso_match := re.match(r"^([a-z]\w+(\([^()]*\))?)(\.)?$", seealso):
                dbg_info(f"\\see :py:func:`{seealso_match.group(1)}`")
                seeline = (
                    f":py:func:`{seealso_match.group(1)}`{seealso_match.group(3) or ''}"
                )

            if full_line_match := re.match(
                r"^\s*\\see +(\w+(?:\.\w+)*)(?:\([^()]*\))?[\s,.:-]*(.*?)$", line
            ):
                if seeline.startswith("http"):
                    return f"\n.. seealso:: {seeline}\n"
                suffix = full_line_match.group(2)
                if suffix:
                    return f"\n.. seealso:: {seeline or seealso} {suffix.strip()}\n"
                else:
                    return f"\n.. seealso:: {seeline or seealso}\n"
            else:
                if seeline:
                    line = (
                        line[: see_match.start()]
                        + seeline
                        + seealso_suffix
                        + line[see_match.end() :]
                    )  # re.sub(r'\\see +(\w+(\.\w+)*(\(\))?)', seeline, line)
                else:
                    line = line.replace("\\see", "see")
    elif not re.search(r"\\throws.*", line):
        line = create_class_links(line)

    # Handle note, warning, and throws
    note_match = re.search(r"[\\@]note (.*)", line)
    if note_match:
        CONTEXT.comment_last_line_note_warning = True
        CONTEXT.prev_indent = CONTEXT.indent
        CONTEXT.indent = ""
        return f"\n.. note::\n\n   {note_match.group(1)}\n"

    warning_match = re.search(r"[\\@]warning (.*)", line)
    if warning_match:
        CONTEXT.prev_indent = CONTEXT.indent
        CONTEXT.indent = ""
        CONTEXT.comment_last_line_note_warning = True
        return f"\n.. warning::\n\n   {warning_match.group(1)}\n"

    throws_match = re.search(r"[\\@]throws (.+?)\b\s*(.*)", line)
    if throws_match:
        CONTEXT.prev_indent = CONTEXT.indent
        CONTEXT.indent = ""
        CONTEXT.comment_last_line_note_warning = True
        return f"\n:raises {throws_match.group(1)}: {throws_match.group(2)}\n"

    if line.strip():
        if CONTEXT.comment_last_line_note_warning:
            dbg_info(f"prepend spaces for multiline warning/note xx{line}")
            line = f"   {line}"
    else:
        CONTEXT.comment_last_line_note_warning = False

    return f"{line}\n"


def detect_and_remove_following_body_or_initializerlist():
    global CONTEXT

    signature = ""

    # Complex regex pattern to match various C++ function declarations and definitions
    pattern1 = r'^(\s*)?((?:(?:explicit|static|const|unsigned|virtual)\s+)*)(([(?:long )\w:]+(<.*?>)?\s+[*&]?)?(~?\w+|(\w+::)?operator.{1,2})\s*\(([\w=()\/ ,&*<>."-]|::)*\)( +(?:const|SIP_[\w_]+?))*)\s*((\s*[:,]\s+\w+\(.*\))*\s*\{.*\}\s*(?:SIP_[\w_]+)?;?|(?!;))(\s*\/\/.*)?$'
    pattern2 = r"SIP_SKIP\s*(?!;)\s*(\/\/.*)?$"
    pattern3 = r"^\s*class.*SIP_SKIP"

    if (
        re.match(pattern1, CONTEXT.current_line)
        or re.search(pattern2, CONTEXT.current_line)
        or re.match(pattern3, CONTEXT.current_line)
    ):

        dbg_info(
            "remove constructor definition, function bodies, member initializing list (1)"
        )

        # Extract the parts we want to keep
        initializer_match = re.match(pattern1, CONTEXT.current_line)
        if initializer_match:
            newline = f"{initializer_match.group(1) or ''}{initializer_match.group(2) or ''}{initializer_match.group(3)};"
        else:
            newline = CONTEXT.current_line

        # Call remove_following_body_or_initializerlist() if necessary
        if not re.search(r"{.*}(\s*SIP_\w+)*\s*(//.*)?$", CONTEXT.current_line):
            signature = remove_following_body_or_initializerlist()

        CONTEXT.current_line = newline

    return signature


def remove_following_body_or_initializerlist():
    global CONTEXT

    signature = ""

    dbg_info(
        "remove constructor definition, function bodies, member initializing list (2)"
    )
    line = read_line()

    # Python signature
    if re.match(r"^\s*\[\s*(\w+\s*)?\(", line):
        dbg_info("python signature detected")
        _nesting_index = 0
        while CONTEXT.line_idx < CONTEXT.line_count:
            _nesting_index += line.count("[")
            _nesting_index -= line.count("]")
            if _nesting_index == 0:
                line_match = re.match(r"^(.*);\s*(//.*)?$", line)
                if line_match:
                    line = line_match.group(1)  # remove semicolon (added later)
                    signature += f"\n{line}"
                    return signature
                break
            signature += f"\n{line}"
            line = read_line()

    # Member initializing list
    while re.match(r"^\s*[:,]\s+([\w<>]|::)+\(.*?\)", line):
        dbg_info("member initializing list")
        line = read_line()

    # Body
    if re.match(r"^\s*\{", line):
        _nesting_index = 0
        while CONTEXT.line_idx < CONTEXT.line_count:
            dbg_info("  remove body")
            _nesting_index += line.count("{")
            _nesting_index -= line.count("}")
            if _nesting_index == 0:
                break
            line = read_line()

    return signature


def replace_alternative_types(text):
    """
    Handle SIP_PYALTERNATIVETYPE annotation
    """
    # Original perl regex was:
    # s/(\w+)(\<(?>[^<>]|(?2))*\>)?\s+SIP_PYALTERNATIVETYPE\(\s*\'?([^()']+)(\(\s*(?:[^()]++|(?2))*\s*\))?\'?\s*\)/$3/g;
    _pattern = r"(\w+)(<(?:[^<>]|<(?:[^<>]|<[^<>]*>)*>)*>)?\s+SIP_PYALTERNATIVETYPE\(\s*\'?([^()\']+)(\(\s*(?:[^()]|\([^()]*\))*\s*\))?\'?\s*\)"

    while True:
        new_text = re.sub(_pattern, r"\3", text, flags=re.S)
        if new_text == text:
            return text
        text = new_text


def split_args(args_string: str) -> list[str]:
    """
    Tries to split a line of arguments into separate parts
    """
    res = []
    current_arg = ""
    paren_level = 0
    angle_level = 0

    for char in args_string:
        if char == "," and paren_level == 0 and angle_level == 0:
            res.append(current_arg.strip())
            current_arg = ""
        else:
            current_arg += char
            if char == "(":
                paren_level += 1
            elif char == ")":
                paren_level -= 1
            elif char == "<":
                angle_level += 1
            elif char == ">":
                angle_level -= 1

    if current_arg:
        res.append(current_arg.strip())

    return res


def remove_sip_pyargremove(input_string: str) -> str:
    """
    Remove SIP_PYARGREMOVE annotated arguments
    """
    global CONTEXT
    # Split the string into function signature and body
    signature_split = re.match(r"(.*?)\((.*)\)(.*)", input_string)
    if signature_split and "SIP_PYARGREMOVE" not in signature_split.group(1):
        prefix, arguments, suffix = signature_split.groups()
        prefix += "("
        suffix = ")" + suffix
    else:
        signature_split = re.match(r"(\s*)(.*)\)(.*)", input_string)
        if signature_split:
            prefix, arguments, suffix = signature_split.groups()
            suffix = ")" + suffix
        else:
            prefix = ""
            arguments = input_string
            suffix = ""

    arguments_list = split_args(arguments)

    if CONTEXT.is_qt6:
        filtered_args = [arg for arg in arguments_list if "SIP_PYARGREMOVE" not in arg]
    else:
        filtered_args = [
            re.sub(r"\s*SIP_PYARGREMOVE6\s*", " ", arg)
            for arg in arguments_list
            if not ("SIP_PYARGREMOVE" in arg and "SIP_PYARGREMOVE6" not in arg)
        ]

    # Reassemble the function signature
    remaining_args = ", ".join(filtered_args)
    if remaining_args and prefix.strip():
        prefix += " "
    if remaining_args and suffix.strip():
        suffix = " " + suffix
    return f"{prefix}{remaining_args}{suffix}"


def fix_annotations(line):
    global CONTEXT

    # Get removed params to be able to drop them out of the API doc
    removed_params = re.findall(r"(\w+)\s+SIP_PYARGREMOVE", line)
    if CONTEXT.is_qt6:
        removed_params = re.findall(r"(\w+)\s+SIP_PYARGREMOVE6?", line)
    for param in removed_params:
        CONTEXT.skipped_params_remove.append(param)
        dbg_info(f"caught removed param: {CONTEXT.skipped_params_remove[-1]}")

    _out_params = re.findall(r"(\w+)\s+SIP_OUT", line)
    for param in _out_params:
        CONTEXT.skipped_params_out.append(param)
        dbg_info(f"caught removed param: {CONTEXT.skipped_params_out[-1]}")

    # Printed annotations
    replacements = {
        r"//\s*SIP_ABSTRACT\b": "/Abstract/",
        r"\bSIP_ABSTRACT\b": "/Abstract/",
        r"\bSIP_ALLOWNONE\b": "/AllowNone/",
        r"\bSIP_ARRAY\b": "/Array/",
        r"\bSIP_ARRAYSIZE\b": "/ArraySize/",
        r"\bSIP_CONSTRAINED\b": "/Constrained/",
        r"\bSIP_EXTERNAL\b": "/External/",
        r"\bSIP_FACTORY\b": "/Factory/",
        r"\bSIP_IN\b": "/In/",
        r"\bSIP_INOUT\b": "/In,Out/",
        r"\bSIP_KEEPREFERENCE\b": "/KeepReference/",
        r"\bSIP_NODEFAULTCTORS\b": "/NoDefaultCtors/",
        r"\bSIP_OUT\b": "/Out/",
        r"\bSIP_RELEASEGIL\b": "/ReleaseGIL/",
        r"\bSIP_HOLDGIL\b": "/HoldGIL/",
        r"\bSIP_TRANSFER\b": "/Transfer/",
        r"\bSIP_TRANSFERBACK\b": "/TransferBack/",
        r"\bSIP_TRANSFERTHIS\b": "/TransferThis/",
        r"\bSIP_GETWRAPPER\b": "/GetWrapper/",
        r"SIP_PYNAME\(\s*(\w+)\s*\)": r"/PyName=\1/",
        r"SIP_TYPEHINT\(\s*([\w\.\s,\[\]]+?)\s*\)": r'/TypeHint="\1"/',
        r"SIP_VIRTUALERRORHANDLER\(\s*(\w+)\s*\)": r"/VirtualErrorHandler=\1/",
    }

    if not CONTEXT.is_qt6:
        replacements[r"SIP_THROW\(\s*([\w\s,]+?)\s*\)"] = r"throw( \1 )"
    else:
        # these have no effect (and aren't required) on sip >= 6
        replacements[r"SIP_THROW\(\s*([\w\s,]+?)\s*\)"] = ""

    if CONTEXT.deprecated_message:
        replacements[r"\bSIP_DEPRECATED\b"] = (
            f'/Deprecated="{CONTEXT.deprecated_message}"/'
        )
    else:
        replacements[r"\bSIP_DEPRECATED\b"] = f"/Deprecated/"

    for _pattern, replacement in replacements.items():
        line = re.sub(_pattern, replacement, line)

    # Combine multiple annotations
    while True:
        new_line = re.sub(
            r'/([\w,]+(="?[^"]+"?)?)/\s*/([\w,]+(="?[^"]+"?)?]?)/', r"/\1,\3/", line
        )
        if new_line == line:
            break
        line = new_line
        dbg_info("combine multiple annotations -- works only for 2")

    # Unprinted annotations
    line = replace_alternative_types(line)
    line = re.sub(r"(\w+)\s+SIP_PYARGRENAME\(\s*(\w+)\s*\)", r"\2", line)

    # Note: this was the original perl regex, which isn't compatible with Python:
    # line = re.sub(r"""=\s+[^=]*?\s+SIP_PYARGDEFAULT\(\s*\'?([^()']+)(\(\s*(?:[^()]++|(?2))*\s*\))?\'?\s*\)""", r'= \1', line)
    line = re.sub(
        r"""=\s+[^=]*?\s+SIP_PYARGDEFAULT\(\s*\'?([^()\']+)(\((?:[^()]|\([^()]*\))*\))?\'?\s*\)""",
        r"= \1",
        line,
    )

    # Remove argument
    if "SIP_PYARGREMOVE" in line:
        dbg_info("remove arg")
        if CONTEXT.multiline_definition != MultiLineType.NotMultiline:
            prev_line = CONTEXT.output.pop().rstrip()
            # Update multi line status
            parenthesis_balance = prev_line.count("(") - prev_line.count(")")
            if parenthesis_balance == 1:
                CONTEXT.multiline_definition = MultiLineType.NotMultiline
            # Concatenate with above line to bring previous commas
            line = f"{prev_line} {line.lstrip()}\n"

        # original perl regex was:
        # (?<coma>, +)?(const )?(\w+)(\<(?>[^<>]|(?4))*\>)?\s+[\w&*]+\s+SIP_PYARGREMOVE( = [^()]*(\(\s*(?:[^()]++|(?6))*\s*\))?)?(?(<coma>)|,?)//
        if "SIP_PYARGREMOVE" in line:
            line = remove_sip_pyargremove(line)

        line = re.sub(r"\(\s+\)", "()", line)

    line = re.sub(r"SIP_FORCE", "", line)
    line = re.sub(r"SIP_DOC_TEMPLATE", "", line)
    line = re.sub(r"\s+;$", ";", line)

    return line


def fix_constants(line):
    line = re.sub(r"\bstd::numeric_limits<double>::max\(\)", "DBL_MAX", line)
    line = re.sub(r"\bstd::numeric_limits<double>::lowest\(\)", "-DBL_MAX", line)
    line = re.sub(r"\bstd::numeric_limits<double>::epsilon\(\)", "DBL_EPSILON", line)
    line = re.sub(r"\bstd::numeric_limits<qlonglong>::min\(\)", "LLONG_MIN", line)
    line = re.sub(r"\bstd::numeric_limits<qlonglong>::max\(\)", "LLONG_MAX", line)
    line = re.sub(r"\bstd::numeric_limits<int>::max\(\)", "INT_MAX", line)
    line = re.sub(r"\bstd::numeric_limits<int>::min\(\)", "INT_MIN", line)
    return line


def detect_comment_block(strict_mode=True):
    # Initialize global or module-level variables if necessary
    global CONTEXT

    CONTEXT.comment_param_list = False
    CONTEXT.indent = ""
    CONTEXT.prev_indent = ""
    CONTEXT.comment_code_snippet = CodeSnippetType.NotCodeSnippet
    CONTEXT.comment_last_line_note_warning = False
    CONTEXT.found_since = False
    if CONTEXT.multiline_definition == MultiLineType.NotMultiline:
        CONTEXT.skipped_params_out = []
        CONTEXT.skipped_params_remove = []

    if re.match(r"^\s*/\*", CONTEXT.current_line) or (
        not strict_mode and "/*" in CONTEXT.current_line
    ):
        dbg_info("found comment block")
        CONTEXT.comment = process_doxygen_line(
            re.sub(r"^\s*/\*(\*)?(.*?)\n?$", r"\2", CONTEXT.current_line)
        )
        CONTEXT.comment = re.sub(r"^\s*$", "", CONTEXT.comment)

        while not re.search(r"\*/\s*(//.*?)?$", CONTEXT.current_line):
            CONTEXT.current_line = read_line()
            CONTEXT.comment += process_doxygen_line(
                re.sub(r"\s*\*?(.*?)(/)?\n?$", r"\1", CONTEXT.current_line)
            )

        CONTEXT.comment = re.sub(r"\n\s+\n", "\n\n", CONTEXT.comment)
        CONTEXT.comment = re.sub(r"\n{3,}", "\n\n", CONTEXT.comment)
        CONTEXT.comment = re.sub(r"\n+$", "", CONTEXT.comment)

        return True

    return False


def detect_non_method_member(line):
    _pattern = r"""^\s*(?:template\s*<\w+>\s+)?(?:(const|mutable|static|friend|unsigned)\s+)*\w+(::\w+)?(<([\w<> *&,()]|::)+>)?(,?\s+\*?\w+( = (-?\d+(\.\d+)?|((QMap|QList)<[^()]+>\(\))|(\w+::)*\w+(\([^()]?\))?)|\[\d+\])?)+;"""
    return re.match(_pattern, line)


def convert_type(cpp_type: str) -> str:
    """
    Converts C++ types to Python types
    """
    type_mapping = {
        "int": "int",
        "float": "float",
        "double": "float",
        "bool": "bool",
        "char": "str",
        "QString": "str",
        "void": "None",
        "qint64": "int",
        "unsigned long long": "int",
        "long long": "int",
        "qlonglong": "int",
        "long": "int",
        "QStringList": "List[str]",
        "QVariantList": "List[object]",
        "QVariantMap": "Dict[str, object]",
        "QVariant": "object",
    }

    # Handle templates
    template_match = re.match(r"(\w+)\s*<\s*(.+)\s*>", cpp_type)
    if template_match:
        container, inner_type = template_match.groups()
        if container in ("QVector", "QList"):
            return f"List[{convert_type(inner_type.strip())}]"
        elif container in ("QSet",):
            return f"Set[{convert_type(inner_type.strip())}]"
        elif container in ("QHash", "QMap"):
            key_type, value_type = (t.strip() for t in inner_type.split(","))
            return f"Dict[{convert_type(key_type)}, {convert_type(value_type)}]"
        else:
            return f"{container}[{convert_type(inner_type.strip())}]"

    if cpp_type not in type_mapping:
        if cpp_type.startswith("Q"):
            cpp_type = cpp_type.replace("::", ".")
            return cpp_type

        assert False, cpp_type

    return type_mapping[cpp_type]


def parse_argument(arg: str) -> tuple[str, str, Optional[str]]:
    # Remove leading/trailing whitespace and 'const'
    arg = re.sub(r"^\s*const\s+", "", arg.strip())

    # Extract default value if present
    default_match = re.search(r"=\s*(.+)$", arg)
    default_value = default_match.group(1).strip() if default_match else None
    arg = re.sub(r"\s*=\s*.+$", "", arg)

    # Handle pointers and references
    is_pointer = "*" in arg
    arg = arg.replace("*", "").replace("&", "").strip()

    # Split type and variable name
    parts = arg.split()
    if len(parts) > 1:
        cpp_type = " ".join(parts[:-1])
        var_name = parts[-1]
    else:
        cpp_type = arg
        var_name = ""

    python_type = convert_type(cpp_type)
    if is_pointer and default_value:
        python_type = f"Optional[{python_type}]"

    # Convert default value
    if default_value:
        default_value_map = {"QVariantList()": "[]"}
        if default_value in default_value_map:
            default_value = default_value_map[default_value]
        elif default_value == "nullptr":
            default_value = "None"
        elif python_type == "int":
            pass
        elif cpp_type in ("QString",):
            if default_value == "QString()":
                default_value = "None"
                python_type = f"Optional[{python_type}]"
            elif default_value.startswith("Q"):
                default_value = default_value.replace("::", ".")
            else:
                default_value = f'"{default_value}"'
        elif cpp_type in ("bool",):
            default_value = f'{"False" if default_value == "false" else "True"}'
        elif cpp_type.startswith("Q"):
            default_value = default_value.replace("::", ".")
        else:
            assert False, (default_value, cpp_type)

    return var_name, python_type, default_value


def cpp_to_python_signature(cpp_function: str) -> str:

    # Extract function name and arguments
    match = re.match(
        r"(\w+)\s*\((.*)\)\s*(?:const)?\s*(?:->)?\s*([\w:]+)?", cpp_function
    )
    if not match:
        raise ValueError("Invalid C++ function signature")

    func_name, args_str, return_type = match.groups()
    args = [arg.strip() for arg in args_str.split(",") if arg.strip()]

    # Parse arguments
    python_args = []
    for arg in args:
        var_name, python_type, default_value = parse_argument(arg)
        if default_value:
            python_args.append(f"{var_name}: {python_type} = {default_value}")
        else:
            python_args.append(f"{var_name}: {python_type}")

    # Construct Python function signature
    python_signature = f"def {func_name}({', '.join(python_args)})"
    if return_type:
        python_signature += f" -> {convert_type(return_type)}"

    return python_signature


while CONTEXT.line_idx < CONTEXT.line_count:

    CONTEXT.python_signature = ""
    CONTEXT.actual_class = CONTEXT.classname[-1] if CONTEXT.classname else None
    CONTEXT.current_line = read_line()

    if re.match(r"^\s*(#define\s+)?SIP_IF_MODULE\(.*\)$", CONTEXT.current_line):
        dbg_info("skipping SIP include condition macro")
        continue

    match = re.match(r"^(.*?)\s*//\s*cppcheck-suppress.*$", CONTEXT.current_line)
    if match:
        CONTEXT.current_line = match.group(1)

    match = re.match(r"^\s*SIP_FEATURE\(\s*(\w+)\s*\)(.*)$", CONTEXT.current_line)
    if match:
        write_output("SF1", f"%Feature {match.group(1)}{match.group(2)}\n")
        continue

    match = re.match(r"^\s*SIP_PROPERTY\((.*)\)$", CONTEXT.current_line)
    if match:
        write_output("SF1", f"%Property({match.group(1)})\n")
        continue

    match = re.match(r"^\s*SIP_IF_FEATURE\(\s*(!?\w+)\s*\)(.*)$", CONTEXT.current_line)
    if match:
        write_output("SF2", f"%If ({match.group(1)}){match.group(2)}\n")
        continue

    match = re.match(r"^\s*SIP_CONVERT_TO_SUBCLASS_CODE(.*)$", CONTEXT.current_line)
    if match:
        # TYPE HEADER CODE
        if CONTEXT.header_code and not re.match(r"^ *//.*$", CONTEXT.current_line):
            CONTEXT.header_code = False
            write_output("HCE", "%End\n")
        CONTEXT.current_line = f"%ConvertToSubClassCode{match.group(1)}"
        # Do not continue here, let the code process the next steps

    match = re.match(r"^\s*SIP_VIRTUAL_CATCHER_CODE(.*)$", CONTEXT.current_line)
    if match:
        CONTEXT.current_line = f"%VirtualCatcherCode{match.group(1)}"
        # Do not continue here, let the code process the next steps

    match = re.match(r"^\s*SIP_END(.*)$", CONTEXT.current_line)
    if match:
        write_output("SEN", f"%End{match.group(1)}\n")
        continue

    match = re.search(r"SIP_WHEN_FEATURE\(\s*(.*?)\s*\)", CONTEXT.current_line)
    if match:
        dbg_info("found SIP_WHEN_FEATURE")
        CONTEXT.if_feature_condition = match.group(1)

    match = re.search(r'SIP_TYPEHEADER_INCLUDE\(\s*"(.*?)"\s*\)', CONTEXT.current_line)
    if match:
        dbg_info("found SIP_TYPEHEADER_INCLUDE")
        write_output("STI", f'#include "{match.group(1)}"\n')
        continue

    if CONTEXT.is_qt6:
        CONTEXT.current_line = re.sub(
            r"int\s*__len__\s*\(\s*\)", "Py_ssize_t __len__()", CONTEXT.current_line
        )
        CONTEXT.current_line = re.sub(
            r"long\s*__hash__\s*\(\s*\)", "Py_hash_t __hash__()", CONTEXT.current_line
        )

    if CONTEXT.is_qt6 and re.match(r"^\s*#ifdef SIP_PYQT5_RUN", CONTEXT.current_line):
        dbg_info("do not process PYQT5 code")
        while not re.match(r"^#endif", CONTEXT.current_line):
            CONTEXT.current_line = read_line()

    if not CONTEXT.is_qt6 and re.match(
        r"^\s*#ifdef SIP_PYQT6_RUN", CONTEXT.current_line
    ):
        dbg_info("do not process PYQT6 code")
        while not re.match(r"^#endif", CONTEXT.current_line):
            CONTEXT.current_line = read_line()

    # Do not process SIP code %XXXCode
    if CONTEXT.sip_run and re.match(
        r"^ *[/]*% *(VirtualErrorHandler|MappedType|Type(?:Header)?Code|Module(?:Header)?Code|Convert(?:From|To)(?:Type|SubClass)Code|MethodCode|Docstring)(.*)?$",
        CONTEXT.current_line,
    ):
        CONTEXT.current_line = (
            f"%{re.match(r'^ *[/]*% *(.*)$', CONTEXT.current_line).group(1)}"
        )
        CONTEXT.comment = ""
        dbg_info("do not process SIP code")
        while not re.match(r"^ *[/]*% *End", CONTEXT.current_line):
            write_output("COD", CONTEXT.current_line + "\n")
            CONTEXT.current_line = read_line()
            if CONTEXT.is_qt6:
                CONTEXT.current_line = re.sub(
                    r"SIP_SSIZE_T", "Py_ssize_t", CONTEXT.current_line
                )
                CONTEXT.current_line = re.sub(
                    r"SIPLong_AsLong", "PyLong_AsLong", CONTEXT.current_line
                )
            CONTEXT.current_line = re.sub(
                r"^ *[/]*% *(VirtualErrorHandler|MappedType|Type(?:Header)?Code|Module(?:Header)?Code|Convert(?:From|To)(?:Type|SubClass)Code|MethodCode|Docstring)(.*)?$",
                r"%\1\2",
                CONTEXT.current_line,
            )
            CONTEXT.current_line = re.sub(
                r"^\s*SIP_END(.*)$", r"%End\1", CONTEXT.current_line
            )

        CONTEXT.current_line = re.sub(r"^\s*[/]*% *End", "%End", CONTEXT.current_line)
        write_output("COD", CONTEXT.current_line + "\n")
        continue

    # Do not process SIP code %Property
    if CONTEXT.sip_run and re.match(
        r"^ *[/]*% *(Property)(.*)?$", CONTEXT.current_line
    ):
        CONTEXT.current_line = (
            f"%{re.match(r'^ *% *(.*)$', CONTEXT.current_line).group(1)}"
        )
        CONTEXT.comment = ""
        write_output("COD", CONTEXT.current_line + "\n")
        continue

    # Do not process SIP code %If %End
    if CONTEXT.sip_run and re.match(r"^ *[/]*% *(If|End)(.*)?$", CONTEXT.current_line):
        CONTEXT.current_line = (
            f"%{re.match(r'^ *% (.*)$', CONTEXT.current_line).group(1)}"
        )
        CONTEXT.comment = ""
        write_output("COD", CONTEXT.current_line)
        continue

    # Skip preprocessor directives
    if re.match(r"^\s*#", CONTEXT.current_line):
        # Skip #if 0 or #if defined(Q_OS_WIN) blocks
        match = re.match(r"^\s*#if (0|defined\(Q_OS_WIN\))", CONTEXT.current_line)
        if match:
            dbg_info(f"skipping #if {match.group(1)} block")
            nesting_index = 0
            while CONTEXT.line_idx < CONTEXT.line_count:
                CONTEXT.current_line = read_line()
                if re.match(r"^\s*#if(def)?\s+", CONTEXT.current_line):
                    nesting_index += 1
                elif nesting_index == 0 and re.match(
                    r"^\s*#(endif|else)", CONTEXT.current_line
                ):
                    CONTEXT.comment = ""
                    break
                elif nesting_index != 0 and re.match(
                    r"^\s*#endif", CONTEXT.current_line
                ):
                    nesting_index -= 1
            continue

        if re.match(r"^\s*#ifdef SIP_RUN", CONTEXT.current_line):
            CONTEXT.sip_run = True
            if CONTEXT.access[-1] == Visibility.Private:
                dbg_info("writing private content (1)")
                if CONTEXT.private_section_line:
                    write_output("PRV1", CONTEXT.private_section_line + "\n")
                CONTEXT.private_section_line = ""
            continue

        if CONTEXT.sip_run:
            if re.match(r"^\s*#endif", CONTEXT.current_line):
                if CONTEXT.ifdef_nesting_idx == 0:
                    CONTEXT.sip_run = False
                    continue
                else:
                    CONTEXT.ifdef_nesting_idx -= 1

            if re.match(r"^\s*#if(def)?\s+", CONTEXT.current_line):
                CONTEXT.ifdef_nesting_idx += 1

            # If there is an else at this level, code will be ignored (i.e., not SIP_RUN)
            if (
                re.match(r"^\s*#else", CONTEXT.current_line)
                and CONTEXT.ifdef_nesting_idx == 0
            ):
                while CONTEXT.line_idx < CONTEXT.line_count:
                    CONTEXT.current_line = read_line()
                    if re.match(r"^\s*#if(def)?\s+", CONTEXT.current_line):
                        CONTEXT.ifdef_nesting_idx += 1
                    elif re.match(r"^\s*#endif", CONTEXT.current_line):
                        if CONTEXT.ifdef_nesting_idx == 0:
                            CONTEXT.comment = ""
                            CONTEXT.sip_run = False
                            break
                        else:
                            CONTEXT.ifdef_nesting_idx -= 1
                continue

        elif re.match(r"^\s*#ifndef SIP_RUN", CONTEXT.current_line):
            # Code is ignored here
            while CONTEXT.line_idx < CONTEXT.line_count:
                CONTEXT.current_line = read_line()
                if re.match(r"^\s*#if(def)?\s+", CONTEXT.current_line):
                    CONTEXT.ifdef_nesting_idx += 1
                elif (
                    re.match(r"^\s*#else", CONTEXT.current_line)
                    and CONTEXT.ifdef_nesting_idx == 0
                ):
                    # Code here will be printed out
                    if CONTEXT.access[-1] == Visibility.Private:
                        dbg_info("writing private content (2)")
                        if CONTEXT.private_section_line != "":
                            write_output("PRV2", CONTEXT.private_section_line + "\n")
                        CONTEXT.private_section_line = ""
                    CONTEXT.sip_run = True
                    break
                elif re.match(r"^\s*#endif", CONTEXT.current_line):
                    if CONTEXT.ifdef_nesting_idx == 0:
                        CONTEXT.sip_run = 0
                        break
                    else:
                        CONTEXT.ifdef_nesting_idx -= 1
            continue

        else:
            continue

    # TYPE HEADER CODE
    if (
        CONTEXT.header_code
        and not CONTEXT.sip_run
        and not re.match(r"^ *//.*$", CONTEXT.current_line)
    ):
        CONTEXT.header_code = False
        write_output("HCE", "%End\n")

    # Skip forward declarations
    match = re.match(
        r"^\s*(template ?<class T> |enum\s+)?(class|struct) \w+(?P<external> *SIP_EXTERNAL)?;\s*(//.*)?$",
        CONTEXT.current_line,
    )
    if match:
        if match.group("external"):
            dbg_info("do not skip external forward declaration")
            CONTEXT.comment = ""
        else:
            dbg_info("skipping forward declaration")
            continue

    # Skip friend declarations
    if re.match(r"^\s*friend class \w+", CONTEXT.current_line):
        continue

    # Insert metaobject for Q_GADGET
    if re.match(r"^\s*Q_GADGET\b.*?$", CONTEXT.current_line):
        if not re.search(r"SIP_SKIP", CONTEXT.current_line):
            dbg_info("Q_GADGET")
            write_output("HCE", "  public:\n")
            write_output("HCE", "    static const QMetaObject staticMetaObject;\n\n")
        continue

    # Insert in Python output (python/module/__init__.py)
    match = re.search(r"Q_(ENUM|FLAG)\(\s*(\w+)\s*\)", CONTEXT.current_line)
    if match:
        if not re.search(r"SIP_SKIP", CONTEXT.current_line):
            is_flag = 1 if match.group(1) == "FLAG" else 0
            enum_helper = f"{CONTEXT.actual_class}.{match.group(2)}.baseClass = {CONTEXT.actual_class}"
            dbg_info(f"Q_ENUM/Q_FLAG {enum_helper}")
            if args.python_output:
                if enum_helper != "":
                    CONTEXT.output_python.append(f"{enum_helper}\n")
                    if is_flag == 1:
                        # SIP seems to introduce the flags in the module rather than in the class itself
                        # as a dirty hack, inject directly in module, hopefully we don't have flags with the same name...
                        CONTEXT.output_python.append(
                            f"{match.group(2)} = {CONTEXT.actual_class}  # dirty hack since SIP seems to introduce the flags in module\n"
                        )
        continue

    # Skip Q_OBJECT, Q_PROPERTY, Q_ENUM, etc.
    if re.match(
        r"^\s*Q_(OBJECT|ENUMS|ENUM|FLAG|PROPERTY|DECLARE_METATYPE|DECLARE_TYPEINFO|NOWARN_DEPRECATED_(PUSH|POP))\b.*?$",
        CONTEXT.current_line,
    ):
        continue

    if re.match(r"^\s*QHASH_FOR_CLASS_ENUM", CONTEXT.current_line):
        continue

    if re.search(r"SIP_SKIP|SIP_PYTHON_SPECIAL_", CONTEXT.current_line):
        dbg_info("SIP SKIP!")
        # if multiline definition, remove previous lines
        if CONTEXT.multiline_definition != MultiLineType.NotMultiline:
            dbg_info("SIP_SKIP with MultiLine")
            opening_line = ""
            while not re.match(
                r"^[^()]*\(([^()]*\([^()]*\)[^()]*)*[^()]*$", opening_line
            ):
                opening_line = CONTEXT.output.pop()
                if len(CONTEXT.output) < 1:
                    exit_with_error("could not reach opening definition")
            dbg_info("removed multiline definition of SIP_SKIP method")
            CONTEXT.multiline_definition = MultiLineType.NotMultiline
            del CONTEXT.static_methods[CONTEXT.current_fully_qualified_class_name()][
                CONTEXT.current_method_name
            ]

        # also skip method body if there is one
        detect_and_remove_following_body_or_initializerlist()

        # line skipped, go to next iteration
        match = re.search(
            r'SIP_PYTHON_SPECIAL_(\w+)\(\s*(".*"|\w+)\s*\)', CONTEXT.current_line
        )
        if match:
            method_or_code = match.group(2)
            dbg_info(f"PYTHON SPECIAL method or code: {method_or_code}")
            pyop = (
                f"{CONTEXT.actual_class}.__{match.group(1).lower()}__ = lambda self: "
            )
            if re.match(r'^".*"$', method_or_code):
                pyop += method_or_code.strip('"')
            else:
                pyop += f"self.{method_or_code}()"
            dbg_info(f"PYTHON SPECIAL {pyop}")
            if args.python_output:
                CONTEXT.output_python.append(f"{pyop}\n")

        CONTEXT.comment = ""
        continue

    # Detect comment block
    if detect_comment_block():
        continue

    struct_match = re.match(
        r"^\s*struct(\s+\w+_EXPORT)?\s+(?P<structname>\w+)$", CONTEXT.current_line
    )
    if struct_match:
        dbg_info("  going to struct => public")
        CONTEXT.class_and_struct.append(struct_match.group("structname"))
        CONTEXT.classname.append(
            CONTEXT.classname[-1]
            if CONTEXT.classname
            else struct_match.group("structname")
        )  # fake new class since struct has considered similarly
        if CONTEXT.access[-1] != Visibility.Private:
            CONTEXT.all_fully_qualified_class_names.append(
                CONTEXT.current_fully_qualified_struct_name()
            )
        CONTEXT.access.append(Visibility.Public)
        CONTEXT.exported.append(CONTEXT.exported[-1])
        CONTEXT.bracket_nesting_idx.append(0)

    # class declaration started
    # https://regex101.com/r/KMQdF5/1 (older versions: https://regex101.com/r/6FWntP/16)
    class_pattern = re.compile(
        r"""^(\s*(class))\s+([A-Z0-9_]+_EXPORT\s+)?(Q_DECL_DEPRECATED\s+)?(?P<classname>\w+)(?P<domain>\s*:\s*(public|protected|private)\s+\w+(< *(\w|::)+ *(, *(\w|::)+ *)*>)?(::\w+(<(\w|::)+(, *(\w|::)+)*>)?)*(,\s*(public|protected|private)\s+\w+(< *(\w|::)+ *(, *(\w|::)+)*>)?(::\w+(<\w+(, *(\w|::)+)?>)?)*)*)?(?P<annot>\s*/?/?\s*SIP_\w+)?\s*?(//.*|(?!;))$"""
    )
    class_pattern_match = class_pattern.match(CONTEXT.current_line)

    if class_pattern_match:
        dbg_info("class definition started")
        CONTEXT.exported.append(0)
        CONTEXT.bracket_nesting_idx.append(0)
        template_inheritance_template = []
        template_inheritance_class1 = []
        template_inheritance_class2 = []
        template_inheritance_class3 = []

        CONTEXT.classname.append(class_pattern_match.group("classname"))
        CONTEXT.class_and_struct.append(class_pattern_match.group("classname"))
        if CONTEXT.access[-1] != Visibility.Private:
            CONTEXT.all_fully_qualified_class_names.append(
                CONTEXT.current_fully_qualified_struct_name()
            )
        CONTEXT.access.append(Visibility.Public)

        if len(CONTEXT.classname) == 1:
            CONTEXT.declared_classes.append(CONTEXT.classname[-1])

        dbg_info(f"class: {CONTEXT.classname[-1]}")

        if (
            re.search(r"\b[A-Z0-9_]+_EXPORT\b", CONTEXT.current_line)
            or len(CONTEXT.classname) != 1
            or re.search(r"^\s*template\s*<", CONTEXT.input_lines[CONTEXT.line_idx - 2])
        ):
            CONTEXT.exported[-1] += 1

        CONTEXT.current_line = (
            f"{class_pattern_match.group(1)} {class_pattern_match.group('classname')}"
        )

        # append to class map file
        if args.class_map:
            with open(args.class_map, "a") as fh3:
                fh3.write(
                    f"{'.'.join(CONTEXT.classname)}: {CONTEXT.header_file}#L{CONTEXT.line_idx}\n"
                )

        # Inheritance
        if class_pattern_match.group("domain"):
            m = class_pattern_match.group("domain")
            m = re.sub(r"public +(\w+, *)*(Ui::\w+,? *)+", "", m)
            m = re.sub(r"public +", "", m)
            m = re.sub(r"[,:]?\s*private +\w+(::\w+)?", "", m)

            # detect template based inheritance
            # https://regex101.com/r/9LGhyy/1
            tpl_pattern = re.compile(
                r"[,:]\s+(?P<tpl>(?!QList)\w+)< *(?P<cls1>(\w|::)+) *(, *(?P<cls2>(\w|::)+)? *(, *(?P<cls3>(\w|::)+)? *)?)? *>"
            )

            for match in tpl_pattern.finditer(m):
                dbg_info("template class")
                template_inheritance_template.append(match.group("tpl"))
                template_inheritance_class1.append(match.group("cls1"))
                template_inheritance_class2.append(match.group("cls2") or "")
                template_inheritance_class3.append(match.group("cls3") or "")

            dbg_info(f"domain: {m}")

            tpl_replace_pattern = re.compile(
                r"\b(?P<tpl>(?!QList)\w+)< *(?P<cls1>(\w|::)+) *(, *(?P<cls2>(\w|::)+)? *(, *(?P<cls3>(\w|::)+)? *)?)? *>"
            )
            m = tpl_replace_pattern.sub(
                lambda tpl_match: f"{tpl_match.group('tpl') or ''}{tpl_match.group('cls1') or ''}{tpl_match.group('cls2') or ''}{tpl_match.group('cls3') or ''}Base",
                m,
            )
            m = re.sub(r"(\w+)< *(?:\w|::)+ *>", "", m)
            m = re.sub(r"([:,])\s*,", r"\1", m)
            m = re.sub(r"(\s*[:,])?\s*$", "", m)
            CONTEXT.current_line += m

        if class_pattern_match.group("annot"):
            CONTEXT.current_line += class_pattern_match.group("annot")
            CONTEXT.current_line = fix_annotations(CONTEXT.current_line)

        CONTEXT.current_line += "\n{\n"
        if CONTEXT.comment.strip():
            CONTEXT.current_line += (
                '%Docstring(signature="appended")\n' + CONTEXT.comment + "\n%End\n"
            )

        CONTEXT.current_line += (
            f'\n%TypeHeaderCode\n#include "{os.path.basename(CONTEXT.header_file)}"'
        )

        # for template based inheritance, add a typedef to define the base type,
        # since SIP doesn't allow inheriting from template classes directly
        while template_inheritance_template:
            tpl = template_inheritance_template.pop()
            cls1 = template_inheritance_class1.pop()
            cls2 = template_inheritance_class2.pop()
            cls3 = template_inheritance_class3.pop()

            if cls2 == "":
                # We use /NoTypeName/ to say that this typedef is not present in actual QGIS headers
                CONTEXT.current_line = f"\ntypedef {tpl}<{cls1}> {tpl}{cls1}Base /NoTypeName/;\n\n{CONTEXT.current_line}"
            elif cls3 == "":
                CONTEXT.current_line = f"\ntypedef {tpl}<{cls1},{cls2}> {tpl}{cls1}{cls2}Base /NoTypeName/;\n\n{CONTEXT.current_line}"
            else:
                CONTEXT.current_line = f"\ntypedef {tpl}<{cls1},{cls2},{cls3}> {tpl}{cls1}{cls2}{cls3}Base /NoTypeName/;\n\n{CONTEXT.current_line}"

            if tpl not in CONTEXT.declared_classes:
                tpl_header = f"{tpl.lower()}.h"
                if tpl in sip_config["class_headerfile"]:
                    tpl_header = sip_config["class_headerfile"][tpl]
                CONTEXT.current_line += f'\n#include "{tpl_header}"'

            if cls2 == "":
                CONTEXT.current_line += f"\ntypedef {tpl}<{cls1}> {tpl}{cls1}Base;"
            elif cls3 == "":
                CONTEXT.current_line += (
                    f"\ntypedef {tpl}<{cls1},{cls2}> {tpl}{cls1}{cls2}Base;"
                )
            else:
                CONTEXT.current_line += f"\ntypedef {tpl}<{cls1},{cls2},{cls3}> {tpl}{cls1}{cls2}{cls3}Base;"

        if (
            any(x == Visibility.Private for x in CONTEXT.access)
            and len(CONTEXT.access) != 1
        ):
            dbg_info("skipping class in private context")
            continue

        CONTEXT.access[-1] = Visibility.Private  # private by default
        write_output("CLS", f"{CONTEXT.current_line}\n")

        # Skip opening curly bracket, incrementing hereunder
        skip = read_line()
        if not re.match(r"^\s*{\s*$", skip):
            exit_with_error("expecting { after class definition")
        CONTEXT.bracket_nesting_idx[-1] += 1

        CONTEXT.comment = ""
        CONTEXT.header_code = True
        CONTEXT.access[-1] = Visibility.Private
        continue

    # Bracket balance in class/struct tree
    if not CONTEXT.sip_run:
        bracket_balance = 0
        bracket_balance += CONTEXT.current_line.count("{")
        bracket_balance -= CONTEXT.current_line.count("}")

        if bracket_balance != 0:
            CONTEXT.bracket_nesting_idx[-1] += bracket_balance

            if CONTEXT.bracket_nesting_idx[-1] == 0:
                dbg_info("going up in class/struct tree")

                if len(CONTEXT.access) > 1:
                    CONTEXT.bracket_nesting_idx.pop()
                    CONTEXT.access.pop()

                    if CONTEXT.exported[-1] == 0 and CONTEXT.classname[
                        -1
                    ] != sip_config.get("no_export_macro"):
                        exit_with_error(
                            f"Class {CONTEXT.classname[-1]} should be exported with appropriate [LIB]_EXPORT macro. "
                            f"If this should not be available in python, wrap it in a `#ifndef SIP_RUN` block."
                        )
                    CONTEXT.exported.pop()

                if CONTEXT.classname:
                    CONTEXT.classname.pop()
                    CONTEXT.class_and_struct.pop()

                if len(CONTEXT.access) == 1:
                    dbg_info("reached top level")
                    CONTEXT.access[-1] = (
                        Visibility.Public
                    )  # Top level should stay public

                CONTEXT.comment = ""
                CONTEXT.return_type = ""
                CONTEXT.private_section_line = ""

            dbg_info(f"new bracket balance: {CONTEXT.bracket_nesting_idx}")

    # Private members (exclude SIP_RUN)
    if re.match(r"^\s*private( slots)?:", CONTEXT.current_line):
        CONTEXT.access[-1] = Visibility.Private
        CONTEXT.last_access_section_line = CONTEXT.current_line
        CONTEXT.private_section_line = CONTEXT.current_line
        CONTEXT.comment = ""
        dbg_info("going private")
        continue

    elif re.match(r"^\s*(public( slots)?):.*$", CONTEXT.current_line):
        dbg_info("going public")
        CONTEXT.last_access_section_line = CONTEXT.current_line
        CONTEXT.access[-1] = Visibility.Public
        CONTEXT.comment = ""

    elif re.match(r"^\s*signals:.*$", CONTEXT.current_line):
        dbg_info("going public for signals")
        CONTEXT.last_access_section_line = CONTEXT.current_line
        CONTEXT.access[-1] = Visibility.Signals
        CONTEXT.comment = ""

    elif re.match(r"^\s*(protected)( slots)?:.*$", CONTEXT.current_line):
        dbg_info("going protected")
        CONTEXT.last_access_section_line = CONTEXT.current_line
        CONTEXT.access[-1] = Visibility.Protected
        CONTEXT.comment = ""

    elif (
        CONTEXT.access[-1] == Visibility.Private and "SIP_FORCE" in CONTEXT.current_line
    ):
        dbg_info("private with SIP_FORCE")
        if CONTEXT.private_section_line:
            write_output("PRV3", CONTEXT.private_section_line + "\n")
        CONTEXT.private_section_line = ""

    elif any(x == Visibility.Private for x in CONTEXT.access) and not CONTEXT.sip_run:
        CONTEXT.comment = ""
        continue

    # Skip operators
    if CONTEXT.access[-1] != Visibility.Private and re.search(
        r"operator(=|<<|>>|->)\s*\(", CONTEXT.current_line
    ):
        dbg_info("skip operator")
        detect_and_remove_following_body_or_initializerlist()
        continue

    # Save comments and do not print them, except in SIP_RUN
    if not CONTEXT.sip_run:
        if re.match(r"^\s*//", CONTEXT.current_line):
            match = re.match(r"^\s*//!\s*(.*?)\n?$", CONTEXT.current_line)
            if match:
                CONTEXT.comment_param_list = False
                CONTEXT.prev_indent = CONTEXT.indent
                CONTEXT.indent = ""
                CONTEXT.comment_last_line_note_warning = False
                CONTEXT.comment = process_doxygen_line(match.group(1))
                CONTEXT.comment = CONTEXT.comment.rstrip()
            elif not re.search(r"\*/", CONTEXT.input_lines[CONTEXT.line_idx - 1]):
                CONTEXT.comment = ""
            continue

    # Handle Q_DECLARE_FLAGS in Qt6
    if CONTEXT.is_qt6 and re.match(
        r"^\s*Q_DECLARE_FLAGS\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)", CONTEXT.current_line
    ):
        flags_name = re.search(
            r"\(\s*(\w+)\s*,\s*(\w+)\s*\)", CONTEXT.current_line
        ).group(1)
        flag_name = re.search(
            r"\(\s*(\w+)\s*,\s*(\w+)\s*\)", CONTEXT.current_line
        ).group(2)
        CONTEXT.output_python.append(
            f"{CONTEXT.actual_class}.{flags_name} = lambda flags=0: {CONTEXT.actual_class}.{flag_name}(flags)\n"
        )

    # Enum declaration
    # For scoped and type-based enum, the type has to be removed
    if re.match(
        r"^\s*Q_DECLARE_FLAGS\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)\s*SIP_MONKEYPATCH_FLAGS_UNNEST\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)\s*$",
        CONTEXT.current_line,
    ):
        flags_name = re.search(
            r"\(\s*(\w+)\s*,\s*(\w+)\s*\)", CONTEXT.current_line
        ).group(1)
        flag_name = re.search(
            r"\(\s*(\w+)\s*,\s*(\w+)\s*\)", CONTEXT.current_line
        ).group(2)
        emkb = re.search(
            r"SIP_MONKEYPATCH_FLAGS_UNNEST\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)",
            CONTEXT.current_line,
        ).group(1)
        emkf = re.search(
            r"SIP_MONKEYPATCH_FLAGS_UNNEST\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)",
            CONTEXT.current_line,
        ).group(2)

        if f"{emkb}.{emkf}" != f"{CONTEXT.actual_class}.{flags_name}":
            CONTEXT.output_python.append(
                f"{emkb}.{emkf} = {CONTEXT.actual_class}.{flags_name}\n"
            )

        CONTEXT.enum_monkey_patched_types.append(
            [CONTEXT.actual_class, flags_name, emkb, emkf]
        )

        CONTEXT.current_line = re.sub(
            r"\s*SIP_MONKEYPATCH_FLAGS_UNNEST\(.*?\)", "", CONTEXT.current_line
        )

    enum_match = re.match(
        r"^(\s*enum(\s+Q_DECL_DEPRECATED)?\s+(?P<isclass>class\s+)?(?P<enum_qualname>\w+))(:?\s+SIP_[^:]*)?(\s*:\s*(?P<enum_type>\w+))?(?:\s*SIP_ENUM_BASETYPE\s*\(\s*(?P<py_enum_type>\w+)\s*\))?(?P<oneliner>.*)$",
        CONTEXT.current_line,
    )
    if enum_match:
        enum_decl = enum_match.group(1)
        enum_qualname = enum_match.group("enum_qualname")
        enum_type = enum_match.group("enum_type")
        isclass = enum_match.group("isclass")
        enum_cpp_name = (
            f"{CONTEXT.actual_class}::{enum_qualname}"
            if CONTEXT.actual_class
            else enum_qualname
        )

        if not isclass and enum_cpp_name not in ALLOWED_NON_CLASS_ENUMS:
            exit_with_error(
                f"Non class enum exposed to Python -- must be a enum class: {enum_cpp_name}"
            )

        oneliner = enum_match.group("oneliner")
        is_scope_based = bool(isclass)
        enum_decl = re.sub(r"\s*\bQ_DECL_DEPRECATED\b", "", enum_decl)

        py_enum_type_match = re.search(
            r"SIP_ENUM_BASETYPE\(\s*(.*?)\s*\)", CONTEXT.current_line
        )
        py_enum_type = py_enum_type_match.group(1) if py_enum_type_match else None

        if py_enum_type == "IntFlag":
            CONTEXT.enum_intflag_types.append(enum_cpp_name)

        if enum_type in ["int", "quint32"]:
            CONTEXT.enum_int_types.append(f"{CONTEXT.actual_class}.{enum_qualname}")
            if CONTEXT.is_qt6:
                enum_decl += f" /BaseType={py_enum_type or 'IntEnum'}/"
        elif enum_type:
            exit_with_error(f"Unhandled enum type {enum_type} for {enum_cpp_name}")
        elif isclass:
            CONTEXT.enum_class_non_int_types.append(
                f"{CONTEXT.actual_class}.{enum_qualname}"
            )
        elif CONTEXT.is_qt6:
            enum_decl += " /BaseType=IntEnum/"

        write_output("ENU1", enum_decl)
        if oneliner:
            write_output("ENU1", oneliner)
        write_output("ENU1", "\n")

        _match = None
        if is_scope_based:
            _match = re.search(
                r"SIP_MONKEYPATCH_SCOPEENUM(_UNNEST)?(:?\(\s*(?P<emkb>\w+)\s*,\s*(?P<emkf>\w+)\s*\))?",
                CONTEXT.current_line,
            )
        monkeypatch = is_scope_based and _match
        enum_mk_base = _match.group("emkb") if _match else ""

        enum_old_name = ""
        if _match and _match.group("emkf") and monkeypatch:
            enum_old_name = _match.group("emkf")
            if CONTEXT.actual_class:
                if (
                    f"{enum_mk_base}.{enum_old_name}"
                    != f"{CONTEXT.actual_class}.{enum_qualname}"
                ):
                    CONTEXT.output_python.append(
                        f"{enum_mk_base}.{enum_old_name} = {CONTEXT.actual_class}.{enum_qualname}\n"
                    )
            else:
                CONTEXT.output_python.append(
                    f"{enum_mk_base}.{enum_old_name} = {enum_qualname}\n"
                )

        if re.search(
            r"\{((\s*\w+)(\s*=\s*[\w\s<|]+.*?)?(,?))+\s*}", CONTEXT.current_line
        ):
            if "=" in CONTEXT.current_line:
                exit_with_error(
                    "Sipify does not handle enum one liners with value assignment. Use multiple lines instead. Or just write a new parser."
                )
            continue
        else:
            CONTEXT.current_line = read_line()
            if not re.match(r"^\s*\{\s*$", CONTEXT.current_line):
                exit_with_error("Unexpected content: enum should be followed by {")
            write_output("ENU2", f"{CONTEXT.current_line}\n")

            if is_scope_based:
                CONTEXT.output_python.append("# monkey patching scoped based enum\n")

            enum_members_doc = []

            while CONTEXT.line_idx < CONTEXT.line_count:
                CONTEXT.current_line = read_line()
                if detect_comment_block():
                    continue
                if re.search(r"};", CONTEXT.current_line):
                    break
                if re.match(
                    r"^\s*\w+\s*\|", CONTEXT.current_line
                ):  # multi line declaration as sum of enums
                    continue

                enum_match = re.match(
                    r"^(\s*(?P<em>\w+))(\s+SIP_PYNAME(?:\(\s*(?P<pyname>[^() ]+)\s*\)\s*)?)?(\s+SIP_MONKEY\w+(?:\(\s*(?P<compat>[^() ]+)\s*\)\s*)?)?(?:\s*=\s*(?P<enum_value>(:?[\w\s|+-]|::|<<)+))?(?P<optional_comma>,?)(:?\s*//!<\s*(?P<co>.*)|.*)$",
                    CONTEXT.current_line,
                )

                enum_decl = (
                    f"{enum_match.group(1) or ''}{enum_match.group(3) or ''}{enum_match.group('optional_comma') or ''}"
                    if enum_match
                    else CONTEXT.current_line
                )
                enum_member = enum_match.group("em") or "" if enum_match else ""
                value_comment = enum_match.group("co") or "" if enum_match else ""
                compat_name = (
                    enum_match.group("compat") or enum_member if enum_match else ""
                )
                enum_value = enum_match.group("enum_value") or "" if enum_match else ""

                value_comment = value_comment.replace("::", ".").replace('"', '\\"')
                value_comment = re.sub(
                    r"\\since .*?([\d.]+)",
                    r"\\n.. versionadded:: \1\\n",
                    value_comment,
                    flags=re.I,
                )
                value_comment = re.sub(
                    r"\\deprecated (?:QGIS )?(.*)",
                    r"\\n.. deprecated:: \1\\n",
                    value_comment,
                    flags=re.I,
                )
                value_comment = re.sub(r"^\\n+", "", value_comment)
                value_comment = re.sub(r"\\n+$", "", value_comment)

                dbg_info(
                    f"is_scope_based:{is_scope_based} enum_mk_base:{enum_mk_base} monkeypatch:{monkeypatch}"
                )

                if enum_value and (
                    re.search(r".*<<.*", enum_value)
                    or re.search(r".*0x0.*", enum_value)
                ):
                    if (
                        f"{CONTEXT.actual_class}::{enum_qualname}"
                        not in CONTEXT.enum_intflag_types
                    ):
                        exit_with_error(
                            f"{CONTEXT.actual_class}::{enum_qualname} is a flags type, but was not declared with IntFlag type. Add 'SIP_ENUM_BASETYPE(IntFlag)' to the enum class declaration line"
                        )

                if is_scope_based and enum_member:
                    value_comment_parts = value_comment.replace("\\n", "\n").split("\n")
                    value_comment_indented = ""
                    for part_idx, part in enumerate(value_comment_parts):
                        if part_idx == 0:
                            if part.strip().startswith(".."):
                                exit_with_error(
                                    f"Enum member description missing for {CONTEXT.actual_class}::{enum_qualname}"
                                )
                            value_comment_indented += part.rstrip()
                        else:
                            if part.startswith(".."):
                                value_comment_indented += "\n"

                            value_comment_indented += "  " + part.rstrip()
                            if part.startswith(".."):
                                value_comment_indented += "\n"

                        if part_idx < len(value_comment_parts) - 1:
                            value_comment_indented += "\n"

                    complete_class_path = ".".join(CONTEXT.classname)
                    if monkeypatch and enum_mk_base:
                        if compat_name != enum_member:
                            value_comment_indented += f"\n\n  Available as ``{enum_mk_base}.{compat_name}`` in older QGIS releases.\n"
                        if CONTEXT.actual_class:
                            CONTEXT.output_python.append(
                                f"{enum_mk_base}.{compat_name} = {complete_class_path}.{enum_qualname}.{enum_member}\n"
                            )
                            if enum_old_name and compat_name != enum_member:
                                CONTEXT.output_python.append(
                                    f"{enum_mk_base}.{enum_old_name}.{compat_name} = {complete_class_path}.{enum_qualname}.{enum_member}\n"
                                )
                            CONTEXT.output_python.append(
                                f"{enum_mk_base}.{compat_name}.is_monkey_patched = True\n"
                            )
                            CONTEXT.output_python.append(
                                f'{enum_mk_base}.{compat_name}.__doc__ = "{value_comment}"\n'
                            )
                            enum_members_doc.append(
                                f"* ``{enum_member}``: {value_comment_indented}"
                            )
                        else:
                            CONTEXT.output_python.append(
                                f"{enum_mk_base}.{compat_name} = {enum_qualname}.{enum_member}\n"
                            )
                            CONTEXT.output_python.append(
                                f"{enum_mk_base}.{compat_name}.is_monkey_patched = True\n"
                            )
                            CONTEXT.output_python.append(
                                f'{enum_mk_base}.{compat_name}.__doc__ = "{value_comment}"\n'
                            )
                            enum_members_doc.append(
                                f"* ``{enum_member}``: {value_comment_indented}"
                            )
                    else:
                        if compat_name != enum_member:
                            value_comment_indented += f"\n\n  Available as ``{CONTEXT.actual_class}.{compat_name}`` in older QGIS releases.\n"

                        if monkeypatch:
                            CONTEXT.output_python.append(
                                f"{complete_class_path}.{compat_name} = {complete_class_path}.{enum_qualname}.{enum_member}\n"
                            )
                            CONTEXT.output_python.append(
                                f"{complete_class_path}.{compat_name}.is_monkey_patched = True\n"
                            )
                        if CONTEXT.actual_class:
                            CONTEXT.output_python.append(
                                f'{complete_class_path}.{enum_qualname}.{compat_name}.__doc__ = "{value_comment}"\n'
                            )
                            enum_members_doc.append(
                                f"* ``{enum_member}``: {value_comment_indented}"
                            )
                        else:
                            CONTEXT.output_python.append(
                                f'{enum_qualname}.{compat_name}.__doc__ = "{value_comment}"\n'
                            )
                            enum_members_doc.append(
                                f"* ``{enum_member}``: {value_comment_indented}"
                            )

                if not is_scope_based and CONTEXT.is_qt6 and enum_member:
                    basename = ".".join(CONTEXT.class_and_struct)
                    if basename:
                        enum_member = "None_" if enum_member == "None" else enum_member
                        CONTEXT.output_python.append(
                            f"{basename}.{enum_member} = {basename}.{enum_qualname}.{enum_member}\n"
                        )

                enum_decl = fix_annotations(enum_decl)
                write_output("ENU3", f"{enum_decl}\n")

                detect_comment_block(strict_mode=False)

            write_output("ENU4", f"{CONTEXT.current_line}\n")

            if is_scope_based:
                enum_member_doc_string = "\n".join(enum_members_doc)
                if CONTEXT.actual_class:
                    CONTEXT.output_python.append(
                        f'{".".join(CONTEXT.classname)}.{enum_qualname}.__doc__ = """{CONTEXT.comment}\n\n{enum_member_doc_string}\n\n"""\n# --\n'
                    )
                else:
                    CONTEXT.output_python.append(
                        f'{enum_qualname}.__doc__ = """{CONTEXT.comment}\n\n{enum_member_doc_string}\n\n"""\n# --\n'
                    )

            # enums don't have Docstring apparently
            CONTEXT.comment = ""
            continue

    # Check for invalid use of doxygen command
    if re.search(r".*//!<", CONTEXT.current_line):
        exit_with_error(
            '"\\!<" doxygen command must only be used for enum documentation'
        )

    # Handle override, final, and make private keywords
    if re.search(r"\boverride\b", CONTEXT.current_line):
        CONTEXT.is_override_or_make_private = PrependType.Virtual
    if re.search(r"\bFINAL\b", CONTEXT.current_line):
        CONTEXT.is_override_or_make_private = PrependType.Virtual
    if re.search(r"\bSIP_MAKE_PRIVATE\b", CONTEXT.current_line):
        CONTEXT.is_override_or_make_private = PrependType.MakePrivate

    # Remove Q_INVOKABLE
    CONTEXT.current_line = re.sub(r"^(\s*)Q_INVOKABLE ", r"\1", CONTEXT.current_line)

    # Keyword fixes
    CONTEXT.current_line = re.sub(
        r"^(\s*template\s*<)(?:class|typename) (\w+>)(.*)$",
        r"\1\2\3",
        CONTEXT.current_line,
    )
    CONTEXT.current_line = re.sub(
        r"^(\s*template\s*<)(?:class|typename) (\w+) *, *(?:class|typename) (\w+>)(.*)$",
        r"\1\2,\3\4",
        CONTEXT.current_line,
    )
    CONTEXT.current_line = re.sub(
        r"^(\s*template\s*<)(?:class|typename) (\w+) *, *(?:class|typename) (\w+) *, *(?:class|typename) (\w+>)(.*)$",
        r"\1\2,\3,\4\5",
        CONTEXT.current_line,
    )
    CONTEXT.current_line = re.sub(r"\s*\boverride\b", "", CONTEXT.current_line)
    CONTEXT.current_line = re.sub(r"\s*\bSIP_MAKE_PRIVATE\b", "", CONTEXT.current_line)
    CONTEXT.current_line = re.sub(
        r"\s*\bFINAL\b", " ${SIP_FINAL}", CONTEXT.current_line
    )
    CONTEXT.current_line = re.sub(r"\s*\bextern \b", "", CONTEXT.current_line)
    CONTEXT.current_line = re.sub(r"\s*\bMAYBE_UNUSED \b", "", CONTEXT.current_line)
    CONTEXT.current_line = re.sub(r"\s*\bNODISCARD \b", "", CONTEXT.current_line)
    CONTEXT.current_line = re.sub(r"\s*\bQ_DECL_DEPRECATED\b", "", CONTEXT.current_line)
    CONTEXT.current_line = re.sub(
        r"^(\s*)?(const |virtual |static )*inline ", r"\1\2", CONTEXT.current_line
    )
    CONTEXT.current_line = re.sub(r"\bconstexpr\b", "const", CONTEXT.current_line)
    CONTEXT.current_line = re.sub(r"\bnullptr\b", "0", CONTEXT.current_line)
    CONTEXT.current_line = re.sub(r"\s*=\s*default\b", "", CONTEXT.current_line)

    # Handle export macros
    if re.search(r"\b\w+_EXPORT\b", CONTEXT.current_line):
        CONTEXT.exported[-1] += 1
        CONTEXT.current_line = re.sub(r"\b\w+_EXPORT\s+", "", CONTEXT.current_line)

    # Skip non-method member declaration in non-public sections
    if (
        not CONTEXT.sip_run
        and CONTEXT.access[-1] != Visibility.Public
        and detect_non_method_member(CONTEXT.current_line)
    ):
        dbg_info("skip non-method member declaration in non-public sections")
        continue

    # Remove static const value assignment
    # https://regex101.com/r/DyWkgn/6
    if re.search(r"^\s*const static \w+", CONTEXT.current_line):
        exit_with_error(
            f"const static should be written static const in {CONTEXT.classname[-1]}"
        )

    # TODO needs fixing!!
    # original perl regex was:
    #       ^(?<staticconst> *(?<static>static )?const (\w+::)*\w+(?:<(?:[\w<>, ]|::)+>)? \w+)(?: = [^()]+?(\((?:[^()]++|(?3))*\))?[^()]*?)?(?<endingchar>[|;]) *(\/\/.*?)?$
    match = re.search(
        r"^(?P<staticconst> *(?P<static>static )?const (\w+::)*\w+(?:<(?:[\w<>, ]|::)+>)? \w+)(?: = [^()]+?(\((?:[^()]|\([^()]*\))*\))?[^()]*?)?(?P<endingchar>[|;]) *(//.*)?$",
        CONTEXT.current_line,
    )
    if match:
        CONTEXT.current_line = f"{match.group('staticconst')};"
        if match.group("static") is None:
            CONTEXT.comment = ""

        if match.group("endingchar") == "|":
            dbg_info("multiline const static assignment")
            skip = ""
            while not re.search(r";\s*(//.*?)?$", skip):
                skip = read_line()

    # Remove struct member assignment
    # https://regex101.com/r/OUwV75/1
    if not CONTEXT.sip_run and CONTEXT.access[-1] == Visibility.Public:
        # original perl regex: ^(\s*\w+[\w<> *&:,]* \*?\w+) = ([\-\w\:\.]+(< *\w+( \*)? *>)?)+(\([^()]*\))?\s*;
        # dbg_info(f"attempt struct member assignment '{CONTEXT.current_line}'")

        python_regex_verbose = r"""
        ^                           # Start of the line
        (                           # Start of capturing group for the left-hand side
            \s*                     # Optional leading whitespace
            (?:const\s+)?           # Optional const qualifier
            (?:                     # Start of non-capturing group for type
                (?:unsigned\s+)?    # Optional unsigned qualifier
                (?:long\s+long|long|int|short|char|float|double|bool|auto|void|size_t|time_t) # Basic types
                |                   # OR
                [\w:]+(?:<[^>]+>)?  # Custom types (with optional template)
            )
            (?:\s+const)?           # Optional const qualifier after type
            \s+                     # Whitespace after type
            \**\s*                  # Optional additional pointer asterisks
            \w+                     # Variable name
        )                           # End of capturing group for the left-hand side
        \s*=\s*                     # Equals sign with optional surrounding whitespace
        (                           # Start of capturing group for the right-hand side
            -?                      # Optional negative sign
            (?:                     # Start of non-capturing group for value
                \d+(?:\.\d*)?       # Integer or floating-point number
                |                   # OR
                nullptr             # nullptr keyword
                |                   # OR
                (?:std::)?          # Optional std:: prefix
                \w+                 # Word characters for function/class names
                (?:<[^>]+>)?        # Optional template arguments
                (?:::[\w<>]+)*      # Optional nested name specifiers
                (?:                 # Start of optional group for function calls
                    \(              # Opening parenthesis
                    [^()]*          # Any characters except parentheses
                    (?:\([^()]*\))* # Allows for one level of nested parentheses
                    [^()]*          # Any characters except parentheses
                    \)              # Closing parenthesis
                )?                  # End of optional group for function calls
            )
        )                           # End of capturing group for the right-hand side
        \s*;                        # Optional whitespace and semicolon
        \s*                         # Optional whitespace after semicolon
        (?:\/\/.*)?                 # Optional single-line comment
        $                           # End of the line
        """
        regex_verbose = re.compile(python_regex_verbose, re.VERBOSE | re.MULTILINE)
        match = regex_verbose.match(CONTEXT.current_line)
        if match:
            dbg_info(f"remove struct member assignment '={match.group(2)}'")
            CONTEXT.current_line = f"{match.group(1)};"

    # Catch Q_DECLARE_FLAGS
    match = re.search(
        r"^(\s*)Q_DECLARE_FLAGS\(\s*(.*?)\s*,\s*(.*?)\s*\)\s*$", CONTEXT.current_line
    )
    if match:
        CONTEXT.actual_class = (
            f"{CONTEXT.classname[-1]}::" if len(CONTEXT.classname) >= 0 else ""
        )
        dbg_info(f"Declare flags: {CONTEXT.actual_class}")
        CONTEXT.current_line = f"{match.group(1)}typedef QFlags<{CONTEXT.actual_class}{match.group(3)}> {match.group(2)};\n"
        CONTEXT.qflag_hash[f"{CONTEXT.actual_class}{match.group(2)}"] = (
            f"{CONTEXT.actual_class}{match.group(3)}"
        )

        if f"{CONTEXT.actual_class}{match.group(3)}" not in CONTEXT.enum_intflag_types:
            exit_with_error(
                f"{CONTEXT.actual_class}{match.group(3)} is a flags type, but was not declared with IntFlag type. Add 'SIP_ENUM_BASETYPE(IntFlag)' to the enum class declaration line"
            )

    # Catch Q_DECLARE_OPERATORS_FOR_FLAGS
    match = re.search(
        r"^(\s*)Q_DECLARE_OPERATORS_FOR_FLAGS\(\s*(.*?)\s*\)\s*$", CONTEXT.current_line
    )
    if match:
        flags = match.group(2)
        flag = CONTEXT.qflag_hash.get(flags)
        if flag is None:
            exit_with_error(f"error reading flags: {flags}")
        CONTEXT.current_line = (
            f"{match.group(1)}QFlags<{flag}> operator|({flag} f1, QFlags<{flag}> f2);\n"
        )

        py_flag = flag.replace("::", ".")

        if py_flag in CONTEXT.enum_class_non_int_types:
            exit_with_error(
                f"{flag} is a flags type, but was not declared with int type. Add ': int' to the enum class declaration line"
            )
        elif py_flag not in CONTEXT.enum_int_types:
            if CONTEXT.is_qt6:
                dbg_info("monkey patching operators for non-class enum")
                if not CONTEXT.has_pushed_force_int:
                    CONTEXT.output_python.append(
                        "from enum import Enum\n\n\ndef _force_int(v): return int(v.value) if isinstance(v, Enum) else v\n\n\n"
                    )
                    CONTEXT.has_pushed_force_int = True
                CONTEXT.output_python.append(
                    f"{py_flag}.__bool__ = lambda flag: bool(_force_int(flag))\n"
                )
                CONTEXT.output_python.append(
                    f"{py_flag}.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)\n"
                )
                CONTEXT.output_python.append(
                    f"{py_flag}.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)\n"
                )
                CONTEXT.output_python.append(
                    f"{py_flag}.__or__ = lambda flag1, flag2: {py_flag}(_force_int(flag1) | _force_int(flag2))\n"
                )

        if not CONTEXT.is_qt6:
            for patched_type in CONTEXT.enum_monkey_patched_types:
                if flags == f"{patched_type[0]}::{patched_type[1]}":
                    dbg_info("monkey patching flags")
                    if not CONTEXT.has_pushed_force_int:
                        CONTEXT.output_python.append(
                            "from enum import Enum\n\n\ndef _force_int(v): return int(v.value) if isinstance(v, Enum) else v\n\n\n"
                        )
                        CONTEXT.has_pushed_force_int = True
                    CONTEXT.output_python.append(
                        f"{py_flag}.__or__ = lambda flag1, flag2: {patched_type[0]}.{patched_type[1]}(_force_int(flag1) | _force_int(flag2))\n"
                    )

    # Remove keywords
    if CONTEXT.is_override_or_make_private != PrependType.NoPrepend:
        # Handle multiline definition to add virtual keyword or make private on opening line
        if CONTEXT.multiline_definition != MultiLineType.NotMultiline:
            rolling_line = CONTEXT.current_line
            rolling_line_idx = CONTEXT.line_idx
            dbg_info(
                "handle multiline definition to add virtual keyword or making private on opening line"
            )
            while not re.match(
                r"^[^()]*\(([^()]*\([^()]*\)[^()]*)*[^()]*$", rolling_line
            ):
                rolling_line_idx -= 1
                rolling_line = CONTEXT.input_lines[rolling_line_idx]
                if rolling_line_idx < 0:
                    exit_with_error("could not reach opening definition")
            dbg_info(f"rolled back to {rolling_line_idx}: {rolling_line}")

            if (
                CONTEXT.is_override_or_make_private == PrependType.Virtual
                and not re.match(r"^(\s*)virtual\b(.*)$", rolling_line)
            ):
                idx = rolling_line_idx - CONTEXT.line_idx + 1
                CONTEXT.output[idx] = fix_annotations(
                    re.sub(r"^(\s*?)\b(.*)$", r"\1 virtual \2\n", rolling_line)
                )
            elif CONTEXT.is_override_or_make_private == PrependType.MakePrivate:
                dbg_info("prepending private access")
                idx = rolling_line_idx - CONTEXT.line_idx
                private_access = re.sub(
                    r"(protected|public)", "private", CONTEXT.last_access_section_line
                )
                CONTEXT.output.insert(idx + 1, private_access + "\n")
                CONTEXT.output[idx + 1] = fix_annotations(rolling_line) + "\n"
        elif CONTEXT.is_override_or_make_private == PrependType.MakePrivate:
            dbg_info("prepending private access")
            CONTEXT.current_line = (
                re.sub(
                    r"(protected|public)", "private", CONTEXT.last_access_section_line
                )
                + "\n"
                + CONTEXT.current_line
                + "\n"
            )
        elif (
            CONTEXT.is_override_or_make_private == PrependType.Virtual
            and not re.match(r"^(\s*)virtual\b(.*)$", CONTEXT.current_line)
        ):
            # SIP often requires the virtual keyword to be present, or it chokes on covariant return types
            # in overridden methods
            dbg_info("adding virtual keyword for overridden method")
            CONTEXT.current_line = re.sub(
                r"^(\s*?)\b(.*)$", r"\1virtual \2\n", CONTEXT.current_line
            )

    # remove constructor definition, function bodies, member initializing list
    CONTEXT.python_signature = detect_and_remove_following_body_or_initializerlist()

    # remove inline declarations
    match = re.search(
        r"^(\s*)?(static |const )*(([(?:long )\w]+(<.*?>)?\s+([*&])?)?(\w+)( const*?)*)\s*(\{.*});(\s*//.*)?$",
        CONTEXT.current_line,
    )
    if match:
        CONTEXT.current_line = f"{match.group(1)}{match.group(3)};"

    pattern = r"^\s*((?:const |virtual |static |inline ))*(?!explicit)([(?:long )\w:]+(?:<.*?>)?)\s+(?:\*|&)?(\w+|operator.{1,2})(\(.*)$"
    match = re.match(pattern, CONTEXT.current_line)
    if match:
        CONTEXT.current_method_name = match.group(3)
        return_type_candidate = match.group(2)
        is_static = bool(match.group(1) and "static" in match.group(1))
        class_name = CONTEXT.current_fully_qualified_class_name()
        if CONTEXT.current_method_name in CONTEXT.static_methods[class_name]:
            if (
                CONTEXT.static_methods[class_name][CONTEXT.current_method_name]
                != is_static
            ):
                CONTEXT.static_methods[class_name][CONTEXT.current_method_name] = False
        else:
            CONTEXT.static_methods[class_name][CONTEXT.current_method_name] = is_static

        if CONTEXT.access[-1] == Visibility.Signals:
            CONTEXT.current_signal_args = []
            signal_args = match.group(4).strip()
            if signal_args.startswith("("):
                signal_args = signal_args[1:]
            if signal_args.endswith(");"):
                signal_args = signal_args[:-2]

            if signal_args.strip():
                CONTEXT.current_signal_args = split_args(signal_args)
            dbg_info(
                "SIGARG "
                + CONTEXT.current_method_name
                + " "
                + str(CONTEXT.current_signal_args)
            )
            if ");" in match.group(4):
                CONTEXT.signal_arguments[class_name][CONTEXT.current_method_name] = (
                    CONTEXT.current_signal_args[:]
                )
                dbg_info(
                    "SIGARG finalizing"
                    + CONTEXT.current_method_name
                    + " "
                    + str(CONTEXT.current_signal_args)
                )

        if not re.search(
            r"(void|SIP_PYOBJECT|operator|return|QFlag)", return_type_candidate
        ):
            # replace :: with . (changes c++ style namespace/class directives to Python style)
            CONTEXT.return_type = return_type_candidate.replace("::", ".")
            # replace with builtin Python types
            CONTEXT.return_type = re.sub(r"\bdouble\b", "float", CONTEXT.return_type)
            CONTEXT.return_type = re.sub(r"\bQString\b", "str", CONTEXT.return_type)
            CONTEXT.return_type = re.sub(
                r"\bQStringList\b", "list of str", CONTEXT.return_type
            )

            list_match = re.match(
                r"^(?:QList|QVector)<\s*(.*?)[\s*]*>$", CONTEXT.return_type
            )
            if list_match:
                CONTEXT.return_type = f"list of {list_match.group(1)}"

            set_match = re.match(r"^QSet<\s*(.*?)[\s*]*>$", CONTEXT.return_type)
            if set_match:
                CONTEXT.return_type = f"set of {set_match.group(1)}"
    elif CONTEXT.access[
        -1
    ] == Visibility.Signals and CONTEXT.current_line.strip() not in ("", "signals:"):
        dbg_info("SIGARG4 " + CONTEXT.current_method_name + " " + CONTEXT.current_line)
        signal_args = CONTEXT.current_line.strip()
        if signal_args.endswith(");"):
            signal_args = signal_args[:-2]

        if signal_args.strip():
            CONTEXT.current_signal_args.extend(split_args(signal_args))
        dbg_info(
            "SIGARG5 "
            + CONTEXT.current_method_name
            + " "
            + str(CONTEXT.current_signal_args)
        )
        if ");" in CONTEXT.current_line:
            class_name = CONTEXT.current_fully_qualified_class_name()
            CONTEXT.signal_arguments[class_name][CONTEXT.current_method_name] = (
                CONTEXT.current_signal_args[:]
            )
            dbg_info(
                "SIGARG finalizing"
                + CONTEXT.current_method_name
                + " "
                + str(CONTEXT.current_signal_args)
            )

    # deleted functions
    if re.match(
        r"^(\s*)?(const )?(virtual |static )?((\w+(<.*?>)?\s+([*&])?)?(\w+|operator.{1,2})\(.*?(\(.*\))*.*\)( const)?)\s*= delete;(\s*//.*)?$",
        CONTEXT.current_line,
    ):
        CONTEXT.comment = ""
        continue

    # remove export macro from struct definition
    CONTEXT.current_line = re.sub(
        r"^(\s*struct )\w+_EXPORT (.+)$", r"\1\2", CONTEXT.current_line
    )

    # Skip comments
    if re.match(
        r"^\s*typedef\s+\w+\s*<\s*\w+\s*>\s+\w+\s+.*SIP_DOC_TEMPLATE",
        CONTEXT.current_line,
    ):
        # support Docstring for template based classes in SIP 4.19.7+
        CONTEXT.comment_template_docstring = True
    elif CONTEXT.multiline_definition == MultiLineType.NotMultiline and (
        re.search(r"//", CONTEXT.current_line)
        or re.match(r"^\s*typedef ", CONTEXT.current_line)
        or re.search(r"\s*struct ", CONTEXT.current_line)
        or re.search(r"operator\[]\(", CONTEXT.current_line)
        or re.match(r"^\s*operator\b", CONTEXT.current_line)
        or re.search(r"operator\s?[!+-=*/\[\]<>]{1,2}", CONTEXT.current_line)
        or re.match(r"^\s*%\w+(.*)?$", CONTEXT.current_line)
        or re.match(r"^\s*namespace\s+\w+", CONTEXT.current_line)
        or re.match(r"^\s*(virtual\s*)?~", CONTEXT.current_line)
        or detect_non_method_member(CONTEXT.current_line)
    ):
        dbg_info(f"skipping comment for {CONTEXT.current_line}")
        if re.search(r"\s*typedef.*?(?!SIP_DOC_TEMPLATE)", CONTEXT.current_line):
            dbg_info("because typedef")
        elif (
            CONTEXT.actual_class
            and detect_non_method_member(CONTEXT.current_line)
            and CONTEXT.comment
        ):
            attribute_name_match = re.match(
                r"^.*?\s[*&]*(\w+);.*$", CONTEXT.current_line
            )
            class_name = CONTEXT.current_fully_qualified_struct_name()
            dbg_info(
                f"storing attribute docstring for {class_name} : {attribute_name_match.group(1)}"
            )
            CONTEXT.attribute_docstrings[class_name][
                attribute_name_match.group(1)
            ] = CONTEXT.comment
        elif (
            CONTEXT.current_fully_qualified_struct_name()
            and re.search(r"\s*struct ", CONTEXT.current_line)
            and CONTEXT.comment
        ):
            class_name = CONTEXT.current_fully_qualified_struct_name()
            dbg_info(f"storing struct docstring for {class_name}")
            CONTEXT.struct_docstrings[class_name] = CONTEXT.comment

        CONTEXT.comment = ""
        CONTEXT.return_type = ""
        CONTEXT.is_override_or_make_private = PrependType.NoPrepend

    CONTEXT.current_line = fix_constants(CONTEXT.current_line)
    CONTEXT.current_line = fix_annotations(CONTEXT.current_line)

    # fix astyle placing space after % character
    CONTEXT.current_line = re.sub(
        r"/\s+GetWrapper\s+/", "/GetWrapper/", CONTEXT.current_line
    )

    # MISSING
    # handle enum/flags QgsSettingsEntryEnumFlag
    match = re.match(
        r"^(\s*)const QgsSettingsEntryEnumFlag<(.*)> (.+);$", CONTEXT.current_line
    )
    if match:
        CONTEXT.indent, enum_type, var_name = match.groups()

        prep_line = f"""class QgsSettingsEntryEnumFlag_{var_name}
    {{
    %TypeHeaderCode
    #include "{os.path.basename(CONTEXT.header_file)}"
    #include "qgssettingsentry.h"
    typedef QgsSettingsEntryEnumFlag<{enum_type}> QgsSettingsEntryEnumFlag_{var_name};
    %End
      public:
        QgsSettingsEntryEnumFlag_{var_name}( const QString &key, QgsSettings::Section section, const {enum_type} &defaultValue, const QString &description = QString() );
        QString key( const QString &dynamicKeyPart = QString() ) const;
        {enum_type} value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, const {enum_type} &defaultValueOverride = {enum_type}() ) const;
    }};"""

        CONTEXT.current_line = (
            f"{CONTEXT.indent}const QgsSettingsEntryEnumFlag_{var_name} {var_name};"
        )
        CONTEXT.comment = ""
        write_output("ENF", f"{prep_line}\n", "prepend")

    write_output("NOR", f"{CONTEXT.current_line}\n")

    # append to class map file
    if args.class_map and CONTEXT.actual_class:
        match = re.match(
            r"^ *(const |virtual |static )* *[\w:]+ +\*?(?P<method>\w+)\(.*$",
            CONTEXT.current_line,
        )
        if match:
            with open(args.class_map, "a") as f:
                f.write(
                    f"{'.'.join(CONTEXT.classname)}.{match.group('method')}: {CONTEXT.header_file}#L{CONTEXT.line_idx}\n"
                )

    if CONTEXT.python_signature:
        write_output("PSI", f"{CONTEXT.python_signature}\n")

    # multiline definition (parenthesis left open)
    if CONTEXT.multiline_definition != MultiLineType.NotMultiline:
        dbg_info("on multiline")
        # https://regex101.com/r/DN01iM/4
        # TODO - original regex is incompatible with python -- it was:
        # ^([^()]+(\((?:[^()]++|(?1))*\)))*[^()]*\)([^()](throw\([^()]+\))?)*$:
        if re.match(
            r"^([^()]+(\((?:[^()]|\([^()]*\))*\)))*[^()]*\)([^()](throw\([^()]+\))?)*",
            CONTEXT.current_line,
        ):
            dbg_info("ending multiline")
            # remove potential following body
            if (
                CONTEXT.multiline_definition != MultiLineType.ConditionalStatement
                and not re.search(r"(\{.*}|;)\s*(//.*)?$", CONTEXT.current_line)
            ):
                dbg_info("remove following body of multiline def")
                last_line = CONTEXT.current_line
                last_line += remove_following_body_or_initializerlist()
                # add missing semi column
                CONTEXT.output.pop()
                write_output("MLT", f"{last_line};\n")
            CONTEXT.multiline_definition = MultiLineType.NotMultiline
        else:
            continue
    elif re.match(r"^[^()]+\([^()]*(?:\([^()]*\)[^()]*)*[^)]*$", CONTEXT.current_line):
        dbg_info(f"Multiline detected:: {CONTEXT.current_line}")
        if re.match(r"^\s*((else )?if|while|for) *\(", CONTEXT.current_line):
            CONTEXT.multiline_definition = MultiLineType.ConditionalStatement
        else:
            CONTEXT.multiline_definition = MultiLineType.Method
        continue

    # write comment
    if re.match(r"^\s*$", CONTEXT.current_line):
        dbg_info("no more override / private")
        CONTEXT.is_override_or_make_private = PrependType.NoPrepend
        continue

    if re.match(r"^\s*template\s*<.*>", CONTEXT.current_line):
        # do not comment now for templates, wait for class definition
        continue

    if CONTEXT.comment.strip() or CONTEXT.return_type:
        if (
            CONTEXT.is_override_or_make_private != PrependType.Virtual
            and not CONTEXT.comment.strip()
        ):
            # overridden method with no new docs - so don't create a Docstring and use
            # parent class Docstring
            pass
        else:
            dbg_info("writing comment")
            if CONTEXT.comment.strip():
                dbg_info("comment non-empty")
                doc_prepend = (
                    "@DOCSTRINGSTEMPLATE@" if CONTEXT.comment_template_docstring else ""
                )
                write_output("CM1", f"{doc_prepend}%Docstring\n")

                doc_string = ""
                comment_lines = CONTEXT.comment.split("\n")
                skipping_param = 0
                out_params = []
                waiting_for_return_to_end = False

                comment_line_idx = 0
                while comment_line_idx < len(comment_lines):
                    comment_line = comment_lines[comment_line_idx]
                    comment_line_idx += 1
                    if (
                        "versionadded:" in comment_line or "deprecated:" in comment_line
                    ) and out_params:
                        dbg_info("out style parameters remain to flush!")
                        # member has /Out/ parameters, but no return type, so flush out out_params docs now
                        first_out_param = out_params.pop(0)
                        doc_string += f"{doc_prepend}:return: - {first_out_param}\n"

                        for out_param in out_params:
                            doc_string += f"{doc_prepend}         - {out_param}\n"

                        doc_string += f"{doc_prepend}\n"
                        out_params = []

                    param_match = re.match(r"^:param\s+(\w+)", comment_line)
                    if param_match:
                        param_name = param_match.group(1)
                        dbg_info(f"found parameter: {param_name}")
                        if (
                            param_name in CONTEXT.skipped_params_out
                            or param_name in CONTEXT.skipped_params_remove
                        ):
                            dbg_info(str(CONTEXT.skipped_params_out))
                            if param_name in CONTEXT.skipped_params_out:
                                dbg_info(
                                    f"deferring docs for parameter {param_name} marked as SIP_OUT"
                                )
                                comment_line = re.sub(
                                    r"^:param\s+(\w+):\s*(.*?)$",
                                    r"\1: \2",
                                    comment_line,
                                )
                                comment_line = re.sub(
                                    r"(?:optional|if specified|if given|storage for|will be set to),?\s*",
                                    "",
                                    comment_line,
                                )
                                out_params.append(comment_line)
                                skipping_param = 2
                            else:
                                skipping_param = 1
                            continue

                    if skipping_param > 0:
                        if re.match(r"^(:.*|\.\..*|\s*)$", comment_line):
                            skipping_param = 0
                        elif skipping_param == 2:
                            comment_line = re.sub(r"^\s+", " ", comment_line)
                            out_params[-1] += comment_line
                            continue
                        else:
                            continue

                    if ":return:" in comment_line and out_params:
                        waiting_for_return_to_end = True
                        comment_line = comment_line.replace(":return:", ":return: -")
                        doc_string += f"{doc_prepend}{comment_line}\n"

                        # scan forward to find end of return description
                        scan_forward_idx = comment_line_idx
                        needs_blank_line_after_return = False
                        while scan_forward_idx < len(comment_lines):
                            scan_forward_line = comment_lines[scan_forward_idx]
                            scan_forward_idx += 1
                            if (
                                not scan_forward_line.strip()
                                and scan_forward_idx < len(comment_lines) - 1
                            ):
                                # check if following line is start of list
                                if re.match(
                                    r"^\s*-(?!-)", comment_lines[scan_forward_idx + 1]
                                ):
                                    doc_string += "\n"
                                    comment_line_idx += 1
                                    needs_blank_line_after_return = True
                                    continue

                            if (
                                re.match(r"^(:.*|\.\..*|\s*)$", scan_forward_line)
                                or not scan_forward_line.strip()
                            ):
                                break

                            doc_string += f"{doc_prepend}  {scan_forward_line}\n"
                            comment_line_idx += 1

                        if needs_blank_line_after_return:
                            doc_string += "\n"

                        for out_param in out_params:
                            doc_string += f"{doc_prepend}         - {out_param}\n"
                        out_params = []
                    else:
                        doc_string += f"{doc_prepend}{comment_line}\n"

                    if waiting_for_return_to_end:
                        if re.match(r"^(:.*|\.\..*|\s*)$", comment_line):
                            waiting_for_return_to_end = False
                        else:
                            pass  # Return docstring should be single line with SIP_OUT params

                if out_params:
                    if CONTEXT.return_type:
                        exit_with_error(
                            f"A method with output parameters must contain a return directive ({CONTEXT.current_method_name} method returns {CONTEXT.return_type})"
                        )
                    else:
                        doc_string += "\n"

                        for out_param_idx, out_param in enumerate(out_params):
                            if out_param_idx == 0:
                                if len(out_params) > 1:
                                    doc_string += f":return: - {out_param}\n"
                                else:
                                    arg_name_match = re.match(
                                        r"^(.*?):\s*(.*?)$", out_param
                                    )
                                    doc_string += (
                                        f":return: {arg_name_match.group(2)}\n"
                                    )
                            else:
                                doc_string += f"{doc_prepend}         - {out_param}\n"

                dbg_info(f"doc_string is {doc_string}")
                write_output("DS", doc_string)
                if CONTEXT.access[-1] == Visibility.Signals and doc_string:
                    dbg_info("storing signal docstring")
                    class_name = ".".join(CONTEXT.classname)
                    CONTEXT.attribute_docstrings[class_name][
                        CONTEXT.current_method_name
                    ] = doc_string
                write_output("CM4", f"{doc_prepend}%End\n")

        CONTEXT.comment = ""
        CONTEXT.return_type = ""
        if CONTEXT.is_override_or_make_private == PrependType.MakePrivate:
            write_output("MKP", CONTEXT.last_access_section_line)
        CONTEXT.is_override_or_make_private = PrependType.NoPrepend
    else:
        if CONTEXT.is_override_or_make_private == PrependType.MakePrivate:
            write_output("MKP", CONTEXT.last_access_section_line)
        CONTEXT.is_override_or_make_private = PrependType.NoPrepend

# Output results
if args.sip_output:
    with open(args.sip_output, "w") as f:
        f.write("".join(sip_header_footer()))
        f.write("".join(CONTEXT.output))
        f.write("".join(sip_header_footer()))
else:
    print(
        "".join(sip_header_footer())
        + "".join(CONTEXT.output)
        + "".join(sip_header_footer()).rstrip()
    )

class_additions = defaultdict(list)

for class_name, attribute_docstrings in CONTEXT.attribute_docstrings.items():
    class_additions[class_name].append(
        f"{class_name}.__attribute_docs__ = {str(attribute_docstrings)}"
    )

for class_name, static_methods in CONTEXT.static_methods.items():
    for method_name, is_static in static_methods.items():
        if not is_static:
            continue

        # TODO -- fix
        if (
            class_name == "QgsProcessingUtils"
            and method_name == "createFeatureSinkPython"
        ):
            method_name = "createFeatureSink"
        elif (
            class_name == "QgsRasterAttributeTable"
            and method_name == "usageInformationInt"
        ):
            method_name = "usageInformation"
        elif (
            class_name == "QgsSymbolLayerUtils"
            and method_name == "wellKnownMarkerFromSld"
        ):
            method_name = "wellKnownMarkerFromSld2"
        elif class_name == "QgsZonalStatistics" and method_name in (
            "calculateStatisticsInt",
            "calculateStatistics",
        ):
            continue
        elif class_name == "QgsServerApiUtils" and method_name == "temporalExtentList":
            method_name = "temporalExtent"

        class_additions[class_name].append(
            f"{class_name}.{method_name} = staticmethod({class_name}.{method_name})"
        )

for class_name, signal_arguments in CONTEXT.signal_arguments.items():
    python_signatures = {}

    for signal, arguments in signal_arguments.items():
        python_args = []
        for argument in arguments:
            var_name, python_type, default_value = parse_argument(argument)
            if default_value:
                python_args.append(f"{var_name}: {python_type} = {default_value}")
            else:
                python_args.append(f"{var_name}: {python_type}")
        if python_args:
            python_signatures[signal] = python_args

    if python_signatures:
        class_additions[class_name].append(
            f"{class_name}.__signal_arguments__ = {str(python_signatures)}"
        )

for class_name, doc_string in CONTEXT.struct_docstrings.items():
    class_additions[class_name].append(f'{class_name}.__doc__ = """{doc_string}"""')

group_match = re.match("^.*src/[a-z0-9_]+/(.*?)/[^/]+$", CONTEXT.header_file)
if group_match:
    groups = list(
        group for group in group_match.group(1).split("/") if group and group != "."
    )
    if groups:
        for class_name in CONTEXT.all_fully_qualified_class_names:
            class_additions[class_name].append(f"{class_name}.__group__ = {groups}")

for _class, additions in class_additions.items():
    if additions:
        this_class_additions = "\n".join("    " + c for c in additions)
        CONTEXT.output_python.append(
            f"try:\n{this_class_additions}\nexcept (NameError, AttributeError):\n    pass\n"
        )

if args.python_output and CONTEXT.output_python:

    with open(args.python_output, "w") as f:
        f.write("".join(python_header()))
        f.write("".join(CONTEXT.output_python))
