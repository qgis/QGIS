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
#include "qgswmsdimensiondialog.h"
#include "qgsapplication.h"
#include "qgsattributeactiondialog.h"
#include "qgscoordinatetransform.h"
#include "qgsdatumtransformdialog.h"
#include "qgsdiagramproperties.h"
#include "qgsdiagramrenderer.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfieldcalculator.h"
#include "qgssourcefieldsproperties.h"
#include "qgsattributesformproperties.h"
#include "qgslabelingwidget.h"
#include "qgsprojectionselectiondialog.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsmetadatawidget.h"
#include "qgsmetadataurlitemdelegate.h"
#include "qgsnative.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgsvectorlayerproperties.h"
#include "qgsconfig.h"
#include "qgsvectordataprovider.h"
#include "qgssubsetstringeditorproviderregistry.h"
#include "qgssubsetstringeditorprovider.h"
#include "qgssubsetstringeditorinterface.h"
#include "qgsdatasourceuri.h"
#include "qgsrenderer.h"
#include "qgsexpressioncontext.h"
#include "qgssettings.h"
#include "qgsrendererpropertiesdialog.h"
#include "qgsstyle.h"
#include "qgsauxiliarystorage.h"
#include "qgsmaplayerserverproperties.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgsnewauxiliaryfielddialog.h"
#include "qgslabelinggui.h"
#include "qgssymbollayer.h"
#include "qgsgeometryoptions.h"
#include "qgsvectorlayersavestyledialog.h"
#include "qgsmaplayerloadstyledialog.h"
#include "qgsmessagebar.h"
#include "qgssymbolwidgetcontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmaskingwidget.h"
#include "qgsvectorlayertemporalpropertieswidget.h"
#include "qgsprovidersourcewidgetproviderregistry.h"
#include "qgsprovidersourcewidget.h"
#include "qgsproviderregistry.h"

#include "layertree/qgslayertreelayer.h"
#include "qgslayertree.h"

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

#include "qgsrendererpropertiesdialog.h"
#include "qgsstyle.h"


