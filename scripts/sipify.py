#!/usr/bin/env python3


import re
import sys
import os
import argparse
import yaml
from typing import List, Dict, Any

# Constants
PRIVATE = 0
PROTECTED = 1
PUBLIC = 2
STRICT = 10
UNSTRICT = 11
MULTILINE_NO = 20
MULTILINE_METHOD = 21
MULTILINE_CONDITIONAL_STATEMENT = 22
CODE_SNIPPET = 30
CODE_SNIPPET_CPP = 31
PREPEND_CODE_NO = 40
PREPEND_CODE_VIRTUAL = 41
PREPEND_CODE_MAKE_PRIVATE = 42

# TO RENAME
LINE = ''

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Convert header file to SIP and Python")
parser.add_argument("-debug", action="store_true", help="Enable debug mode")
parser.add_argument("-qt6", action="store_true", help="Enable Qt6 mode")
parser.add_argument("-sip_output", help="SIP output file")
parser.add_argument("-python_output", help="Python output file")
parser.add_argument("-class_map", help="Class map file")
parser.add_argument("headerfile", help="Input header file")
args = parser.parse_args()

is_qt6 = args.qt6
headerfile = args.headerfile

# Read the input file
try:
    with open(headerfile, "r") as f:
        input_lines = f.read().splitlines()
except IOError as e:
    print(f"Couldn't open '{headerfile}' for reading because: {e}", file=sys.stderr)
    sys.exit(1)

# Read configuration
cfg_file = os.path.join(os.path.dirname(__file__), '../python/sipify.yaml')
try:
    with open(cfg_file, 'r') as f:
        sip_config = yaml.safe_load(f)
except IOError as e:
    print(f"Couldn't open configuration file '{cfg_file}' because: {e}", file=sys.stderr)
    sys.exit(1)

# Initialize contexts
sip_run = False
header_code = False
access = [PUBLIC]
classname: List[str] = []
class_and_struct: List[str] = []
declared_classes: List[str] = []
exported = [False]
multiline_definition = MULTILINE_NO
actual_class = ''
python_signature = ''
enum_int_types: List[str] = []
enum_intflag_types: List[str] = []
enum_class_non_int_types: List[str] = []
enum_monkey_patched_types: List = []
indent = ''
prev_indent = ''
comment = ''
comment_param_list = False
comment_last_line_note_warning = False
comment_code_snippet = 0
comment_template_docstring = False
skipped_params_out: List[str] = []
skipped_params_remove: List[str] = []
glob_ifdef_nesting_idx = 0
glob_bracket_nesting_idx = [0]
private_section_line = ''
last_access_section_line = ''
return_type = ''
is_override_or_make_private = PREPEND_CODE_NO
if_feature_condition = ''
found_since = False
qflag_hash: Dict[str, Any] = {}
line_count = len(input_lines)
line_idx = 0
output: List[str] = []
output_python: List[str] = []
doxy_inside_sip_run = 0
has_pushed_force_int = False
debug = args.debug

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
    "QgsDiagramSettings::DiagramOrientation",
    "QgsDiagramSettings::Direction",
    "QgsDiagramSettings::LabelPlacementMethod",
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
    "TextFormatTable"
]


def replace_macros(line):
    global is_qt6

    line = re.sub(r'\bTRUE\b', '``True``', line)
    line = re.sub(r'\bFALSE\b', '``False``', line)
    line = re.sub(r'\bNULLPTR\b', '``None``', line)

    if is_qt6:
        # sip for Qt6 chokes on QList/QVector<QVariantMap>, but is happy if you expand out the map explicitly
        line = re.sub(r'(QList<\s*|QVector<\s*)QVariantMap', r'\1QMap<QString, QVariant>', line)

    return line


def read_line():
    global debug
    global line_idx, input_lines, access, glob_bracket_nesting_idx, sip_run, multiline_definition
    global is_override_or_make_private, actual_class, classname

    new_line = input_lines[line_idx]
    line_idx += 1

    if debug != 0:
        print(f'LIN:{line_idx} DEPTH:{len(access)} ACC:{access[-1]} '
              f'BRCK:{glob_bracket_nesting_idx[-1]} SIP:{sip_run} MLT:{multiline_definition} '
              f'OVR: {is_override_or_make_private} CLSS: {actual_class}/{len(classname)} :: {new_line}')

    new_line = replace_macros(new_line)
    return new_line


def write_output(dbg_code, out, prepend="no"):
    global line_idx, debug, output, if_feature_condition

    if debug == 1:
        dbg_code = f"{line_idx} {dbg_code:<4} :: "
    else:
        dbg_code = ''

    if prepend == "prepend":
        output.insert(0, dbg_code + out)
    else:
        if if_feature_condition != '':
            output.append(f"%If ({if_feature_condition})\n")
        output.append(dbg_code + out)
        if if_feature_condition != '':
            output.append("%End\n")

    if_feature_condition = ''


def dbg_info(info):
    global debug, output, line_idx, access, sip_run, multiline_definition

    if debug == 1:
        output.append(f"{info}\n")
        print(f"{line_idx} {len(access)} {sip_run} {multiline_definition} {info}")


def exit_with_error(message):
    global headerfile, line_idx
    sys.exit(f"! Sipify error in {headerfile} at line :: {line_idx}\n! {message}")


def sip_header_footer():
    global headerfile
    header_footer = []
    # small hack to turn files src/core/3d/X.h to src/core/./3d/X.h
    # otherwise "sip up to date" test fails. This is because the test uses %Include entries
    # and over there we have to use ./3d/X.h entries because SIP parser does not allow a number
    # as the first letter of a relative path
    headerfile_x = re.sub(r'src/core/3d', r'src/core/./3d', headerfile)
    header_footer.append("/************************************************************************\n")
    header_footer.append(" * This file has been generated automatically from                      *\n")
    header_footer.append(" *                                                                      *\n")
    header_footer.append(f" * {headerfile_x:<68} *\n")
    header_footer.append(" *                                                                      *\n")
    header_footer.append(" * Do not edit manually ! Edit header and run scripts/sipify.py again   *\n")
    header_footer.append(" ************************************************************************/\n")
    return header_footer


def python_header():
    global headerfile
    header = []
    headerfile_x = re.sub(r'src/core/3d', r'src/core/./3d', headerfile)
    header.append("# The following has been generated automatically from ")
    header.append(f"{headerfile_x}\n")
    return header


def create_class_links(line):
    global actual_class

    # Replace Qgs classes (but not the current class) with :py:class: links
    _match = re.search(r'\b(Qgs[A-Z]\w+|Qgis)\b(\.?$|\W{2})', line)
    if _match:
        if actual_class and _match.group(1) != actual_class:
            line = re.sub(r'\b(Qgs[A-Z]\w+)\b(\.?$|\W{2})', r':py:class:`\1`\2', line)

    # Replace Qgs class methods with :py:func: links
    line = re.sub(r'\b((Qgs[A-Z]\w+|Qgis)\.[a-z]\w+\(\))(?!\w)', r':py:func:`\1`', line)

    # Replace other methods with :py:func: links
    if actual_class:
        line = re.sub(r'(?<!\.)\b([a-z]\w+)\(\)(?!\w)', rf':py:func:`~{actual_class}.\1`', line)
    else:
        line = re.sub(r'(?<!\.)\b([a-z]\w+)\(\)(?!\w)', r':py:func:`~\1`', line)

    # Replace Qgs classes (but not the current class) with :py:class: links
    _match = re.search(r'\b(?<![`~])(Qgs[A-Z]\w+|Qgis)\b(?!\()', line)
    if _match:
        if not actual_class or _match.group(1) != actual_class:
            line = re.sub(r'\b(?<![`~])(Qgs[A-Z]\w+|Qgis)\b(?!\()', r':py:class:`\1`', line)

    return line


