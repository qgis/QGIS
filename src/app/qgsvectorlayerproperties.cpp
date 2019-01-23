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

#include "qgisapp.h"
#include "qgsactionmanager.h"
#include "qgsjoindialog.h"
#include "qgsapplication.h"
#include "qgsattributeactiondialog.h"
#include "qgsapplydialog.h"
#include "qgscoordinatetransform.h"
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
#include "qgsnative.h"
#include "qgspluginmetadata.h"
#include "qgspluginregistry.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgsvectorlayerproperties.h"
#include "qgsconfig.h"
#include "qgsvectordataprovider.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsrenderer.h"
#include "qgsexpressioncontext.h"
#include "qgssettings.h"
#include "qgsrendererpropertiesdialog.h"
#include "qgsstyle.h"
#include "qgsauxiliarystorage.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgsnewauxiliaryfielddialog.h"
#include "qgslabelinggui.h"
#include "qgssymbollayer.h"
#include "qgsgeometryoptions.h"
#include "qgsgeometrycheckfactory.h"
#include "qgsvectorlayersavestyledialog.h"
#include "qgsvectorlayerloadstyledialog.h"
#include "qgsmessagebar.h"
#include "qgsgeometrycheckregistry.h"
#include "qgsgeometrycheck.h"
#include "qgsanalysis.h"
#include "qgssymbolwidgetcontext.h"

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
#include <QUrl>

#include "qgsrendererpropertiesdialog.h"
#include "qgsstyle.h"

#ifdef HAVE_3D
#include "qgsvectorlayer3drendererwidget.h"
#endif