QgsVectorLayerProperties::QgsVectorLayerProperties(
  QgsMapCanvas *canvas,
  QgsMessageBar *messageBar,
  QgsVectorLayer *lyr,
  QWidget *parent,
  Qt::WindowFlags fl
)
  : QgsOptionsDialogBase( QStringLiteral( "VectorLayerProperties" ), parent, fl )
  , mCanvas( canvas )
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

  mBtnMetadata = new QPushButton( tr( "Metadata" ), this );
  QMenu *menuMetadata = new QMenu( this );
  mActionLoadMetadata = menuMetadata->addAction( tr( "Load Metadata…" ), this, &QgsVectorLayerProperties::loadMetadata );
  mActionSaveMetadataAs = menuMetadata->addAction( tr( "Save Metadata…" ), this, &QgsVectorLayerProperties::saveMetadataAs );
  menuMetadata->addSeparator();
  menuMetadata->addAction( tr( "Save as Default" ), this, &QgsVectorLayerProperties::saveDefaultMetadata );
  menuMetadata->addAction( tr( "Restore Default" ), this, &QgsVectorLayerProperties::loadDefaultMetadata );
  mBtnMetadata->setMenu( menuMetadata );
  buttonBox->addButton( mBtnMetadata, QDialogButtonBox::ResetRole );

  connect( lyr->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsVectorLayerProperties::syncToLayer );

  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsVectorLayerProperties::apply );
  connect( this, &QDialog::accepted, this, &QgsVectorLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsVectorLayerProperties::onCancel );

  mContext << QgsExpressionContextUtils::globalScope()
           << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
           << QgsExpressionContextUtils::atlasScope( nullptr )
           << QgsExpressionContextUtils::mapSettingsScope( mCanvas->mapSettings() )
           << QgsExpressionContextUtils::layerScope( mLayer );

  mMapTipExpressionFieldWidget->setLayer( lyr );
  mMapTipExpressionFieldWidget->registerExpressionContextGenerator( this );
  mDisplayExpressionWidget->setLayer( lyr );
  mDisplayExpressionWidget->registerExpressionContextGenerator( this );

  connect( mInsertExpressionButton, &QAbstractButton::clicked, this, &QgsVectorLayerProperties::insertFieldOrExpression );

  if ( !mLayer )
    return;

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

  syncToLayer();

  if ( mLayer->dataProvider() )
  {
    //enable spatial index button group if supported by provider, or if one already exists
    QgsVectorDataProvider::Capabilities capabilities = mLayer->dataProvider()->capabilities();
    if ( !( capabilities & QgsVectorDataProvider::CreateSpatialIndex ) )
    {
      pbnIndex->setEnabled( false );
    }
    if ( mLayer->dataProvider()->hasSpatialIndex() == QgsFeatureSource::SpatialIndexPresent )
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
  connect( teMetadataViewer, &QTextBrowser::anchorClicked, this, &QgsVectorLayerProperties::urlClicked );
  mMetadataFilled = true;

  QgsSettings settings;
  // if dialog hasn't been opened/closed yet, default to Styles tab, which is used most often
  // this will be read by restoreOptionsBaseUi()
  if ( !settings.contains( QStringLiteral( "/Windows/VectorLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/VectorLayerProperties/tab" ),
                       mOptStackedWidget->indexOf( mOptsPage_Style ) );
  }

  QString title = tr( "Layer Properties — %1" ).arg( mLayer->name() );
  if ( !mLayer->styleManager()->isDefault( mLayer->styleManager()->currentStyle() ) )
    title += QStringLiteral( " (%1)" ).arg( mLayer->styleManager()->currentStyle() );
  restoreOptionsBaseUi( title );

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

  connect( mRefreshLayerCheckBox, &QCheckBox::toggled, mRefreshLayerIntervalSpinBox, &QDoubleSpinBox::setEnabled );

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
  mOptsPage_Variables->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#variables-properties" ) );
  mOptsPage_Metadata->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#metadata-properties" ) );
  mOptsPage_DataDependencies->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#dependencies-properties" ) ) ;
  mOptsPage_Legend->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#legend-properties" ) );
  mOptsPage_Server->setProperty( "helpPage", QStringLiteral( "working_with_vector/vector_properties.html#qgis-server-properties" ) );


  optionsStackedWidget_CurrentChanged( mOptStackedWidget->currentIndex() );
}

void QgsVectorLayerProperties::toggleEditing()
{
  if ( !mLayer )
    return;

  emit toggleEditing( mLayer );

  setPbnQueryBuilderEnabled();
}

void QgsVectorLayerProperties::addPropertiesPageFactory( const QgsMapLayerConfigWidgetFactory *factory )
{
  if ( !factory->supportsLayer( mLayer ) || !factory->supportLayerPropertiesDialog() )
  {
    return;
  }

  QgsMapLayerConfigWidget *page = factory->createWidget( mLayer, nullptr, false, this );

  if ( page )
  {
    mLayerPropertiesPages << page;

    const QString beforePage = factory->layerPropertiesPagePositionHint();
    if ( beforePage.isEmpty() )
      addPage( factory->title(), factory->title(), factory->icon(), page );
    else
      insertPage( factory->title(), factory->title(), factory->icon(), page, beforePage );
  }
}

