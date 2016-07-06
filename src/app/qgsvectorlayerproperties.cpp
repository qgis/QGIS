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
#include "qgsjoindialog.h"
#include "qgsapplication.h"
#include "qgsattributeactiondialog.h"
#include "qgsapplydialog.h"
#include "qgscontexthelp.h"
#include "qgscoordinatetransform.h"
#include "qgsdiagramproperties.h"
#include "qgsdiagramrendererv2.h"
#include "qgsfieldcalculator.h"
#include "qgsfieldsproperties.h"
#include "qgslabeldialog.h"
#include "qgslabelingwidget.h"
#include "qgslabel.h"
#include "qgsgenericprojectionselector.h"
#include "qgslogger.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgspluginmetadata.h"
#include "qgspluginregistry.h"
#include "qgsproject.h"
#include "qgssavestyletodbdialog.h"
#include "qgsloadstylefromdbdialog.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerproperties.h"
#include "qgsconfig.h"
#include "qgsvectordataprovider.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsrendererv2.h"
#include "qgsexpressioncontext.h"

#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QSettings>
#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QColorDialog>

#include "qgsrendererv2propertiesdialog.h"
#include "qgsstylev2.h"
#include "qgssymbologyv2conversion.h"

QgsVectorLayerProperties::QgsVectorLayerProperties(
  QgsVectorLayer *lyr,
  QWidget * parent,
  Qt::WindowFlags fl
)
    : QgsOptionsDialogBase( "VectorLayerProperties", parent, fl )
    , mLayer( lyr )
    , mMetadataFilled( false )
    , mOriginalSubsetSQL( lyr->subsetString() )
    , mSaveAsMenu( nullptr )
    , mLoadStyleMenu( nullptr )
    , mRendererDialog( nullptr )
    , labelingDialog( nullptr )
    , labelDialog( nullptr )
    , mActionDialog( nullptr )
    , diagramPropertiesDialog( nullptr )
    , mFieldsPropertiesDialog( nullptr )
{
  setupUi( this );
  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  QPushButton* b = new QPushButton( tr( "Style" ) );
  QMenu* m = new QMenu( this );
  mActionLoadStyle = m->addAction( tr( "Load Style" ), this, SLOT( loadStyle_clicked() ) );
  mActionSaveStyleAs = m->addAction( tr( "Save Style" ), this, SLOT( saveStyleAs_clicked() ) );
  m->addSeparator();
  m->addAction( tr( "Save as Default" ), this, SLOT( saveDefaultStyle_clicked() ) );
  m->addAction( tr( "Restore Default" ), this, SLOT( loadDefaultStyle_clicked() ) );
  b->setMenu( m );
  connect( m, SIGNAL( aboutToShow() ), this, SLOT( aboutToShowStyleMenu() ) );
  buttonBox->addButton( b, QDialogButtonBox::ResetRole );

  connect( lyr->styleManager(), SIGNAL( currentStyleChanged( QString ) ), this, SLOT( syncToLayer() ) );

  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( apply() ) );
  connect( this, SIGNAL( rejected() ), this, SLOT( onCancel() ) );

  connect( mOptionsStackedWidget, SIGNAL( currentChanged( int ) ), this, SLOT( mOptionsStackedWidget_CurrentChanged( int ) ) );

  fieldComboBox->setLayer( lyr );
  displayFieldComboBox->setLayer( lyr );
  connect( insertFieldButton, SIGNAL( clicked() ), this, SLOT( insertField() ) );
  connect( insertExpressionButton, SIGNAL( clicked() ), this, SLOT( insertExpression() ) );

  // connections for Map Tip display
  connect( htmlRadio, SIGNAL( toggled( bool ) ), htmlMapTip, SLOT( setEnabled( bool ) ) );
  connect( htmlRadio, SIGNAL( toggled( bool ) ), insertFieldButton, SLOT( setEnabled( bool ) ) );
  connect( htmlRadio, SIGNAL( toggled( bool ) ), fieldComboBox, SLOT( setEnabled( bool ) ) );
  connect( htmlRadio, SIGNAL( toggled( bool ) ), insertExpressionButton, SLOT( setEnabled( bool ) ) );
  connect( fieldComboRadio, SIGNAL( toggled( bool ) ), displayFieldComboBox, SLOT( setEnabled( bool ) ) );

  if ( !mLayer )
    return;

  QVBoxLayout *layout;

  if ( mLayer->hasGeometryType() )
  {
    // Create the Labeling dialog tab
    layout = new QVBoxLayout( labelingFrame );
    layout->setMargin( 0 );
    labelingDialog = new QgsLabelingWidget( mLayer, QgisApp::instance()->mapCanvas(), labelingFrame );
    labelingDialog->layout()->setContentsMargins( -1, 0, -1, 0 );
    layout->addWidget( labelingDialog );
    labelingFrame->setLayout( layout );

    // Create the Labeling (deprecated) dialog tab
    layout = new QVBoxLayout( labelOptionsFrame );
    layout->setMargin( 0 );
    labelDialog = new QgsLabelDialog( mLayer->label(), labelOptionsFrame );
    labelDialog->layout()->setMargin( 0 );
    layout->addWidget( labelDialog );
    labelOptionsFrame->setLayout( layout );
    connect( labelDialog, SIGNAL( labelSourceSet() ), this, SLOT( setLabelCheckBox() ) );
  }
  else
  {
    labelingDialog = nullptr;
    labelDialog = nullptr;
    mOptsPage_Labels->setEnabled( false ); // disable labeling item
    mOptsPage_LabelsOld->setEnabled( false ); // disable labeling (deprecated) item
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
  if ( mLayer->dataProvider()->isSaveAndLoadStyleToDBSupported() )
  {
    //for loading
    mLoadStyleMenu = new QMenu( this );
    mLoadStyleMenu->addAction( tr( "Load from file..." ) );
    mLoadStyleMenu->addAction( tr( "Load from database" ) );
    //mActionLoadStyle->setContextMenuPolicy( Qt::PreventContextMenu );
    mActionLoadStyle->setMenu( mLoadStyleMenu );

    QObject::connect( mLoadStyleMenu, SIGNAL( triggered( QAction * ) ),
                      this, SLOT( loadStyleMenuTriggered( QAction * ) ) );

    //for saving
    mSaveAsMenu->addAction( tr( "Save in database (%1)" ).arg( mLayer->providerType() ) );
  }

  QObject::connect( mSaveAsMenu, SIGNAL( triggered( QAction * ) ),
                    this, SLOT( saveStyleAsMenuTriggered( QAction * ) ) );

  mFieldsPropertiesDialog = new QgsFieldsProperties( mLayer, mFieldsFrame );
  mFieldsPropertiesDialog->layout()->setMargin( 0 );
  mFieldsFrame->setLayout( new QVBoxLayout( mFieldsFrame ) );
  mFieldsFrame->layout()->setMargin( 0 );
  mFieldsFrame->layout()->addWidget( mFieldsPropertiesDialog );

  connect( mFieldsPropertiesDialog, SIGNAL( toggleEditing() ), this, SLOT( toggleEditing() ) );
  connect( this, SIGNAL( toggleEditing( QgsMapLayer* ) ), QgisApp::instance(), SLOT( toggleEditing( QgsMapLayer* ) ) );

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
    else if ( mLayer->dataProvider()->name() == "ogr" )
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
  const QList< QgsVectorJoinInfo >& joins = mLayer->vectorJoins();
  for ( int i = 0; i < joins.size(); ++i )
  {
    addJoinToTreeWidget( joins[i] );
  }

  mOldJoins = mLayer->vectorJoins();

  QVBoxLayout* diagLayout = new QVBoxLayout( mDiagramFrame );
  diagLayout->setMargin( 0 );
  diagramPropertiesDialog = new QgsDiagramProperties( mLayer, mDiagramFrame, nullptr );
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

  QSettings settings;
  // if dialog hasn't been opened/closed yet, default to Styles tab, which is used most often
  // this will be read by restoreOptionsBaseUi()
  if ( !settings.contains( QString( "/Windows/VectorLayerProperties/tab" ) ) )
  {
    settings.setValue( QString( "/Windows/VectorLayerProperties/tab" ),
                       mOptStackedWidget->indexOf( mOptsPage_Style ) );
  }

  QString title = QString( tr( "Layer Properties - %1" ) ).arg( mLayer->name() );
  restoreOptionsBaseUi( title );
} // QgsVectorLayerProperties ctor


