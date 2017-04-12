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
#include "qgscontexthelp.h"
#include "qgscoordinatetransform.h"
#include "qgsdiagramproperties.h"
#include "qgsdiagramrenderer.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfieldcalculator.h"
#include "qgsfieldsproperties.h"
#include "qgslabelingwidget.h"
#include "qgsprojectionselectiondialog.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgspluginmetadata.h"
#include "qgspluginregistry.h"
#include "qgsproject.h"
#include "qgssavestyletodbdialog.h"
#include "qgsloadstylefromdbdialog.h"
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

#include "layertree/qgslayertreelayer.h"
#include "qgslayertree.h"

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

#include "qgsrendererpropertiesdialog.h"
#include "qgsstyle.h"

QgsVectorLayerProperties::QgsVectorLayerProperties(
  QgsVectorLayer *lyr,
  QWidget *parent,
  Qt::WindowFlags fl
)
  : QgsOptionsDialogBase( QStringLiteral( "VectorLayerProperties" ), parent, fl )
  , mLayer( lyr )
  , mMetadataFilled( false )
  , mOriginalSubsetSQL( lyr->subsetString() )
  , mSaveAsMenu( nullptr )
  , mLoadStyleMenu( nullptr )
  , mRendererDialog( nullptr )
  , labelingDialog( nullptr )
  , mActionDialog( nullptr )
  , diagramPropertiesDialog( nullptr )
  , mFieldsPropertiesDialog( nullptr )
{
  setupUi( this );
  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  QPushButton *b = new QPushButton( tr( "Style" ) );
  QMenu *m = new QMenu( this );
  mActionLoadStyle = m->addAction( tr( "Load Style" ), this, SLOT( loadStyle_clicked() ) );
  mActionSaveStyleAs = m->addAction( tr( "Save Style" ), this, SLOT( saveStyleAs_clicked() ) );
  m->addSeparator();
  m->addAction( tr( "Save as Default" ), this, SLOT( saveDefaultStyle_clicked() ) );
  m->addAction( tr( "Restore Default" ), this, SLOT( loadDefaultStyle_clicked() ) );
  b->setMenu( m );
  connect( m, &QMenu::aboutToShow, this, &QgsVectorLayerProperties::aboutToShowStyleMenu );
  buttonBox->addButton( b, QDialogButtonBox::ResetRole );

  connect( lyr->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsVectorLayerProperties::syncToLayer );

  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsVectorLayerProperties::apply );
  connect( this, &QDialog::accepted, this, &QgsVectorLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsVectorLayerProperties::onCancel );

  connect( mOptionsStackedWidget, &QStackedWidget::currentChanged, this, &QgsVectorLayerProperties::mOptionsStackedWidget_CurrentChanged );

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

  if ( mLayer->hasGeometryType() )
  {
    // Create the Labeling dialog tab
    layout = new QVBoxLayout( labelingFrame );
    layout->setMargin( 0 );
    labelingDialog = new QgsLabelingWidget( mLayer, QgisApp::instance()->mapCanvas(), labelingFrame );
    labelingDialog->layout()->setContentsMargins( -1, 0, -1, 0 );
    layout->addWidget( labelingDialog );
    labelingFrame->setLayout( layout );
  }
  else
  {
    labelingDialog = nullptr;
    mOptsPage_Labels->setEnabled( false ); // disable labeling item
  }

  // Create the Actions dialog tab
  QVBoxLayout *actionLayout = new QVBoxLayout( actionOptionsFrame );
  actionLayout->setMargin( 0 );
  mActionDialog = new QgsAttributeActionDialog( *mLayer->actions(), actionOptionsFrame );
  mActionDialog->layout()->setMargin( 0 );
  actionLayout->addWidget( mActionDialog );

  // Create the menu for the save style button to choose the output format
  mSaveAsMenu = new QMenu( this );
  mSaveAsMenu->addAction( tr( "QGIS Layer Style File..." ) );
  mSaveAsMenu->addAction( tr( "SLD File..." ) );

  //Only if the provider support loading & saving styles to db add new choices
  if ( mLayer->dataProvider()->isSaveAndLoadStyleToDatabaseSupported() )
  {
    //for loading
    mLoadStyleMenu = new QMenu( this );
    mLoadStyleMenu->addAction( tr( "Load from file..." ) );
    mLoadStyleMenu->addAction( tr( "Database styles manager" ) );
    //mActionLoadStyle->setContextMenuPolicy( Qt::PreventContextMenu );
    mActionLoadStyle->setMenu( mLoadStyleMenu );

    connect( mLoadStyleMenu, &QMenu::triggered,
             this, &QgsVectorLayerProperties::loadStyleMenuTriggered );

    //for saving
    QString providerName = mLayer->providerType();
    if ( providerName == "ogr" )
    {
      providerName = mLayer->dataProvider()->storageType();
      if ( providerName == "GPKG" )
        providerName = "GeoPackage";
    }
    mSaveAsMenu->addAction( tr( "Save in database (%1)" ).arg( providerName ) );
  }

  connect( mSaveAsMenu, &QMenu::triggered,
           this, &QgsVectorLayerProperties::saveStyleAsMenuTriggered );

  mFieldsPropertiesDialog = new QgsFieldsProperties( mLayer, mFieldsFrame );
  mFieldsPropertiesDialog->layout()->setMargin( 0 );
  mFieldsFrame->setLayout( new QVBoxLayout( mFieldsFrame ) );
  mFieldsFrame->layout()->setMargin( 0 );
  mFieldsFrame->layout()->addWidget( mFieldsPropertiesDialog );

  connect( mFieldsPropertiesDialog, &QgsFieldsProperties::toggleEditing, this, static_cast<void ( QgsVectorLayerProperties::* )()>( &QgsVectorLayerProperties::toggleEditing ) );
  connect( this, static_cast<void ( QgsVectorLayerProperties::* )( QgsMapLayer * )>( &QgsVectorLayerProperties::toggleEditing ),
  QgisApp::instance(), [ = ]( QgsMapLayer * layer ) { QgisApp::instance()->toggleEditing( layer ); } );

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
  for ( int i = 0; i < joins.size(); ++i )
  {
    addJoinToTreeWidget( joins[i] );
  }

  mOldJoins = mLayer->vectorJoins();

  QVBoxLayout *diagLayout = new QVBoxLayout( mDiagramFrame );
  diagLayout->setMargin( 0 );
  diagramPropertiesDialog = new QgsDiagramProperties( mLayer, mDiagramFrame, QgisApp::instance()->mapCanvas() );
  diagramPropertiesDialog->layout()->setContentsMargins( -1, 0, -1, 0 );
  diagLayout->addWidget( diagramPropertiesDialog );
  mDiagramFrame->setLayout( diagLayout );

  // Legend tab
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
  teMetadataViewer->clear();
  teMetadataViewer->document()->setDefaultStyleSheet( myStyle );
  teMetadataViewer->setHtml( htmlMetadata() );
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

  mLayersDependenciesTreeView->setModel( mLayersDependenciesTreeModel.get() );

  connect( mRefreshLayerCheckBox, &QCheckBox::toggled, mRefreshLayerIntervalSpinBox, &QDoubleSpinBox::setEnabled );

} // QgsVectorLayerProperties ctor