void QgsVectorLayerProperties::insertFieldOrExpression()
{
  // Convert the selected field to an expression and
  // insert it into the action at the cursor position
  QString expression = QStringLiteral( "[% " );
  expression += mMapTipExpressionFieldWidget->asExpression();
  expression += QLatin1String( " %]" );

  mMapTipWidget->insertText( expression );
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

// in raster props, this method is called sync()
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
      mSourceGroupBox->show();

      connect( mSourceWidget, &QgsProviderSourceWidget::validChanged, this, [ = ]( bool isValid )
      {
        buttonBox->button( QDialogButtonBox::Apply )->setEnabled( isValid );
        buttonBox->button( QDialogButtonBox::Ok )->setEnabled( isValid );
      } );
    }
  }

  if ( mSourceWidget )
    mSourceWidget->setSourceUri( mLayer->source() );

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

  mMapTipWidget->setText( mLayer->mapTipTemplate() );
  mDisplayExpressionWidget->setField( mLayer->displayExpression() );

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
  if ( mLayer->geometryType() == QgsWkbTypes::PointGeometry )
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

  mRefreshLayerCheckBox->setChecked( mLayer->hasAutoRefreshEnabled() );
  mRefreshLayerIntervalSpinBox->setEnabled( mLayer->hasAutoRefreshEnabled() );
  mRefreshLayerIntervalSpinBox->setValue( mLayer->autoRefreshInterval() / 1000.0 );

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
  const auto constMLayerPropertiesPages = mLayerPropertiesPages;
  for ( QgsMapLayerConfigWidget *page : constMLayerPropertiesPages )
  {
    page->syncToLayer( mLayer );
  }

  mMetadataWidget->setMetadata( &mLayer->metadata() );

  mTemporalWidget->syncToLayer();

  mLegendWidget->setLayer( mLayer );

}

void QgsVectorLayerProperties::apply()
{
  if ( mSourceWidget )
  {
    const QString newSource = mSourceWidget->sourceUri();
    if ( newSource != mLayer->source() )
    {
      mLayer->setDataSource( newSource, mLayer->name(), mLayer->providerType(),
                             QgsDataProvider::ProviderOptions(), QgsDataProvider::ReadFlags() );
    }
  }

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

  //
  // Set up sql subset query if applicable
  //
  mSubsetGroupBox->setEnabled( true );

  if ( txtSubsetSQL->text() != mLayer->subsetString() )
  {
    // set the subset sql for the layer
    mLayer->setSubsetString( txtSubsetSQL->text() );
    mMetadataFilled = false;
  }
  mOriginalSubsetSQL = mLayer->subsetString();

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
  const auto constMLayerPropertiesPages = mLayerPropertiesPages;
  for ( QgsMapLayerConfigWidget *page : constMLayerPropertiesPages )
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

  mLayer->setAutoRefreshInterval( mRefreshLayerIntervalSpinBox->value() * 1000.0 );
  mLayer->setAutoRefreshEnabled( mRefreshLayerCheckBox->isChecked() );

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

  mLayer->triggerRepaint();
  // notify the project we've made a change
  QgsProject::instance()->setDirty( true );
}

void QgsVectorLayerProperties::onCancel()
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

  if ( mOldStyle.xmlData() != mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() ).xmlData() )
  {
    // need to reset style to previous - style applied directly to the layer (not in apply())
    QString myMessage;
    QDomDocument doc( QStringLiteral( "qgis" ) );
    int errorLine, errorColumn;
    doc.setContent( mOldStyle.xmlData(), false, &myMessage, &errorLine, &errorColumn );
    mLayer->importNamedStyle( doc, myMessage );
  }

  if ( mBackupCrs != mLayer->crs() )
    mLayer->setCrs( mBackupCrs );
}

void QgsVectorLayerProperties::urlClicked( const QUrl &url )
{
  QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
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

void QgsVectorLayerProperties::loadDefaultStyle_clicked()
{
  QString msg;
  bool defaultLoadedFlag = false;

  const QgsVectorDataProvider *provider = mLayer->dataProvider();
  if ( !provider )
    return;
  if ( provider->isSaveAndLoadStyleToDatabaseSupported() )
  {
    QMessageBox askToUser;
    askToUser.setText( tr( "Load default style from: " ) );
    askToUser.setIcon( QMessageBox::Question );
    askToUser.addButton( tr( "Cancel" ), QMessageBox::RejectRole );
    askToUser.addButton( tr( "Local Database" ), QMessageBox::NoRole );
    askToUser.addButton( tr( "Datasource Database" ), QMessageBox::YesRole );

    switch ( askToUser.exec() )
    {
      case 0:
        return;
      case 2:
        msg = mLayer->loadNamedStyle( mLayer->styleURI(), defaultLoadedFlag );
        if ( !defaultLoadedFlag )
        {
          //something went wrong - let them know why
          QMessageBox::information( this, tr( "Default Style" ), msg );
        }
        if ( msg.compare( tr( "Loaded from Provider" ) ) )
        {
          QMessageBox::information( this, tr( "Default Style" ),
                                    tr( "No default style was found for this layer." ) );
        }
        else
        {
          syncToLayer();
        }

        return;
      default:
        break;
    }
  }

  QString myMessage = mLayer->loadNamedStyle( mLayer->styleURI(), defaultLoadedFlag, true );
//  QString myMessage = layer->loadDefaultStyle( defaultLoadedFlag );
  //reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    // all worked OK so no need to inform user
    syncToLayer();
  }
  else
  {
    //something went wrong - let them know why
    QMessageBox::information( this, tr( "Default Style" ), myMessage );
  }
}