QgsVectorLayerProperties::~QgsVectorLayerProperties()
{
  if ( mOptsPage_LabelsOld && labelDialog && mLayer->hasGeometryType() )
  {
    disconnect( labelDialog, SIGNAL( labelSourceSet() ), this, SLOT( setLabelCheckBox() ) );
  }
}

void QgsVectorLayerProperties::toggleEditing()
{
  if ( !mLayer )
    return;

  emit toggleEditing( mLayer );

  setPbnQueryBuilderEnabled();
}

void QgsVectorLayerProperties::setLabelCheckBox()
{
  labelCheckBox->setCheckState( Qt::Checked );
}

void QgsVectorLayerProperties::addPropertiesPageFactory( QgsMapLayerConfigWidgetFactory* factory )
{
  if ( !factory->supportLayerPropertiesDialog() )
  {
    return;
  }

  QListWidgetItem* item = new QListWidgetItem();
  item->setIcon( factory->icon() );
  item->setText( factory->title() );
  item->setToolTip( factory->title() );

  mOptionsListWidget->addItem( item );

  QgsMapLayerConfigWidget* page = factory->createWidget( mLayer, nullptr, false, this );
  mLayerPropertiesPages << page;
  mOptionsStackedWidget->addWidget( page );
}

void QgsVectorLayerProperties::insertField()
{
  // Convert the selected field to an expression and
  // insert it into the action at the cursor position
  QString field = "[% \"";
  field += fieldComboBox->currentField();
  field += "\" %]";
  htmlMapTip->insertPlainText( field );
}