QgsVectorLayerProperties::QgsVectorLayerProperties(
  QgsVectorLayer *lyr,
  QWidget *parent,
  Qt::WindowFlags fl
)
  : QgsOptionsDialogBase( QStringLiteral( "VectorLayerProperties" ), parent, fl )
  , mLayer( lyr )
  , mOriginalSubsetSQL( lyr->subsetString() )
{
  setupUi( this );
  connect( mLayerOrigNameLineEdit, &QLineEdit::textEdited, this, &QgsVectorLayerProperties::mLayerOrigNameLineEdit_textEdited );
  connect( pbnQueryBuilder, &QPushButton::clicked, this, &QgsVectorLayerProperties::pbnQueryBuilder_clicked );
  connect( pbnIndex, &QPushButton::clicked, this, &QgsVectorLayerProperties::pbnIndex_clicked );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsVectorLayerProperties::mCrsSelector_crsChanged );
  connect( pbnUpdateExtents, &QPushButton::clicked, this, &QgsVectorLayerProperties::pbnUpdateExtents_clicked );
  connect( mButtonAddJoin, &QPushButton::clicked, this, &QgsVectorLayerProperties::mButtonAddJoin_clicked );
  connect( mButtonEditJoin, &QPushButton::clicked, this, &QgsVectorLayerProperties::mButtonEditJoin_clicked );
  connect( mJoinTreeWidget, &QTreeWidget::itemDoubleClicked, this, &QgsVectorLayerProperties::mJoinTreeWidget_itemDoubleClicked );
  connect( mButtonRemoveJoin, &QPushButton::clicked, this, &QgsVectorLayerProperties::mButtonRemoveJoin_clicked );
  connect( mSimplifyDrawingGroupBox, &QGroupBox::toggled, this, &QgsVectorLayerProperties::mSimplifyDrawingGroupBox_toggled );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorLayerProperties::showHelp );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  mBtnStyle = new QPushButton( tr( "Style" ), this );
  QMenu *menuStyle = new QMenu( this );
  mActionLoadStyle = menuStyle->addAction( tr( "Load Style…" ) );
  connect( mActionLoadStyle, &QAction::triggered, this, &QgsVectorLayerProperties::loadStyle );
  mActionSaveStyle = menuStyle->addAction( tr( "Save Style…" ) );
  connect( mActionSaveStyle, &QAction::triggered, this, &QgsVectorLayerProperties::saveStyleAs );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsVectorLayerProperties::saveDefaultStyle_clicked );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsVectorLayerProperties::loadDefaultStyle_clicked );
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
           << QgsExpressionContextUtils::mapSettingsScope( QgisApp::instance()->mapCanvas()->mapSettings() )
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
    layout->setMargin( 0 );
    labelingDialog = new QgsLabelingWidget( mLayer, QgisApp::instance()->mapCanvas(), labelingFrame );
    labelingDialog->layout()->setContentsMargins( -1, 0, -1, 0 );
    connect( labelingDialog, &QgsLabelingWidget::auxiliaryFieldCreated, this, [ = ] { updateAuxiliaryStoragePage(); } );
    layout->addWidget( labelingDialog );
    labelingFrame->setLayout( layout );
  }
  else
  {
    labelingDialog = nullptr;
    mOptsPage_Labels->setEnabled( false ); // disable labeling item
    mGeometryGroupBox->setEnabled( false );
    mGeometryGroupBox->setVisible( false );
  }

  // Create the Actions dialog tab
  QVBoxLayout *actionLayout = new QVBoxLayout( actionOptionsFrame );
  actionLayout->setMargin( 0 );
  mActionDialog = new QgsAttributeActionDialog( *mLayer->actions(), actionOptionsFrame );
  mActionDialog->layout()->setMargin( 0 );
  actionLayout->addWidget( mActionDialog );

  mSourceFieldsPropertiesDialog = new QgsSourceFieldsProperties( mLayer, mSourceFieldsFrame );
  mSourceFieldsPropertiesDialog->layout()->setMargin( 0 );
  mSourceFieldsFrame->setLayout( new QVBoxLayout( mSourceFieldsFrame ) );
  mSourceFieldsFrame->layout()->setMargin( 0 );
  mSourceFieldsFrame->layout()->addWidget( mSourceFieldsPropertiesDialog );

  connect( mSourceFieldsPropertiesDialog, &QgsSourceFieldsProperties::toggleEditing, this, static_cast<void ( QgsVectorLayerProperties::* )()>( &QgsVectorLayerProperties::toggleEditing ) );
  connect( this, static_cast<void ( QgsVectorLayerProperties::* )( QgsMapLayer * )>( &QgsVectorLayerProperties::toggleEditing ),
  QgisApp::instance(), [ = ]( QgsMapLayer * layer ) { QgisApp::instance()->toggleEditing( layer ); } );

  mAttributesFormPropertiesDialog = new QgsAttributesFormProperties( mLayer, mAttributesFormFrame );
  mAttributesFormPropertiesDialog->layout()->setMargin( 0 );
  mAttributesFormFrame->setLayout( new QVBoxLayout( mAttributesFormFrame ) );
  mAttributesFormFrame->layout()->setMargin( 0 );
  mAttributesFormFrame->layout()->addWidget( mAttributesFormPropertiesDialog );

#ifdef HAVE_3D
  mVector3DWidget = new QgsVectorLayer3DRendererWidget( mLayer, QgisApp::instance()->mapCanvas(), mOptsPage_3DView );

  mOptsPage_3DView->setLayout( new QVBoxLayout( mOptsPage_3DView ) );
  mOptsPage_3DView->layout()->addWidget( mVector3DWidget );
#else
  delete mOptsPage_3DView;  // removes both the "3d view" list item and its page