void QgsVectorLayerProperties::saveDefaultStyle_clicked()
{
  apply();
  QString errorMsg;
  const QgsVectorDataProvider *provider = mLayer->dataProvider();
  if ( !provider )
    return;
  if ( provider->isSaveAndLoadStyleToDatabaseSupported() )
  {
    QMessageBox askToUser;
    askToUser.setText( tr( "Save default style to: " ) );
    askToUser.setIcon( QMessageBox::Question );
    askToUser.addButton( tr( "Cancel" ), QMessageBox::RejectRole );
    askToUser.addButton( tr( "Local Database" ), QMessageBox::NoRole );
    askToUser.addButton( tr( "Datasource Database" ), QMessageBox::YesRole );

    switch ( askToUser.exec() )
    {
      case 0:
        return;
      case 2:
      {
        QString errorMessage;
        if ( QgsProviderRegistry::instance()->styleExists( mLayer->providerType(), mLayer->source(), QString(), errorMessage ) )
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
          QMessageBox::warning( nullptr, QObject::tr( "Save style in database" ),
                                errorMessage );
          return;
        }

        mLayer->saveStyleToDatabase( QString(), QString(), true, QString(), errorMsg );
        if ( errorMsg.isNull() )
        {
          return;
        }
        break;
      }
      default:
        break;
    }
  }

  bool defaultSavedFlag = false;
  errorMsg = mLayer->saveDefaultStyle( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    QMessageBox::warning( this, tr( "Default Style" ), errorMsg );
  }
}

void QgsVectorLayerProperties::loadMetadata()
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load Layer Metadata from Metadata File" ), myLastUsedDir,
                       tr( "QGIS Layer Metadata File" ) + " (*.qmd)" );
  if ( myFileName.isNull() )
  {
    return;
  }

  QString myMessage;
  bool defaultLoadedFlag = false;
  myMessage = mLayer->loadNamedMetadata( myFileName, defaultLoadedFlag );

  //reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    mMetadataWidget->setMetadata( &mLayer->metadata() );
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::warning( this, tr( "Load Metadata" ), myMessage );
  }

  QFileInfo myFI( myFileName );
  QString myPath = myFI.path();
  myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), myPath );

  activateWindow(); // set focus back to properties dialog
}

void QgsVectorLayerProperties::saveMetadataAs()
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString myOutputFileName = QFileDialog::getSaveFileName( this, tr( "Save Layer Metadata as QMD" ),
                             myLastUsedDir, tr( "QMD File" ) + " (*.qmd)" );
  if ( myOutputFileName.isNull() ) //dialog canceled
  {
    return;
  }

  mMetadataWidget->acceptMetadata();

  //ensure the user never omitted the extension from the file name
  if ( !myOutputFileName.endsWith( QgsMapLayer::extensionPropertyType( QgsMapLayer::Metadata ), Qt::CaseInsensitive ) )
  {
    myOutputFileName += QgsMapLayer::extensionPropertyType( QgsMapLayer::Metadata );
  }

  QString myMessage;
  bool defaultLoadedFlag = false;
  myMessage = mLayer->saveNamedMetadata( myOutputFileName, defaultLoadedFlag );

  //reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    syncToLayer();
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::information( this, tr( "Save Metadata" ), myMessage );
  }

  QFileInfo myFI( myOutputFileName );
  QString myPath = myFI.path();
  // Persist last used dir
  myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), myPath );
}