void QgsVectorLayerProperties::insertExpression()
{
  QString selText = htmlMapTip->textCursor().selectedText();

  // edit the selected expression if there's one
  if ( selText.startsWith( "[%" ) && selText.endsWith( "%]" ) )
    selText = selText.mid( 2, selText.size() - 4 );

  // display the expression builder
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::atlasScope( nullptr )
  << QgsExpressionContextUtils::mapSettingsScope( QgisApp::instance()->mapCanvas()->mapSettings() )
  << QgsExpressionContextUtils::layerScope( mLayer );

  QgsExpressionBuilderDialog dlg( mLayer, selText.replace( QChar::ParagraphSeparator, '\n' ), this, "generic", context );
  dlg.setWindowTitle( tr( "Insert expression" ) );
  if ( dlg.exec() == QDialog::Accepted )
  {
    QString expression = dlg.expressionBuilder()->expressionText();
    //Only add the expression if the user has entered some text.
    if ( !expression.isEmpty() )
    {
      htmlMapTip->insertPlainText( "[%" + expression + "%]" );
    }
  }
}

void QgsVectorLayerProperties::setDisplayField( const QString& name )
{
  if ( mLayer->fields().fieldNameIndex( name ) == -1 )
  {
    htmlRadio->setChecked( true );
    htmlMapTip->setPlainText( name );
  }
  else
  {
    fieldComboRadio->setChecked( true );
    displayFieldComboBox->setField( name );
  }
}