QgsVectorLayerProperties::~QgsVectorLayerProperties()
{
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
  if ( !factory->supportLayerPropertiesDialog() )
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
  QString expression = QStringLiteral( "[% \"" );
  expression += mMapTipExpressionFieldWidget->asExpression();
  expression += QLatin1String( "\" %]" );

  mMapTipWidget->insertText( expression );
}

// in raster props, this method is called sync()
void QgsVectorLayerProperties::syncToLayer()
{
  // populate the general information
  mLayerOrigNameLineEdit->setText( mLayer->originalName() );
  txtDisplayName->setText( mLayer->name() );
  pbnQueryBuilder->setWhatsThis( tr( "This button opens the query "
                                     "builder and allows you to create a subset of features to display on "
                                     "the map canvas rather than displaying all features in the layer" ) );
  txtSubsetSQL->setWhatsThis( tr( "The query used to limit the features in the "
                                  "layer is shown here. To enter or modify the query, click on the Query Builder button" ) );

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
  mScaleRangeWidget->setScaleRange( 1.0 / mLayer->maximumScale(), 1.0 / mLayer->minimumScale() ); // caution: layer uses scale denoms, widget uses true scales
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
  mSimplifyAlgorithmComboBox->addItem( tr( "Distance" ), ( int )QgsVectorSimplifyMethod::Distance );
  mSimplifyAlgorithmComboBox->addItem( tr( "SnapToGrid" ), ( int )QgsVectorSimplifyMethod::SnapToGrid );
  mSimplifyAlgorithmComboBox->addItem( tr( "Visvalingam" ), ( int )QgsVectorSimplifyMethod::Visvalingam );
  mSimplifyAlgorithmComboBox->setCurrentIndex( mSimplifyAlgorithmComboBox->findData( ( int )simplifyMethod.simplifyAlgorithm() ) );

  QStringList myScalesList = PROJECT_SCALES.split( ',' );
  myScalesList.append( QStringLiteral( "1:1" ) );
  mSimplifyMaximumScaleComboBox->updateScales( myScalesList );
  mSimplifyMaximumScaleComboBox->setScale( 1.0 / simplifyMethod.maximumScale() );

  mForceRasterCheckBox->setChecked( mLayer->renderer() && mLayer->renderer()->forceRasterRender() );

  mRefreshLayerCheckBox->setChecked( mLayer->hasAutoRefreshEnabled() );
  mRefreshLayerIntervalSpinBox->setEnabled( mLayer->hasAutoRefreshEnabled() );
  mRefreshLayerIntervalSpinBox->setValue( mLayer->autoRefreshInterval() / 1000.0 );

  // load appropriate symbology page (V1 or V2)
  updateSymbologyPage();

  mActionDialog->init( *mLayer->actions(), mLayer->attributeTableConfig() );

  if ( labelingDialog )
    labelingDialog->adaptToLayer();

  mFieldsPropertiesDialog->init();

  // set initial state for variable editor
  updateVariableEditor();

  // updates the init python code and ui
  updateFieldsPropertiesDialog();

} // syncToLayer()