def processDoxygenLine(line):
    global doxy_inside_sip_run, comment_code_snippet, prev_indent, indent
    global comment_param_list, comment_last_line_note_warning, found_since, line_idx

    # Handle SIP_RUN preprocessor directives
    if re.search(r'\s*#ifdef SIP_RUN', line):
        doxy_inside_sip_run = 1
        return ""
    elif re.search(r'\s*#ifndef SIP_RUN', line):
        doxy_inside_sip_run = 2
        return ""
    elif doxy_inside_sip_run != 0 and re.search(r'\s*#else', line):
        doxy_inside_sip_run = 2 if doxy_inside_sip_run == 1 else 1
        return ""
    elif doxy_inside_sip_run != 0 and re.search(r'\s*#endif', line):
        doxy_inside_sip_run = 0
        return ""

    if doxy_inside_sip_run == 2:
        return ""

    # Detect code snippet
    code_match = re.search(r'\\code(\{\.?(\w+)})?', line)
    if code_match:
        codelang = f" {code_match.group(2)}" if code_match.group(2) else ""
        if not re.search(r'(cpp|py|unparsed)', codelang):
            exit_with_error(f"invalid code snippet format: {codelang}")
        comment_code_snippet = CODE_SNIPPET
        if re.search(r'cpp', codelang):
            comment_code_snippet = CODE_SNIPPET_CPP
        codelang = codelang.replace('py', 'python').replace('unparsed', 'raw')
        return "\n" if comment_code_snippet == CODE_SNIPPET_CPP else f"\n.. code-block::{codelang}\n\n"

    if re.search(r'\\endcode', line):
        comment_code_snippet = 0
        return "\n"

    if comment_code_snippet != 0:
        if comment_code_snippet == CODE_SNIPPET_CPP:
            return ""
        else:
            return f"    {line}\n" if line != '' else "\n"

    # Remove prepending spaces and apply various replacements
    line = re.sub(r'^\s+', '', line)
    line = re.sub(r'\\a (.+?)\b', r'``\1``', line)
    line = line.replace('::', '.')
    line = re.sub(r'\bnullptr\b', 'None', line)

    # Handle section and subsection
    section_match = re.match(r'^\\(?P<SUB>sub)?section', line)
    if section_match:
        sep = "~" if section_match.group('SUB') else "-"
        line = re.sub(r'^\\(sub)?section \w+ ', '', line)
        sep_line = re.sub(r'[\w ()]', sep, line)
        line += f"\n{sep_line}"

    # Convert ### style headings
    _match = re.match(r'^###\s+(.*)$', line)
    if _match:
        line = f"{_match.group(1)}\n{'-' * len(_match.group(1))}"
    _match = re.match(r'^##\s+(.*)$', line)
    if _match:
        line = f"{_match.group(1)}\n{'=' * len(_match.group(1))}"

    if line == '*':
        line = ''

    # Handle multi-line parameters/returns/lists
    if line != '':
        if re.match(r'^\s*[\-#]', line):
            line = f"{prev_indent}{line}"
            indent = f"{prev_indent}  "
        elif not re.match(r'^\s*[\\:]+(param|note|since|return|deprecated|warning|throws)', line):
            line = f"{indent}{line}"
    else:
        prev_indent = indent
        indent = ''

    # Replace \returns with :return:
    if re.search(r'\\return(s)?', line):
        line = re.sub(r'\s*\\return(s)?\s*', '\n:return: ', line)
        line = re.sub(r'\s*$', '', line)
        indent = ' ' * (line.index(':', 4) + 1)

    # Handle params
    if re.search(r'\\param ', line):
        line = re.sub(r'\s*\\param\s+(\w+)\b\s*', r':param \1: ', line)
        line = re.sub(r'\s*$', '', line)
        indent = ' ' * (line.index(':', 2) + 2)
        if line.startswith(':param'):
            if not comment_param_list:
                line = f"\n{line}"
            comment_param_list = True
            comment_last_line_note_warning = False

    # Handle brief
    if re.match(r'^\s*[\\@]brief', line):
        line = re.sub(r'[\\@]brief\s*', '', line)
        if found_since:
            exit_with_error(f"{headerfile}::{line_idx} Since annotation must come after brief")
        found_since = False
        if re.match(r'^\s*$', line):
            return ""

    # Handle ingroup and class
    if re.search(r'[\\@](ingroup|class)', line):
        prev_indent = indent
        indent = ''
        return ""

    # Handle since
    since_match = re.search(r'\\since .*?([\d.]+)', line, re.IGNORECASE)
    if since_match:
        prev_indent = indent
        indent = ''
        found_since = True
        return f"\n.. versionadded:: {since_match.group(1)}\n"

    # Handle deprecated
    deprecated_match = re.search(
        r'\\deprecated(?:\s+since\s+QGIS\s+(?P<DEPR_VERSION>[0-9.]+)(,\s*)?)?(?P<DEPR_MESSAGE>.*)?', line,
        re.IGNORECASE)
    if deprecated_match:
        prev_indent = indent
        indent = ''
        depr_line = "\n.. deprecated::"
        if deprecated_match.group('DEPR_VERSION'):
            depr_line += f" QGIS {deprecated_match.group('DEPR_VERSION')}"
        if deprecated_match.group('DEPR_MESSAGE'):
            depr_line += f"\n  {deprecated_match.group('DEPR_MESSAGE')}\n"
        return create_class_links(depr_line)

    # Handle see also
    see_matches = list(re.finditer(r'\\see +(\w+(\.\w+)*)(\([^()]*\))?', line))
    if see_matches:
        for see_match in reversed(see_matches):
            seealso = see_match.group(1)
            seeline = ''
            dbg_info(f"see also: `{seealso}`")
            if re.match(r'^Qgs[A-Z]\w+(\([^()]*\))?$', seealso):
                dbg_info(f"\\see :py:class:`{seealso}`")
                seeline = f":py:class:`{seealso}`"
            elif re.match(r'^(Qgs[A-Z]\w+)\.(\w+)(\([^()]*\))?$', seealso):
                dbg_info(f"\\see py:func with param: :py:func:`{seealso}`")
                seeline = f":py:func:`{seealso}`"
            elif re.match(r'^[a-z]\w+(\([^()]*\))?$', seealso):
                dbg_info(f"\\see :py:func:`{seealso}`")
                seeline = f":py:func:`{seealso}`"

            if re.match(r'^\s*\\see', line):
                return f"\n.. seealso:: {seeline or seealso}\n"
            else:
                if seeline:
                    line = line[:see_match.start()] + seeline + line[see_match.end():] #re.sub(r'\\see +(\w+(\.\w+)*(\(\))?)', seeline, line)
                else:
                    line = line.replace('\\see', 'see')
    elif not re.search(r'\\throws.*', line):
        line = create_class_links(line)

    # Handle note, warning, and throws
    note_match = re.search(r'[\\@]note (.*)', line)
    if note_match:
        comment_last_line_note_warning = True
        prev_indent = indent
        indent = ''
        return f"\n.. note::\n\n   {note_match.group(1)}\n"

    warning_match = re.search(r'[\\@]warning (.*)', line)
    if warning_match:
        prev_indent = indent
        indent = ''
        comment_last_line_note_warning = True
        return f"\n.. warning::\n\n   {warning_match.group(1)}\n"

    throws_match = re.search(r'[\\@]throws (.+?)\b\s*(.*)', line)
    if throws_match:
        prev_indent = indent
        indent = ''
        comment_last_line_note_warning = True
        return f"\n:raises {throws_match.group(1)}: {throws_match.group(2)}\n"

    if line.strip():
        if comment_last_line_note_warning:
            dbg_info(f"prepend spaces for multiline warning/note xx{line}")
            line = f"   {line}"
    else:
        comment_last_line_note_warning = False

    return f"{line}\n"


def detect_and_remove_following_body_or_initializerlist():
    global LINE

    signature = ''

    # Complex regex pattern to match various C++ function declarations and definitions
    pattern1 = r'^(\s*)?((?:(?:explicit|static|const|unsigned|virtual)\s+)*)(([(?:long )\w:]+(<.*?>)?\s+[*&]?)?(~?\w+|(\w+::)?operator.{1,2})\s*\(([\w=()\/ ,&*<>."-]|::)*\)( +(?:const|SIP_[\w_]+?))*)\s*((\s*[:,]\s+\w+\(.*\))*\s*\{.*\}\s*(?:SIP_[\w_]+)?;?|(?!;))(\s*\/\/.*)?$'
    pattern2 = r'SIP_SKIP\s*(?!;)\s*(\/\/.*)?$'
    pattern3 = r'^\s*class.*SIP_SKIP'

    if (re.match(pattern1, LINE) or
        re.search(pattern2, LINE) or
            re.match(pattern3, LINE)):

        dbg_info("remove constructor definition, function bodies, member initializing list (1)")

        # Extract the parts we want to keep
        _match = re.match(pattern1, LINE)
        if _match:
            newline = f"{_match.group(1) or ''}{_match.group(2) or ''}{_match.group(3)};"
        else:
            newline = LINE

        # Call remove_following_body_or_initializerlist() if necessary
        if not re.search(r'{.*}(\s*SIP_\w+)*\s*(//.*)?$', LINE):
            signature = remove_following_body_or_initializerlist()

        LINE = newline

    return signature


def remove_following_body_or_initializerlist():
    global line_idx, line_count

    signature = ''

    dbg_info("remove constructor definition, function bodies, member initializing list (2)")
    line = read_line()

    # Python signature
    if re.match(r'^\s*\[\s*(\w+\s*)?\(', line):
        dbg_info("python signature detected")
        _nesting_index = 0
        while line_idx < line_count:
            _nesting_index += line.count('[')
            _nesting_index -= line.count(']')
            if _nesting_index == 0:
                _match = re.match(r'^(.*);\s*(//.*)?$', line)
                if _match:
                    line = _match.group(1)  # remove semicolon (added later)
                    signature += f"\n{line}"
                    return signature
                break
            signature += f"\n{line}"
            line = read_line()

    # Member initializing list
    while re.match(r'^\s*[:,]\s+([\w<>]|::)+\(.*?\)', line):
        dbg_info("member initializing list")
        line = read_line()

    # Body
    if re.match(r'^\s*\{', line):
        _nesting_index = 0
        while line_idx < line_count:
            dbg_info("  remove body")
            _nesting_index += line.count('{')
            _nesting_index -= line.count('}')
            if _nesting_index == 0:
                break
            line = read_line()

    return signature