void QgsVectorLayerProperties::saveDefaultMetadata()
{
  mMetadataWidget->acceptMetadata();

  bool defaultSavedFlag = false;
  QString errorMsg = mLayer->saveDefaultMetadata( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    QMessageBox::warning( this, tr( "Default Metadata" ), errorMsg );
  }
}

void QgsVectorLayerProperties::loadDefaultMetadata()
{
  bool defaultLoadedFlag = false;
  QString myMessage = mLayer->loadNamedMetadata( mLayer->metadataUri(), defaultLoadedFlag );
  //reset if the default metadata was loaded OK only
  if ( defaultLoadedFlag )
  {
    mMetadataWidget->setMetadata( &mLayer->metadata() );
  }
  else
  {
    QMessageBox::information( this, tr( "Default Metadata" ), myMessage );
  }
}


void QgsVectorLayerProperties::saveStyleAs()
{
  if ( !mLayer->dataProvider() )
    return;
  QgsVectorLayerSaveStyleDialog dlg( mLayer );
  QgsSettings settings;

  if ( dlg.exec() )
  {
    apply();

    bool defaultLoadedFlag = false;

    StyleType type = dlg.currentStyleType();
    switch ( type )
    {
      case QML:
      case SLD:
      {
        QString message;
        QString filePath = dlg.outputFilePath();
        if ( type == QML )
          message = mLayer->saveNamedStyle( filePath, defaultLoadedFlag, dlg.styleCategories() );
        else
          message = mLayer->saveSldStyle( filePath, defaultLoadedFlag );

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
      case DB:
      {
        QString infoWindowTitle = QObject::tr( "Save style to DB (%1)" ).arg( mLayer->providerType() );
        QString msgError;

        QgsVectorLayerSaveStyleDialog::SaveToDbSettings dbSettings = dlg.saveToDbSettings();

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
          mMessageBar->pushMessage( infoWindowTitle, errorMessage, Qgis::MessageLevel::Warning );
          return;
        }

        mLayer->saveStyleToDatabase( dbSettings.name, dbSettings.description, dbSettings.isDefault, dbSettings.uiFileContent, msgError );

        if ( !msgError.isNull() )
        {
          mMessageBar->pushMessage( infoWindowTitle, msgError, Qgis::MessageLevel::Warning );
        }
        else
        {
          mMessageBar->pushMessage( infoWindowTitle, tr( "Style saved" ), Qgis::MessageLevel::Success );
        }
        break;
      }
    }
  }
}