void QgsVectorLayerProperties::apply()
{
  if ( labelingDialog )
  {
    labelingDialog->writeSettingsToLayer();
  }

  // apply legend settings
  mLegendConfigEmbeddedWidget->applyToLayer();

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
  // caution: layer uses scale denoms, widget uses true scales
  mLayer->setMaximumScale( mScaleRangeWidget->maximumScaleDenom() );
  mLayer->setMinimumScale( mScaleRangeWidget->minimumScaleDenom() );

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

  // Apply fields settings
  mFieldsPropertiesDialog->apply();

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
  simplifyMethod.setMaximumScale( 1.0 / mSimplifyMaximumScaleComboBox->scale() );
  mLayer->setSimplifyMethod( simplifyMethod );

  if ( mLayer->renderer() )
    mLayer->renderer()->setForceRasterRender( mForceRasterCheckBox->isChecked() );

  mLayer->setAutoRefreshInterval( mRefreshLayerIntervalSpinBox->value() * 1000.0 );
  mLayer->setAutoRefreshEnabled( mRefreshLayerCheckBox->isChecked() );

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
    QMessageBox::warning( nullptr, tr( "Dependency cycle" ), tr( "This configuration introduces a cycle in data dependencies and will be ignored" ) );
  }

  // update symbology
  emit refreshLegend( mLayer->id() );

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

void QgsVectorLayerProperties::on_pbnQueryBuilder_clicked()
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

void QgsVectorLayerProperties::on_pbnIndex_clicked()
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

void QgsVectorLayerProperties::on_mLayerOrigNameLineEdit_textEdited( const QString &text )
{
  txtDisplayName->setText( mLayer->capitalizeLayerName( text ) );
}