#endif

  // Metadata tab, before the syncToLayer
  QVBoxLayout *metadataLayout = new QVBoxLayout( metadataFrame );
  metadataLayout->setMargin( 0 );
  mMetadataWidget = new QgsMetadataWidget( this, mLayer );
  mMetadataWidget->layout()->setContentsMargins( -1, 0, -1, 0 );
  mMetadataWidget->setMapCanvas( QgisApp::instance()->mapCanvas() );
  metadataLayout->addWidget( mMetadataWidget );
  metadataFrame->setLayout( metadataLayout );

  syncToLayer();

  if ( mLayer->dataProvider() )//enable spatial index button group if supported by provider
  {
    int capabilities = mLayer->dataProvider()->capabilities();
    if ( !( capabilities & QgsVectorDataProvider::CreateSpatialIndex ) )
    {
      pbnIndex->setEnabled( false );
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
    else if ( mLayer->dataProvider()->name() == QLatin1String( "ogr" ) )
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
  diagLayout->setMargin( 0 );
  diagramPropertiesDialog = new QgsDiagramProperties( mLayer, mDiagramFrame, QgisApp::instance()->mapCanvas() );
  diagramPropertiesDialog->layout()->setContentsMargins( -1, 0, -1, 0 );
  connect( diagramPropertiesDialog, &QgsDiagramProperties::auxiliaryFieldCreated, this, [ = ] { updateAuxiliaryStoragePage(); } );
  diagLayout->addWidget( diagramPropertiesDialog );
  mDiagramFrame->setLayout( diagLayout );

  // Legend tab
  mLegendWidget->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mLegendWidget->setLayer( mLayer );
  mLegendConfigEmbeddedWidget->setLayer( mLayer );

  // WMS Name as layer short name
  mLayerShortNameLineEdit->setText( mLayer->shortName() );
  // WMS Name validator
  QValidator *shortNameValidator = new QRegExpValidator( QgsApplication::shortNameRegExp(), this );
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
  //layer attribution and metadataUrl
  mLayerAttributionLineEdit->setText( mLayer->attribution() );
  mLayerAttributionUrlLineEdit->setText( mLayer->attributionUrl() );
  mLayerMetadataUrlLineEdit->setText( mLayer->metadataUrl() );
  mLayerMetadataUrlTypeComboBox->setCurrentIndex(
    mLayerMetadataUrlTypeComboBox->findText(
      mLayer->metadataUrlType()
    )
  );
  mLayerMetadataUrlFormatComboBox->setCurrentIndex(
    mLayerMetadataUrlFormatComboBox->findText(
      mLayer->metadataUrlFormat()
    )
  );
  mLayerLegendUrlLineEdit->setText( mLayer->legendUrl() );
  mLayerLegendUrlFormatComboBox->setCurrentIndex(
    mLayerLegendUrlFormatComboBox->findText(
      mLayer->legendUrlFormat()
    )
  );

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

  QString title = QString( tr( "Layer Properties - %1" ) ).arg( mLayer->name() );
  if ( !mLayer->styleManager()->isDefault( mLayer->styleManager()->currentStyle() ) )
    title += QStringLiteral( " (%1)" ).arg( mLayer->styleManager()->currentStyle() );
  restoreOptionsBaseUi( title );

  mLayersDependenciesTreeGroup.reset( QgsProject::instance()->layerTreeRoot()->clone() );
  QgsLayerTreeLayer *layer = mLayersDependenciesTreeGroup->findLayer( mLayer->id() );
  if ( layer )
  {
    layer->parent()->takeChild( layer );
  }
  mLayersDependenciesTreeModel.reset( new QgsLayerTreeModel( mLayersDependenciesTreeGroup.get() ) );
  // use visibility as selection
  mLayersDependenciesTreeModel->setFlag( QgsLayerTreeModel::AllowNodeChangeVisibility );

  mLayersDependenciesTreeGroup->setItemVisibilityChecked( false );

  QSet<QString> dependencySources;
  Q_FOREACH ( const QgsMapLayerDependency &dep, mLayer->dependencies() )
  {
    dependencySources << dep.layerId();
  }
  Q_FOREACH ( QgsLayerTreeLayer *layer, mLayersDependenciesTreeGroup->findLayers() )
  {
    layer->setItemVisibilityChecked( dependencySources.contains( layer->layerId() ) );
  }

  QgsMapSettings ms = QgisApp::instance()->mapCanvas()->mapSettings();
  mLayersDependenciesTreeModel->setLegendMapViewData( QgisApp::instance()->mapCanvas()->mapUnitsPerPixel(), ms.outputDpi(), ms.scale() );
  mLayersDependenciesTreeView->setModel( mLayersDependenciesTreeModel.get() );

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
  connect( mAuxiliaryLayerActionExport, &QAction::triggered, this, &QgsVectorLayerProperties::onAuxiliaryLayerExport );

  mAuxiliaryStorageActions->setMenu( menu );

  connect( mAuxiliaryStorageFieldsDeleteBtn, &QPushButton::clicked, this, &QgsVectorLayerProperties::onAuxiliaryLayerDeleteField );
  connect( mAuxiliaryStorageFieldsAddBtn, &QPushButton::clicked, this, &QgsVectorLayerProperties::onAuxiliaryLayerAddField );

  updateAuxiliaryStoragePage();

  if ( mLayer->isSpatial() )
  {
    mRemoveDuplicateNodesCheckbox->setEnabled( true );
    mGeometryPrecisionLineEdit->setEnabled( true );
    mGeometryPrecisionLineEdit->setValidator( new QDoubleValidator( mGeometryPrecisionLineEdit ) );

    double precision( mLayer->geometryOptions()->geometryPrecision() );
    bool ok = true;
    QString precisionStr( QLocale().toString( precision, ok ) );
    if ( precision == 0.0 || ! ok )
      precisionStr = QString();
    mGeometryPrecisionLineEdit->setText( precisionStr );

    mRemoveDuplicateNodesManuallyActivated = mLayer->geometryOptions()->removeDuplicateNodes();
    mRemoveDuplicateNodesCheckbox->setChecked( mRemoveDuplicateNodesManuallyActivated );
    if ( !precisionStr.isNull() )
      mRemoveDuplicateNodesCheckbox->setEnabled( false );
    connect( mGeometryPrecisionLineEdit, &QLineEdit::textChanged, this, [this]
    {
      if ( !mGeometryPrecisionLineEdit->text().isEmpty() )
      {
        if ( mRemoveDuplicateNodesCheckbox->isEnabled() )
          mRemoveDuplicateNodesManuallyActivated  = mRemoveDuplicateNodesCheckbox->isChecked();
        mRemoveDuplicateNodesCheckbox->setEnabled( false );
        mRemoveDuplicateNodesCheckbox->setChecked( true );
      }
      else
      {
        mRemoveDuplicateNodesCheckbox->setEnabled( true );
        mRemoveDuplicateNodesCheckbox->setChecked( mRemoveDuplicateNodesManuallyActivated );
      }
    } );

    mPrecisionUnitsLabel->setText( QStringLiteral( "[%1]" ).arg( QgsUnitTypes::toAbbreviatedString( mLayer->crs().mapUnits() ) ) );

    QLayout *geometryCheckLayout = new QVBoxLayout();
    const QList<QgsGeometryCheckFactory *> geometryCheckFactories = QgsAnalysis::instance()->geometryCheckRegistry()->geometryCheckFactories( mLayer, QgsGeometryCheck::FeatureNodeCheck, QgsGeometryCheck::Flag::AvailableInValidation );
    const QStringList activeChecks = mLayer->geometryOptions()->geometryChecks();
    for ( const QgsGeometryCheckFactory *factory : geometryCheckFactories )
    {
      QCheckBox *cb = new QCheckBox( factory->description() );
      cb->setChecked( activeChecks.contains( factory->id() ) );
      mGeometryCheckFactoriesGroupBoxes.insert( cb, factory->id() );
      geometryCheckLayout->addWidget( cb );
    }
    mGeometryValidationGroupBox->setLayout( geometryCheckLayout );
    mGeometryValidationGroupBox->setVisible( !geometryCheckFactories.isEmpty() );

    QLayout *topologyCheckLayout = new QVBoxLayout();
    const QList<QgsGeometryCheckFactory *> topologyCheckFactories = QgsAnalysis::instance()->geometryCheckRegistry()->geometryCheckFactories( mLayer, QgsGeometryCheck::LayerCheck, QgsGeometryCheck::Flag::AvailableInValidation );

    for ( const QgsGeometryCheckFactory *factory : topologyCheckFactories )
    {
      QCheckBox *cb = new QCheckBox( factory->description() );
      cb->setChecked( activeChecks.contains( factory->id() ) );
      mGeometryCheckFactoriesGroupBoxes.insert( cb, factory->id() );
      topologyCheckLayout->addWidget( cb );
    }
    mTopologyChecksGroupBox->setLayout( topologyCheckLayout );
    mTopologyChecksGroupBox->setVisible( !topologyCheckFactories.isEmpty() );
  }
  else
  {
    mRemoveDuplicateNodesCheckbox->setEnabled( false );
    mGeometryPrecisionLineEdit->setEnabled( false );
    mGeometryAutoFixesGroupBox->setEnabled( false );
  }

  optionsStackedWidget_CurrentChanged( mOptStackedWidget->currentIndex() );
}

void QgsVectorLayerProperties::toggleEditing()
{
  if ( !mLayer )
    return;

  emit toggleEditing( mLayer );

  setPbnQueryBuilderEnabled();
}

void QgsVectorLayerProperties::addPropertiesPageFactory( QgsMapLayerConfigWidgetFactory *factory )
{
  if ( !factory->supportsLayer( mLayer ) || !factory->supportLayerPropertiesDialog() )
  {
    return;
  }

  QListWidgetItem *item = new QListWidgetItem();
  item->setIcon( factory->icon() );
  item->setText( factory->title() );
  item->setToolTip( factory->title() );

  mOptionsListWidget->addItem( item );

  QgsMapLayerConfigWidget *page = factory->createWidget( mLayer, nullptr, false, this );
  mLayerPropertiesPages << page;
  mOptionsStackedWidget->addWidget( page );
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

// in raster props, this method is called sync()
void QgsVectorLayerProperties::syncToLayer()
{
  // populate the general information
  mLayerOrigNameLineEdit->setText( mLayer->name() );
  txtDisplayName->setText( mLayer->name() );

  //see if we are dealing with a pg layer here
  mSubsetGroupBox->setEnabled( true );
  txtSubsetSQL->setText( mLayer->subsetString() );
  // if the user is allowed to type an adhoc query, the app will crash if the query
  // is bad. For this reason, the sql box is disabled and the query must be built
  // using the query builder, either by typing it in by hand or using the buttons, etc
  // on the builder. If the ability to enter a query directly into the box is required,
  // a mechanism to check it must be implemented.
  txtSubsetSQL->setEnabled( false );
  setPbnQueryBuilderEnabled();

  mMapTipWidget->setText( mLayer->mapTipTemplate() );
  mDisplayExpressionWidget->setField( mLayer->displayExpression() );

  // set up the scale based layer visibility stuff....
  mScaleRangeWidget->setScaleRange( mLayer->minimumScale(), mLayer->maximumScale() );
  mScaleVisibilityGroupBox->setChecked( mLayer->hasScaleBasedVisibility() );
  mScaleRangeWidget->setMapCanvas( QgisApp::instance()->mapCanvas() );

  // get simplify drawing configuration
  const QgsVectorSimplifyMethod &simplifyMethod = mLayer->simplifyMethod();
  mSimplifyDrawingGroupBox->setChecked( simplifyMethod.simplifyHints() != QgsVectorSimplifyMethod::NoSimplification );
  mSimplifyDrawingSpinBox->setValue( simplifyMethod.threshold() );

  QString remark = QStringLiteral( " (%1)" ).arg( tr( "Not supported" ) );
  if ( !( mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::SimplifyGeometries ) )
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

  QStringList myScalesList = PROJECT_SCALES.split( ',' );
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

#ifdef HAVE_3D
  mVector3DWidget->setLayer( mLayer );
#endif

  mMetadataWidget->setMetadata( &mLayer->metadata() );

} // syncToLayer()

void QgsVectorLayerProperties::apply()
{
  if ( labelingDialog )
  {
    labelingDialog->writeSettingsToLayer();
  }

  // apply legend settings
  mLegendWidget->applyToLayer();
  mLegendConfigEmbeddedWidget->applyToLayer();

  // save metadata
  mMetadataWidget->acceptMetadata();
  mMetadataFilled = false;

  //
  // Set up sql subset query if applicable
  //
  mSubsetGroupBox->setEnabled( true );

  if ( txtSubsetSQL->toPlainText() != mLayer->subsetString() )
  {
    // set the subset sql for the layer
    mLayer->setSubsetString( txtSubsetSQL->toPlainText() );
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

  mLayer->setDisplayExpression( mDisplayExpressionWidget->currentField() );
  mLayer->setMapTipTemplate( mMapTipWidget->text() );

  mLayer->actions()->clearActions();
  Q_FOREACH ( const QgsAction &action, mActionDialog->actions() )
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

  if ( mLayer->renderer() )
  {
    QgsRendererPropertiesDialog *dlg = static_cast<QgsRendererPropertiesDialog *>( widgetStackRenderers->currentWidget() );
    dlg->apply();
  }

  //apply diagram settings
  diagramPropertiesDialog->apply();

  // apply all plugin dialogs
  Q_FOREACH ( QgsMapLayerConfigWidget *page, mLayerPropertiesPages )
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

  //layer attribution and metadataUrl
  if ( mLayer->attribution() != mLayerAttributionLineEdit->text() )
    mMetadataFilled = false;
  mLayer->setAttribution( mLayerAttributionLineEdit->text() );

  if ( mLayer->attributionUrl() != mLayerAttributionUrlLineEdit->text() )
    mMetadataFilled = false;
  mLayer->setAttributionUrl( mLayerAttributionUrlLineEdit->text() );

  if ( mLayer->metadataUrl() != mLayerMetadataUrlLineEdit->text() )
    mMetadataFilled = false;
  mLayer->setMetadataUrl( mLayerMetadataUrlLineEdit->text() );

  if ( mLayer->metadataUrlType() != mLayerMetadataUrlTypeComboBox->currentText() )
    mMetadataFilled = false;
  mLayer->setMetadataUrlType( mLayerMetadataUrlTypeComboBox->currentText() );

  if ( mLayer->metadataUrlFormat() != mLayerMetadataUrlFormatComboBox->currentText() )
    mMetadataFilled = false;
  mLayer->setMetadataUrlFormat( mLayerMetadataUrlFormatComboBox->currentText() );

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
    mLayer->renderer()->setForceRasterRender( mForceRasterCheckBox->isChecked() );

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
  Q_FOREACH ( const QgsLayerTreeLayer *layer, mLayersDependenciesTreeGroup->findLayers() )
  {
    if ( layer->isVisible() )
      deps << QgsMapLayerDependency( layer->layerId() );
  }
  if ( ! mLayer->setDependencies( deps ) )
  {
    QMessageBox::warning( nullptr, tr( "Save Dependency" ), tr( "This configuration introduces a cycle in data dependencies and will be ignored." ) );
  }

#ifdef HAVE_3D
  mVector3DWidget->apply();
#endif

  mLayer->geometryOptions()->setRemoveDuplicateNodes( mRemoveDuplicateNodesCheckbox->isChecked() );
  bool ok = true;
  double precision( QLocale().toDouble( mGeometryPrecisionLineEdit->text(), &ok ) );
  if ( ! ok )
    precision = 0.0;
  mLayer->geometryOptions()->setGeometryPrecision( precision );

  QStringList activeChecks;
  QHash<QCheckBox *, QString>::const_iterator it;
  for ( it = mGeometryCheckFactoriesGroupBoxes.constBegin(); it != mGeometryCheckFactoriesGroupBoxes.constEnd(); ++it )
  {
    if ( it.key()->isChecked() )
      activeChecks << it.value();
  }
  mLayer->geometryOptions()->setGeometryChecks( activeChecks );

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

    Q_FOREACH ( const QgsVectorLayerJoinInfo &info, mLayer->vectorJoins() )
      mLayer->removeJoin( info.joinLayerId() );

    Q_FOREACH ( const QgsVectorLayerJoinInfo &info, mOldJoins )
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
    syncToLayer();
  }
}

void QgsVectorLayerProperties::urlClicked( const QUrl &url )
{
  QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::instance()->nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
}

void QgsVectorLayerProperties::pbnQueryBuilder_clicked()
{
  // launch the query builder
  QgsQueryBuilder *qb = new QgsQueryBuilder( mLayer, this );

  // Set the sql in the query builder to the same in the prop dialog
  // (in case the user has already changed it)
  qb->setSql( txtSubsetSQL->toPlainText() );
  // Open the query builder
  if ( qb->exec() )
  {
    // if the sql is changed, update it in the prop subset text box
    txtSubsetSQL->setText( qb->sql() );
    //TODO If the sql is changed in the prop dialog, the layer extent should be recalculated

    // The datasource for the layer needs to be updated with the new sql since this gets
    // saved to the project file. This should happen at the map layer level...

  }
  // delete the query builder object
  delete qb;
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
      QMessageBox::information( this, tr( "Spatial Index" ), tr( "Creation of spatial index successful" ) );
    }
    else
    {
      // TODO: Remind the user to use OGR >= 1.2.6 and Shapefile
      QMessageBox::information( this, tr( "Spatial Index" ), tr( "Creation of spatial index failed" ) );
    }
  }
}