//! @note in raster props, this method is called sync()
void QgsVectorLayerProperties::syncToLayer()
{
  // populate the general information
  mLayerOrigNameLineEdit->setText( mLayer->originalName() );
  txtDisplayName->setText( mLayer->name() );
  txtLayerSource->setText( mLayer->publicSource() );
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

  setDisplayField( mLayer->displayField() );

  // set up the scale based layer visibility stuff....
  mScaleRangeWidget->setScaleRange( 1.0 / mLayer->maximumScale(), 1.0 / mLayer->minimumScale() ); // caution: layer uses scale denoms, widget uses true scales
  mScaleVisibilityGroupBox->setChecked( mLayer->hasScaleBasedVisibility() );
  mScaleRangeWidget->setMapCanvas( QgisApp::instance()->mapCanvas() );

  // get simplify drawing configuration
  const QgsVectorSimplifyMethod& simplifyMethod = mLayer->simplifyMethod();
  mSimplifyDrawingGroupBox->setChecked( simplifyMethod.simplifyHints() != QgsVectorSimplifyMethod::NoSimplification );
  mSimplifyDrawingSpinBox->setValue( simplifyMethod.threshold() );

  QString remark = QString( " (%1)" ).arg( tr( "Not supported" ) );
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
  if ( mLayer->geometryType() == QGis::Point )
  {
    mSimplifyDrawingGroupBox->setChecked( false );
    mSimplifyDrawingGroupBox->setEnabled( false );
  }

  // Default local simplification algorithm
  mSimplifyAlgorithmComboBox->addItem( tr( "Distance" ), ( int )QgsVectorSimplifyMethod::Distance );
  mSimplifyAlgorithmComboBox->addItem( tr( "SnapToGrid" ), ( int )QgsVectorSimplifyMethod::SnapToGrid );
  mSimplifyAlgorithmComboBox->addItem( tr( "Visvalingam" ), ( int )QgsVectorSimplifyMethod::Visvalingam );
  mSimplifyAlgorithmComboBox->setCurrentIndex( mSimplifyAlgorithmComboBox->findData(( int )simplifyMethod.simplifyAlgorithm() ) );

  QStringList myScalesList = PROJECT_SCALES.split( ',' );
  myScalesList.append( "1:1" );
  mSimplifyMaximumScaleComboBox->updateScales( myScalesList );
  mSimplifyMaximumScaleComboBox->setScale( 1.0 / simplifyMethod.maximumScale() );

  mForceRasterCheckBox->setChecked( mLayer->rendererV2() && mLayer->rendererV2()->forceRasterRender() );

  // load appropriate symbology page (V1 or V2)
  updateSymbologyPage();

  mActionDialog->init( *mLayer->actions(), mLayer->attributeTableConfig() );

  if ( labelingDialog )
    labelingDialog->adaptToLayer();

  // reset fields in label dialog
  mLayer->label()->setFields( mLayer->fields() );

  Q_NOWARN_DEPRECATED_PUSH
  if ( mOptsPage_LabelsOld )
  {
    if ( labelDialog && mLayer->hasGeometryType() )
    {
      labelDialog->init();
    }
    labelCheckBox->setChecked( mLayer->hasLabelsEnabled() );
    labelOptionsFrame->setEnabled( mLayer->hasLabelsEnabled() );
    QObject::connect( labelCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( enableLabelOptions( bool ) ) );
  }

  mFieldsPropertiesDialog->init();

  if ( mLayer->hasLabelsEnabled() )
  {
    // though checked on projectRead, can reoccur after applying a style with enabled deprecated labels
    // otherwise, the deprecated labels will render, but the tab to disable them will not show up
    QgsProject::instance()->writeEntry( "DeprecatedLabels", "/Enabled", true );
    // (this also overrides any '/Enabled, false' project property the user may have manually set)
  }
  Q_NOWARN_DEPRECATED_POP

  // delete deprecated labels tab if not already used by project
  // NOTE: this is not ideal, but a quick fix for QGIS 2.0 release
  bool ok;
  bool dl = QgsProject::instance()->readBoolEntry( "DeprecatedLabels", "/Enabled", false, &ok );
  if ( !ok || !dl ) // project not flagged or set to use deprecated labels
  {
    if ( mOptsPage_LabelsOld )
    {
      if ( labelDialog )
      {
        disconnect( labelDialog, SIGNAL( labelSourceSet() ), this, SLOT( setLabelCheckBox() ) );
      }
      delete mOptsPage_LabelsOld;
      mOptsPage_LabelsOld = nullptr;
    }
  }

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
  mLayer->setMaximumScale( 1.0 / mScaleRangeWidget->minimumScale() );
  mLayer->setMinimumScale( 1.0 / mScaleRangeWidget->maximumScale() );

  // provider-specific options
  if ( mLayer->dataProvider() )
  {
    if ( mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::SelectEncoding )
    {
      mLayer->setProviderEncoding( cboProviderEncoding->currentText() );
    }
  }

  // update the display field
  if ( htmlRadio->isChecked() )
  {
    mLayer->setDisplayField( htmlMapTip->toPlainText() );
  }

  if ( fieldComboRadio->isChecked() )
  {
    mLayer->setDisplayField( displayFieldComboBox->currentField() );
  }

  mLayer->actions()->clearActions();
  Q_FOREACH ( const QgsAction& action, mActionDialog->actions() )
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

  Q_NOWARN_DEPRECATED_PUSH
  if ( mOptsPage_LabelsOld )
  {
    if ( labelDialog )
    {
      labelDialog->apply();
    }
    mLayer->enableLabels( labelCheckBox->isChecked() );
  }
  Q_NOWARN_DEPRECATED_POP

  mLayer->setName( mLayerOrigNameLineEdit->text() );

  // Apply fields settings
  mFieldsPropertiesDialog->apply();

  if ( mLayer->rendererV2() )
  {
    QgsRendererV2PropertiesDialog* dlg = static_cast<QgsRendererV2PropertiesDialog*>( widgetStackRenderers->currentWidget() );
    dlg->apply();
  }

  //apply diagram settings
  diagramPropertiesDialog->apply();

  // apply all plugin dialogs
  Q_FOREACH ( QgsMapLayerConfigWidget* page, mLayerPropertiesPages )
  {
    page->apply();
  }

  //layer title and abstract
  mLayer->setShortName( mLayerShortNameLineEdit->text() );
  mLayer->setTitle( mLayerTitleLineEdit->text() );
  mLayer->setAbstract( mLayerAbstractTextEdit->toPlainText() );
  mLayer->setKeywordList( mLayerKeywordListLineEdit->text() );
  mLayer->setDataUrl( mLayerDataUrlLineEdit->text() );
  mLayer->setDataUrlFormat( mLayerDataUrlFormatComboBox->currentText() );
  //layer attribution and metadataUrl
  mLayer->setAttribution( mLayerAttributionLineEdit->text() );
  mLayer->setAttributionUrl( mLayerAttributionUrlLineEdit->text() );
  mLayer->setMetadataUrl( mLayerMetadataUrlLineEdit->text() );
  mLayer->setMetadataUrlType( mLayerMetadataUrlTypeComboBox->currentText() );
  mLayer->setMetadataUrlFormat( mLayerMetadataUrlFormatComboBox->currentText() );
  mLayer->setLegendUrl( mLayerLegendUrlLineEdit->text() );
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
  simplifyMethod.setSimplifyAlgorithm( static_cast< QgsVectorSimplifyMethod::SimplifyAlgorithm >( mSimplifyAlgorithmComboBox->itemData( mSimplifyAlgorithmComboBox->currentIndex() ).toInt() ) );
  simplifyMethod.setThreshold( mSimplifyDrawingSpinBox->value() );
  simplifyMethod.setForceLocalOptimization( !mSimplifyDrawingAtProvider->isChecked() );
  simplifyMethod.setMaximumScale( 1.0 / mSimplifyMaximumScaleComboBox->scale() );
  mLayer->setSimplifyMethod( simplifyMethod );

  if ( mLayer->rendererV2() )
    mLayer->rendererV2()->setForceRasterRender( mForceRasterCheckBox->isChecked() );

  mOldJoins = mLayer->vectorJoins();

  //save variables
  QgsExpressionContextUtils::setLayerVariables( mLayer, mVariableEditor->variablesInActiveScope() );
  updateVariableEditor();

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

    Q_FOREACH ( const QgsVectorJoinInfo& info, mLayer->vectorJoins() )
      mLayer->removeJoin( info.joinLayerId );

    Q_FOREACH ( const QgsVectorJoinInfo& info, mOldJoins )
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
    QDomDocument doc( "qgis" );
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
  QgsVectorDataProvider* pr = mLayer->dataProvider();
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

QString QgsVectorLayerProperties::metadata()
{
  return mLayer->metadata();
}

void QgsVectorLayerProperties::on_mLayerOrigNameLineEdit_textEdited( const QString& text )
{
  txtDisplayName->setText( mLayer->capitaliseLayerName( text ) );
}

void QgsVectorLayerProperties::on_mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem& crs )
{
  mLayer->setCrs( crs );
}