void QgsVectorLayerProperties::on_mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  mLayer->setCrs( crs );
  mMetadataFilled = false;
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
    askToUser.addButton( tr( "Local database" ), QMessageBox::NoRole );
    askToUser.addButton( tr( "Datasource database" ), QMessageBox::YesRole );

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
                                    tr( "No default style was found for this layer" ) );
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
  //reset if the default style was loaded ok only
  if ( defaultLoadedFlag )
  {
    // all worked ok so no need to inform user
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
    askToUser.addButton( tr( "Local database" ), QMessageBox::NoRole );
    askToUser.addButton( tr( "Datasource database" ), QMessageBox::YesRole );

    switch ( askToUser.exec() )
    {
      case 0:
        return;
      case 2:
        mLayer->saveStyleToDatabase( QLatin1String( "" ), QLatin1String( "" ), true, QLatin1String( "" ), errorMsg );
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


void QgsVectorLayerProperties::loadStyle_clicked()
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load layer properties from style file" ), myLastUsedDir,
                       tr( "QGIS Layer Style File" ) + " (*.qml);;" + tr( "SLD File" ) + " (*.sld)" );
  if ( myFileName.isNull() )
  {
    return;
  }

  mOldStyle = mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() );

  QString myMessage;
  bool defaultLoadedFlag = false;

  if ( myFileName.endsWith( QLatin1String( ".sld" ), Qt::CaseInsensitive ) )
  {
    // load from SLD
    myMessage = mLayer->loadSldStyle( myFileName, defaultLoadedFlag );
  }
  else
  {
    myMessage = mLayer->loadNamedStyle( myFileName, defaultLoadedFlag );
  }
  //reset if the default style was loaded ok only
  if ( defaultLoadedFlag )
  {
    syncToLayer();
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::warning( this, tr( "Load Style" ), myMessage );
  }

  QFileInfo myFI( myFileName );
  QString myPath = myFI.path();
  myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), myPath );

  activateWindow(); // set focus back to properties dialog
}


void QgsVectorLayerProperties::saveStyleAs_clicked()
{
  saveStyleAs( QML );
}

void QgsVectorLayerProperties::saveStyleAsMenuTriggered( QAction *action )
{
  QMenu *menu = qobject_cast<QMenu *>( sender() );
  if ( !menu )
    return;

  int index = mSaveAsMenu->actions().indexOf( action );
  if ( index < 0 )
    return;

  saveStyleAs( static_cast< StyleType >( index ) );
}

void QgsVectorLayerProperties::saveStyleAs( StyleType styleType )
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  if ( styleType == DB )
  {
    QString infoWindowTitle = QObject::tr( "Save style to DB (%1)" ).arg( mLayer->providerType() );
    QString msgError;

    QgsSaveStyleToDbDialog askToUser;
    //Ask the user for a name and a description about the style
    if ( askToUser.exec() == QDialog::Accepted )
    {
      QString styleName = askToUser.getName();
      QString styleDesc = askToUser.getDescription();
      QString uiFileContent = askToUser.getUIFileContent();
      bool isDefault = askToUser.isDefault();

      apply();

      mLayer->saveStyleToDatabase( styleName, styleDesc, isDefault, uiFileContent, msgError );

      if ( !msgError.isNull() )
      {
        QgisApp::instance()->messageBar()->pushMessage( infoWindowTitle, msgError, QgsMessageBar::WARNING, QgisApp::instance()->messageTimeout() );
      }
      else
      {
        QgisApp::instance()->messageBar()->pushMessage( infoWindowTitle, tr( "Style saved" ), QgsMessageBar::INFO, QgisApp::instance()->messageTimeout() );
      }

    }
    else
    {
      return;
    }
  }
  else
  {

    QString format, extension;
    if ( styleType == SLD )
    {
      format = tr( "SLD File" ) + " (*.sld)";
      extension = QStringLiteral( ".sld" );
    }
    else
    {
      format = tr( "QGIS Layer Style File" ) + " (*.qml)";
      extension = QStringLiteral( ".qml" );
    }

    QString myOutputFileName = QFileDialog::getSaveFileName( this, tr( "Save layer properties as style file" ),
                               myLastUsedDir, format );
    if ( myOutputFileName.isNull() ) //dialog canceled
    {
      return;
    }

    apply(); // make sure the style to save is uptodate

    QString myMessage;
    bool defaultLoadedFlag = false;

    //ensure the user never omitted the extension from the file name
    if ( !myOutputFileName.endsWith( extension, Qt::CaseInsensitive ) )
    {
      myOutputFileName += extension;
    }

    if ( styleType == SLD )
    {
      // convert to SLD
      myMessage = mLayer->saveSldStyle( myOutputFileName, defaultLoadedFlag );
    }
    else
    {
      myMessage = mLayer->saveNamedStyle( myOutputFileName, defaultLoadedFlag );
    }

    //reset if the default style was loaded ok only
    if ( defaultLoadedFlag )
    {
      syncToLayer();
    }
    else
    {
      //let the user know what went wrong
      QMessageBox::information( this, tr( "Saved Style" ), myMessage );
    }

    QFileInfo myFI( myOutputFileName );
    QString myPath = myFI.path();
    // Persist last used dir
    myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), myPath );
  }
}