QString QgsVectorLayerProperties::htmlMetadata()
{
  return mLayer->htmlMetadata();
}

void QgsVectorLayerProperties::mLayerOrigNameLineEdit_textEdited( const QString &text )
{
  txtDisplayName->setText( mLayer->formatLayerName( text ) );
}

void QgsVectorLayerProperties::mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  QgisApp::instance()->askUserForDatumTransform( crs, QgsProject::instance()->crs() );
  mLayer->setCrs( crs );
  mMetadataFilled = false;
  mMetadataWidget->crsChanged();
}

void QgsVectorLayerProperties::loadDefaultStyle_clicked()
{
  QString msg;
  bool defaultLoadedFlag = false;

  if ( mLayer->dataProvider()->isSaveAndLoadStyleToDatabaseSupported() )
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
  if ( mLayer->dataProvider()->isSaveAndLoadStyleToDatabaseSupported() )
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
        mLayer->saveStyleToDatabase( QString(), QString(), true, QString(), errorMsg );
        if ( errorMsg.isNull() )
        {
          return;
        }
        break;
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

        mLayer->saveStyleToDatabase( dbSettings.name, dbSettings.description, dbSettings.isDefault, dbSettings.uiFileContent, msgError );

        if ( !msgError.isNull() )
        {
          QgisApp::instance()->messageBar()->pushMessage( infoWindowTitle, msgError, Qgis::Warning, QgisApp::instance()->messageTimeout() );
        }
        else
        {
          QgisApp::instance()->messageBar()->pushMessage( infoWindowTitle, tr( "Style saved" ), Qgis::Info, QgisApp::instance()->messageTimeout() );
        }
        break;
      }
    }
  }
}