void QgsVectorLayerProperties::loadDefaultStyle_clicked()
{
  QString msg;
  bool defaultLoadedFlag = false;

  if ( mLayer->dataProvider()->isSaveAndLoadStyleToDBSupported() )
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
  if ( mLayer->dataProvider()->isSaveAndLoadStyleToDBSupported() )
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
        mLayer->saveStyleToDatabase( "", "", true, "", errorMsg );
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
  QSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( "style/lastStyleDir", QDir::homePath() ).toString();

  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load layer properties from style file" ), myLastUsedDir,
                       tr( "QGIS Layer Style File" ) + " (*.qml);;" + tr( "SLD File" ) + " (*.sld)" );
  if ( myFileName.isNull() )
  {
    return;
  }

  mOldStyle = mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() );

  QString myMessage;
  bool defaultLoadedFlag = false;

  if ( myFileName.endsWith( ".sld", Qt::CaseInsensitive ) )
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
  myQSettings.setValue( "style/lastStyleDir", myPath );

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
  QSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( "style/lastStyleDir", QDir::homePath() ).toString();

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
        QMessageBox::warning( this, infoWindowTitle, msgError );
      }
      else
      {
        QMessageBox::information( this, infoWindowTitle, tr( "Style saved" ) );
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
      extension = ".sld";
    }
    else
    {
      format = tr( "QGIS Layer Style File" ) + " (*.qml)";
      extension = ".qml";
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
    myQSettings.setValue( "style/lastStyleDir", myPath );
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

  QMenu* m = qobject_cast<QMenu*>( sender() );
  if ( !m )
    return;

  // first get rid of previously added style manager actions (they are dynamic)
  bool gotFirstSeparator = false;
  QList<QAction*> actions = m->actions();
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
    Q_NOWARN_DEPRECATED_PUSH
    if ( mLayer->applyNamedStyle( qmlStyle, errorMsg ) )
    {
      syncToLayer();
    }
    else
    {
      QMessageBox::warning( this, tr( "Error occurred retrieving styles from database" ),
                            tr( "The retrieved style is not a valid named style. Error message: %1" )
                            .arg( errorMsg ) );
    }
    Q_NOWARN_DEPRECATED_POP
  }
}