void QgsVectorLayerProperties::loadStyleMenuTriggered( QAction *action )
{
  QMenu *menu = qobject_cast<QMenu *>( sender() );
  if ( !menu )
    return;

  int index = mLoadStyleMenu->actions().indexOf( action );

  if ( index == 0 ) //Load from filesystem
  {
    loadStyle_clicked();
  }
  else if ( index == 1 ) //Load from database
  {
    showListOfStylesFromDatabase();
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

void QgsVectorLayerProperties::showListOfStylesFromDatabase()
{
  QString errorMsg;
  QStringList ids, names, descriptions;

  //get the list of styles in the db
  int sectionLimit = mLayer->listStylesInDatabase( ids, names, descriptions, errorMsg );
  if ( !errorMsg.isNull() )
  {
    QMessageBox::warning( this, tr( "Error occurred retrieving styles from database" ), errorMsg );
    return;
  }

  QgsLoadStyleFromDBDialog dialog;
  dialog.setLayer( mLayer );
  dialog.initializeLists( ids, names, descriptions, sectionLimit );

  if ( dialog.exec() == QDialog::Accepted )
  {
    QString selectedStyleId = dialog.getSelectedStyleId();

    QString qmlStyle = mLayer->getStyleFromDatabase( selectedStyleId, errorMsg );
    if ( !errorMsg.isNull() )
    {
      QMessageBox::warning( this, tr( "Error occurred retrieving styles from database" ), errorMsg );
      return;
    }

    QDomDocument myDocument( QStringLiteral( "qgis" ) );
    myDocument.setContent( qmlStyle );

    if ( mLayer->importNamedStyle( myDocument, errorMsg ) )
    {
      syncToLayer();
    }
    else
    {
      QMessageBox::warning( this, tr( "Error occurred retrieving styles from database" ),
                            tr( "The retrieved style is not a valid named style. Error message: %1" )
                            .arg( errorMsg ) );
    }
  }
}

void QgsVectorLayerProperties::on_mButtonAddJoin_clicked()
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
    mFieldsPropertiesDialog->init();
  }
}

void QgsVectorLayerProperties::on_mButtonEditJoin_clicked()
{
  QTreeWidgetItem *currentJoinItem = mJoinTreeWidget->currentItem();
  on_mJoinTreeWidget_itemDoubleClicked( currentJoinItem, 0 );
}

void QgsVectorLayerProperties::on_mJoinTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int )
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
    mFieldsPropertiesDialog->init();
  }
}