void QgsVectorLayerProperties::aboutToShowStyleMenu()
{
  // this should be unified with QgsRasterLayerProperties::aboutToShowStyleMenu()

  QMenu *m = qobject_cast<QMenu *>( sender() );
  if ( !m )
    return;

  // first get rid of previously added style manager actions (they are dynamic)
  bool gotFirstSeparator = false;
  QList<QAction *> actions = m->actions();
  for ( int i = 0; i < actions.count(); ++i )
  {
    if ( actions[i]->isSeparator() )
    {
      if ( gotFirstSeparator )
      {
        // remove all actions after second separator (including it)
        while ( actions.count() != i )
          delete actions.takeAt( i );
        break;
      }
      else
        gotFirstSeparator = true;
    }
  }

  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mLayer );
}

void QgsVectorLayerProperties::loadStyle()
{

  QgsSettings settings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString errorMsg;
  QStringList ids, names, descriptions;

  //get the list of styles in the db
  int sectionLimit = mLayer->listStylesInDatabase( ids, names, descriptions, errorMsg );
  QgsVectorLayerLoadStyleDialog dlg( mLayer );
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

  joinItem->setText( 0, QStringLiteral( "Join layer" ) );
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
  childJoinField->setText( 0, QStringLiteral( "Join field" ) );
  childJoinField->setText( 1, join.joinFieldName() );
  childJoinField->setFlags( Qt::ItemIsEnabled );
  joinItem->addChild( childJoinField );

  QTreeWidgetItem *childTargetField = new QTreeWidgetItem();
  childTargetField->setText( 0, QStringLiteral( "Target field" ) );
  childTargetField->setText( 1, join.targetFieldName() );
  joinItem->addChild( childTargetField );

  QTreeWidgetItem *childMemCache = new QTreeWidgetItem();
  childMemCache->setText( 0, QStringLiteral( "Cache join layer in virtual memory" ) );
  if ( join.isUsingMemoryCache() )
    childMemCache->setText( 1, QChar( 0x2714 ) );
  joinItem->addChild( childMemCache );

  QTreeWidgetItem *childDynForm = new QTreeWidgetItem();
  childDynForm->setText( 0, QStringLiteral( "Dynamic form" ) );
  if ( join.isDynamicFormEnabled() )
    childDynForm->setText( 1, QChar( 0x2714 ) );
  joinItem->addChild( childDynForm );

  QTreeWidgetItem *childEditable = new QTreeWidgetItem();
  childEditable->setText( 0, QStringLiteral( "Editable join layer" ) );
  if ( join.isEditable() )
    childEditable->setText( 1, QChar( 0x2714 ) );
  joinItem->addChild( childEditable );

  QTreeWidgetItem *childUpsert = new QTreeWidgetItem();
  childUpsert->setText( 0, QStringLiteral( "Upsert on edit" ) );
  if ( join.hasUpsertOnEdit() )
    childUpsert->setText( 1, QChar( 0x2714 ) );
  joinItem->addChild( childUpsert );

  QTreeWidgetItem *childCascade = new QTreeWidgetItem();
  childCascade->setText( 0, QStringLiteral( "Delete cascade" ) );
  if ( join.hasCascadedDelete() )
    childCascade->setText( 1, QChar( 0x2714 ) );
  joinItem->addChild( childCascade );

  QTreeWidgetItem *childPrefix = new QTreeWidgetItem();
  childPrefix->setText( 0, QStringLiteral( "Custom field name prefix" ) );
  childPrefix->setText( 1, join.prefix() );
  joinItem->addChild( childPrefix );

  QTreeWidgetItem *childFields = new QTreeWidgetItem();
  childFields->setText( 0, QStringLiteral( "Joined fields" ) );
  const QStringList *list = join.joinFieldNamesSubset();
  if ( list )
    childFields->setText( 1, QStringLiteral( "%1" ).arg( list->count() ) );
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
    context.setMapCanvas( QgisApp::instance()->mapCanvas() );
    context.setMessageBar( QgisApp::instance()->messageBar() );
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
    mRendererDialog->layout()->setMargin( 0 );
    widgetStackRenderers->addWidget( mRendererDialog );
    widgetStackRenderers->setCurrentWidget( mRendererDialog );
    widgetStackRenderers->currentWidget()->layout()->setMargin( 0 );
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

  if ( index != mOptStackedWidget->indexOf( mOptsPage_Information ) || mMetadataFilled )
    return;

  //set the metadata contents (which can be expensive)
  teMetadataViewer->clear();
  teMetadataViewer->setHtml( htmlMetadata() );
  mMetadataFilled = true;
}