void QgsVectorLayerProperties::saveMultipleStylesAs()
{
  QgsVectorLayerSaveStyleDialog dlg( mLayer );
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
            QString safePath { filePath };
            if ( styleIndex > 0 && stylesSelected.count( ) > 1 )
            {
              int i = 1;
              while ( QFile::exists( safePath ) )
              {
                const QFileInfo fi { filePath };
                safePath = QString( filePath ).replace( '.' + fi.completeSuffix(), QStringLiteral( "_%1.%2" )
                                                        .arg( QString::number( i ) )
                                                        .arg( fi.completeSuffix() ) );
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
          case DB:
          {
            QString infoWindowTitle = QObject::tr( "Save style '%1' to DB (%2)" )
                                      .arg( styleName )
                                      .arg( mLayer->providerType() );
            QString msgError;

            QgsVectorLayerSaveStyleDialog::SaveToDbSettings dbSettings = dlg.saveToDbSettings();

            // If a name is defined, we add _1 etc. else we use the style name
            QString name { dbSettings.name };
            if ( name.isEmpty() )
            {
              name = styleName;
            }
            else
            {
              QStringList ids, names, descriptions;
              mLayer->listStylesInDatabase( ids, names, descriptions, msgError );
              int i = 1;
              while ( names.contains( name ) )
              {
                name = QStringLiteral( "%1 %2" ).arg( name ).arg( QString::number( i ) );
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
              mMessageBar->pushMessage( infoWindowTitle, errorMessage, Qgis::MessageLevel::Warning );
              return;
            }

            mLayer->saveStyleToDatabase( name, dbSettings.description, dbSettings.isDefault, dbSettings.uiFileContent, msgError );

            if ( !msgError.isNull() )
            {
              mMessageBar->pushMessage( infoWindowTitle, msgError, Qgis::MessageLevel::Warning );
            }
            else
            {
              mMessageBar->pushMessage( infoWindowTitle, tr( "Style '%1' saved" ).arg( styleName ),
                                        Qgis::MessageLevel::Success );
            }
            break;
          }
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
  m->addAction( tr( "Save as Default" ), this, &QgsVectorLayerProperties::saveDefaultStyle_clicked );
  m->addAction( tr( "Restore Default" ), this, &QgsVectorLayerProperties::loadDefaultStyle_clicked );

  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mLayer );
}

void QgsVectorLayerProperties::loadStyle()
{
  QgsSettings settings;  // where we keep last used filter in persistent state

  QString errorMsg;
  QStringList ids, names, descriptions;

  //get the list of styles in the db
  int sectionLimit = mLayer->listStylesInDatabase( ids, names, descriptions, errorMsg );
  QgsMapLayerLoadStyleDialog dlg( mLayer );
  dlg.initializeLists( ids, names, descriptions, sectionLimit );

  if ( dlg.exec() )
  {
    mOldStyle = mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() );
    QgsMapLayer::StyleCategories categories = dlg.styleCategories();
    StyleType type = dlg.currentStyleType();
    switch ( type )
    {
      case QML:
      case SLD:
      {
        QString message;
        bool defaultLoadedFlag = false;
        QString filePath = dlg.filePath();
        if ( type == SLD )
        {
          message = mLayer->loadSldStyle( filePath, defaultLoadedFlag );
        }
        else
        {
          message = mLayer->loadNamedStyle( filePath, defaultLoadedFlag, true, categories );
        }
        //reset if the default style was loaded OK only
        if ( defaultLoadedFlag )
        {
          syncToLayer();
        }
        else
        {
          //let the user know what went wrong
          QMessageBox::warning( this, tr( "Load Style" ), message );
        }
        break;
      }
      case DB:
      {
        QString selectedStyleId = dlg.selectedStyleId();

        QString qmlStyle = mLayer->getStyleFromDatabase( selectedStyleId, errorMsg );
        if ( !errorMsg.isNull() )
        {
          QMessageBox::warning( this, tr( "Load Styles from Database" ), errorMsg );
          return;
        }

        QDomDocument myDocument( QStringLiteral( "qgis" ) );
        myDocument.setContent( qmlStyle );

        if ( mLayer->importNamedStyle( myDocument, errorMsg, categories ) )
        {
          syncToLayer();
        }
        else
        {
          QMessageBox::warning( this, tr( "Load Styles from Database" ),
                                tr( "The retrieved style is not a valid named style. Error message: %1" )
                                .arg( errorMsg ) );
        }
        break;
      }
    }
    activateWindow(); // set focus back to properties dialog
  }
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
  childWmsDimensionDefaultValue->setText( 1, QgsMapLayerServerProperties::wmsDimensionDefaultDisplayLabels()[wmsDim.defaultDisplayType] );
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
  QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged( index );

  bool isMetadataPanel = ( index == mOptStackedWidget->indexOf( mOptsPage_Metadata ) );
  mBtnStyle->setVisible( ! isMetadataPanel );
  mBtnMetadata->setVisible( isMetadataPanel );

  if ( index == mOptStackedWidget->indexOf( mOptsPage_Information ) && ! mMetadataFilled )
  {
    //set the metadata contents (which can be expensive)
    teMetadataViewer->clear();
    teMetadataViewer->setHtml( htmlMetadata() );
    mMetadataFilled = true;
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