def fix_annotations(line):
    global skipped_params_remove, skipped_params_out, is_qt6, multiline_definition, output

    # Get removed params to be able to drop them out of the API doc
    removed_params = re.findall(r'(\w+)\s+SIP_PYARGREMOVE', line)
    if is_qt6:
        removed_params = re.findall(r'(\w+)\s+SIP_PYARGREMOVE6?', line)
    for param in removed_params:
        skipped_params_remove.append(param)
        dbg_info(f"caught removed param: {skipped_params_remove[-1]}")

    _out_params = re.findall(r'(\w+)\s+SIP_OUT', line)
    for param in _out_params:
        skipped_params_out.append(param)
        dbg_info(f"caught removed param: {skipped_params_out[-1]}")

    # Printed annotations
    replacements = {
        r'//\s*SIP_ABSTRACT\b': '/Abstract/',
        r'\bSIP_ABSTRACT\b': '/Abstract/',
        r'\bSIP_ALLOWNONE\b': '/AllowNone/',
        r'\bSIP_ARRAY\b': '/Array/',
        r'\bSIP_ARRAYSIZE\b': '/ArraySize/',
        r'\bSIP_DEPRECATED\b': '/Deprecated/',
        r'\bSIP_CONSTRAINED\b': '/Constrained/',
        r'\bSIP_EXTERNAL\b': '/External/',
        r'\bSIP_FACTORY\b': '/Factory/',
        r'\bSIP_IN\b': '/In/',
        r'\bSIP_INOUT\b': '/In,Out/',
        r'\bSIP_KEEPREFERENCE\b': '/KeepReference/',
        r'\bSIP_NODEFAULTCTORS\b': '/NoDefaultCtors/',
        r'\bSIP_OUT\b': '/Out/',
        r'\bSIP_RELEASEGIL\b': '/ReleaseGIL/',
        r'\bSIP_HOLDGIL\b': '/HoldGIL/',
        r'\bSIP_TRANSFER\b': '/Transfer/',
        r'\bSIP_TRANSFERBACK\b': '/TransferBack/',
        r'\bSIP_TRANSFERTHIS\b': '/TransferThis/',
        r'\bSIP_GETWRAPPER\b': '/GetWrapper/',
        r'SIP_PYNAME\(\s*(\w+)\s*\)': r'/PyName=\1/',
        r'SIP_TYPEHINT\(\s*([\w\.\s,\[\]]+?)\s*\)': r'/TypeHint="\1"/',
        r'SIP_VIRTUALERRORHANDLER\(\s*(\w+)\s*\)': r'/VirtualErrorHandler=\1/',
        r'SIP_THROW\(\s*([\w\s,]+?)\s*\)': r'throw( \1 )',
    }

    for pattern, replacement in replacements.items():
        line = re.sub(pattern, replacement, line)

    # Combine multiple annotations
    while True:
        new_line = re.sub(r'/([\w,]+(="?[\w, \[\]]+"?)?)/\s*/([\w,]+(="?[\w, \[\]]+"?)?]?)/', r'/\1,\3/', line)
        if new_line == line:
            break
        line = new_line
        dbg_info("combine multiple annotations -- works only for 2")

    # Unprinted annotations
    # Original perl regex was:
    # s/(\w+)(\<(?>[^<>]|(?2))*\>)?\s+SIP_PYALTERNATIVETYPE\(\s*\'?([^()']+)(\(\s*(?:[^()]++|(?2))*\s*\))?\'?\s*\)/$3/g;
    line = re.sub(r'(\w+)(<[^>]*>)?\s+SIP_PYALTERNATIVETYPE\(\s*\'?([^()\']+)(\([^()]*\))?\'?\s*\)', r'\3', line)
    line = re.sub(r'(\w+)\s+SIP_PYARGRENAME\(\s*(\w+)\s*\)', r'\2', line)

    # Note: this was the original perl regex, which isn't compatible with Python:
    # line = re.sub(r"""=\s+[^=]*?\s+SIP_PYARGDEFAULT\(\s*\'?([^()']+)(\(\s*(?:[^()]++|(?2))*\s*\))?\'?\s*\)""", r'= \1', line)
    line = re.sub(r"""=\s+[^=]*?\s+SIP_PYARGDEFAULT\(\s*\'?([^()\']+)(\((?:[^()]|\([^()]*\))*\))?\'?\s*\)""", r'= \1',
                  line)

    # Remove argument
    if 'SIP_PYARGREMOVE' in line:
        dbg_info("remove arg")
        if multiline_definition != MULTILINE_NO:
            prev_line = output.pop().rstrip()
            # Update multi line status
            parenthesis_balance = prev_line.count('(') - prev_line.count(')')
            if parenthesis_balance == 1:
                multiline_definition = MULTILINE_NO
            # Concatenate with above line to bring previous commas
            line = f"{prev_line} {line.lstrip()}\n"

        if is_qt6:
            # original perl regex was
            # (?<coma>, +)?(const )?(\w+)(\<(?>[^<>]|(?4))*\>)?\s+[\w&*]+\s+SIP_PYARGREMOVE6{0,1}( = [^()]*(\(\s*(?:[^()]++|(?6))*\s*\))?)?(?(<coma>)|,?)
            line = re.sub(
                r"(?P<coma>, +)?(const )?(\w+)(<[^>]*>)?\s+[\w&*]+\s+SIP_PYARGREMOVE6{0,1}( = [^()]*(\([^()]*\))?)?(?(coma)|,?)",
                '', line)
        else:
            line = re.sub(r'SIP_PYARGREMOVE6\s*', '', line)

        # original perl regex was:
        # (?<coma>, +)?(const )?(\w+)(\<(?>[^<>]|(?4))*\>)?\s+[\w&*]+\s+SIP_PYARGREMOVE( = [^()]*(\(\s*(?:[^()]++|(?6))*\s*\))?)?(?(<coma>)|,?)//
        line = re.sub(
            r'(?P<coma>, +)?(const )?(\w+)(\<(?:[^<>]|\<(?:[^<>]|\<[^<>]*\>)*\>)*\>)?\s+[\w&*]+\s+SIP_PYARGREMOVE( = [^()]*(\(\s*(?:[^()]++|\([^()]*\))*\s*\))?)?(?(coma)|,?)',
            '', line)
        line = re.sub(r'\(\s+\)', '()', line)

    line = re.sub(r'SIP_FORCE', '', line)
    line = re.sub(r'SIP_DOC_TEMPLATE', '', line)
    line = re.sub(r'\s+;$', ';', line)

    return line


def fix_constants(line):
    line = re.sub(r'\bstd::numeric_limits<double>::max\(\)', 'DBL_MAX', line)
    line = re.sub(r'\bstd::numeric_limits<double>::lowest\(\)', '-DBL_MAX', line)
    line = re.sub(r'\bstd::numeric_limits<double>::epsilon\(\)', 'DBL_EPSILON', line)
    line = re.sub(r'\bstd::numeric_limits<qlonglong>::min\(\)', 'LLONG_MIN', line)
    line = re.sub(r'\bstd::numeric_limits<qlonglong>::max\(\)', 'LLONG_MAX', line)
    line = re.sub(r'\bstd::numeric_limits<int>::max\(\)', 'INT_MAX', line)
    line = re.sub(r'\bstd::numeric_limits<int>::min\(\)', 'INT_MIN', line)
    return line


def detect_comment_block(strict_mode=True):
    # Initialize global or module-level variables if necessary
    global LINE, comment, comment_param_list, indent, prev_indent, comment_code_snippet, comment_last_line_note_warning, found_since, skipped_params_out, skipped_params_remove

    comment_param_list = False
    indent = ''
    prev_indent = ''
    comment_code_snippet = 0
    comment_last_line_note_warning = False
    found_since = False
    skipped_params_out = []
    skipped_params_remove = []

    if re.match(r'^\s*/\*', LINE) or (not strict_mode and '/*' in LINE):
        dbg_info("found comment block")
        comment = processDoxygenLine(re.sub(r'^\s*/\*(\*)?(.*?)\n?$', r'\2', LINE))
        comment = re.sub(r'^\s*$', '', comment)

        while not re.search(r'\*/\s*(//.*?)?$', LINE):
            LINE = read_line()
            comment += processDoxygenLine(re.sub(r'\s*\*?(.*?)(/)?\n?$', r'\1', LINE))

        comment = re.sub(r'\n\s+\n', '\n\n', comment)
        comment = re.sub(r'\n{3,}', '\n\n', comment)
        comment = re.sub(r'\n+$', '', comment)

        return True

    return False


def detect_non_method_member(line):
    _pattern = r'''^\s*(?:template\s*<\w+>\s+)?(?:(const|mutable|static|friend|unsigned)\s+)*\w+(::\w+)?(<([\w<> *&,()]|::)+>)?(,?\s+\*?\w+( = (-?\d+(\.\d+)?|((QMap|QList)<[^()]+>\(\))|(\w+::)*\w+(\([^()]?\))?)|\[\d+\])?)+;'''
    return re.match(_pattern, line)


