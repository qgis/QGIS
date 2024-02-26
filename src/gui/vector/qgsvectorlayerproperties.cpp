/***************************************************************************
                          qgsdlgvectorlayerproperties.cpp
                   Unified property dialog for vector layers
                             -------------------
    begin                : 2004-01-28
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>
#include <limits>

#include "qgsactionmanager.h"
#include "qgsjoindialog.h"
#include "qgssldexportcontext.h"
#include "qgsvectorlayerselectionproperties.h"
#include "qgswmsdimensiondialog.h"
#include "qgsapplication.h"
#include "qgsattributeactiondialog.h"
#include "qgsdatumtransformdialog.h"
#include "qgsdiagramproperties.h"
#include "qgssourcefieldsproperties.h"
#include "qgsattributesformproperties.h"
#include "qgslabelingwidget.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsmetadatawidget.h"
#include "qgsmetadataurlitemdelegate.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgsvectorlayerproperties.h"
#include "qgsvectordataprovider.h"
#include "qgssubsetstringeditorinterface.h"
#include "qgsdatasourceuri.h"
#include "qgsrenderer.h"
#include "qgsexpressioncontext.h"
#include "qgssettings.h"
#include "qgsrendererpropertiesdialog.h"
#include "qgsstyle.h"
#include "qgsauxiliarystorage.h"
#include "qgsmaplayersavestyledialog.h"
#include "qgsmaplayerserverproperties.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgsnewauxiliaryfielddialog.h"
#include "qgslabelinggui.h"
#include "qgsmessagebar.h"
#include "qgssymbolwidgetcontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmaskingwidget.h"
#include "qgsvectorlayertemporalpropertieswidget.h"
#include "qgsprovidersourcewidget.h"
#include "qgsproviderregistry.h"
#include "qgsmaplayerstylemanager.h"
#include "qgslayertreemodel.h"
#include "qgsmaptip.h"
#include "qgsgui.h"
#include "qgsnative.h"
#include "qgssubsetstringeditorproviderregistry.h"
#include "qgsprovidersourcewidgetproviderregistry.h"
#include "qgsfileutils.h"
#include "qgswebview.h"
#include "qgswebframe.h"
#include "qgsexpressionfinder.h"
#if WITH_QTWEBKIT
#include <QWebElement>
#endif

#include <QDesktopServices>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QColorDialog>
#include <QMenu>
#include <QUrl>
#include <QRegularExpressionValidator>


QgsVectorLayerProperties::QgsVectorLayerProperties(
  QgsMapCanvas *canvas,
  QgsMessageBar *messageBar,
  QgsVectorLayer *lyr,
  QWidget *parent,
  Qt::WindowFlags fl
)
  : QgsLayerPropertiesDialog( lyr, canvas, QStringLiteral( "VectorLayerProperties" ), parent, fl )
  , mMessageBar( messageBar )
  , mLayer( lyr )
  , mOriginalSubsetSQL( lyr->subsetString() )
{
  setupUi( this );
  connect( pbnQueryBuilder, &QPushButton::clicked, this, &QgsVectorLayerProperties::pbnQueryBuilder_clicked );
  connect( pbnIndex, &QPushButton::clicked, this, &QgsVectorLayerProperties::pbnIndex_clicked );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsVectorLayerProperties::mCrsSelector_crsChanged );
  connect( pbnUpdateExtents, &QPushButton::clicked, this, &QgsVectorLayerProperties::pbnUpdateExtents_clicked );
  connect( mButtonAddJoin, &QPushButton::clicked, this, &QgsVectorLayerProperties::mButtonAddJoin_clicked );
  connect( mButtonEditJoin, &QPushButton::clicked, this, &QgsVectorLayerProperties::mButtonEditJoin_clicked );
  connect( mJoinTreeWidget, &QTreeWidget::itemDoubleClicked, this, &QgsVectorLayerProperties::mJoinTreeWidget_itemDoubleClicked );
  connect( mButtonRemoveJoin, &QPushButton::clicked, this, &QgsVectorLayerProperties::mButtonRemoveJoin_clicked );
  connect( mButtonAddWmsDimension, &QPushButton::clicked, this, &QgsVectorLayerProperties::mButtonAddWmsDimension_clicked );
  connect( mButtonEditWmsDimension, &QPushButton::clicked, this, &QgsVectorLayerProperties::mButtonEditWmsDimension_clicked );
  connect( mWmsDimensionsTreeWidget, &QTreeWidget::itemDoubleClicked, this, &QgsVectorLayerProperties::mWmsDimensionsTreeWidget_itemDoubleClicked );
  connect( mButtonRemoveWmsDimension, &QPushButton::clicked, this, &QgsVectorLayerProperties::mButtonRemoveWmsDimension_clicked );
  connect( mSimplifyDrawingGroupBox, &QGroupBox::toggled, this, &QgsVectorLayerProperties::mSimplifyDrawingGroupBox_toggled );
  connect( buttonRemoveMetadataUrl, &QPushButton::clicked, this, &QgsVectorLayerProperties::removeSelectedMetadataUrl );
  connect( buttonAddMetadataUrl, &QPushButton::clicked, this, &QgsVectorLayerProperties::addMetadataUrl );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorLayerProperties::showHelp );

  mProjectDirtyBlocker = std::make_unique<QgsProjectDirtyBlocker>( QgsProject::instance() );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  mBtnStyle = new QPushButton( tr( "Style" ), this );
  QMenu *menuStyle = new QMenu( this );
  mActionLoadStyle = new QAction( tr( "Load Style…" ), this );
  connect( mActionLoadStyle, &QAction::triggered, this, &QgsVectorLayerProperties::loadStyle );

  mActionSaveStyle = new QAction( tr( "Save Current Style…" ), this );
  connect( mActionSaveStyle, &QAction::triggered, this, &QgsVectorLayerProperties::saveStyleAs );

  mActionSaveMultipleStyles = new QAction( tr( "Save Multiple Styles…" ), this );
  connect( mActionSaveMultipleStyles, &QAction::triggered, this, &QgsVectorLayerProperties::saveMultipleStylesAs );

  mSourceGroupBox->hide();

  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsVectorLayerProperties::aboutToShowStyleMenu );
  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  connect( lyr->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsVectorLayerProperties::syncToLayer );

  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsVectorLayerProperties::apply );
  connect( this, &QDialog::accepted, this, &QgsVectorLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsVectorLayerProperties::rollback );

  mContext << QgsExpressionContextUtils::globalScope()
           << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
           << QgsExpressionContextUtils::atlasScope( nullptr )
           << QgsExpressionContextUtils::mapSettingsScope( mCanvas->mapSettings() )
           << QgsExpressionContextUtils::layerScope( mLayer );

  mMapTipFieldComboBox->setLayer( lyr );
  mDisplayExpressionWidget->setLayer( lyr );
  mDisplayExpressionWidget->registerExpressionContextGenerator( this );
  initMapTipPreview();

  connect( mMapTipInsertFieldButton, &QAbstractButton::clicked, this, &QgsVectorLayerProperties::insertField );
  connect( mMapTipInsertExpressionButton, &QAbstractButton::clicked, this, &QgsVectorLayerProperties::insertOrEditExpression );

  if ( !mLayer )
    return;

  connect( mEnableMapTips, &QAbstractButton::toggled, mHtmlMapTipGroupBox, &QWidget::setEnabled );
  mEnableMapTips->setChecked( mLayer->mapTipsEnabled() );

  QVBoxLayout *layout = nullptr;

  if ( mLayer->isSpatial() )
  {
    // Create the Labeling dialog tab
    layout = new QVBoxLayout( labelingFrame );
    layout->setContentsMargins( 0, 0, 0, 0 );
    labelingDialog = new QgsLabelingWidget( mLayer, mCanvas, labelingFrame );
    labelingDialog->layout()->setContentsMargins( 0, 0, 0, 0 );
    connect( labelingDialog, &QgsLabelingWidget::auxiliaryFieldCreated, this, [ = ] { updateAuxiliaryStoragePage(); } );
    layout->addWidget( labelingDialog );
    labelingFrame->setLayout( layout );

    // Create the masking dialog tab
    layout = new QVBoxLayout( mMaskingFrame );
    layout->setContentsMargins( 0, 0, 0, 0 );
    mMaskingWidget = new QgsMaskingWidget( mMaskingFrame );
    mMaskingWidget->setLayer( mLayer );
    mMaskingWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
    layout->addWidget( mMaskingWidget );
    mMaskingFrame->setLayout( layout );
  }
  else
  {
    labelingDialog = nullptr;
    mOptsPage_Labels->setEnabled( false ); // disable labeling item
    mOptsPage_Masks->setEnabled( false ); // disable masking item
    mGeomGroupBox->setEnabled( false );
    mGeomGroupBox->setVisible( false );
    mCrsGroupBox->setEnabled( false );
    mCrsGroupBox->setVisible( false );
  }

  // Create the Actions dialog tab
  QVBoxLayout *actionLayout = new QVBoxLayout( actionOptionsFrame );
  actionLayout->setContentsMargins( 0, 0, 0, 0 );
  mActionDialog = new QgsAttributeActionDialog( *mLayer->actions(), actionOptionsFrame );
  mActionDialog->layout()->setContentsMargins( 0, 0, 0, 0 );
  actionLayout->addWidget( mActionDialog );

  mSourceFieldsPropertiesDialog = new QgsSourceFieldsProperties( mLayer, mSourceFieldsFrame );
  mSourceFieldsPropertiesDialog->layout()->setContentsMargins( 0, 0, 0, 0 );
  mSourceFieldsFrame->setLayout( new QVBoxLayout( mSourceFieldsFrame ) );
  mSourceFieldsFrame->layout()->setContentsMargins( 0, 0, 0, 0 );
  mSourceFieldsFrame->layout()->addWidget( mSourceFieldsPropertiesDialog );

  connect( mSourceFieldsPropertiesDialog, &QgsSourceFieldsProperties::toggleEditing, this, static_cast<void ( QgsVectorLayerProperties::* )()>( &QgsVectorLayerProperties::toggleEditing ) );

  mAttributesFormPropertiesDialog = new QgsAttributesFormProperties( mLayer, mAttributesFormFrame );
  mAttributesFormPropertiesDialog->layout()->setContentsMargins( 0, 0, 0, 0 );
  mAttributesFormFrame->setLayout( new QVBoxLayout( mAttributesFormFrame ) );
  mAttributesFormFrame->layout()->setContentsMargins( 0, 0, 0, 0 );
  mAttributesFormFrame->layout()->addWidget( mAttributesFormPropertiesDialog );

  // Metadata tab, before the syncToLayer
  QVBoxLayout *metadataLayout = new QVBoxLayout( metadataFrame );
  metadataLayout->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget = new QgsMetadataWidget( this, mLayer );
  mMetadataWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget->setMapCanvas( mCanvas );
  metadataLayout->addWidget( mMetadataWidget );
  metadataFrame->setLayout( metadataLayout );

  QVBoxLayout *temporalLayout = new QVBoxLayout( temporalFrame );
  temporalLayout->setContentsMargins( 0, 0, 0, 0 );
  mTemporalWidget = new QgsVectorLayerTemporalPropertiesWidget( this, mLayer );
  temporalLayout->addWidget( mTemporalWidget );

  setMetadataWidget( mMetadataWidget, mOptsPage_Metadata );

  mBtnMetadata = new QPushButton( tr( "Metadata" ), this );
  QMenu *menuMetadata = new QMenu( this );
  mActionLoadMetadata = menuMetadata->addAction( tr( "Load Metadata from File…" ), this, &QgsVectorLayerProperties::loadMetadataFromFile );
  mActionSaveMetadataAs = menuMetadata->addAction( tr( "Save Metadata to File…" ), this, &QgsVectorLayerProperties::saveMetadataToFile );
  menuMetadata->addSeparator();
  menuMetadata->addAction( tr( "Save to Default Location" ), this, &QgsVectorLayerProperties::saveMetadataAsDefault );
  menuMetadata->addAction( tr( "Restore from Default Location" ), this, &QgsVectorLayerProperties::loadDefaultMetadata );
  mBtnMetadata->setMenu( menuMetadata );
  buttonBox->addButton( mBtnMetadata, QDialogButtonBox::ResetRole );

  mSelectionColorButton->setAllowOpacity( true );
  mSelectionColorButton->setColorDialogTitle( tr( "Override Selection Color" ) );
  if ( mCanvas )
  {
    mSelectionColorButton->setColor( mCanvas->selectionColor() );
    mSelectionColorButton->setDefaultColor( mCanvas->selectionColor() );
  }
  connect( mRadioOverrideSelectionColor, &QRadioButton::toggled, mSelectionColorButton, &QWidget::setEnabled );
  mSelectionColorButton->setEnabled( false );
  connect( mRadioOverrideSelectionSymbol, &QRadioButton::toggled, mSelectionSymbolButton, &QWidget::setEnabled );
  switch ( mLayer->geometryType() )
  {

    case Qgis::GeometryType::Point:
      mSelectionSymbolButton->setSymbolType( Qgis::SymbolType::Marker );
      break;
    case Qgis::GeometryType::Line:
      mSelectionSymbolButton->setSymbolType( Qgis::SymbolType::Line );
      break;
    case Qgis::GeometryType::Polygon:
      mSelectionSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
      break;

    case Qgis::GeometryType::Unknown:
    case Qgis::GeometryType::Null:
      break;
  }
  mSelectionSymbolButton->setEnabled( false );
  mRadioDefaultSelectionColor->setChecked( true );

  syncToLayer();

  if ( mLayer->dataProvider() )
  {
    //enable spatial index button group if supported by provider, or if one already exists
    QgsVectorDataProvider::Capabilities capabilities = mLayer->dataProvider()->capabilities();
    if ( !( capabilities & QgsVectorDataProvider::CreateSpatialIndex ) )
    {
      pbnIndex->setEnabled( false );
    }
    if ( mLayer->dataProvider()->hasSpatialIndex() == Qgis::SpatialIndexPresence::Present )
    {
      pbnIndex->setEnabled( false );
      pbnIndex->setText( tr( "Spatial Index Exists" ) );
    }

    if ( capabilities & QgsVectorDataProvider::SelectEncoding )
    {
      cboProviderEncoding->addItems( QgsVectorDataProvider::availableEncodings() );
      QString enc = mLayer->dataProvider()->encoding();
      int encindex = cboProviderEncoding->findText( enc );
      if ( encindex < 0 )
      {
        cboProviderEncoding->insertItem( 0, enc );
        encindex = 0;
      }
      cboProviderEncoding->setCurrentIndex( encindex );
    }
    else if ( mLayer->providerType() == QLatin1String( "ogr" ) )
    {
      // if OGR_L_TestCapability(OLCStringsAsUTF8) returns true, OGR provider encoding can be set to only UTF-8
      // so make encoding box grayed out
      cboProviderEncoding->addItem( mLayer->dataProvider()->encoding() );
      cboProviderEncoding->setEnabled( false );
    }
    else
    {
      // other providers do not use mEncoding, so hide the group completely
      mDataSourceEncodingFrame->hide();
    }
  }

  mCrsSelector->setCrs( mLayer->crs() );

  //insert existing join info
  const QList< QgsVectorLayerJoinInfo > &joins = mLayer->vectorJoins();
  for ( const QgsVectorLayerJoinInfo &join : joins )
  {
    addJoinToTreeWidget( join );
  }

  mOldJoins = mLayer->vectorJoins();

  QVBoxLayout *diagLayout = new QVBoxLayout( mDiagramFrame );
  diagLayout->setContentsMargins( 0, 0, 0, 0 );
  diagramPropertiesDialog = new QgsDiagramProperties( mLayer, mDiagramFrame, mCanvas );
  diagramPropertiesDialog->layout()->setContentsMargins( 0, 0, 0, 0 );
  connect( diagramPropertiesDialog, &QgsDiagramProperties::auxiliaryFieldCreated, this, [ = ] { updateAuxiliaryStoragePage(); } );
  diagLayout->addWidget( diagramPropertiesDialog );
  mDiagramFrame->setLayout( diagLayout );

  // Legend tab
  mLegendWidget->setMapCanvas( mCanvas );
  mLegendWidget->setLayer( mLayer );
  mLegendConfigEmbeddedWidget->setLayer( mLayer );

  // WMS Name as layer short name
  mLayerShortNameLineEdit->setText( mLayer->shortName() );
  // WMS Name validator
  QValidator *shortNameValidator = new QRegularExpressionValidator( QgsApplication::shortNameRegularExpression(), this );
  mLayerShortNameLineEdit->setValidator( shortNameValidator );

  //layer title and abstract
  mLayerTitleLineEdit->setText( mLayer->title() );
  mLayerAbstractTextEdit->setPlainText( mLayer->abstract() );
  mLayerKeywordListLineEdit->setText( mLayer->keywordList() );
  mLayerDataUrlLineEdit->setText( mLayer->dataUrl() );
  mLayerDataUrlFormatComboBox->setCurrentIndex(
    mLayerDataUrlFormatComboBox->findText(
      mLayer->dataUrlFormat()
    )
  );
  //layer attribution
  mLayerAttributionLineEdit->setText( mLayer->attribution() );
  mLayerAttributionUrlLineEdit->setText( mLayer->attributionUrl() );

  // Setup the layer metadata URL
  tableViewMetadataUrl->setSelectionMode( QAbstractItemView::SingleSelection );
  tableViewMetadataUrl->setSelectionBehavior( QAbstractItemView::SelectRows );
  tableViewMetadataUrl->horizontalHeader()->setStretchLastSection( true );
  tableViewMetadataUrl->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

  mMetadataUrlModel = new QStandardItemModel( tableViewMetadataUrl );
  mMetadataUrlModel->clear();
  mMetadataUrlModel->setColumnCount( 3 );
  QStringList metadataUrlHeaders;
  metadataUrlHeaders << tr( "URL" ) << tr( "Type" ) << tr( "Format" );
  mMetadataUrlModel->setHorizontalHeaderLabels( metadataUrlHeaders );
  tableViewMetadataUrl->setModel( mMetadataUrlModel );
  tableViewMetadataUrl->setItemDelegate( new MetadataUrlItemDelegate( this ) );

  const QList<QgsMapLayerServerProperties::MetadataUrl> &metaUrls = mLayer->serverProperties()->metadataUrls();
  for ( const QgsMapLayerServerProperties::MetadataUrl &metaUrl : metaUrls )
  {
    const int row = mMetadataUrlModel->rowCount();
    mMetadataUrlModel->setItem( row, 0, new QStandardItem( metaUrl.url ) );
    mMetadataUrlModel->setItem( row, 1, new QStandardItem( metaUrl.type ) );
    mMetadataUrlModel->setItem( row, 2, new QStandardItem( metaUrl.format ) );
  }

  // layer legend url
  mLayerLegendUrlLineEdit->setText( mLayer->legendUrl() );
  mLayerLegendUrlFormatComboBox->setCurrentIndex(
    mLayerLegendUrlFormatComboBox->findText(
      mLayer->legendUrlFormat()
    )
  );

  //insert existing dimension info
  QgsMapLayerServerProperties *serverProperties = static_cast<QgsMapLayerServerProperties *>( mLayer->serverProperties() );
  const QList<QgsMapLayerServerProperties::WmsDimensionInfo> &wmsDims = serverProperties->wmsDimensions();
  for ( const QgsMapLayerServerProperties::WmsDimensionInfo &dim : wmsDims )
  {
    addWmsDimensionInfoToTreeWidget( dim );
  }

  QString myStyle = QgsApplication::reportStyleSheet();
  myStyle.append( QStringLiteral( "body { margin: 10px; }\n " ) );
  teMetadataViewer->clear();
  teMetadataViewer->document()->setDefaultStyleSheet( myStyle );
  teMetadataViewer->setHtml( htmlMetadata() );
  teMetadataViewer->setOpenLinks( false );
  connect( teMetadataViewer, &QTextBrowser::anchorClicked, this, &QgsVectorLayerProperties::openUrl );
  mMetadataFilled = true;

  QgsSettings settings;
  // if dialog hasn't been opened/closed yet, default to Styles tab, which is used most often
  // this will be read by restoreOptionsBaseUi()
  if ( !settings.contains( QStringLiteral( "/Windows/VectorLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/VectorLayerProperties/tab" ),
                       mOptStackedWidget->indexOf( mOptsPage_Style ) );
  }

  QList<QgsMapLayer *> dependencySources;
  const QSet<QgsMapLayerDependency> constDependencies = mLayer->dependencies();
  for ( const QgsMapLayerDependency &dep : constDependencies )
  {
    QgsMapLayer *layer = QgsProject::instance()->mapLayer( dep.layerId() );
    if ( layer )
      dependencySources << layer;
  }

  mLayersDependenciesTreeModel = new QgsLayerTreeFilterProxyModel( this );
  mLayersDependenciesTreeModel->setLayerTreeModel( new QgsLayerTreeModel( QgsProject::instance()->layerTreeRoot(), mLayersDependenciesTreeModel ) );
  mLayersDependenciesTreeModel->setCheckedLayers( dependencySources );
  connect( QgsProject::instance(), &QObject::destroyed, this, [ = ] {mLayersDependenciesTreeView->setModel( nullptr );} );
  mLayersDependenciesTreeView->setModel( mLayersDependenciesTreeModel );

  mRefreshSettingsWidget->setLayer( mLayer );

  // auxiliary layer
  QMenu *menu = new QMenu( this );

  mAuxiliaryLayerActionNew = new QAction( tr( "Create" ), this );
  menu->addAction( mAuxiliaryLayerActionNew );
  connect( mAuxiliaryLayerActionNew, &QAction::triggered, this, &QgsVectorLayerProperties::onAuxiliaryLayerNew );

  mAuxiliaryLayerActionClear = new QAction( tr( "Clear" ), this );
  menu->addAction( mAuxiliaryLayerActionClear );
  connect( mAuxiliaryLayerActionClear, &QAction::triggered, this, &QgsVectorLayerProperties::onAuxiliaryLayerClear );

  mAuxiliaryLayerActionDelete = new QAction( tr( "Delete" ), this );
  menu->addAction( mAuxiliaryLayerActionDelete );
  connect( mAuxiliaryLayerActionDelete, &QAction::triggered, this, &QgsVectorLayerProperties::onAuxiliaryLayerDelete );

  mAuxiliaryLayerActionExport = new QAction( tr( "Export" ), this );
  menu->addAction( mAuxiliaryLayerActionExport );
  connect( mAuxiliaryLayerActionExport, &QAction::triggered, this, [ = ] { emit exportAuxiliaryLayer( mLayer->auxiliaryLayer() ); } );

  mAuxiliaryStorageActions->setMenu( menu );

  connect( mAuxiliaryStorageFieldsDeleteBtn, &QPushButton::clicked, this, &QgsVectorLayerProperties::onAuxiliaryLayerDeleteField );
  connect( mAuxiliaryStorageFieldsAddBtn, &QPushButton::clicked, this, &QgsVectorLayerProperties::onAuxiliaryLayerAddField );

  updateAuxiliaryStoragePage();

  mOptsPage_Information->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#information-properties" ) );
  mOptsPage_Source->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#source-properties" ) );
  mOptsPage_Style->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#symbology-properties" ) );
  mOptsPage_Labels->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#labels-properties" ) );
  mOptsPage_Masks->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#masks-properties" ) );
  mOptsPage_Diagrams->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#diagrams-properties" ) );
  mOptsPage_SourceFields->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#fields-properties" ) );
  mOptsPage_AttributesForm->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#attributes-form-properties" ) );
  mOptsPage_Joins->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#joins-properties" ) );
  mOptsPage_AuxiliaryStorage->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#auxiliary-storage-properties" ) );
  mOptsPage_Actions->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#actions-properties" ) );
  mOptsPage_Display->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#display-properties" ) );
  mOptsPage_Rendering->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#rendering-properties" ) );
  mOptsPage_Temporal->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#temporal-properties" ) );
  mOptsPage_Variables->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#variables-properties" ) );
  mOptsPage_Metadata->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#metadata-properties" ) );
  mOptsPage_DataDependencies->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#dependencies-properties" ) ) ;
  mOptsPage_Legend->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#legend-properties" ) );
  mOptsPage_Server->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#qgis-server-properties" ) );


  optionsStackedWidget_CurrentChanged( mOptStackedWidget->currentIndex() );

  initialize();
}

void QgsVectorLayerProperties::toggleEditing()
{
  if ( !mLayer )
    return;

  emit toggleEditing( mLayer );

  setPbnQueryBuilderEnabled();
}

void QgsVectorLayerProperties::insertField()
{
  // Convert the selected field to an expression and
  // insert it into the action at the cursor position
  if ( mMapTipFieldComboBox->currentField().isEmpty() )
    return;
  QString expression = QStringLiteral( "[%\"" );
  expression += mMapTipFieldComboBox->currentField();
  expression += QLatin1String( "\"%]" );

  mMapTipWidget->insertText( expression );
}

void QgsVectorLayerProperties::insertOrEditExpression()
{
  // Get the linear indexes if the start and end of the selection
  int selectionStart = mMapTipWidget->selectionStart();
  int selectionEnd = mMapTipWidget->selectionEnd();
  QString expression = QgsExpressionFinder::findAndSelectActiveExpression( mMapTipWidget );

  QgsExpressionContext context = createExpressionContext();
  QgsExpressionBuilderDialog exprDlg( mLayer, expression, this, QStringLiteral( "generic" ), context );

  exprDlg.setWindowTitle( tr( "Insert Expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted && !exprDlg.expressionText().trimmed().isEmpty() )
    mMapTipWidget->insertText( "[%" + exprDlg.expressionText().trimmed() + "%]" );
  else // Restore the selection
    mMapTipWidget->setLinearSelection( selectionStart, selectionEnd );
}

void QgsVectorLayerProperties::addMetadataUrl()
{
  const int row = mMetadataUrlModel->rowCount();
  mMetadataUrlModel->setItem( row, 0, new QStandardItem( QLatin1String() ) );
  mMetadataUrlModel->setItem( row, 1, new QStandardItem( QLatin1String() ) );
  mMetadataUrlModel->setItem( row, 2, new QStandardItem( QLatin1String() ) );
}

void QgsVectorLayerProperties::removeSelectedMetadataUrl()
{
  const QModelIndexList selectedRows = tableViewMetadataUrl->selectionModel()->selectedRows();
  if ( selectedRows.empty() )
    return;
  mMetadataUrlModel->removeRow( selectedRows[0].row() );
}

void QgsVectorLayerProperties::syncToLayer()
{
  if ( !mSourceWidget )
  {
    mSourceWidget = QgsGui::sourceWidgetProviderRegistry()->createWidget( mLayer );
    if ( mSourceWidget )
    {
      QHBoxLayout *layout = new QHBoxLayout();
      layout->addWidget( mSourceWidget );
      mSourceGroupBox->setLayout( layout );
      if ( !mSourceWidget->groupTitle().isEmpty() )
        mSourceGroupBox->setTitle( mSourceWidget->groupTitle() );

      mSourceGroupBox->show();

      connect( mSourceWidget, &QgsProviderSourceWidget::validChanged, this, [ = ]( bool isValid )
      {
        buttonBox->button( QDialogButtonBox::Apply )->setEnabled( isValid );
        buttonBox->button( QDialogButtonBox::Ok )->setEnabled( isValid );
      } );
    }
  }

  if ( mSourceWidget )
  {
    mSourceWidget->setMapCanvas( mCanvas );
    mSourceWidget->setSourceUri( mLayer->source() );
  }

  // populate the general information
  mLayerOrigNameLineEdit->setText( mLayer->name() );
  mBackupCrs = mLayer->crs();

  //see if we are dealing with a pg layer here
  mSubsetGroupBox->setEnabled( true );
  txtSubsetSQL->setText( mLayer->subsetString() );
  // if the user is allowed to type an adhoc query, the app will crash if the query
  // is bad. For this reason, the sql box is disabled and the query must be built
  // using the query builder, either by typing it in by hand or using the buttons, etc
  // on the builder. If the ability to enter a query directly into the box is required,
  // a mechanism to check it must be implemented.
  txtSubsetSQL->setReadOnly( true );
  txtSubsetSQL->setCaretWidth( 0 );
  txtSubsetSQL->setCaretLineVisible( false );
  setPbnQueryBuilderEnabled();
  if ( mLayer->dataProvider() && !mLayer->dataProvider()->supportsSubsetString() )
  {
    // hide subset box entirely if not supported by data provider
    mSubsetGroupBox->hide();
  }

  mDisplayExpressionWidget->setField( mLayer->displayExpression() );
  mEnableMapTips->setChecked( mLayer->mapTipsEnabled() );
  mMapTipWidget->setText( mLayer->mapTipTemplate() );

  // set up the scale based layer visibility stuff....
  mScaleRangeWidget->setScaleRange( mLayer->minimumScale(), mLayer->maximumScale() );
  mScaleVisibilityGroupBox->setChecked( mLayer->hasScaleBasedVisibility() );
  mScaleRangeWidget->setMapCanvas( mCanvas );

  mUseReferenceScaleGroupBox->setChecked( mLayer->renderer() && mLayer->renderer()->referenceScale() > 0 );
  mReferenceScaleWidget->setShowCurrentScaleButton( true );
  mReferenceScaleWidget->setMapCanvas( mCanvas );
  if ( mUseReferenceScaleGroupBox->isChecked() )
    mReferenceScaleWidget->setScale( mLayer->renderer()->referenceScale() );
  else if ( mCanvas )
    mReferenceScaleWidget->setScale( mCanvas->scale() );

  // get simplify drawing configuration
  const QgsVectorSimplifyMethod &simplifyMethod = mLayer->simplifyMethod();
  mSimplifyDrawingGroupBox->setChecked( simplifyMethod.simplifyHints() != QgsVectorSimplifyMethod::NoSimplification );
  mSimplifyDrawingSpinBox->setValue( simplifyMethod.threshold() );
  mSimplifyDrawingSpinBox->setClearValue( 1.0 );

  QgsVectorLayerSelectionProperties *selectionProperties = qobject_cast< QgsVectorLayerSelectionProperties *>( mLayer->selectionProperties() );
  if ( selectionProperties->selectionColor().isValid() )
  {
    mSelectionColorButton->setColor( selectionProperties->selectionColor() );
  }
  if ( QgsSymbol *symbol = selectionProperties->selectionSymbol() )
  {
    mSelectionSymbolButton->setSymbol( symbol->clone() );
  }
  switch ( selectionProperties->selectionRenderingMode() )
  {
    case Qgis::SelectionRenderingMode::Default:
      mRadioDefaultSelectionColor->setChecked( true );
      break;

    case Qgis::SelectionRenderingMode::CustomColor:
    {
      if ( selectionProperties->selectionColor().isValid() )
      {
        mRadioOverrideSelectionColor->setChecked( true );
      }
      else
      {
        mRadioDefaultSelectionColor->setChecked( true );
      }
      break;
    }

    case Qgis::SelectionRenderingMode::CustomSymbol:
      if ( selectionProperties->selectionSymbol() )
      {
        mRadioOverrideSelectionSymbol->setChecked( true );
      }
      else
      {
        mRadioDefaultSelectionColor->setChecked( true );
      }
      break;
  }

  QString remark = QStringLiteral( " (%1)" ).arg( tr( "Not supported" ) );
  const QgsVectorDataProvider *provider = mLayer->dataProvider();
  if ( !( provider && ( provider->capabilities() & QgsVectorDataProvider::SimplifyGeometries ) ) )
  {
    mSimplifyDrawingAtProvider->setChecked( false );
    mSimplifyDrawingAtProvider->setEnabled( false );
    if ( !mSimplifyDrawingAtProvider->text().endsWith( remark ) )
      mSimplifyDrawingAtProvider->setText( mSimplifyDrawingAtProvider->text().append( remark ) );
  }
  else
  {
    mSimplifyDrawingAtProvider->setChecked( !simplifyMethod.forceLocalOptimization() );
    mSimplifyDrawingAtProvider->setEnabled( mSimplifyDrawingGroupBox->isChecked() );
    if ( mSimplifyDrawingAtProvider->text().endsWith( remark ) )
    {
      QString newText = mSimplifyDrawingAtProvider->text();
      newText.chop( remark.size() );
      mSimplifyDrawingAtProvider->setText( newText );
    }
  }

  // disable simplification for point layers, now it is not implemented
  if ( mLayer->geometryType() == Qgis::GeometryType::Point )
  {
    mSimplifyDrawingGroupBox->setChecked( false );
    mSimplifyDrawingGroupBox->setEnabled( false );
  }

  // Default local simplification algorithm
  mSimplifyAlgorithmComboBox->addItem( tr( "Distance" ), QgsVectorSimplifyMethod::Distance );
  mSimplifyAlgorithmComboBox->addItem( tr( "SnapToGrid" ), QgsVectorSimplifyMethod::SnapToGrid );
  mSimplifyAlgorithmComboBox->addItem( tr( "Visvalingam" ), QgsVectorSimplifyMethod::Visvalingam );
  mSimplifyAlgorithmComboBox->setCurrentIndex( mSimplifyAlgorithmComboBox->findData( simplifyMethod.simplifyAlgorithm() ) );

  QStringList myScalesList = Qgis::defaultProjectScales().split( ',' );
  myScalesList.append( QStringLiteral( "1:1" ) );
  mSimplifyMaximumScaleComboBox->updateScales( myScalesList );
  mSimplifyMaximumScaleComboBox->setScale( simplifyMethod.maximumScale() );

  mForceRasterCheckBox->setChecked( mLayer->renderer() && mLayer->renderer()->forceRasterRender() );

  mRefreshSettingsWidget->syncToLayer();

  mRefreshLayerNotificationCheckBox->setChecked( mLayer->isRefreshOnNotifyEnabled() );
  mNotificationMessageCheckBox->setChecked( !mLayer->refreshOnNotifyMessage().isEmpty() );
  mNotifyMessagValueLineEdit->setText( mLayer->refreshOnNotifyMessage() );


  // load appropriate symbology page (V1 or V2)
  updateSymbologyPage();

  mActionDialog->init( *mLayer->actions(), mLayer->attributeTableConfig() );

  if ( labelingDialog )
    labelingDialog->adaptToLayer();

  mSourceFieldsPropertiesDialog->init();
  mAttributesFormPropertiesDialog->init();

  // set initial state for variable editor
  updateVariableEditor();

  if ( diagramPropertiesDialog )
    diagramPropertiesDialog->syncToLayer();

  // sync all plugin dialogs
  for ( QgsMapLayerConfigWidget *page : std::as_const( mConfigWidgets ) )
  {
    page->syncToLayer( mLayer );
  }

  mMetadataWidget->setMetadata( &mLayer->metadata() );

  mTemporalWidget->syncToLayer();

  mLegendWidget->setLayer( mLayer );

}

void QgsVectorLayerProperties::apply()
{
  if ( labelingDialog )
  {
    labelingDialog->writeSettingsToLayer();
  }
  mBackupCrs = mLayer->crs();
  // apply legend settings
  mLegendWidget->applyToLayer();
  mLegendConfigEmbeddedWidget->applyToLayer();

  // save metadata
  mMetadataWidget->acceptMetadata();
  mMetadataFilled = false;

  // save masking settings
  if ( mMaskingWidget && mMaskingWidget->hasBeenPopulated() )
    mMaskingWidget->apply();

  // set up the scale based layer visibility stuff....
  mLayer->setScaleBasedVisibility( mScaleVisibilityGroupBox->isChecked() );
  mLayer->setMaximumScale( mScaleRangeWidget->maximumScale() );
  mLayer->setMinimumScale( mScaleRangeWidget->minimumScale() );

  // provider-specific options
  if ( mLayer->dataProvider() )
  {
    if ( mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::SelectEncoding )
    {
      mLayer->setProviderEncoding( cboProviderEncoding->currentText() );
    }
  }

  mLayer->setDisplayExpression( mDisplayExpressionWidget->asExpression() );
  mLayer->setMapTipsEnabled( mEnableMapTips->isChecked() );
  mLayer->setMapTipTemplate( mMapTipWidget->text() );

  mLayer->actions()->clearActions();
  const auto constActions = mActionDialog->actions();
  for ( const QgsAction &action : constActions )
  {
    mLayer->actions()->addAction( action );
  }
  QgsAttributeTableConfig attributeTableConfig = mLayer->attributeTableConfig();
  attributeTableConfig.update( mLayer->fields() );
  attributeTableConfig.setActionWidgetStyle( mActionDialog->attributeTableWidgetStyle() );
  QVector<QgsAttributeTableConfig::ColumnConfig> columns = attributeTableConfig.columns();

  for ( int i = 0; i < columns.size(); ++i )
  {
    if ( columns.at( i ).type == QgsAttributeTableConfig::Action )
    {
      columns[i].hidden = !mActionDialog->showWidgetInAttributeTable();
    }
  }

  attributeTableConfig.setColumns( columns );

  mLayer->setAttributeTableConfig( attributeTableConfig );

  mLayer->setName( mLayerOrigNameLineEdit->text() );

  mAttributesFormPropertiesDialog->apply();
  mSourceFieldsPropertiesDialog->apply();

  // Update temporal properties
  mTemporalWidget->saveTemporalProperties();

  if ( mLayer->renderer() )
  {
    QgsRendererPropertiesDialog *dlg = static_cast<QgsRendererPropertiesDialog *>( widgetStackRenderers->currentWidget() );
    dlg->apply();
  }

  //apply diagram settings
  diagramPropertiesDialog->apply();

  // apply all plugin dialogs
  for ( QgsMapLayerConfigWidget *page : std::as_const( mConfigWidgets ) )
  {
    page->apply();
  }

  //layer title and abstract
  if ( mLayer->shortName() != mLayerShortNameLineEdit->text() )
    mMetadataFilled = false;
  mLayer->setShortName( mLayerShortNameLineEdit->text() );

  if ( mLayer->title() != mLayerTitleLineEdit->text() )
    mMetadataFilled = false;
  mLayer->setTitle( mLayerTitleLineEdit->text() );

  if ( mLayer->abstract() != mLayerAbstractTextEdit->toPlainText() )
    mMetadataFilled = false;
  mLayer->setAbstract( mLayerAbstractTextEdit->toPlainText() );

  if ( mLayer->keywordList() != mLayerKeywordListLineEdit->text() )
    mMetadataFilled = false;
  mLayer->setKeywordList( mLayerKeywordListLineEdit->text() );

  if ( mLayer->dataUrl() != mLayerDataUrlLineEdit->text() )
    mMetadataFilled = false;
  mLayer->setDataUrl( mLayerDataUrlLineEdit->text() );

  if ( mLayer->dataUrlFormat() != mLayerDataUrlFormatComboBox->currentText() )
    mMetadataFilled = false;
  mLayer->setDataUrlFormat( mLayerDataUrlFormatComboBox->currentText() );

  //layer attribution
  if ( mLayer->attribution() != mLayerAttributionLineEdit->text() )
    mMetadataFilled = false;
  mLayer->setAttribution( mLayerAttributionLineEdit->text() );

  if ( mLayer->attributionUrl() != mLayerAttributionUrlLineEdit->text() )
    mMetadataFilled = false;
  mLayer->setAttributionUrl( mLayerAttributionUrlLineEdit->text() );

  // Metadata URL
  QList<QgsMapLayerServerProperties::MetadataUrl> metaUrls;
  for ( int row = 0; row < mMetadataUrlModel->rowCount() ; row++ )
  {
    QgsMapLayerServerProperties::MetadataUrl metaUrl;
    metaUrl.url = mMetadataUrlModel->item( row, 0 )->text();
    metaUrl.type = mMetadataUrlModel->item( row, 1 )->text();
    metaUrl.format = mMetadataUrlModel->item( row, 2 )->text();
    metaUrls.append( metaUrl );
    mMetadataFilled = false;
  }
  mLayer->serverProperties()->setMetadataUrls( metaUrls );

  // LegendURL
  if ( mLayer->legendUrl() != mLayerLegendUrlLineEdit->text() )
    mMetadataFilled = false;
  mLayer->setLegendUrl( mLayerLegendUrlLineEdit->text() );

  if ( mLayer->legendUrlFormat() != mLayerLegendUrlFormatComboBox->currentText() )
    mMetadataFilled = false;
  mLayer->setLegendUrlFormat( mLayerLegendUrlFormatComboBox->currentText() );

  //layer simplify drawing configuration
  QgsVectorSimplifyMethod::SimplifyHints simplifyHints = QgsVectorSimplifyMethod::NoSimplification;
  if ( mSimplifyDrawingGroupBox->isChecked() )
  {
    simplifyHints |= QgsVectorSimplifyMethod::GeometrySimplification;
    if ( mSimplifyDrawingSpinBox->value() > 1 ) simplifyHints |= QgsVectorSimplifyMethod::AntialiasingSimplification;
  }
  QgsVectorSimplifyMethod simplifyMethod = mLayer->simplifyMethod();
  simplifyMethod.setSimplifyHints( simplifyHints );
  simplifyMethod.setSimplifyAlgorithm( static_cast< QgsVectorSimplifyMethod::SimplifyAlgorithm >( mSimplifyAlgorithmComboBox->currentData().toInt() ) );
  simplifyMethod.setThreshold( mSimplifyDrawingSpinBox->value() );
  simplifyMethod.setForceLocalOptimization( !mSimplifyDrawingAtProvider->isChecked() );
  simplifyMethod.setMaximumScale( mSimplifyMaximumScaleComboBox->scale() );
  mLayer->setSimplifyMethod( simplifyMethod );

  if ( mLayer->renderer() )
  {
    mLayer->renderer()->setForceRasterRender( mForceRasterCheckBox->isChecked() );
    mLayer->renderer()->setReferenceScale( mUseReferenceScaleGroupBox->isChecked() ? mReferenceScaleWidget->scale() : -1 );
  }

  QgsVectorLayerSelectionProperties *selectionProperties = qobject_cast< QgsVectorLayerSelectionProperties *>( mLayer->selectionProperties() );
  if ( mSelectionColorButton->color() != mSelectionColorButton->defaultColor() )
    selectionProperties->setSelectionColor( mSelectionColorButton->color() );
  else
    selectionProperties->setSelectionColor( QColor() );
  if ( QgsSymbol *symbol = mSelectionSymbolButton->symbol() )
    selectionProperties->setSelectionSymbol( symbol->clone() );

  if ( mRadioOverrideSelectionSymbol->isChecked() )
  {
    selectionProperties->setSelectionRenderingMode( Qgis::SelectionRenderingMode::CustomSymbol );
  }
  else if ( mRadioOverrideSelectionColor->isChecked() )
  {
    selectionProperties->setSelectionRenderingMode( Qgis::SelectionRenderingMode::CustomColor );
  }
  else
  {
    selectionProperties->setSelectionRenderingMode( Qgis::SelectionRenderingMode::Default );
  }

  mRefreshSettingsWidget->saveToLayer();

  mLayer->setRefreshOnNotifyEnabled( mRefreshLayerNotificationCheckBox->isChecked() );
  mLayer->setRefreshOnNofifyMessage( mNotificationMessageCheckBox->isChecked() ? mNotifyMessagValueLineEdit->text() : QString() );

  mOldJoins = mLayer->vectorJoins();

  //save variables
  QgsExpressionContextUtils::setLayerVariables( mLayer, mVariableEditor->variablesInActiveScope() );
  updateVariableEditor();

  // save dependencies
  QSet<QgsMapLayerDependency> deps;
  const auto checkedLayers = mLayersDependenciesTreeModel->checkedLayers();
  for ( const QgsMapLayer *layer : checkedLayers )
    deps << QgsMapLayerDependency( layer->id() );
  if ( ! mLayer->setDependencies( deps ) )
  {
    QMessageBox::warning( nullptr, tr( "Save Dependency" ), tr( "This configuration introduces a cycle in data dependencies and will be ignored." ) );
  }

  // Why is this here? Well, we if we're making changes to the layer's source then potentially
  // we are changing the geometry type of the layer, or even going from spatial <-> non spatial types.
  // So we need to ensure that anything from the dialog which sets things like renderer properties
  // happens BEFORE we change the source, otherwise we might end up with a renderer which is not
  // compatible with the new geometry type of the layer. (And likewise for other properties like
  // fields!)
  bool dialogNeedsResync = false;
  if ( mSourceWidget )
  {
    const QString newSource = mSourceWidget->sourceUri();
    if ( newSource != mLayer->source() )
    {
      mLayer->setDataSource( newSource, mLayer->name(), mLayer->providerType(),
                             QgsDataProvider::ProviderOptions(), QgsDataProvider::ReadFlags() );

      // resync dialog to layer's new state -- this allows any changed layer properties
      // (such as a forced creation of a new renderer compatible with the new layer, new field configuration, etc)
      // to show in the dialog correctly
      dialogNeedsResync = true;
    }
  }
  // now apply the subset string AFTER setting the layer's source. It's messy, but the subset string
  // can form part of the layer's source, but it WON'T be present in the URI returned by the source widget!
  // If we don't apply the subset string AFTER changing the source, then the subset string will be lost.
  mSubsetGroupBox->setEnabled( true );
  if ( txtSubsetSQL->text() != mLayer->subsetString() )
  {
    // set the subset sql for the layer
    mLayer->setSubsetString( txtSubsetSQL->text() );
    mMetadataFilled = false;
    // need to resync the dialog, the subset string may have changed the layer's geometry type!
    dialogNeedsResync = true;
  }
  mOriginalSubsetSQL = mLayer->subsetString();

  if ( dialogNeedsResync )
    syncToLayer();

  mLayer->triggerRepaint();
  // notify the project we've made a change
  mProjectDirtyBlocker.reset();
  QgsProject::instance()->setDirty( true );
  mProjectDirtyBlocker = std::make_unique<QgsProjectDirtyBlocker>( QgsProject::instance() );

}

void QgsVectorLayerProperties::rollback()
{
  if ( mOldJoins != mLayer->vectorJoins() )
  {
    // need to undo changes in vector layer joins - they are applied directly to the layer (not in apply())
    // so other parts of the properties dialog can use the fields from the joined layers

    const auto constVectorJoins = mLayer->vectorJoins();
    for ( const QgsVectorLayerJoinInfo &info : constVectorJoins )
      mLayer->removeJoin( info.joinLayerId() );

    for ( const QgsVectorLayerJoinInfo &info : std::as_const( mOldJoins ) )
      mLayer->addJoin( info );
  }

  if ( mOriginalSubsetSQL != mLayer->subsetString() )
  {
    // need to undo changes in subset string - they are applied directly to the layer (not in apply())
    // by QgsQueryBuilder::accept()

    mLayer->setSubsetString( mOriginalSubsetSQL );
  }

  // Store it because QgsLayerPropertiesDialog::rollback() calls syncToLayer() which
  // resets the backupCrs
  const QgsCoordinateReferenceSystem backupCrs { mBackupCrs };

  QgsLayerPropertiesDialog::rollback();

  if ( backupCrs != mLayer->crs() )
    mLayer->setCrs( backupCrs );

}

void QgsVectorLayerProperties::pbnQueryBuilder_clicked()
{
  // launch the query builder
  QgsSubsetStringEditorInterface *dialog = QgsGui::subsetStringEditorProviderRegistry()->createDialog( mLayer, this );

  // Set the sql in the query builder to the same in the prop dialog
  // (in case the user has already changed it)
  dialog->setSubsetString( txtSubsetSQL->text() );
  // Open the query builder
  if ( dialog->exec() )
  {
    // if the sql is changed, update it in the prop subset text box
    txtSubsetSQL->setText( dialog->subsetString() );
    //TODO If the sql is changed in the prop dialog, the layer extent should be recalculated

    // The datasource for the layer needs to be updated with the new sql since this gets
    // saved to the project file. This should happen at the map layer level...

  }
  // delete the query builder object
  delete dialog;
}

void QgsVectorLayerProperties::pbnIndex_clicked()
{
  QgsVectorDataProvider *pr = mLayer->dataProvider();
  if ( pr )
  {
    setCursor( Qt::WaitCursor );
    bool errval = pr->createSpatialIndex();
    setCursor( Qt::ArrowCursor );
    if ( errval )
    {
      pbnIndex->setEnabled( false );
      pbnIndex->setText( tr( "Spatial Index Exists" ) );
      QMessageBox::information( this, tr( "Spatial Index" ), tr( "Creation of spatial index successful" ) );
    }
    else
    {
      QMessageBox::warning( this, tr( "Spatial Index" ), tr( "Creation of spatial index failed" ) );
    }
  }
}

QString QgsVectorLayerProperties::htmlMetadata()
{
  return mLayer->htmlMetadata();
}

void QgsVectorLayerProperties::mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs )
{

  QgsDatumTransformDialog::run( crs, QgsProject::instance()->crs(), this, mCanvas, tr( "Select Transformation for the vector layer" ) );
  mLayer->setCrs( crs );
  mMetadataFilled = false;
  mMetadataWidget->crsChanged();
}

void QgsVectorLayerProperties::saveMultipleStylesAs()
{
  QgsMapLayerSaveStyleDialog dlg( mLayer );
  dlg.setSaveOnlyCurrentStyle( false );
  QgsSettings settings;

  if ( dlg.exec() )
  {
    apply();

    // Store the original style, that we can restore at the end
    const QString originalStyle { mLayer->styleManager()->currentStyle() };
    const QListWidget *stylesWidget { dlg.stylesWidget() };

    // Collect selected (checked) styles for export/save
    QStringList stylesSelected;
    for ( int i = 0; i < stylesWidget->count(); i++ )
    {
      if ( stylesWidget->item( i )->checkState() == Qt::CheckState::Checked )
      {
        stylesSelected.push_back( stylesWidget->item( i )->text() );
      }
    }

    if ( ! stylesSelected.isEmpty() )
    {
      int styleIndex = 0;
      for ( const QString &styleName : std::as_const( stylesSelected ) )
      {
        bool defaultLoadedFlag = false;

        StyleType type = dlg.currentStyleType();
        mLayer->styleManager()->setCurrentStyle( styleName );
        switch ( type )
        {
          case QML:
          case SLD:
          {
            QString message;
            const QString filePath { dlg.outputFilePath() };
            const QFileInfo fi { filePath };
            QString safePath { QString( filePath ).replace( fi.baseName(),
                               QStringLiteral( "%1_%2" ).arg( fi.baseName(), QgsFileUtils::stringToSafeFilename( styleName ) ) ) };
            if ( styleIndex > 0 && stylesSelected.count( ) > 1 )
            {
              int i = 1;
              while ( QFile::exists( safePath ) )
              {
                const QFileInfo fi { safePath };
                safePath = QString( safePath ).replace( '.' + fi.completeSuffix(),
                                                        QStringLiteral( "_%1.%2" ).arg( QString::number( i ), fi.completeSuffix() ) );
                i++;
              }
            }
            if ( type == QML )
              message = mLayer->saveNamedStyle( safePath, defaultLoadedFlag, dlg.styleCategories() );
            else
              message = mLayer->saveSldStyle( safePath, defaultLoadedFlag );

            //reset if the default style was loaded OK only
            if ( defaultLoadedFlag )
            {
              syncToLayer();
            }
            else
            {
              //let the user know what went wrong
              QMessageBox::information( this, tr( "Save Style" ), message );
            }

            break;
          }
          case DatasourceDatabase:
          {
            QString infoWindowTitle = QObject::tr( "Save style '%1' to DB (%2)" )
                                      .arg( styleName, mLayer->providerType() );
            QString msgError;

            QgsMapLayerSaveStyleDialog::SaveToDbSettings dbSettings = dlg.saveToDbSettings();

            // If a name is defined, we add _1 etc. else we use the style name
            QString name { dbSettings.name };
            if ( name.isEmpty() )
            {
              name = styleName;
            }
            else
            {
              name += QStringLiteral( "_%1" ).arg( styleName );
              QStringList ids, names, descriptions;
              mLayer->listStylesInDatabase( ids, names, descriptions, msgError );
              int i = 1;
              while ( names.contains( name ) )
              {
                name = QStringLiteral( "%1 %2" ).arg( name, QString::number( i ) );
                i++;
              }
            }

            QString errorMessage;
            if ( QgsProviderRegistry::instance()->styleExists( mLayer->providerType(), mLayer->source(), dbSettings.name, errorMessage ) )
            {
              if ( QMessageBox::question( nullptr, QObject::tr( "Save style in database" ),
                                          QObject::tr( "A matching style already exists in the database for this layer. Do you want to overwrite it?" ),
                                          QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
              {
                return;
              }
            }
            else if ( !errorMessage.isEmpty() )
            {
              QMessageBox::warning( this, infoWindowTitle, errorMessage );
              return;
            }

            mLayer->saveStyleToDatabase( name, dbSettings.description, dbSettings.isDefault, dbSettings.uiFileContent, msgError, dlg.styleCategories() );

            if ( !msgError.isNull() )
            {
              QMessageBox::warning( this, infoWindowTitle, msgError );
            }
            else
            {
              QMessageBox::information( this, infoWindowTitle, tr( "Style '%1' saved" ).arg( styleName ) );
            }
            break;
          }
          case UserDatabase:
            break;
        }
        styleIndex ++;
      }
      // Restore original style
      mLayer->styleManager()->setCurrentStyle( originalStyle );
    }
  } // Nothing selected!
}

void QgsVectorLayerProperties::aboutToShowStyleMenu()
{
  // this should be unified with QgsRasterLayerProperties::aboutToShowStyleMenu()
  QMenu *m = qobject_cast<QMenu *>( sender() );
  m->clear();

  m->addAction( mActionLoadStyle );
  m->addAction( mActionSaveStyle );

  // If we have multiple styles, offer an option to save them at once
  if ( mLayer->styleManager()->styles().count() > 1 )
  {
    mActionSaveStyle->setText( tr( "Save Current Style…" ) );
    m->addAction( mActionSaveMultipleStyles );
  }
  else
  {
    mActionSaveStyle->setText( tr( "Save Style…" ) );
  }

  m->addSeparator();
  m->addAction( tr( "Save as Default" ), this, &QgsVectorLayerProperties::saveDefaultStyle );
  m->addAction( tr( "Restore Default" ), this, &QgsVectorLayerProperties::loadDefaultStyle );

  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mLayer );
}

void QgsVectorLayerProperties::mButtonAddJoin_clicked()
{
  if ( !mLayer )
    return;

  QList<QgsMapLayer *> joinedLayers;
  const QList< QgsVectorLayerJoinInfo > &joins = mLayer->vectorJoins();
  joinedLayers.reserve( joins.size() );
  for ( int i = 0; i < joins.size(); ++i )
  {
    joinedLayers.append( joins[i].joinLayer() );
  }

  QgsJoinDialog d( mLayer, joinedLayers );
  if ( d.exec() == QDialog::Accepted )
  {
    QgsVectorLayerJoinInfo info = d.joinInfo();
    //create attribute index if possible
    if ( d.createAttributeIndex() )
    {
      QgsVectorLayer *joinLayer = info.joinLayer();
      if ( joinLayer )
      {
        joinLayer->dataProvider()->createAttributeIndex( joinLayer->fields().indexFromName( info.joinFieldName() ) );
      }
    }
    mLayer->addJoin( info );
    addJoinToTreeWidget( info );
    setPbnQueryBuilderEnabled();
    mSourceFieldsPropertiesDialog->init();
    mAttributesFormPropertiesDialog->init();
  }
}

void QgsVectorLayerProperties::mButtonEditJoin_clicked()
{
  QTreeWidgetItem *currentJoinItem = mJoinTreeWidget->currentItem();
  mJoinTreeWidget_itemDoubleClicked( currentJoinItem, 0 );
}

void QgsVectorLayerProperties::mJoinTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int )
{
  if ( !mLayer || !item )
  {
    return;
  }

  QList<QgsMapLayer *> joinedLayers;
  QString joinLayerId = item->data( 0, Qt::UserRole ).toString();
  const QList< QgsVectorLayerJoinInfo > &joins = mLayer->vectorJoins();
  int j = -1;
  for ( int i = 0; i < joins.size(); ++i )
  {
    QgsVectorLayer *joinLayer = joins[i].joinLayer();
    if ( !joinLayer )
      continue;  // invalid join (unresolved join layer)

    if ( joinLayer->id() == joinLayerId )
    {
      j = i;
    }
    else
    {
      // remove already joined layers from possible list to be displayed in dialog
      joinedLayers.append( joinLayer );
    }
  }
  if ( j == -1 )
  {
    return;
  }

  QgsJoinDialog d( mLayer, joinedLayers );
  d.setWindowTitle( tr( "Edit Vector Join" ) );
  d.setJoinInfo( joins[j] );

  if ( d.exec() == QDialog::Accepted )
  {
    QgsVectorLayerJoinInfo info = d.joinInfo();

    // remove old join
    mLayer->removeJoin( joinLayerId );
    int idx = mJoinTreeWidget->indexOfTopLevelItem( item );
    mJoinTreeWidget->takeTopLevelItem( idx );

    // add the new edited

    //create attribute index if possible
    if ( d.createAttributeIndex() )
    {
      QgsVectorLayer *joinLayer = info.joinLayer();
      if ( joinLayer )
      {
        joinLayer->dataProvider()->createAttributeIndex( joinLayer->fields().indexFromName( info.joinFieldName() ) );
      }
    }
    mLayer->addJoin( info );
    addJoinToTreeWidget( info, idx );

    setPbnQueryBuilderEnabled();
    mSourceFieldsPropertiesDialog->init();
    mAttributesFormPropertiesDialog->init();
  }
}

void QgsVectorLayerProperties::addJoinToTreeWidget( const QgsVectorLayerJoinInfo &join, const int insertIndex )
{
  QTreeWidgetItem *joinItem = new QTreeWidgetItem();
  joinItem->setFlags( Qt::ItemIsEnabled );

  QgsVectorLayer *joinLayer = join.joinLayer();
  if ( !mLayer || !joinLayer )
  {
    return;
  }

  joinItem->setText( 0, tr( "Join layer" ) );
  if ( mLayer->auxiliaryLayer() && mLayer->auxiliaryLayer()->id() == join.joinLayerId() )
  {
    return;
  }

  joinItem->setText( 1, joinLayer->name() );

  QFont f = joinItem->font( 0 );
  f.setBold( true );
  joinItem->setFont( 0, f );
  joinItem->setFont( 1, f );

  joinItem->setData( 0, Qt::UserRole, join.joinLayerId() );

  QTreeWidgetItem *childJoinField = new QTreeWidgetItem();
  childJoinField->setText( 0, tr( "Join field" ) );
  childJoinField->setText( 1, join.joinFieldName() );
  childJoinField->setFlags( Qt::ItemIsEnabled );
  joinItem->addChild( childJoinField );

  QTreeWidgetItem *childTargetField = new QTreeWidgetItem();
  childTargetField->setText( 0, tr( "Target field" ) );
  childTargetField->setText( 1, join.targetFieldName() );
  joinItem->addChild( childTargetField );

  QTreeWidgetItem *childMemCache = new QTreeWidgetItem();
  childMemCache->setText( 0, tr( "Cache join layer in virtual memory" ) );
  if ( join.isUsingMemoryCache() )
    childMemCache->setText( 1, QChar( 0x2714 ) );
  joinItem->addChild( childMemCache );

  QTreeWidgetItem *childDynForm = new QTreeWidgetItem();
  childDynForm->setText( 0, tr( "Dynamic form" ) );
  if ( join.isDynamicFormEnabled() )
    childDynForm->setText( 1, QChar( 0x2714 ) );
  joinItem->addChild( childDynForm );

  QTreeWidgetItem *childEditable = new QTreeWidgetItem();
  childEditable->setText( 0, tr( "Editable join layer" ) );
  if ( join.isEditable() )
    childEditable->setText( 1, QChar( 0x2714 ) );
  joinItem->addChild( childEditable );

  QTreeWidgetItem *childUpsert = new QTreeWidgetItem();
  childUpsert->setText( 0, tr( "Upsert on edit" ) );
  if ( join.hasUpsertOnEdit() )
    childUpsert->setText( 1, QChar( 0x2714 ) );
  joinItem->addChild( childUpsert );

  QTreeWidgetItem *childCascade = new QTreeWidgetItem();
  childCascade->setText( 0, tr( "Delete cascade" ) );
  if ( join.hasCascadedDelete() )
    childCascade->setText( 1, QChar( 0x2714 ) );
  joinItem->addChild( childCascade );

  QTreeWidgetItem *childPrefix = new QTreeWidgetItem();
  childPrefix->setText( 0, tr( "Custom field name prefix" ) );
  childPrefix->setText( 1, join.prefix() );
  joinItem->addChild( childPrefix );

  QTreeWidgetItem *childFields = new QTreeWidgetItem();
  childFields->setText( 0, tr( "Joined fields" ) );
  const QStringList *list = join.joinFieldNamesSubset();
  if ( list )
    childFields->setText( 1, QLocale().toString( list->count() ) );
  else
    childFields->setText( 1, tr( "all" ) );
  joinItem->addChild( childFields );

  if ( insertIndex >= 0 )
    mJoinTreeWidget->insertTopLevelItem( insertIndex, joinItem );
  else
    mJoinTreeWidget->addTopLevelItem( joinItem );

  mJoinTreeWidget->setCurrentItem( joinItem );
  mJoinTreeWidget->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
}

QgsExpressionContext QgsVectorLayerProperties::createExpressionContext() const
{
  return mContext;
}

void QgsVectorLayerProperties::openPanel( QgsPanelWidget *panel )
{
  QDialog *dlg = new QDialog();
  QString key = QStringLiteral( "/UI/paneldialog/%1" ).arg( panel->panelTitle() );
  QgsSettings settings;
  dlg->restoreGeometry( settings.value( key ).toByteArray() );
  dlg->setWindowTitle( panel->panelTitle() );
  dlg->setLayout( new QVBoxLayout() );
  dlg->layout()->addWidget( panel );
  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok );
  connect( buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept );
  dlg->layout()->addWidget( buttonBox );
  dlg->exec();
  settings.setValue( key, dlg->saveGeometry() );
  panel->acceptPanel();
}

void QgsVectorLayerProperties::mButtonRemoveJoin_clicked()
{
  QTreeWidgetItem *currentJoinItem = mJoinTreeWidget->currentItem();
  if ( !mLayer || !currentJoinItem )
  {
    return;
  }

  mLayer->removeJoin( currentJoinItem->data( 0, Qt::UserRole ).toString() );
  mJoinTreeWidget->takeTopLevelItem( mJoinTreeWidget->indexOfTopLevelItem( currentJoinItem ) );
  setPbnQueryBuilderEnabled();
  mSourceFieldsPropertiesDialog->init();
  mAttributesFormPropertiesDialog->init();
}


void QgsVectorLayerProperties::mButtonAddWmsDimension_clicked()
{
  if ( !mLayer )
    return;

  // get wms dimensions name
  QStringList alreadyDefinedDimensions;
  QgsMapLayerServerProperties *serverProperties = static_cast<QgsMapLayerServerProperties *>( mLayer->serverProperties() );
  const QList<QgsMapLayerServerProperties::WmsDimensionInfo> &dims = serverProperties->wmsDimensions();
  for ( const QgsMapLayerServerProperties::WmsDimensionInfo &dim : dims )
  {
    alreadyDefinedDimensions << dim.name;
  }

  QgsWmsDimensionDialog d( mLayer, alreadyDefinedDimensions );
  if ( d.exec() == QDialog::Accepted )
  {
    QgsMapLayerServerProperties::WmsDimensionInfo info = d.info();
    // save dimension
    serverProperties->addWmsDimension( info );
    addWmsDimensionInfoToTreeWidget( info );
  }
}

void QgsVectorLayerProperties::mButtonEditWmsDimension_clicked()
{
  QTreeWidgetItem *currentWmsDimensionItem = mWmsDimensionsTreeWidget->currentItem();
  mWmsDimensionsTreeWidget_itemDoubleClicked( currentWmsDimensionItem, 0 );
}

void QgsVectorLayerProperties::mWmsDimensionsTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int )
{
  if ( !mLayer || !item )
  {
    return;
  }

  QString wmsDimName = item->data( 0, Qt::UserRole ).toString();
  QgsMapLayerServerProperties *serverProperties = static_cast<QgsMapLayerServerProperties *>( mLayer->serverProperties() );
  const QList<QgsMapLayerServerProperties::WmsDimensionInfo> &dims = serverProperties->wmsDimensions();
  QStringList alreadyDefinedDimensions;
  int j = -1;
  for ( int i = 0; i < dims.size(); ++i )
  {
    QString dimName = dims[i].name;
    if ( dimName == wmsDimName )
    {
      j = i;
    }
    else
    {
      alreadyDefinedDimensions << dimName;
    }
  }
  if ( j == -1 )
  {
    return;
  }

  QgsWmsDimensionDialog d( mLayer, alreadyDefinedDimensions );
  d.setWindowTitle( tr( "Edit WMS Dimension" ) );
  d.setInfo( dims[j] );

  if ( d.exec() == QDialog::Accepted )
  {
    QgsMapLayerServerProperties::WmsDimensionInfo info = d.info();

    // remove old
    QgsMapLayerServerProperties *serverProperties = static_cast<QgsMapLayerServerProperties *>( mLayer->serverProperties() );
    serverProperties->removeWmsDimension( wmsDimName );
    int idx = mWmsDimensionsTreeWidget->indexOfTopLevelItem( item );
    mWmsDimensionsTreeWidget->takeTopLevelItem( idx );

    // save new
    serverProperties->addWmsDimension( info );
    addWmsDimensionInfoToTreeWidget( info, idx );
  }
}

void QgsVectorLayerProperties::addWmsDimensionInfoToTreeWidget( const QgsMapLayerServerProperties::WmsDimensionInfo &wmsDim, const int insertIndex )
{
  QTreeWidgetItem *wmsDimensionItem = new QTreeWidgetItem();
  wmsDimensionItem->setFlags( Qt::ItemIsEnabled );

  wmsDimensionItem->setText( 0, tr( "Dimension" ) );
  wmsDimensionItem->setText( 1, wmsDim.name );

  QFont f = wmsDimensionItem->font( 0 );
  f.setBold( true );
  wmsDimensionItem->setFont( 0, f );
  wmsDimensionItem->setFont( 1, f );

  wmsDimensionItem->setData( 0, Qt::UserRole, wmsDim.name );

  QTreeWidgetItem *childWmsDimensionField = new QTreeWidgetItem();
  childWmsDimensionField->setText( 0, tr( "Field" ) );
  childWmsDimensionField->setText( 1, wmsDim.fieldName );
  childWmsDimensionField->setFlags( Qt::ItemIsEnabled );
  wmsDimensionItem->addChild( childWmsDimensionField );

  QTreeWidgetItem *childWmsDimensionEndField = new QTreeWidgetItem();
  childWmsDimensionEndField->setText( 0, tr( "End field" ) );
  childWmsDimensionEndField->setText( 1, wmsDim.endFieldName );
  childWmsDimensionEndField->setFlags( Qt::ItemIsEnabled );
  wmsDimensionItem->addChild( childWmsDimensionEndField );

  QTreeWidgetItem *childWmsDimensionUnits = new QTreeWidgetItem();
  childWmsDimensionUnits->setText( 0, tr( "Units" ) );
  childWmsDimensionUnits->setText( 1, wmsDim.units );
  childWmsDimensionUnits->setFlags( Qt::ItemIsEnabled );
  wmsDimensionItem->addChild( childWmsDimensionUnits );

  QTreeWidgetItem *childWmsDimensionUnitSymbol = new QTreeWidgetItem();
  childWmsDimensionUnitSymbol->setText( 0, tr( "Unit symbol" ) );
  childWmsDimensionUnitSymbol->setText( 1, wmsDim.unitSymbol );
  childWmsDimensionUnitSymbol->setFlags( Qt::ItemIsEnabled );
  wmsDimensionItem->addChild( childWmsDimensionUnitSymbol );

  QTreeWidgetItem *childWmsDimensionDefaultValue = new QTreeWidgetItem();
  childWmsDimensionDefaultValue->setText( 0, tr( "Default display" ) );
  childWmsDimensionDefaultValue->setText( 1, QgsMapLayerServerProperties::wmsDimensionDefaultDisplayLabels().value( wmsDim.defaultDisplayType ) );
  childWmsDimensionDefaultValue->setFlags( Qt::ItemIsEnabled );
  wmsDimensionItem->addChild( childWmsDimensionDefaultValue );

  QTreeWidgetItem *childWmsDimensionRefValue = new QTreeWidgetItem();
  childWmsDimensionRefValue->setText( 0, tr( "Reference value" ) );
  childWmsDimensionRefValue->setText( 1, wmsDim.referenceValue.toString() );
  childWmsDimensionRefValue->setFlags( Qt::ItemIsEnabled );
  wmsDimensionItem->addChild( childWmsDimensionRefValue );

  if ( insertIndex >= 0 )
    mWmsDimensionsTreeWidget->insertTopLevelItem( insertIndex, wmsDimensionItem );
  else
    mWmsDimensionsTreeWidget->addTopLevelItem( wmsDimensionItem );

  mWmsDimensionsTreeWidget->setCurrentItem( wmsDimensionItem );
  mWmsDimensionsTreeWidget->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
}

void QgsVectorLayerProperties::mButtonRemoveWmsDimension_clicked()
{
  QTreeWidgetItem *currentWmsDimensionItem = mWmsDimensionsTreeWidget->currentItem();
  if ( !mLayer || !currentWmsDimensionItem )
  {
    return;
  }

  QgsMapLayerServerProperties *serverProperties = static_cast<QgsMapLayerServerProperties *>( mLayer->serverProperties() );
  serverProperties->removeWmsDimension( currentWmsDimensionItem->data( 0, Qt::UserRole ).toString() );
  mWmsDimensionsTreeWidget->takeTopLevelItem( mWmsDimensionsTreeWidget->indexOfTopLevelItem( currentWmsDimensionItem ) );
}


void QgsVectorLayerProperties::updateSymbologyPage()
{

  //find out the type of renderer in the vectorlayer, create a dialog with these settings and add it to the form
  delete mRendererDialog;
  mRendererDialog = nullptr;

  if ( mLayer->renderer() )
  {
    mRendererDialog = new QgsRendererPropertiesDialog( mLayer, QgsStyle::defaultStyle(), true, this );
    mRendererDialog->setDockMode( false );
    QgsSymbolWidgetContext context;
    context.setMapCanvas( mCanvas );
    context.setMessageBar( mMessageBar );
    mRendererDialog->setContext( context );
    connect( mRendererDialog, &QgsRendererPropertiesDialog::showPanel, this, &QgsVectorLayerProperties::openPanel );
    connect( mRendererDialog, &QgsRendererPropertiesDialog::layerVariablesChanged, this, &QgsVectorLayerProperties::updateVariableEditor );
    connect( mRendererDialog, &QgsRendererPropertiesDialog::widgetChanged, this,  [ = ] { updateAuxiliaryStoragePage(); } );
  }
  else
  {
    mOptsPage_Style->setEnabled( false ); // hide symbology item
  }

  if ( mRendererDialog )
  {
    mRendererDialog->layout()->setContentsMargins( 0, 0, 0, 0 );
    widgetStackRenderers->addWidget( mRendererDialog );
    widgetStackRenderers->setCurrentWidget( mRendererDialog );
    widgetStackRenderers->currentWidget()->layout()->setContentsMargins( 0, 0, 0, 0 );
  }
}

void QgsVectorLayerProperties::setPbnQueryBuilderEnabled()
{
  pbnQueryBuilder->setEnabled( mLayer &&
                               mLayer->dataProvider() &&
                               mLayer->dataProvider()->supportsSubsetString() &&
                               !mLayer->isEditable() );

  if ( mLayer && mLayer->isEditable() )
  {
    pbnQueryBuilder->setToolTip( tr( "Stop editing mode to enable this." ) );
  }

}

void QgsVectorLayerProperties::pbnUpdateExtents_clicked()
{
  mLayer->updateExtents( true ); // force update whatever options activated
  mMetadataFilled = false;
}

void QgsVectorLayerProperties::optionsStackedWidget_CurrentChanged( int index )
{
  QgsLayerPropertiesDialog::optionsStackedWidget_CurrentChanged( index );

  if ( index == mOptStackedWidget->indexOf( mOptsPage_Information ) && ! mMetadataFilled )
  {
    // set the metadata contents (which can be expensive)
    teMetadataViewer->clear();
    teMetadataViewer->setHtml( htmlMetadata() );
    mMetadataFilled = true;
  }
  else if ( index == mOptStackedWidget->indexOf( mOptsPage_SourceFields ) || index == mOptStackedWidget->indexOf( mOptsPage_Joins ) )
  {
    // store any edited attribute form field configuration to prevent loss of edits when adding/removing fields and/or joins
    mAttributesFormPropertiesDialog->store();
  }

  resizeAlltabs( index );
}

void QgsVectorLayerProperties::mSimplifyDrawingGroupBox_toggled( bool checked )
{
  const QgsVectorDataProvider *provider = mLayer->dataProvider();
  if ( !( provider && ( provider->capabilities() & QgsVectorDataProvider::SimplifyGeometries ) != 0 ) )
  {
    mSimplifyDrawingAtProvider->setEnabled( false );
  }
  else
  {
    mSimplifyDrawingAtProvider->setEnabled( checked );
  }
}

void QgsVectorLayerProperties::updateVariableEditor()
{
  QgsExpressionContext context;
  mVariableEditor->setContext( &context );
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::globalScope() );
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::layerScope( mLayer ) );
  mVariableEditor->reloadContext();
  mVariableEditor->setEditableScopeIndex( 2 );
}

void QgsVectorLayerProperties::showHelp()
{
  const QVariant helpPage = mOptionsStackedWidget->currentWidget()->property( "helpPage" );

  if ( helpPage.isValid() )
  {
    QgsHelp::openHelp( helpPage.toString() );
  }
  else
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html" ) );
  }
}

void QgsVectorLayerProperties::updateAuxiliaryStoragePage()
{
  const QgsAuxiliaryLayer *alayer = mLayer->auxiliaryLayer();

  if ( alayer )
  {
    // set widgets to enable state
    mAuxiliaryStorageInformationGrpBox->setEnabled( true );
    mAuxiliaryStorageFieldsGrpBox->setEnabled( true );

    // update key
    mAuxiliaryStorageKeyLineEdit->setText( alayer->joinInfo().targetFieldName() );

    // update feature count
    const qlonglong features = alayer->featureCount();
    mAuxiliaryStorageFeaturesLineEdit->setText( QLocale().toString( features ) );

    // update actions
    mAuxiliaryLayerActionClear->setEnabled( true );
    mAuxiliaryLayerActionDelete->setEnabled( true );
    mAuxiliaryLayerActionExport->setEnabled( true );
    mAuxiliaryLayerActionNew->setEnabled( false );

    const QgsAuxiliaryLayer *alayer = mLayer->auxiliaryLayer();
    if ( alayer )
    {
      const int fields = alayer->auxiliaryFields().count();
      mAuxiliaryStorageFieldsLineEdit->setText( QLocale().toString( fields ) );

      // add fields
      mAuxiliaryStorageFieldsTree->clear();
      for ( const QgsField &field : alayer->auxiliaryFields() )
      {
        const QgsPropertyDefinition prop = QgsAuxiliaryLayer::propertyDefinitionFromField( field );
        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setText( 0, prop.origin() );
        item->setText( 1, prop.name() );
        item->setText( 2, prop.comment() );
        item->setText( 3, field.typeName() );
        item->setText( 4, field.name() );

        mAuxiliaryStorageFieldsTree->addTopLevelItem( item );
      }
    }
  }
  else
  {
    mAuxiliaryStorageInformationGrpBox->setEnabled( false );
    mAuxiliaryStorageFieldsGrpBox->setEnabled( false );

    mAuxiliaryLayerActionClear->setEnabled( false );
    mAuxiliaryLayerActionDelete->setEnabled( false );
    mAuxiliaryLayerActionExport->setEnabled( false );
    mAuxiliaryLayerActionNew->setEnabled( true );

    mAuxiliaryStorageFieldsTree->clear();
    mAuxiliaryStorageKeyLineEdit->setText( QString() );
    mAuxiliaryStorageFieldsLineEdit->setText( QString() );
    mAuxiliaryStorageFeaturesLineEdit->setText( QString() );
  }
}

void QgsVectorLayerProperties::onAuxiliaryLayerNew()
{
  QgsAuxiliaryLayer *alayer = mLayer->auxiliaryLayer();

  if ( alayer )
    return;

  QgsNewAuxiliaryLayerDialog dlg( mLayer, this );
  if ( dlg.exec() == QDialog::Accepted )
  {
    updateAuxiliaryStoragePage();
  }
}

void QgsVectorLayerProperties::onAuxiliaryLayerClear()
{
  QgsAuxiliaryLayer *alayer = mLayer->auxiliaryLayer();

  if ( !alayer )
    return;

  const QString msg = tr( "Are you sure you want to clear auxiliary data for %1?" ).arg( mLayer->name() );
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question( this, "Clear Auxiliary Data", msg, QMessageBox::Yes | QMessageBox::No );

  if ( reply == QMessageBox::Yes )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    alayer->clear();
    QApplication::restoreOverrideCursor();
    updateAuxiliaryStoragePage();
    mLayer->triggerRepaint();
  }
}

void QgsVectorLayerProperties::onAuxiliaryLayerDelete()
{
  QgsAuxiliaryLayer *alayer = mLayer->auxiliaryLayer();
  if ( !alayer )
    return;

  const QString msg = tr( "Are you sure you want to delete auxiliary storage for %1?" ).arg( mLayer->name() );
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question( this, "Delete Auxiliary Storage", msg, QMessageBox::Yes | QMessageBox::No );

  if ( reply == QMessageBox::Yes )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    QgsDataSourceUri uri( alayer->source() );

    // delete each attribute to correctly update layer settings and data
    // defined buttons
    while ( alayer->auxiliaryFields().size() > 0 )
    {
      QgsField aField = alayer->auxiliaryFields()[0];
      deleteAuxiliaryField( alayer->fields().indexOf( aField.name() ) );
    }

    mLayer->setAuxiliaryLayer(); // remove auxiliary layer
    QgsAuxiliaryStorage::deleteTable( uri );
    QApplication::restoreOverrideCursor();
    updateAuxiliaryStoragePage();
    mLayer->triggerRepaint();
  }
}

void QgsVectorLayerProperties::onAuxiliaryLayerDeleteField()
{
  QgsAuxiliaryLayer *alayer = mLayer->auxiliaryLayer();
  if ( !alayer )
    return;

  QList<QTreeWidgetItem *> items = mAuxiliaryStorageFieldsTree->selectedItems();
  if ( items.count() < 1 )
    return;

  // get auxiliary field name and index from item
  const QTreeWidgetItem *item = items[0];
  QgsPropertyDefinition def;
  def.setOrigin( item->text( 0 ) );
  def.setName( item->text( 1 ) );
  def.setComment( item->text( 2 ) );

  const QString fieldName = QgsAuxiliaryLayer::nameFromProperty( def );

  const int index = mLayer->auxiliaryLayer()->fields().indexOf( fieldName );
  if ( index < 0 )
    return;

  // should be only 1 field
  const QString msg = tr( "Are you sure you want to delete auxiliary field %1 for %2?" ).arg( item->text( 1 ), item->text( 0 ) );

  QMessageBox::StandardButton reply;
  const QString title = QObject::tr( "Delete Auxiliary Field" );
  reply = QMessageBox::question( this, title, msg, QMessageBox::Yes | QMessageBox::No );

  if ( reply == QMessageBox::Yes )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    deleteAuxiliaryField( index );
    mLayer->triggerRepaint();
    QApplication::restoreOverrideCursor();
  }
}

void QgsVectorLayerProperties::onAuxiliaryLayerAddField()
{
  QgsAuxiliaryLayer *alayer = mLayer->auxiliaryLayer();
  if ( !alayer )
    return;

  QgsNewAuxiliaryFieldDialog dlg( QgsPropertyDefinition(), mLayer, false );
  if ( dlg.exec() == QDialog::Accepted )
  {
    updateAuxiliaryStoragePage();
  }
}

void QgsVectorLayerProperties::deleteAuxiliaryField( int index )
{
  if ( !mLayer->auxiliaryLayer() )
    return;

  int key = mLayer->auxiliaryLayer()->propertyFromIndex( index );
  QgsPropertyDefinition def = mLayer->auxiliaryLayer()->propertyDefinitionFromIndex( index );

  if ( mLayer->auxiliaryLayer()->deleteAttribute( index ) )
  {
    mLayer->updateFields();

    // immediately deactivate data defined button
    if ( key >= 0 && def.origin().compare( "labeling", Qt::CaseInsensitive ) == 0
         && labelingDialog
         && labelingDialog->labelingGui() )
    {
      labelingDialog->labelingGui()->deactivateField( static_cast<QgsPalLayerSettings::Property>( key ) );
    }

    updateAuxiliaryStoragePage();
    mSourceFieldsPropertiesDialog->init();
  }
  else
  {
    const QString title = QObject::tr( "Delete Auxiliary Field" );
    const QString errors = mLayer->auxiliaryLayer()->commitErrors().join( QLatin1String( "\n  " ) );
    const QString msg = QObject::tr( "Unable to remove auxiliary field (%1)" ).arg( errors );
    mMessageBar->pushMessage( title, msg, Qgis::MessageLevel::Warning );
  }
}

bool QgsVectorLayerProperties::eventFilter( QObject *obj, QEvent *ev )
{
  // If the map tip preview container is resized, resize the map tip
  if ( obj == mMapTipPreviewContainer && ev->type() == QEvent::Resize )
  {
    resizeMapTip();
  }
  return QgsOptionsDialogBase::eventFilter( obj, ev );
}

void QgsVectorLayerProperties::initMapTipPreview()
{
  // HTML editor and preview are in a splitter. By default, the editor takes 2/3 of the space
  mMapTipSplitter->setSizes( { 400, 200 } );
  // Event filter is used to resize the map tip when the container is resized
  mMapTipPreviewContainer->installEventFilter( this );

  // Note: there's quite a bit of overlap between this and the code in QgsMapTip::showMapTip
  // Create the WebView
  mMapTipPreview = new QgsWebView( mMapTipPreviewContainer );

#if WITH_QTWEBKIT
  mMapTipPreview->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );//Handle link clicks by yourself
  mMapTipPreview->setContextMenuPolicy( Qt::NoContextMenu ); //No context menu is allowed if you don't need it
  connect( mMapTipPreview, &QWebView::loadFinished, this, &QgsVectorLayerProperties::resizeMapTip );
#endif

  mMapTipPreview->page()->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
  mMapTipPreview->page()->settings()->setAttribute( QWebSettings::JavascriptEnabled, true );
  mMapTipPreview->page()->settings()->setAttribute( QWebSettings::LocalStorageEnabled, true );

  // Disable scrollbars, avoid random resizing issues
  mMapTipPreview->page()->mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
  mMapTipPreview->page()->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );

  // Update the map tip preview when the expression or the map tip template changes
  connect( mMapTipWidget, &QgsCodeEditorHTML::textChanged, this, &QgsVectorLayerProperties::updateMapTipPreview );
  connect( mDisplayExpressionWidget, qOverload< const QString & >( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsVectorLayerProperties::updateMapTipPreview );
}

void QgsVectorLayerProperties::updateMapTipPreview()
{
  mMapTipPreview->setMaximumSize( mMapTipPreviewContainer->width(), mMapTipPreviewContainer->height() );
  const QString htmlContent = QgsMapTip::vectorMapTipPreviewText( mLayer, mCanvas, mMapTipWidget->text(), mDisplayExpressionWidget->asExpression() );
  mMapTipPreview->setHtml( htmlContent );
}

void QgsVectorLayerProperties::resizeMapTip()
{
  // Ensure the map tip is not bigger than the container
  mMapTipPreview->setMaximumSize( mMapTipPreviewContainer->width(), mMapTipPreviewContainer->height() );
#if WITH_QTWEBKIT
  // Get the content size
  const QWebElement container = mMapTipPreview->page()->mainFrame()->findFirstElement(
                                  QStringLiteral( "#QgsWebViewContainer" ) );
  const int width = container.geometry().width();
  const int height = container.geometry().height();
  mMapTipPreview->resize( width, height );

  // Move the map tip to the center of the container
  mMapTipPreview->move( ( mMapTipPreviewContainer->width() - mMapTipPreview->width() ) / 2,
                        ( mMapTipPreviewContainer->height() - mMapTipPreview->height() ) / 2 );

#else
  mMapTipPreview->adjustSize();
#endif
}