void QgsVectorLayerProperties::on_mButtonAddJoin_clicked()
{
  if ( !mLayer )
    return;

  QList<QgsMapLayer*> joinedLayers;
  const QList< QgsVectorJoinInfo >& joins = mLayer->vectorJoins();
  for ( int i = 0; i < joins.size(); ++i )
  {
    joinedLayers.append( QgsMapLayerRegistry::instance()->mapLayer( joins[i].joinLayerId ) );
  }

  QgsJoinDialog d( mLayer, joinedLayers );
  if ( d.exec() == QDialog::Accepted )
  {
    QgsVectorJoinInfo info = d.joinInfo();
    //create attribute index if possible
    if ( d.createAttributeIndex() )
    {
      QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( info.joinLayerId ) );
      if ( joinLayer )
      {
        joinLayer->dataProvider()->createAttributeIndex( joinLayer->fields().indexFromName( info.joinFieldName ) );
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
  QTreeWidgetItem* currentJoinItem = mJoinTreeWidget->currentItem();
  on_mJoinTreeWidget_itemDoubleClicked( currentJoinItem, 0 );
}

void QgsVectorLayerProperties::on_mJoinTreeWidget_itemDoubleClicked( QTreeWidgetItem* item, int )
{
  if ( !mLayer || !item )
  {
    return;
  }

  QList<QgsMapLayer*> joinedLayers;
  QString joinLayerId = item->data( 0, Qt::UserRole ).toString();
  const QList< QgsVectorJoinInfo >& joins = mLayer->vectorJoins();
  int j = -1;
  for ( int i = 0; i < joins.size(); ++i )
  {
    if ( joins[i].joinLayerId == joinLayerId )
    {
      j = i;
    }
    else
    {
      // remove already joined layers from possible list to be displayed in dialog
      joinedLayers.append( QgsMapLayerRegistry::instance()->mapLayer( joins[i].joinLayerId ) );
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
    QgsVectorJoinInfo info = d.joinInfo();

    // remove old join
    mLayer->removeJoin( joinLayerId );
    int idx = mJoinTreeWidget->indexOfTopLevelItem( item );
    mJoinTreeWidget->takeTopLevelItem( idx );

    // add the new edited

    //create attribute index if possible
    if ( d.createAttributeIndex() )
    {
      QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( info.joinLayerId ) );
      if ( joinLayer )
      {
        joinLayer->dataProvider()->createAttributeIndex( joinLayer->fields().indexFromName( info.joinFieldName ) );
      }
    }
    mLayer->addJoin( info );
    addJoinToTreeWidget( info, idx );

    setPbnQueryBuilderEnabled();
    mFieldsPropertiesDialog->init();
  }
}

void QgsVectorLayerProperties::addJoinToTreeWidget( const QgsVectorJoinInfo& join, const int insertIndex )
{
  QTreeWidgetItem* joinItem = new QTreeWidgetItem();

  QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( join.joinLayerId ) );
  if ( !mLayer || !joinLayer )
  {
    return;
  }

  joinItem->setText( 0, joinLayer->name() );
  joinItem->setData( 0, Qt::UserRole, join.joinLayerId );

  if ( join.joinFieldName.isEmpty() && join.joinFieldIndex >= 0 && join.joinFieldIndex < joinLayer->fields().count() )
  {
    joinItem->setText( 1, joinLayer->fields().field( join.joinFieldIndex ).name() );   //for compatibility with 1.x
  }
  else
  {
    joinItem->setText( 1, join.joinFieldName );
  }

  if ( join.targetFieldName.isEmpty() && join.targetFieldIndex >= 0 && join.targetFieldIndex < mLayer->fields().count() )
  {
    joinItem->setText( 2, mLayer->fields().field( join.targetFieldIndex ).name() );   //for compatibility with 1.x
  }
  else
  {
    joinItem->setText( 2, join.targetFieldName );
  }

  if ( join.memoryCache )
  {
    joinItem->setText( 3, QChar( 0x2714 ) );
  }

  joinItem->setText( 4, join.prefix );

  const QStringList* list = join.joinFieldNamesSubset();
  if ( list )
  {
    joinItem->setText( 5, QString( "%1" ).arg( list->count() ) );
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

void QgsVectorLayerProperties::openPanel( QgsPanelWidget *panel )
{
  QDialog* dlg = new QDialog();
  QString key =  QString( "/UI/paneldialog/%1" ).arg( panel->panelTitle() );
  QSettings settings;
  dlg->restoreGeometry( settings.value( key ).toByteArray() );
  dlg->setWindowTitle( panel->panelTitle() );
  dlg->setLayout( new QVBoxLayout() );
  dlg->layout()->addWidget( panel );
  QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok );
  connect( buttonBox, SIGNAL( accepted() ), dlg, SLOT( accept() ) );
  dlg->layout()->addWidget( buttonBox );
  dlg->exec();
  settings.setValue( key, dlg->saveGeometry() );
  panel->acceptPanel();
}

void QgsVectorLayerProperties::on_mButtonRemoveJoin_clicked()
{
  QTreeWidgetItem* currentJoinItem = mJoinTreeWidget->currentItem();
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

  if ( mLayer->rendererV2() )
  {
    mRendererDialog = new QgsRendererV2PropertiesDialog( mLayer, QgsStyleV2::defaultStyle(), true, this );
    mRendererDialog->setDockMode( false );
    mRendererDialog->setMapCanvas( QgisApp::instance()->mapCanvas() );
    connect( mRendererDialog, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SLOT( openPanel( QgsPanelWidget* ) ) );
    connect( mRendererDialog, SIGNAL( layerVariablesChanged() ), this, SLOT( updateVariableEditor() ) );

    // display the menu to choose the output format (fix #5136)
    mActionSaveStyleAs->setText( tr( "Save Style" ) );
    mActionSaveStyleAs->setMenu( mSaveAsMenu );
    disconnect( mActionSaveStyleAs, SIGNAL( triggered() ), this, SLOT( saveStyleAs_clicked() ) );
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
  if ( indx != mOptStackedWidget->indexOf( mOptsPage_Metadata ) || mMetadataFilled )
    return;

  //set the metadata contents (which can be expensive)
  QString myStyle = QgsApplication::reportStyleSheet();
  teMetadata->clear();
  teMetadata->document()->setDefaultStyleSheet( myStyle );
  teMetadata->setHtml( metadata() );
  mMetadataFilled = true;
}

void QgsVectorLayerProperties::enableLabelOptions( bool theFlag )
{
  labelOptionsFrame->setEnabled( theFlag );
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
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::projectScope() );
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::layerScope( mLayer ) );
  mVariableEditor->reloadContext();
  mVariableEditor->setEditableScopeIndex( 2 );
}

void QgsVectorLayerProperties::updateFieldsPropertiesDialog()
{
  QgsEditFormConfig* cfg = mLayer->editFormConfig();
  mFieldsPropertiesDialog->setEditFormInit( cfg->uiForm(), cfg->initFunction(), cfg->initCode(), cfg->initFilePath(), cfg->initCodeSource() );
}