void QgsVectorLayerProperties::addJoinToTreeWidget( const QgsVectorLayerJoinInfo &join, const int insertIndex )
{
  QTreeWidgetItem *joinItem = new QTreeWidgetItem();

  QgsVectorLayer *joinLayer = join.joinLayer();
  if ( !mLayer || !joinLayer )
  {
    return;
  }

  joinItem->setText( 0, joinLayer->name() );
  joinItem->setData( 0, Qt::UserRole, join.joinLayerId() );

  joinItem->setText( 1, join.joinFieldName() );
  joinItem->setText( 2, join.targetFieldName() );

  if ( join.isUsingMemoryCache() )
  {
    joinItem->setText( 3, QChar( 0x2714 ) );
  }

  joinItem->setText( 4, join.prefix() );

  const QStringList *list = join.joinFieldNamesSubset();
  if ( list )
  {
    joinItem->setText( 5, QStringLiteral( "%1" ).arg( list->count() ) );
  }
  else
  {
    joinItem->setText( 5, tr( "all" ) );
  }

  if ( insertIndex >= 0 )
  {
    mJoinTreeWidget->insertTopLevelItem( insertIndex, joinItem );
  }
  else
  {
    mJoinTreeWidget->addTopLevelItem( joinItem );
  }
  for ( int c = 0; c < 5; c++ )
  {
    mJoinTreeWidget->resizeColumnToContents( c );
  }
  mJoinTreeWidget->setCurrentItem( joinItem );
}

QgsExpressionContext QgsVectorLayerProperties::createExpressionContext() const
{
  return mContext;
}

void QgsVectorLayerProperties::openPanel( QgsPanelWidget *panel )
{
  QDialog *dlg = new QDialog();
  QString key =  QStringLiteral( "/UI/paneldialog/%1" ).arg( panel->panelTitle() );
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

void QgsVectorLayerProperties::on_mButtonRemoveJoin_clicked()
{
  QTreeWidgetItem *currentJoinItem = mJoinTreeWidget->currentItem();
  if ( !mLayer || !currentJoinItem )
  {
    return;
  }

  mLayer->removeJoin( currentJoinItem->data( 0, Qt::UserRole ).toString() );
  mJoinTreeWidget->takeTopLevelItem( mJoinTreeWidget->indexOfTopLevelItem( currentJoinItem ) );
  setPbnQueryBuilderEnabled();
  mFieldsPropertiesDialog->init();
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
    mRendererDialog->setMapCanvas( QgisApp::instance()->mapCanvas() );
    connect( mRendererDialog, &QgsRendererPropertiesDialog::showPanel, this, &QgsVectorLayerProperties::openPanel );
    connect( mRendererDialog, &QgsRendererPropertiesDialog::layerVariablesChanged, this, &QgsVectorLayerProperties::updateVariableEditor );

    // display the menu to choose the output format (fix #5136)
    mActionSaveStyleAs->setText( tr( "Save Style" ) );
    mActionSaveStyleAs->setMenu( mSaveAsMenu );
    disconnect( mActionSaveStyleAs, &QAction::triggered, this, &QgsVectorLayerProperties::saveStyleAs_clicked );
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

void QgsVectorLayerProperties::on_pbnUpdateExtents_clicked()
{
  mLayer->updateExtents();
  mMetadataFilled = false;
}

void QgsVectorLayerProperties::mOptionsStackedWidget_CurrentChanged( int indx )
{
  if ( indx != mOptStackedWidget->indexOf( mOptsPage_Information ) || mMetadataFilled )
    return;

  //set the metadata contents (which can be expensive)
  teMetadataViewer->clear();
  teMetadataViewer->setHtml( htmlMetadata() );
  mMetadataFilled = true;
}

void QgsVectorLayerProperties::on_mSimplifyDrawingGroupBox_toggled( bool checked )
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

void QgsVectorLayerProperties::updateFieldsPropertiesDialog()
{
  QgsEditFormConfig cfg = mLayer->editFormConfig();
  mFieldsPropertiesDialog->setEditFormInit( cfg.uiForm(), cfg.initFunction(), cfg.initCode(), cfg.initFilePath(), cfg.initCodeSource() );
}