void QgsVectorLayerProperties::mSimplifyDrawingGroupBox_toggled( bool checked )
{
  if ( !( mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::SimplifyGeometries ) )
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
  if ( mOptionsListWidget->currentIndex().data().toString() == "Form" )
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#configure-the-field-behavior" ) );
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
    int features = alayer->featureCount();
    mAuxiliaryStorageFeaturesLineEdit->setText( QString::number( features ) );

    // update actions
    mAuxiliaryLayerActionClear->setEnabled( true );
    mAuxiliaryLayerActionDelete->setEnabled( true );
    mAuxiliaryLayerActionExport->setEnabled( true );
    mAuxiliaryLayerActionNew->setEnabled( false );

    const QgsAuxiliaryLayer *alayer = mLayer->auxiliaryLayer();
    if ( alayer )
    {
      const int fields = alayer->auxiliaryFields().count();
      mAuxiliaryStorageFieldsLineEdit->setText( QString::number( fields ) );

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

void QgsVectorLayerProperties::onAuxiliaryLayerExport()
{
  const QgsAuxiliaryLayer *alayer = mLayer->auxiliaryLayer();
  if ( !alayer )
    return;

  std::unique_ptr<QgsVectorLayer> clone;
  clone.reset( alayer->toSpatialLayer() );

  QgisApp::instance()->saveAsFile( clone.get() );
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
    const int timeout = QgisApp::instance()->messageTimeout();
    const QString errors = mLayer->auxiliaryLayer()->commitErrors().join( QStringLiteral( "\n  " ) );
    const QString msg = QObject::tr( "Unable to remove auxiliary field (%1)" ).arg( errors );
    QgisApp::instance()->messageBar()->pushMessage( title, msg, Qgis::Warning, timeout );
  }
}