while line_idx < line_count:

    python_signature = ''
    actual_class = classname[-1] if classname else None
    LINE = read_line()

    if re.match(r'^\s*(#define\s+)?SIP_IF_MODULE\(.*\)$', LINE):
        dbg_info('skipping SIP include condition macro')
        continue

    match = re.match(r'^(.*?)\s*//\s*cppcheck-suppress.*$', LINE)
    if match:
        LINE = match.group(1)

    match = re.match(r'^\s*SIP_FEATURE\(\s*(\w+)\s*\)(.*)$', LINE)
    if match:
        write_output("SF1", f"%Feature {match.group(1)}{match.group(2)}\n")
        continue

    match = re.match(r'^\s*SIP_PROPERTY\((.*)\)$', LINE)
    if match:
        write_output("SF1", f"%Property({match.group(1)})\n")
        continue

    match = re.match(r'^\s*SIP_IF_FEATURE\(\s*(!?\w+)\s*\)(.*)$', LINE)
    if match:
        write_output("SF2", f"%If ({match.group(1)}){match.group(2)}\n")
        continue

    match = re.match(r'^\s*SIP_CONVERT_TO_SUBCLASS_CODE(.*)$', LINE)
    if match:
        LINE = f"%ConvertToSubClassCode{match.group(1)}"
        # Do not continue here, let the code process the next steps

    match = re.match(r'^\s*SIP_VIRTUAL_CATCHER_CODE(.*)$', LINE)
    if match:
        LINE = f"%VirtualCatcherCode{match.group(1)}"
        # Do not continue here, let the code process the next steps

    match = re.match(r'^\s*SIP_END(.*)$', LINE)
    if match:
        write_output("SEN", f"%End{match.group(1)}\n")
        continue

    match = re.search(r'SIP_WHEN_FEATURE\(\s*(.*?)\s*\)', LINE)
    if match:
        dbg_info('found SIP_WHEN_FEATURE')
        if_feature_condition = match.group(1)

    if is_qt6:
        LINE = re.sub(r'int\s*__len__\s*\(\s*\)', 'Py_ssize_t __len__()', LINE)
        LINE = re.sub(r'long\s*__hash__\s*\(\s*\)', 'Py_hash_t __hash__()', LINE)

    if is_qt6 and re.match(r'^\s*#ifdef SIP_PYQT5_RUN', LINE):
        dbg_info("do not process PYQT5 code")
        while not re.match(r'^#endif', LINE):
            LINE = read_line()

    if not is_qt6 and re.match(r'^\s*#ifdef SIP_PYQT6_RUN', LINE):
        dbg_info("do not process PYQT6 code")
        while not re.match(r'^#endif', LINE):
            LINE = read_line()

    # Do not process SIP code %XXXCode
    if sip_run and re.match(
        r'^ *% *(VirtualErrorHandler|MappedType|Type(?:Header)?Code|Module(?:Header)?Code|Convert(?:From|To)(?:Type|SubClass)Code|MethodCode|Docstring)(.*)?$',
            LINE):
        LINE = f"%{re.match(r'^ *% *(.*)$', LINE).group(1)}"
        comment = ''
        dbg_info("do not process SIP code")
        while not re.match(r'^ *% *End', LINE):
            write_output("COD", LINE + "\n")
            LINE = read_line()
            if is_qt6:
                LINE = re.sub(r'SIP_SSIZE_T', 'Py_ssize_t', LINE)
                LINE = re.sub(r'SIPLong_AsLong', 'PyLong_AsLong', LINE)
            LINE = re.sub(
                r'^ *% *(VirtualErrorHandler|MappedType|Type(?:Header)?Code|Module(?:Header)?Code|Convert(?:From|To)(?:Type|SubClass)Code|MethodCode|Docstring)(.*)?$',
                r'%\1\2', LINE)
            LINE = re.sub(r'^\s*SIP_END(.*)$', r'%End\1', LINE)

        LINE = re.sub(r'^\s*% End', '%End', LINE)
        write_output("COD", LINE + "\n")
        continue

    # Do not process SIP code %Property
    if sip_run and re.match(r'^ *% *(Property)(.*)?$', LINE):
        LINE = f"%{re.match(r'^ *% *(.*)$', LINE).group(1)}"
        comment = ''
        write_output("COD", LINE + "\n")
        continue

    # Do not process SIP code %If %End
    if sip_run and re.match(r'^ *% (If|End)(.*)?$', LINE):
        LINE = f"%{re.match(r'^ *% (.*)$', LINE).group(1)}"
        comment = ''
        write_output("COD", LINE)
        continue

    # Skip preprocessor directives
    if re.match(r'^\s*#', LINE):
        # Skip #if 0 or #if defined(Q_OS_WIN) blocks
        match = re.match(r'^\s*#if (0|defined\(Q_OS_WIN\))', LINE)
        if match:
            dbg_info(f"skipping #if {match.group(1)} block")
            nesting_index = 0
            while line_idx < line_count:
                LINE = read_line()
                if re.match(r'^\s*#if(def)?\s+', LINE):
                    nesting_index += 1
                elif nesting_index == 0 and re.match(r'^\s*#(endif|else)', LINE):
                    comment = ''
                    break
                elif nesting_index != 0 and re.match(r'^\s*#endif', LINE):
                    nesting_index -= 1
            continue

        if re.match(r'^\s*#ifdef SIP_RUN', LINE):
            sip_run = True
            if access[-1] == PRIVATE:
                dbg_info("writing private content (1)")
                if private_section_line:
                    write_output("PRV1", private_section_line + "\n")
                private_section_line = ''
            continue

        if sip_run:
            if re.match(r'^\s*#endif', LINE):
                if glob_ifdef_nesting_idx == 0:
                    sip_run = False
                    continue
                else:
                    glob_ifdef_nesting_idx -= 1

            if re.match(r'^\s*#if(def)?\s+', LINE):
                glob_ifdef_nesting_idx += 1

            # If there is an else at this level, code will be ignored (i.e., not SIP_RUN)
            if re.match(r'^\s*#else', LINE) and glob_ifdef_nesting_idx == 0:
                while line_idx < line_count:
                    LINE = read_line()
                    if re.match(r'^\s*#if(def)?\s+', LINE):
                        glob_ifdef_nesting_idx += 1
                    elif re.match(r'^\s*#endif', LINE):
                        if glob_ifdef_nesting_idx == 0:
                            comment = ''
                            sip_run = False
                            break
                        else:
                            glob_ifdef_nesting_idx -= 1
                continue

        elif re.match(r'^\s*#ifndef SIP_RUN', LINE):
            # Code is ignored here
            while line_idx < line_count:
                LINE = read_line()
                if re.match(r'^\s*#if(def)?\s+', LINE):
                    glob_ifdef_nesting_idx += 1
                elif re.match(r'^\s*#else', LINE) and glob_ifdef_nesting_idx == 0:
                    # Code here will be printed out
                    if access[-1] == PRIVATE:
                        dbg_info("writing private content (2)")
                        if private_section_line != '':
                            write_output("PRV2", private_section_line + "\n")
                        private_section_line = ''
                    sip_run = True
                    break
                elif re.match(r'^\s*#endif', LINE):
                    if glob_ifdef_nesting_idx == 0:
                        sip_run = 0
                        break
                    else:
                        glob_ifdef_nesting_idx -= 1
            continue

        else:
            continue

    # TYPE HEADER CODE
    if header_code and not sip_run:
        header_code = False
        write_output("HCE", "%End\n")

    # Skip forward declarations
    match = re.match(r'^\s*(template ?<class T> |enum\s+)?(class|struct) \w+(?P<external> *SIP_EXTERNAL)?;\s*(//.*)?$',
                     LINE)
    if match:
        if match.group('external'):
            dbg_info('do not skip external forward declaration')
            comment = ''
        else:
            dbg_info('skipping forward declaration')
            continue

    # Skip friend declarations
    if re.match(r'^\s*friend class \w+', LINE):
        continue

    # Insert metaobject for Q_GADGET
    if re.match(r'^\s*Q_GADGET\b.*?$', LINE):
        if not re.search(r'SIP_SKIP', LINE):
            dbg_info('Q_GADGET')
            write_output("HCE", "  public:\n")
            write_output("HCE", "    static const QMetaObject staticMetaObject;\n\n")
        continue

    # Insert in Python output (python/module/__init__.py)
    match = re.search(r'Q_(ENUM|FLAG)\(\s*(\w+)\s*\)', LINE)
    if match:
        if not re.search(r'SIP_SKIP', LINE):
            is_flag = 1 if match.group(1) == 'FLAG' else 0
            enum_helper = f"{actual_class}.{match.group(2)}.baseClass = {actual_class}"
            dbg_info(f"Q_ENUM/Q_FLAG {enum_helper}")
            if args.python_output:
                if enum_helper != '':
                    output_python.append(f"{enum_helper}\n")
                    if is_flag == 1:
                        # SIP seems to introduce the flags in the module rather than in the class itself
                        # as a dirty hack, inject directly in module, hopefully we don't have flags with the same name...
                        output_python.append(
                            f"{match.group(2)} = {actual_class}  # dirty hack since SIP seems to introduce the flags in module\n")
        continue

    # Skip Q_OBJECT, Q_PROPERTY, Q_ENUM, etc.
    if re.match(
        r'^\s*Q_(OBJECT|ENUMS|ENUM|FLAG|PROPERTY|DECLARE_METATYPE|DECLARE_TYPEINFO|NOWARN_DEPRECATED_(PUSH|POP))\b.*?$',
            LINE):
        continue

    if re.match(r'^\s*QHASH_FOR_CLASS_ENUM', LINE):
        continue

    if re.search(r'SIP_SKIP|SIP_PYTHON_SPECIAL_', LINE):
        dbg_info('SIP SKIP!')
        # if multiline definition, remove previous lines
        if multiline_definition != MULTILINE_NO:
            dbg_info('SIP_SKIP with MultiLine')
            opening_line = ''
            while not re.match(r'^[^()]*\(([^()]*\([^()]*\)[^()]*)*[^()]*$', opening_line):
                opening_line = output.pop()
                if len(output) < 1:
                    exit_with_error('could not reach opening definition')
            dbg_info("removed multiline definition of SIP_SKIP method")
            multiline_definition = MULTILINE_NO

        # also skip method body if there is one
        detect_and_remove_following_body_or_initializerlist()

        # line skipped, go to next iteration
        match = re.search(r'SIP_PYTHON_SPECIAL_(\w+)\(\s*(".*"|\w+)\s*\)', LINE)
        if match:
            method_or_code = match.group(2)
            dbg_info(f"PYTHON SPECIAL method or code: {method_or_code}")
            pyop = f"{actual_class}.__{match.group(1).lower()}__ = lambda self: "
            if re.match(r'^".*"$', method_or_code):
                pyop += method_or_code.strip('"')
            else:
                pyop += f"self.{method_or_code}()"
            dbg_info(f"PYTHON SPECIAL {pyop}")
            if args.python_output:
                output_python.append(f"{pyop}\n")

        comment = ''
        continue

    # Detect comment block
    if detect_comment_block():
        continue

    struct_match = re.match(r'^\s*struct(\s+\w+_EXPORT)?\s+(?P<structname>\w+)$', LINE)
    if struct_match:
        dbg_info("  going to struct => public")
        class_and_struct.append(struct_match.group('structname'))
        classname.append(classname[-1] if classname else struct_match.group(
            'structname'))  # fake new class since struct has considered similarly
        access.append(PUBLIC)
        exported.append(exported[-1])
        glob_bracket_nesting_idx.append(0)

    # class declaration started
    # https://regex101.com/r/KMQdF5/1 (older versions: https://regex101.com/r/6FWntP/16)
    class_pattern = re.compile(
        r"""^(\s*(class))\s+([A-Z0-9_]+_EXPORT\s+)?(Q_DECL_DEPRECATED\s+)?(?P<classname>\w+)(?P<domain>\s*:\s*(public|protected|private)\s+\w+(< *(\w|::)+ *(, *(\w|::)+ *)*>)?(::\w+(<(\w|::)+(, *(\w|::)+)*>)?)*(,\s*(public|protected|private)\s+\w+(< *(\w|::)+ *(, *(\w|::)+)*>)?(::\w+(<\w+(, *(\w|::)+)?>)?)*)*)?(?P<annot>\s*/?/?\s*SIP_\w+)?\s*?(//.*|(?!;))$"""
    )
    class_pattern_match = class_pattern.match(LINE)

    if class_pattern_match:
        dbg_info("class definition started")
        access.append(PUBLIC)
        exported.append(0)
        glob_bracket_nesting_idx.append(0)
        template_inheritance_template = []
        template_inheritance_class1 = []
        template_inheritance_class2 = []
        template_inheritance_class3 = []

        classname.append(class_pattern_match.group('classname'))
        class_and_struct.append(class_pattern_match.group('classname'))

        if len(classname) == 1:
            declared_classes.append(classname[-1])

        dbg_info(f"class: {classname[-1]}")

        if (
            re.search(r'\b[A-Z0-9_]+_EXPORT\b', LINE)
            or len(classname) != 1
            or re.search(r'^\s*template\s*<', input_lines[line_idx - 2])
        ):
            exported[-1] += 1

        LINE = f"{class_pattern_match.group(1)} {class_pattern_match.group('classname')}"

        # append to class map file
        if args.class_map:
            with open(args.class_map, 'a') as fh3:
                fh3.write(f"{'.'.join(classname)}: {headerfile}#L{line_idx}\n")

        # Inheritance
        if class_pattern_match.group('domain'):
            m = class_pattern_match.group('domain')
            m = re.sub(r'public +(\w+, *)*(Ui::\w+,? *)+', '', m)
            m = re.sub(r'public +', '', m)
            m = re.sub(r'[,:]?\s*private +\w+(::\w+)?', '', m)

            # detect template based inheritance
            # https://regex101.com/r/9LGhyy/1
            tpl_pattern = re.compile(
                r'[,:]\s+(?P<tpl>(?!QList)\w+)< *(?P<cls1>(\w|::)+) *(, *(?P<cls2>(\w|::)+)? *(, *(?P<cls3>(\w|::)+)? *)?)? *>'
            )

            for match in tpl_pattern.finditer(m):
                dbg_info("template class")
                template_inheritance_template.append(match.group('tpl'))
                template_inheritance_class1.append(match.group('cls1'))
                template_inheritance_class2.append(match.group('cls2') or "")
                template_inheritance_class3.append(match.group('cls3') or "")

            dbg_info(f"domain: {m}")

            tpl_replace_pattern = re.compile(
                r'\b(?P<tpl>(?!QList)\w+)< *(?P<cls1>(\w|::)+) *(, *(?P<cls2>(\w|::)+)? *(, *(?P<cls3>(\w|::)+)? *)?)? *>'
            )
            m = tpl_replace_pattern.sub(lambda
                                        match: f"{match.group('tpl') or ''}{match.group('cls1') or ''}{match.group('cls2') or ''}{match.group('cls3') or ''}Base",
                                        m)
            m = re.sub(r'(\w+)< *(?:\w|::)+ *>', '', m)
            m = re.sub(r'([:,])\s*,', r'\1', m)
            m = re.sub(r'(\s*[:,])?\s*$', '', m)
            LINE += m

        if class_pattern_match.group('annot'):
            LINE += class_pattern_match.group('annot')
            LINE = fix_annotations(LINE)

        LINE += "\n{\n"
        if comment.strip():
            LINE += "%Docstring(signature=\"appended\")\n" + comment + "\n%End\n"

        LINE += f"\n%TypeHeaderCode\n#include \"{os.path.basename(headerfile)}\""

        # for template based inheritance, add a typedef to define the base type
        while template_inheritance_template:
            tpl = template_inheritance_template.pop()
            cls1 = template_inheritance_class1.pop()
            cls2 = template_inheritance_class2.pop()
            cls3 = template_inheritance_class3.pop()

            if cls2 == "":
                LINE = f"\ntypedef {tpl}<{cls1}> {tpl}{cls1}Base;\n\n{LINE}"
            elif cls3 == "":
                LINE = f"\ntypedef {tpl}<{cls1},{cls2}> {tpl}{cls1}{cls2}Base;\n\n{LINE}"
            else:
                LINE = f"\ntypedef {tpl}<{cls1},{cls2},{cls3}> {tpl}{cls1}{cls2}{cls3}Base;\n\n{LINE}"

            if tpl not in declared_classes:
                tpl_header = f"{tpl.lower()}.h"
                if tpl in sip_config['class_headerfile']:
                    tpl_header = sip_config['class_headerfile'][tpl]
                LINE += f"\n#include \"{tpl_header}\""

            if cls2 == "":
                LINE += f"\ntypedef {tpl}<{cls1}> {tpl}{cls1}Base;"
            elif cls3 == "":
                LINE += f"\ntypedef {tpl}<{cls1},{cls2}> {tpl}{cls1}{cls2}Base;"
            else:
                LINE += f"\ntypedef {tpl}<{cls1},{cls2},{cls3}> {tpl}{cls1}{cls2}{cls3}Base;"

        if any(x == PRIVATE for x in access) and len(access) != 1:
            dbg_info("skipping class in private context")
            continue

        access[-1] = PRIVATE  # private by default
        write_output("CLS", f"{LINE}\n")

        # Skip opening curly bracket, incrementing hereunder
        skip = read_line()
        if not re.match(r'^\s*{\s*$', skip):
            exit_with_error("expecting { after class definition")
        glob_bracket_nesting_idx[-1] += 1

        comment = ''
        header_code = True
        access[-1] = PRIVATE
        continue

    # Bracket balance in class/struct tree
    if not sip_run:
        bracket_balance = 0
        bracket_balance += LINE.count('{')
        bracket_balance -= LINE.count('}')

        if bracket_balance != 0:
            glob_bracket_nesting_idx[-1] += bracket_balance

            if glob_bracket_nesting_idx[-1] == 0:
                dbg_info("going up in class/struct tree")

                if len(access) > 1:
                    glob_bracket_nesting_idx.pop()
                    access.pop()

                    if exported[-1] == 0 and classname[-1] != sip_config.get('no_export_macro'):
                        exit_with_error(
                            f"Class {classname[-1]} should be exported with appropriate [LIB]_EXPORT macro. "
                            f"If this should not be available in python, wrap it in a `#ifndef SIP_RUN` block."
                        )
                    exported.pop()

                try:
                    classname.pop()
                    class_and_struct.pop()
                except IndexError:
                    print('arghghghg')
                    pass

                if len(access) == 1:
                    dbg_info("reached top level")
                    access[-1] = PUBLIC  # Top level should stay public

                comment = ''
                return_type = ''
                private_section_line = ''

            dbg_info(f"new bracket balance: {glob_bracket_nesting_idx}")

    # Private members (exclude SIP_RUN)
    if re.match(r'^\s*private( slots)?:', LINE):
        access[-1] = PRIVATE
        last_access_section_line = LINE
        private_section_line = LINE
        comment = ''
        dbg_info("going private")
        continue

    elif re.match(r'^\s*(public( slots)?|signals):.*$', LINE):
        dbg_info("going public")
        last_access_section_line = LINE
        access[-1] = PUBLIC
        comment = ''

    elif re.match(r'^\s*(protected)( slots)?:.*$', LINE):
        dbg_info("going protected")
        last_access_section_line = LINE
        access[-1] = PROTECTED
        comment = ''

    elif access[-1] == PRIVATE and 'SIP_FORCE' in LINE:
        dbg_info("private with SIP_FORCE")
        if private_section_line:
            write_output("PRV3", private_section_line + "\n")
        private_section_line = ''

    elif any(x == PRIVATE for x in access) and not sip_run:
        comment = ''
        continue

    # Skip operators
    if access[-1] != PRIVATE and re.search(r'operator(=|<<|>>|->)\s*\(', LINE):
        dbg_info("skip operator")
        detect_and_remove_following_body_or_initializerlist()
        continue

    # Save comments and do not print them, except in SIP_RUN
    if not sip_run:
        if re.match(r'^\s*//', LINE):
            match = re.match(r'^\s*//!\s*(.*?)\n?$', LINE)
            if match:
                comment_param_list = False
                prev_indent = indent
                indent = ''
                comment_last_line_note_warning = False
                comment = processDoxygenLine(match.group(1))
                comment = comment.rstrip()
            elif not re.search(r'\*/', input_lines[line_idx - 1]):
                comment = ''
            continue

    # Handle Q_DECLARE_FLAGS in Qt6
    if is_qt6 and re.match(r'^\s*Q_DECLARE_FLAGS\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)', LINE):
        flags_name = re.search(r'\(\s*(\w+)\s*,\s*(\w+)\s*\)', LINE).group(1)
        flag_name = re.search(r'\(\s*(\w+)\s*,\s*(\w+)\s*\)', LINE).group(2)
        output_python.append(f"{actual_class}.{flags_name} = lambda flags=0: {actual_class}.{flag_name}(flags)\n")

    # Enum declaration
    # For scoped and type-based enum, the type has to be removed
    if re.match(
        r'^\s*Q_DECLARE_FLAGS\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)\s*SIP_MONKEYPATCH_FLAGS_UNNEST\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)\s*$',
            LINE):
        flags_name = re.search(r'\(\s*(\w+)\s*,\s*(\w+)\s*\)', LINE).group(1)
        flag_name = re.search(r'\(\s*(\w+)\s*,\s*(\w+)\s*\)', LINE).group(2)
        emkb = re.search(r'SIP_MONKEYPATCH_FLAGS_UNNEST\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)', LINE).group(1)
        emkf = re.search(r'SIP_MONKEYPATCH_FLAGS_UNNEST\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)', LINE).group(2)

        if f"{emkb}.{emkf}" != f"{actual_class}.{flags_name}":
            output_python.append(f"{emkb}.{emkf} = {actual_class}.{flags_name}\n")

        enum_monkey_patched_types.append([actual_class, flags_name, emkb, emkf])

        LINE = re.sub(r'\s*SIP_MONKEYPATCH_FLAGS_UNNEST\(.*?\)', '', LINE)

    enum_match = re.match(
        r'^(\s*enum(\s+Q_DECL_DEPRECATED)?\s+(?P<isclass>class\s+)?(?P<enum_qualname>\w+))(:?\s+SIP_[^:]*)?(\s*:\s*(?P<enum_type>\w+))?(?:\s*SIP_ENUM_BASETYPE\s*\(\s*(?P<py_enum_type>\w+)\s*\))?(?P<oneliner>.*)$',
        LINE)
    if enum_match:
        enum_decl = enum_match.group(1)
        enum_qualname = enum_match.group('enum_qualname')
        enum_type = enum_match.group('enum_type')
        isclass = enum_match.group('isclass')
        enum_cpp_name = f"{actual_class}::{enum_qualname}" if actual_class else enum_qualname

        if not isclass and enum_cpp_name not in ALLOWED_NON_CLASS_ENUMS:
            exit_with_error(f"Non class enum exposed to Python -- must be a enum class: {enum_cpp_name}")

        oneliner = enum_match.group('oneliner')
        is_scope_based = bool(isclass)
        enum_decl = re.sub(r'\s*\bQ_DECL_DEPRECATED\b', '', enum_decl)

        py_enum_type_match = re.search(r'SIP_ENUM_BASETYPE\(\s*(.*?)\s*\)', LINE)
        py_enum_type = py_enum_type_match.group(1) if py_enum_type_match else None

        if py_enum_type == "IntFlag":
            enum_intflag_types.append(enum_cpp_name)

        if enum_type in ["int", "quint32"]:
            enum_int_types.append(f"{actual_class}.{enum_qualname}")
            if is_qt6:
                enum_decl += f" /BaseType={py_enum_type or 'IntEnum'}/"
        elif enum_type:
            exit_with_error(f"Unhandled enum type {enum_type} for {enum_cpp_name}")
        elif isclass:
            enum_class_non_int_types.append(f"{actual_class}.{enum_qualname}")
        elif is_qt6:
            enum_decl += " /BaseType=IntEnum/"

        write_output("ENU1", enum_decl)
        if oneliner:
            write_output("ENU1", oneliner)
        write_output("ENU1", "\n")

        _match = None
        if is_scope_based:
            _match = re.search(
                r'SIP_MONKEYPATCH_SCOPEENUM(_UNNEST)?(:?\(\s*(?P<emkb>\w+)\s*,\s*(?P<emkf>\w+)\s*\))?', LINE)
        monkeypatch = is_scope_based and _match
        enum_mk_base = _match.group('emkb') if _match else ''

        enum_old_name = ''
        if _match and _match.group('emkf') and monkeypatch:
            enum_old_name = _match.group('emkf')
            if actual_class:
                if f"{enum_mk_base}.{enum_old_name}" != f"{actual_class}.{enum_qualname}":
                    output_python.append(f"{enum_mk_base}.{enum_old_name} = {actual_class}.{enum_qualname}\n")
            else:
                output_python.append(f"{enum_mk_base}.{enum_old_name} = {enum_qualname}\n")

        if re.search(r'\{((\s*\w+)(\s*=\s*[\w\s<|]+.*?)?(,?))+\s*}', LINE):
            if '=' in LINE:
                exit_with_error(
                    "Sipify does not handle enum one liners with value assignment. Use multiple lines instead. Or just write a new parser.")
            continue
        else:
            LINE = read_line()
            if not re.match(r'^\s*\{\s*$', LINE):
                exit_with_error('Unexpected content: enum should be followed by {')
            write_output("ENU2", f"{LINE}\n")

            if is_scope_based:
                output_python.append("# monkey patching scoped based enum\n")

            enum_members_doc = []

            while line_idx < line_count:
                LINE = read_line()
                if detect_comment_block():
                    continue
                if re.search(r'};', LINE):
                    break
                if re.match(r'^\s*\w+\s*\|', LINE):  # multi line declaration as sum of enums
                    continue

                enum_match = re.match(
                    r'^(\s*(?P<em>\w+))(\s+SIP_PYNAME(?:\(\s*(?P<pyname>[^() ]+)\s*\)\s*)?)?(\s+SIP_MONKEY\w+(?:\(\s*(?P<compat>[^() ]+)\s*\)\s*)?)?(?:\s*=\s*(?P<enum_value>(:?[\w\s\d|+-]|::|<<)+))?(?P<optional_comma>,?)(:?\s*//!<\s*(?P<co>.*)|.*)$',
                    LINE)

                enum_decl = f"{enum_match.group(1) or ''}{enum_match.group(3) or ''}{enum_match.group('optional_comma') or ''}" if enum_match else LINE
                enum_member = enum_match.group('em') or '' if enum_match else ''
                value_comment = enum_match.group('co') or '' if enum_match else ''
                compat_name = enum_match.group('compat') or enum_member if enum_match else ''
                enum_value = enum_match.group('enum_value') or '' if enum_match else ''

                value_comment = value_comment.replace('::', '.').replace('"', '\\"')
                value_comment = re.sub(r'\\since .*?([\d.]+)', r'\\n.. versionadded:: \1\\n', value_comment, flags=re.I)
                value_comment = re.sub(r'\\deprecated (.*)', r'\\n.. deprecated:: \1\\n', value_comment, flags=re.I)
                value_comment = re.sub(r'^\\n+', '', value_comment)
                value_comment = re.sub(r'\\n+$', '', value_comment)

                dbg_info(f"is_scope_based:{is_scope_based} enum_mk_base:{enum_mk_base} monkeypatch:{monkeypatch}")

                if enum_value and (re.search(r'.*<<.*', enum_value) or re.search(r'.*0x0.*', enum_value)):
                    if f"{actual_class}::{enum_qualname}" not in enum_intflag_types:
                        exit_with_error(
                            f"{actual_class}::{enum_qualname} is a flags type, but was not declared with IntFlag type. Add 'SIP_ENUM_BASETYPE(IntFlag)' to the enum class declaration line")

                if is_scope_based and enum_member:
                    if monkeypatch and enum_mk_base:
                        if actual_class:
                            output_python.append(
                                f"{enum_mk_base}.{compat_name} = {actual_class}.{enum_qualname}.{enum_member}\n")
                            if enum_old_name and compat_name != enum_member:
                                output_python.append(
                                    f"{enum_mk_base}.{enum_old_name}.{compat_name} = {actual_class}.{enum_qualname}.{enum_member}\n")
                            output_python.append(f"{enum_mk_base}.{compat_name}.is_monkey_patched = True\n")
                            output_python.append(f"{enum_mk_base}.{compat_name}.__doc__ = \"{value_comment}\"\n")
                            enum_members_doc.append(
                                f"'* ``{compat_name}``: ' + {actual_class}.{enum_qualname}.{enum_member}.__doc__")
                        else:
                            output_python.append(f"{enum_mk_base}.{compat_name} = {enum_qualname}.{enum_member}\n")
                            output_python.append(f"{enum_mk_base}.{compat_name}.is_monkey_patched = True\n")
                            output_python.append(f"{enum_mk_base}.{compat_name}.__doc__ = \"{value_comment}\"\n")
                            enum_members_doc.append(
                                f"'* ``{compat_name}``: ' + {enum_qualname}.{enum_member}.__doc__")
                    else:
                        if monkeypatch:
                            output_python.append(
                                f"{actual_class}.{compat_name} = {actual_class}.{enum_qualname}.{enum_member}\n")
                            output_python.append(f"{actual_class}.{compat_name}.is_monkey_patched = True\n")
                        if actual_class:
                            complete_class_path = '.'.join(classname)
                            output_python.append(
                                f"{complete_class_path}.{enum_qualname}.{compat_name}.__doc__ = \"{value_comment}\"\n")
                            enum_members_doc.append(
                                f"'* ``{compat_name}``: ' + {actual_class}.{enum_qualname}.{enum_member}.__doc__")
                        else:
                            output_python.append(f"{enum_qualname}.{compat_name}.__doc__ = \"{value_comment}\"\n")
                            enum_members_doc.append(
                                f"'* ``{compat_name}``: ' + {enum_qualname}.{enum_member}.__doc__")

                if not is_scope_based and is_qt6 and enum_member:
                    basename = '.'.join(class_and_struct)
                    if basename:
                        enum_member = 'None_' if enum_member == 'None' else enum_member
                        output_python.append(
                            f"{basename}.{enum_member} = {basename}.{enum_qualname}.{enum_member}\n")

                enum_decl = fix_annotations(enum_decl)
                write_output("ENU3", f"{enum_decl}\n")

                detect_comment_block(strict_mode=False)

            write_output("ENU4", f"{LINE}\n")

            if is_scope_based:
                comment = comment.replace('\n', '\\n').replace('"', '\\"')

                if actual_class:
                    output_python.append(f'{actual_class}.{enum_qualname}.__doc__ = "{comment}\\n\\n" + ' +
                                         " + '\\n' + ".join(enum_members_doc) + '\n# --\n')
                else:
                    output_python.append(f'{enum_qualname}.__doc__ = \'{comment}\\n\\n\' + ' +
                                         " + '\\n' + ".join(enum_members_doc) + '\n# --\n')

            # enums don't have Docstring apparently
            comment = ''
            continue

    # Check for invalid use of doxygen command
    if re.search(r'.*//!<', LINE):
        exit_with_error('"\\!<" doxygen command must only be used for enum documentation')

    # Handle override, final, and make private keywords
    if re.search(r'\boverride\b', LINE):
        is_override_or_make_private = PREPEND_CODE_VIRTUAL
    if re.search(r'\bFINAL\b', LINE):
        is_override_or_make_private = PREPEND_CODE_VIRTUAL
    if re.search(r'\bSIP_MAKE_PRIVATE\b', LINE):
        is_override_or_make_private = PREPEND_CODE_MAKE_PRIVATE

    # Remove Q_INVOKABLE
    LINE = re.sub(r'^(\s*)Q_INVOKABLE ', r'\1', LINE)

    # Keyword fixes
    LINE = re.sub(r'^(\s*template\s*<)(?:class|typename) (\w+>)(.*)$', r'\1\2\3', LINE)
    LINE = re.sub(r'^(\s*template\s*<)(?:class|typename) (\w+) *, *(?:class|typename) (\w+>)(.*)$', r'\1\2,\3\4',
                  LINE)
    LINE = re.sub(
        r'^(\s*template\s*<)(?:class|typename) (\w+) *, *(?:class|typename) (\w+) *, *(?:class|typename) (\w+>)(.*)$',
        r'\1\2,\3,\4\5', LINE)
    LINE = re.sub(r'\s*\boverride\b', '', LINE)
    LINE = re.sub(r'\s*\bSIP_MAKE_PRIVATE\b', '', LINE)
    LINE = re.sub(r'\s*\bFINAL\b', ' ${SIP_FINAL}', LINE)
    LINE = re.sub(r'\s*\bextern \b', '', LINE)
    LINE = re.sub(r'\s*\bMAYBE_UNUSED \b', '', LINE)
    LINE = re.sub(r'\s*\bNODISCARD \b', '', LINE)
    LINE = re.sub(r'\s*\bQ_DECL_DEPRECATED\b', '', LINE)
    LINE = re.sub(r'^(\s*)?(const |virtual |static )*inline ', r'\1\2', LINE)
    LINE = re.sub(r'\bconstexpr\b', 'const', LINE)
    LINE = re.sub(r'\bnullptr\b', '0', LINE)
    LINE = re.sub(r'\s*=\s*default\b', '', LINE)

    # Handle export macros
    if re.search(r'\b\w+_EXPORT\b', LINE):
        exported[-1] += 1
        LINE = re.sub(r'\b\w+_EXPORT\s+', '', LINE)

    # Skip non-method member declaration in non-public sections
    if not sip_run and access[-1] != PUBLIC and detect_non_method_member(LINE):
        dbg_info("skip non-method member declaration in non-public sections")
        continue

    # Remove static const value assignment
    # https://regex101.com/r/DyWkgn/6
    if re.search(r'^\s*const static \w+', LINE):
        exit_with_error(f"const static should be written static const in {classname[-1]}")

    # TODO needs fixing!!
    # original perl regex was:
    #       ^(?<staticconst> *(?<static>static )?const \w+(?:<(?:[\w<>, ]|::)+>)? \w+)(?: = [^()]+?(\((?:[^()]++|(?3))*\))?[^()]*?)?(?<endingchar>[|;]) *(\/\/.*?)?$
    match = re.search(
        r'^(?P<staticconst> *(?P<static>static )?const \w+(?:<(?:[\w<>, ]|::)+>)? \w+)(?: = [^()]+?(\((?:[^()]|\([^()]*\))*\))?[^()]*?)?(?P<endingchar>[|;]) *(\/\/.*)?$',
        LINE
    )
    if match:
        LINE = f"{match.group('staticconst')};"
        if match.group('static') is None:
            comment = ''

        if match.group('endingchar') == '|':
            dbg_info("multiline const static assignment")
            skip = ''
            while not re.search(r';\s*(//.*?)?$', skip):
                skip = read_line()

    # Remove struct member assignment
    # https://regex101.com/r/OUwV75/1
    if not sip_run and access[-1] == PUBLIC:
        # original perl regex: ^(\s*\w+[\w<> *&:,]* \*?\w+) = ([\-\w\:\.]+(< *\w+( \*)? *>)?)+(\([^()]*\))?\s*;
        # dbg_info(f"attempt struct member assignment '{LINE}'")

        python_regex_verbose = r'''
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
        '''
        regex_verbose = re.compile(python_regex_verbose, re.VERBOSE | re.MULTILINE)
        match = regex_verbose.match(LINE)
        if match:
            dbg_info(f"remove struct member assignment '={match.group(2)}'")
            LINE = f"{match.group(1)};"

    # Catch Q_DECLARE_FLAGS
    match = re.search(r'^(\s*)Q_DECLARE_FLAGS\(\s*(.*?)\s*,\s*(.*?)\s*\)\s*$', LINE)
    if match:
        actual_class = f"{classname[-1]}::" if len(classname) >= 0 else ''
        dbg_info(f"Declare flags: {actual_class}")
        LINE = f"{match.group(1)}typedef QFlags<{actual_class}{match.group(3)}> {match.group(2)};\n"
        qflag_hash[f"{actual_class}{match.group(2)}"] = f"{actual_class}{match.group(3)}"

        if f"{actual_class}{match.group(3)}" not in enum_intflag_types:
            exit_with_error(
                f"{actual_class}{match.group(3)} is a flags type, but was not declared with IntFlag type. Add 'SIP_ENUM_BASETYPE(IntFlag)' to the enum class declaration line")

    # Catch Q_DECLARE_OPERATORS_FOR_FLAGS
    match = re.search(r'^(\s*)Q_DECLARE_OPERATORS_FOR_FLAGS\(\s*(.*?)\s*\)\s*$', LINE)
    if match:
        flags = match.group(2)
        flag = qflag_hash.get(flags)
        LINE = f"{match.group(1)}QFlags<{flag}> operator|({flag} f1, QFlags<{flag}> f2);\n"

        py_flag = flag.replace("::", ".")

        if py_flag in enum_class_non_int_types:
            exit_with_error(
                f"{flag} is a flags type, but was not declared with int type. Add ': int' to the enum class declaration line")
        elif py_flag not in enum_int_types:
            if is_qt6:
                dbg_info("monkey patching operators for non-class enum")
                if not has_pushed_force_int:
                    output_python.append(
                        "from enum import Enum\n\n\ndef _force_int(v): return int(v.value) if isinstance(v, Enum) else v\n\n\n")
                    has_pushed_force_int = True
                output_python.append(f"{py_flag}.__bool__ = lambda flag: bool(_force_int(flag))\n")
                output_python.append(
                    f"{py_flag}.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)\n")
                output_python.append(
                    f"{py_flag}.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)\n")
                output_python.append(
                    f"{py_flag}.__or__ = lambda flag1, flag2: {py_flag}(_force_int(flag1) | _force_int(flag2))\n")

        if not is_qt6:
            for patched_type in enum_monkey_patched_types:
                if flags == f"{patched_type[0]}::{patched_type[1]}":
                    dbg_info("monkey patching flags")
                    if not has_pushed_force_int:
                        output_python.append(
                            "from enum import Enum\n\n\ndef _force_int(v): return int(v.value) if isinstance(v, Enum) else v\n\n\n")
                        has_pushed_force_int = True
                    output_python.append(
                        f"{py_flag}.__or__ = lambda flag1, flag2: {patched_type[0]}.{patched_type[1]}(_force_int(flag1) | _force_int(flag2))\n")

    # Remove keywords
    if is_override_or_make_private != PREPEND_CODE_NO:
        # Handle multiline definition to add virtual keyword or make private on opening line
        if multiline_definition != MULTILINE_NO:
            rolling_line = LINE
            rolling_line_idx = line_idx
            dbg_info("handle multiline definition to add virtual keyword or making private on opening line")
            while not re.match(r'^[^()]*\(([^()]*\([^()]*\)[^()]*)*[^()]*$', rolling_line):
                rolling_line_idx -= 1
                rolling_line = input_lines[rolling_line_idx]
                if rolling_line_idx < 0:
                    exit_with_error('could not reach opening definition')
            dbg_info(f'rolled back to {rolling_line_idx}: {rolling_line}')

            if is_override_or_make_private == PREPEND_CODE_VIRTUAL and not re.match(r'^(\s*)virtual\b(.*)$',
                                                                                    rolling_line):
                idx = rolling_line_idx - line_idx + 1
                output[idx] = fix_annotations(re.sub(r'^(\s*?)\b(.*)$', r'\1 virtual \2\n', rolling_line))
            elif is_override_or_make_private == PREPEND_CODE_MAKE_PRIVATE:
                dbg_info("prepending private access")
                idx = rolling_line_idx - line_idx
                private_access = re.sub(r'(protected|public)', 'private', last_access_section_line)
                output.insert(idx + 1, private_access + "\n")
                output[idx + 1] = fix_annotations(rolling_line) + "\n"
        elif is_override_or_make_private == PREPEND_CODE_MAKE_PRIVATE:
            dbg_info("prepending private access")
            LINE = re.sub(r'(protected|public)', 'private', last_access_section_line) + "\n" + LINE + "\n"
        elif is_override_or_make_private == PREPEND_CODE_VIRTUAL and not re.match(r'^(\s*)virtual\b(.*)$', LINE):
            # SIP often requires the virtual keyword to be present, or it chokes on covariant return types
            # in overridden methods
            dbg_info('adding virtual keyword for overridden method')
            LINE = re.sub(r'^(\s*?)\b(.*)$', r'\1virtual \2\n', LINE)

    # remove constructor definition, function bodies, member initializing list
    python_signature = detect_and_remove_following_body_or_initializerlist()

    # remove inline declarations
    match = re.search(
        r'^(\s*)?(static |const )*(([(?:long )\w:]+(<.*?>)?\s+(\*|&)?)?(\w+)( (?:const*?))*)\s*(\{.*\});(\s*\/\/.*)?$',
        LINE)
    if match:
        LINE = f"{match.group(1)}{match.group(3)};"

    pattern = r'^\s*(?:const |virtual |static |inline )*(?!explicit)([(?:long )\w:]+(?:<.*?>)?)\s+(?:\*|&)?(?:\w+|operator.{1,2})\(.*$'
    match = re.match(pattern, LINE)
    if match:
        return_type_candidate = match.group(1)
        if not re.search(r'(void|SIP_PYOBJECT|operator|return|QFlag)', return_type_candidate):
            # replace :: with . (changes c++ style namespace/class directives to Python style)
            return_type = return_type_candidate.replace('::', '.')
            # replace with builtin Python types
            return_type = re.sub(r'\bdouble\b', 'float', return_type)
            return_type = re.sub(r'\bQString\b', 'str', return_type)
            return_type = re.sub(r'\bQStringList\b', 'list of str', return_type)

            list_match = re.match(r'^(?:QList|QVector)<\s*(.*?)[\s*]*>$', return_type)
            if list_match:
                return_type = f"list of {list_match.group(1)}"

            set_match = re.match(r'^QSet<\s*(.*?)[\s*]*>$', return_type)
            if set_match:
                return_type = f"set of {set_match.group(1)}"

    # deleted functions
    if re.match(
        r'^(\s*)?(const )?(virtual |static )?((\w+(<.*?>)?\s+([*&])?)?(\w+|operator.{1,2})\(.*?(\(.*\))*.*\)( const)?)\s*= delete;(\s*//.*)?$',
            LINE):
        comment = ''
        continue

    # remove export macro from struct definition
    LINE = re.sub(r'^(\s*struct )\w+_EXPORT (.+)$', r'\1\2', LINE)

    # Skip comments
    if re.match(r'^\s*typedef\s+\w+\s*<\s*\w+\s*>\s+\w+\s+.*SIP_DOC_TEMPLATE', LINE):
        # support Docstring for template based classes in SIP 4.19.7+
        comment_template_docstring = True
    elif (multiline_definition == MULTILINE_NO and
          (re.search(r'//', LINE) or
           re.match(r'^\s*typedef ', LINE) or
           re.search(r'\s*struct ', LINE) or
           re.search(r'operator\[]\(', LINE) or
           re.match(r'^\s*operator\b', LINE) or
           re.search(r'operator\s?[!+-=*/\[\]<>]{1,2}', LINE) or
           re.match(r'^\s*%\w+(.*)?$', LINE) or
           re.match(r'^\s*namespace\s+\w+', LINE) or
           re.match(r'^\s*(virtual\s*)?~', LINE) or
           detect_non_method_member(LINE)
           )):
        dbg_info('skipping comment')
        if re.search(r'\s*typedef.*?(?!SIP_DOC_TEMPLATE)', LINE):
            dbg_info('because typedef')
        comment = ''
        return_type = ''
        is_override_or_make_private = PREPEND_CODE_NO

    LINE = fix_constants(LINE)
    LINE = fix_annotations(LINE)

    # fix astyle placing space after % character
    LINE = re.sub(r'/\s+GetWrapper\s+/', '/GetWrapper/', LINE)

    # MISSING
    # handle enum/flags QgsSettingsEntryEnumFlag
    match = re.match(r'^(\s*)const QgsSettingsEntryEnumFlag<(.*)> (.+);$', LINE)
    if match:
        indent, enum_type, var_name = match.groups()

        prep_line = f"""class QgsSettingsEntryEnumFlag_{var_name}
    {{
    %TypeHeaderCode
    #include "{os.path.basename(headerfile)}"
    #include "qgssettingsentry.h"
    typedef QgsSettingsEntryEnumFlag<{enum_type}> QgsSettingsEntryEnumFlag_{var_name};
    %End
      public:
        QgsSettingsEntryEnumFlag_{var_name}( const QString &key, QgsSettings::Section section, const {enum_type} &defaultValue, const QString &description = QString() );
        QString key( const QString &dynamicKeyPart = QString() ) const;
        {enum_type} value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, const {enum_type} &defaultValueOverride = {enum_type}() ) const;
    }};"""

        LINE = f"{indent}const QgsSettingsEntryEnumFlag_{var_name} {var_name};"
        comment = ''
        write_output("ENF", f"{prep_line}\n", "prepend")

    write_output("NOR", f"{LINE}\n")

    # append to class map file
    if args.class_map and actual_class:
        match = re.match(r'^ *(const |virtual |static )* *[\w:]+ +\*?(?P<method>\w+)\(.*$', LINE)
        if match:
            with open(args.class_map, 'a') as f:
                f.write(f"{'.'.join(classname)}.{match.group('method')}: {headerfile}#L{line_idx}\n")

    if python_signature:
        write_output("PSI", f"{python_signature}\n")

    # multiline definition (parenthesis left open)
    if multiline_definition != MULTILINE_NO:
        dbg_info("on multiline")
        # https://regex101.com/r/DN01iM/4
        # TODO - original regex is incompatible with python -- it was:
        # ^([^()]+(\((?:[^()]++|(?1))*\)))*[^()]*\)([^()](throw\([^()]+\))?)*$:
        if re.match(r'^([^()]+(\((?:[^()]|\([^()]*\))*\)))*[^()]*\)([^()](throw\([^()]+\))?)*', LINE):
            dbg_info("ending multiline")
            # remove potential following body
            if multiline_definition != MULTILINE_CONDITIONAL_STATEMENT and not re.search(r'(\{.*}|;)\s*(//.*)?$',
                                                                                         LINE):
                dbg_info("remove following body of multiline def")
                last_line = LINE
                last_line += remove_following_body_or_initializerlist()
                # add missing semi column
                output.pop()
                write_output("MLT", f"{last_line};\n")
            multiline_definition = MULTILINE_NO
        else:
            continue
    elif re.match(r'^[^()]+\([^()]*(?:\([^()]*\)[^()]*)*[^)]*$', LINE):
        dbg_info(f"Multiline detected:: {LINE}")
        if re.match(r'^\s*((else )?if|while|for) *\(', LINE):
            multiline_definition = MULTILINE_CONDITIONAL_STATEMENT
        else:
            multiline_definition = MULTILINE_METHOD
        continue

    # write comment
    if re.match(r'^\s*$', LINE):
        dbg_info("no more override / private")
        is_override_or_make_private = PREPEND_CODE_NO
        continue

    if re.match(r'^\s*template\s*<.*>', LINE):
        # do not comment now for templates, wait for class definition
        continue

    if comment.strip() or return_type:
        if is_override_or_make_private != PREPEND_CODE_VIRTUAL and not comment.strip():
            # overridden method with no new docs - so don't create a Docstring and use
            # parent class Docstring
            pass
        else:
            dbg_info('writing comment')
            if comment.strip():
                dbg_info('comment non-empty')
                doc_prepend = "@DOCSTRINGSTEMPLATE@" if comment_template_docstring else ""
                write_output("CM1", f"{doc_prepend}%Docstring\n")

                comment_lines = comment.split('\n')
                skipping_param = 0
                out_params = []
                waiting_for_return_to_end = False

                for comment_line in comment_lines:
                    if ('versionadded:' in comment_line or 'deprecated:' in comment_line) and out_params:
                        dbg_info('out style parameters remain to flush!')
                        # member has /Out/ parameters, but no return type, so flush out out_params docs now
                        first_out_param = out_params.pop(0)
                        write_output("CM7", f"{doc_prepend}:return: - {first_out_param}\n")

                        for out_param in out_params:
                            write_output("CM7", f"{doc_prepend}         - {out_param}\n")
                        write_output("CM7", f"{doc_prepend}\n")
                        out_params = []

                    param_match = re.match(r'^:param\s+(\w+)', comment_line)
                    if param_match:
                        param_name = param_match.group(1)
                        if param_name in skipped_params_out or param_name in skipped_params_remove:
                            if param_name in skipped_params_out:
                                comment_line = re.sub(r'^:param\s+(\w+):\s*(.*?)$', r'\1: \2', comment_line)
                                comment_line = re.sub(r'(?:optional|if specified|if given),?\s*', '',
                                                      comment_line)
                                out_params.append(comment_line)
                                skipping_param = 2
                            else:
                                skipping_param = 1
                            continue

                    if skipping_param > 0:
                        if re.match(r'^(:.*|\.\..*|\s*)$', comment_line):
                            skipping_param = 0
                        elif skipping_param == 2:
                            comment_line = re.sub(r'^\s+', ' ', comment_line)
                            out_params[-1] += comment_line
                            continue
                        else:
                            continue

                    if ':return:' in comment_line and out_params:
                        waiting_for_return_to_end = True
                        comment_line = comment_line.replace(':return:', ':return: -')
                        write_output("CM2", f"{doc_prepend}{comment_line}\n")
                        for out_param in out_params:
                            write_output("CM7", f"{doc_prepend}         - {out_param}\n")
                        out_params = []
                    else:
                        write_output("CM2", f"{doc_prepend}{comment_line}\n")

                    if waiting_for_return_to_end:
                        if re.match(r'^(:.*|\.\..*|\s*)$', comment_line):
                            waiting_for_return_to_end = False
                        else:
                            pass  # Return docstring should be single line with SIP_OUT params

                if out_params and return_type:
                    exit_with_error(
                        f"A method with output parameters must contain a return directive (method returns {return_type})")

                write_output("CM4", f"{doc_prepend}%End\n")

        comment = ''
        return_type = ''
        if is_override_or_make_private == PREPEND_CODE_MAKE_PRIVATE:
            write_output("MKP", last_access_section_line)
        is_override_or_make_private = PREPEND_CODE_NO
    else:
        if is_override_or_make_private == PREPEND_CODE_MAKE_PRIVATE:
            write_output("MKP", last_access_section_line)
        is_override_or_make_private = PREPEND_CODE_NO

# Output results
if args.sip_output:
    with open(args.sip_output, 'w') as f:
        f.write(''.join(sip_header_footer()))
        f.write(''.join(output))
        f.write(''.join(sip_header_footer()))
else:
    print(''.join(sip_header_footer()))
    print(''.join(output))
    print(''.join(sip_header_footer()))

if args.python_output and output_python:
    with open(args.python_output, 'w') as f:
        f.write(''.join(python_header()))
        f.write(''.join(output_python))
