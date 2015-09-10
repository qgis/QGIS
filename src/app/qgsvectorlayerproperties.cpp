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
#include "qgslabelinggui.h"
#include "qgslabel.h"
#include "qgsgenericprojectionselector.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsmaplayerstylemanager.h"
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
    , layer( lyr )
    , mMetadataFilled( false )
    , mSaveAsMenu( 0 )
    , mLoadStyleMenu( 0 )
    , mRendererDialog( 0 )
    , labelingDialog( 0 )
    , labelDialog( 0 )
    , actionDialog( 0 )
    , diagramPropertiesDialog( 0 )
    , mFieldsPropertiesDialog( 0 )
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

  connect( insertFieldButton, SIGNAL( clicked() ), this, SLOT( insertField() ) );
  connect( insertExpressionButton, SIGNAL( clicked() ), this, SLOT( insertExpression() ) );

  // connections for Map Tip display
  connect( htmlRadio, SIGNAL( toggled( bool ) ), htmlMapTip, SLOT( setEnabled( bool ) ) );
  connect( htmlRadio, SIGNAL( toggled( bool ) ), insertFieldButton, SLOT( setEnabled( bool ) ) );
  connect( htmlRadio, SIGNAL( toggled( bool ) ), fieldComboBox, SLOT( setEnabled( bool ) ) );
  connect( htmlRadio, SIGNAL( toggled( bool ) ), insertExpressionButton, SLOT( setEnabled( bool ) ) );
  connect( fieldComboRadio, SIGNAL( toggled( bool ) ), displayFieldComboBox, SLOT( setEnabled( bool ) ) );

  if ( !layer )
    return;

  QVBoxLayout *layout;

  if ( layer->hasGeometryType() )
  {
    // Create the Labeling dialog tab
    layout = new QVBoxLayout( labelingFrame );
    layout->setMargin( 0 );
    labelingDialog = new QgsLabelingGui( layer, QgisApp::instance()->mapCanvas(), labelingFrame );
    labelingDialog->layout()->setContentsMargins( -1, 0, -1, 0 );
    layout->addWidget( labelingDialog );
    labelingFrame->setLayout( layout );

    // Create the Labeling (deprecated) dialog tab
    layout = new QVBoxLayout( labelOptionsFrame );
    layout->setMargin( 0 );
    labelDialog = new QgsLabelDialog( layer->label(), labelOptionsFrame );
    labelDialog->layout()->setMargin( 0 );
    layout->addWidget( labelDialog );
    labelOptionsFrame->setLayout( layout );
    connect( labelDialog, SIGNAL( labelSourceSet() ), this, SLOT( setLabelCheckBox() ) );
  }
  else
  {
    labelingDialog = 0;
    labelDialog = 0;
    mOptsPage_Labels->setEnabled( false ); // disable labeling item
    mOptsPage_LabelsOld->setEnabled( false ); // disable labeling (deprecated) item
  }

  // Create the Actions dialog tab
  QVBoxLayout *actionLayout = new QVBoxLayout( actionOptionsFrame );
  actionLayout->setMargin( 0 );
  const QgsFields &fields = layer->fields();
  actionDialog = new QgsAttributeActionDialog( layer->actions(), fields, actionOptionsFrame );
  actionDialog->layout()->setMargin( 0 );
  actionLayout->addWidget( actionDialog );

  // Create the menu for the save style button to choose the output format
  mSaveAsMenu = new QMenu( this );
  mSaveAsMenu->addAction( tr( "QGIS Layer Style File..." ) );
  mSaveAsMenu->addAction( tr( "SLD File..." ) );

  //Only if the provider support loading & saving styles to db add new choices
  if ( layer->dataProvider()->isSaveAndLoadStyleToDBSupported() )
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
    mSaveAsMenu->addAction( tr( "Save in database (%1)" ).arg( layer->providerType() ) );
  }

  QObject::connect( mSaveAsMenu, SIGNAL( triggered( QAction * ) ),
                    this, SLOT( saveStyleAsMenuTriggered( QAction * ) ) );

  mFieldsPropertiesDialog = new QgsFieldsProperties( layer, mFieldsFrame );
  mFieldsPropertiesDialog->layout()->setMargin( 0 );
  mFieldsFrame->setLayout( new QVBoxLayout( mFieldsFrame ) );
  mFieldsFrame->layout()->setMargin( 0 );
  mFieldsFrame->layout()->addWidget( mFieldsPropertiesDialog );

  connect( mFieldsPropertiesDialog, SIGNAL( toggleEditing() ), this, SLOT( toggleEditing() ) );
  connect( this, SIGNAL( toggleEditing( QgsMapLayer* ) ), QgisApp::instance(), SLOT( toggleEditing( QgsMapLayer* ) ) );

  syncToLayer();

  if ( layer->dataProvider() )//enable spatial index button group if supported by provider
  {
    int capabilities = layer->dataProvider()->capabilities();
    if ( !( capabilities & QgsVectorDataProvider::CreateSpatialIndex ) )
    {
      pbnIndex->setEnabled( false );
    }

    if ( capabilities & QgsVectorDataProvider::SelectEncoding )
    {
      cboProviderEncoding->addItems( QgsVectorDataProvider::availableEncodings() );
      QString enc = layer->dataProvider()->encoding();
      int encindex = cboProviderEncoding->findText( enc );
      if ( encindex < 0 )
      {
        cboProviderEncoding->insertItem( 0, enc );
        encindex = 0;
      }
      cboProviderEncoding->setCurrentIndex( encindex );
    }
    else if ( layer->dataProvider()->name() == "ogr" )
    {
      // if OGR_L_TestCapability(OLCStringsAsUTF8) returns true, OGR provider encoding can be set to only UTF-8
      // so make encoding box grayed out
      cboProviderEncoding->addItem( layer->dataProvider()->encoding() );
      cboProviderEncoding->setEnabled( false );
    }
    else
    {
      // other providers do not use mEncoding, so hide the group completely
      mDataSourceEncodingFrame->hide();
    }
  }

  mCrsSelector->setCrs( layer->crs() );

  //insert existing join info
  const QList< QgsVectorJoinInfo >& joins = layer->vectorJoins();
  for ( int i = 0; i < joins.size(); ++i )
  {
    addJoinToTreeWidget( joins[i] );
  }

  mOldJoins = layer->vectorJoins();

  QVBoxLayout* diagLayout = new QVBoxLayout( mDiagramFrame );
  diagLayout->setMargin( 0 );
  diagramPropertiesDialog = new QgsDiagramProperties( layer, mDiagramFrame );
  diagramPropertiesDialog->layout()->setContentsMargins( -1, 0, -1, 0 );
  diagLayout->addWidget( diagramPropertiesDialog );
  mDiagramFrame->setLayout( diagLayout );


  //layer title and abstract
  mLayerTitleLineEdit->setText( layer->title() );
  mLayerAbstractTextEdit->setPlainText( layer->abstract() );
  mLayerKeywordListLineEdit->setText( layer->keywordList() );
  mLayerDataUrlLineEdit->setText( layer->dataUrl() );
  mLayerDataUrlFormatComboBox->setCurrentIndex(
    mLayerDataUrlFormatComboBox->findText(
      layer->dataUrlFormat()
    )
  );
  //layer attribution and metadataUrl
  mLayerAttributionLineEdit->setText( layer->attribution() );
  mLayerAttributionUrlLineEdit->setText( layer->attributionUrl() );
  mLayerMetadataUrlLineEdit->setText( layer->metadataUrl() );
  mLayerMetadataUrlTypeComboBox->setCurrentIndex(
    mLayerMetadataUrlTypeComboBox->findText(
      layer->metadataUrlType()
    )
  );
  mLayerMetadataUrlFormatComboBox->setCurrentIndex(
    mLayerMetadataUrlFormatComboBox->findText(
      layer->metadataUrlFormat()
    )
  );
  mLayerLegendUrlLineEdit->setText( layer->legendUrl() );
  mLayerLegendUrlFormatComboBox->setCurrentIndex(
    mLayerLegendUrlFormatComboBox->findText(
      layer->legendUrlFormat()
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

  QString title = QString( tr( "Layer Properties - %1" ) ).arg( layer->name() );
  restoreOptionsBaseUi( title );
} // QgsVectorLayerProperties ctor


QgsVectorLayerProperties::~QgsVectorLayerProperties()
{
  if ( mOptsPage_LabelsOld && labelDialog && layer->hasGeometryType() )
  {
    disconnect( labelDialog, SIGNAL( labelSourceSet() ), this, SLOT( setLabelCheckBox() ) );
  }
}

void QgsVectorLayerProperties::toggleEditing()
{
  if ( !layer )
    return;

  emit toggleEditing( layer );

  pbnQueryBuilder->setEnabled( layer->dataProvider() && layer->dataProvider()->supportsSubsetString() &&
                               !layer->isEditable() );
  if ( layer->isEditable() )
  {
    pbnQueryBuilder->setToolTip( tr( "Stop editing mode to enable this." ) );
  }
}

void QgsVectorLayerProperties::setLabelCheckBox()
{
  labelCheckBox->setCheckState( Qt::Checked );
}

void QgsVectorLayerProperties::insertField()
{
  // Convert the selected field to an expression and
  // insert it into the action at the cursor position

  if ( !fieldComboBox->currentText().isNull() )
  {
    QString field = "[% \"";
    field += fieldComboBox->currentText();
    field += "\" %]";
    htmlMapTip->insertPlainText( field );
  }
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
  << QgsExpressionContextUtils::atlasScope( 0 )
  << QgsExpressionContextUtils::mapSettingsScope( QgisApp::instance()->mapCanvas()->mapSettings() )
  << QgsExpressionContextUtils::layerScope( layer );

  QgsExpressionBuilderDialog dlg( layer, selText.replace( QChar::ParagraphSeparator, '\n' ), this, "generic", context );
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

void QgsVectorLayerProperties::setDisplayField( QString name )
{
  int idx = displayFieldComboBox->findText( name );
  if ( idx == -1 )
  {
    htmlRadio->setChecked( true );
    htmlMapTip->setPlainText( name );
  }
  else
  {
    fieldComboRadio->setChecked( true );
    displayFieldComboBox->setCurrentIndex( idx );
  }
}

//! @note in raster props, this method is called sync()
void QgsVectorLayerProperties::syncToLayer( void )
{
  // populate the general information
  mLayerOrigNameLineEdit->setText( layer->originalName() );
  txtDisplayName->setText( layer->name() );
  txtLayerSource->setText( layer->publicSource() );
  pbnQueryBuilder->setWhatsThis( tr( "This button opens the query "
                                     "builder and allows you to create a subset of features to display on "
                                     "the map canvas rather than displaying all features in the layer" ) );
  txtSubsetSQL->setWhatsThis( tr( "The query used to limit the features in the "
                                  "layer is shown here. To enter or modify the query, click on the Query Builder button" ) );

  //see if we are dealing with a pg layer here
  mSubsetGroupBox->setEnabled( true );
  txtSubsetSQL->setText( layer->subsetString() );
  // if the user is allowed to type an adhoc query, the app will crash if the query
  // is bad. For this reason, the sql box is disabled and the query must be built
  // using the query builder, either by typing it in by hand or using the buttons, etc
  // on the builder. If the ability to enter a query directly into the box is required,
  // a mechanism to check it must be implemented.
  txtSubsetSQL->setEnabled( false );
  pbnQueryBuilder->setEnabled( layer->dataProvider() && layer->dataProvider()->supportsSubsetString() &&
                               !layer->isEditable() );
  if ( layer->isEditable() )
  {
    pbnQueryBuilder->setToolTip( tr( "Stop editing mode to enable this." ) );
  }

  //get field list for display field combo
  const QgsFields& myFields = layer->fields();
  for ( int idx = 0; idx < myFields.count(); ++idx )
  {
    displayFieldComboBox->addItem( myFields[idx].name() );
    fieldComboBox->addItem( myFields[idx].name() );
  }

  setDisplayField( layer->displayField() );

  // set up the scale based layer visibility stuff....
  mScaleRangeWidget->setScaleRange( 1.0 / layer->maximumScale(), 1.0 / layer->minimumScale() ); // caution: layer uses scale denoms, widget uses true scales
  mScaleVisibilityGroupBox->setChecked( layer->hasScaleBasedVisibility() );
  mScaleRangeWidget->setMapCanvas( QgisApp::instance()->mapCanvas() );

  // get simplify drawing configuration
  const QgsVectorSimplifyMethod& simplifyMethod = layer->simplifyMethod();
  mSimplifyDrawingGroupBox->setChecked( simplifyMethod.simplifyHints() != QgsVectorSimplifyMethod::NoSimplification );
  mSimplifyDrawingSpinBox->setValue( simplifyMethod.threshold() );

  if ( !( layer->dataProvider()->capabilities() & QgsVectorDataProvider::SimplifyGeometries ) )
  {
    mSimplifyDrawingAtProvider->setChecked( false );
    mSimplifyDrawingAtProvider->setEnabled( false );
    mSimplifyDrawingAtProvider->setText( QString( "%1 (%2)" ).arg( mSimplifyDrawingAtProvider->text(), tr( "Not supported" ) ) );
  }
  else
  {
    mSimplifyDrawingAtProvider->setChecked( !simplifyMethod.forceLocalOptimization() );
    mSimplifyDrawingAtProvider->setEnabled( mSimplifyDrawingGroupBox->isChecked() );
  }

  // disable simplification for point layers, now it is not implemented
  if ( layer->geometryType() == QGis::Point )
  {
    mSimplifyDrawingGroupBox->setChecked( false );
    mSimplifyDrawingGroupBox->setEnabled( false );
  }

  QStringList myScalesList = PROJECT_SCALES.split( "," );
  myScalesList.append( "1:1" );
  mSimplifyMaximumScaleComboBox->updateScales( myScalesList );
  mSimplifyMaximumScaleComboBox->setScale( 1.0 / simplifyMethod.maximumScale() );

  mForceRasterCheckBox->setChecked( layer->rendererV2() && layer->rendererV2()->forceRasterRender() );

  // load appropriate symbology page (V1 or V2)
  updateSymbologyPage();

  actionDialog->init();

  // reset fields in label dialog
  layer->label()->setFields( layer->fields() );

  if ( layer->hasGeometryType() )
  {
    labelingDialog->init();
  }

  Q_NOWARN_DEPRECATED_PUSH
  if ( mOptsPage_LabelsOld )
  {
    if ( labelDialog && layer->hasGeometryType() )
    {
      labelDialog->init();
    }
    labelCheckBox->setChecked( layer->hasLabelsEnabled() );
    labelOptionsFrame->setEnabled( layer->hasLabelsEnabled() );
    QObject::connect( labelCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( enableLabelOptions( bool ) ) );
  }

  mFieldsPropertiesDialog->init();

  if ( layer->hasLabelsEnabled() )
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
  if ( !ok || ( ok && !dl ) ) // project not flagged or set to use deprecated labels
  {
    if ( mOptsPage_LabelsOld )
    {
      if ( labelDialog )
      {
        disconnect( labelDialog, SIGNAL( labelSourceSet() ), this, SLOT( setLabelCheckBox() ) );
      }
      delete mOptsPage_LabelsOld;
      mOptsPage_LabelsOld = 0;
    }
  }

  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::globalScope() );
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::projectScope() );
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::layerScope( layer ) );
  mVariableEditor->reloadContext();
  mVariableEditor->setEditableScopeIndex( 2 );

} // syncToLayer()



void QgsVectorLayerProperties::apply()
{
  if ( labelingDialog )
  {
    labelingDialog->writeSettingsToLayer();
  }

  //
  // Set up sql subset query if applicable
  //
  mSubsetGroupBox->setEnabled( true );

  if ( txtSubsetSQL->toPlainText() != layer->subsetString() )
  {
    // set the subset sql for the layer
    layer->setSubsetString( txtSubsetSQL->toPlainText() );
    mMetadataFilled = false;
  }

  // set up the scale based layer visibility stuff....
  layer->setScaleBasedVisibility( mScaleVisibilityGroupBox->isChecked() );
  // caution: layer uses scale denoms, widget uses true scales
  layer->setMaximumScale( 1.0 / mScaleRangeWidget->minimumScale() );
  layer->setMinimumScale( 1.0 / mScaleRangeWidget->maximumScale() );

  // provider-specific options
  if ( layer->dataProvider() )
  {
    if ( layer->dataProvider()->capabilities() & QgsVectorDataProvider::SelectEncoding )
    {
      layer->setProviderEncoding( cboProviderEncoding->currentText() );
    }
  }

  // update the display field
  if ( htmlRadio->isChecked() )
  {
    layer->setDisplayField( htmlMapTip->toPlainText() );
  }

  if ( fieldComboRadio->isChecked() )
  {
    layer->setDisplayField( displayFieldComboBox->currentText() );
  }

  actionDialog->apply();

  Q_NOWARN_DEPRECATED_PUSH
  if ( mOptsPage_LabelsOld )
  {
    if ( labelDialog )
    {
      labelDialog->apply();
    }
    layer->enableLabels( labelCheckBox->isChecked() );
  }
  Q_NOWARN_DEPRECATED_POP

  layer->setLayerName( mLayerOrigNameLineEdit->text() );

  // Apply fields settings
  mFieldsPropertiesDialog->apply();

  if ( layer->rendererV2() )
  {
    QgsRendererV2PropertiesDialog* dlg = static_cast<QgsRendererV2PropertiesDialog*>( widgetStackRenderers->currentWidget() );
    dlg->apply();
  }

  //apply diagram settings
  diagramPropertiesDialog->apply();

  //layer title and abstract
  layer->setTitle( mLayerTitleLineEdit->text() );
  layer->setAbstract( mLayerAbstractTextEdit->toPlainText() );
  layer->setKeywordList( mLayerKeywordListLineEdit->text() );
  layer->setDataUrl( mLayerDataUrlLineEdit->text() );
  layer->setDataUrlFormat( mLayerDataUrlFormatComboBox->currentText() );
  //layer attribution and metadataUrl
  layer->setAttribution( mLayerAttributionLineEdit->text() );
  layer->setAttributionUrl( mLayerAttributionUrlLineEdit->text() );
  layer->setMetadataUrl( mLayerMetadataUrlLineEdit->text() );
  layer->setMetadataUrlType( mLayerMetadataUrlTypeComboBox->currentText() );
  layer->setMetadataUrlFormat( mLayerMetadataUrlFormatComboBox->currentText() );
  layer->setLegendUrl( mLayerLegendUrlLineEdit->text() );
  layer->setLegendUrlFormat( mLayerLegendUrlFormatComboBox->currentText() );

  //layer simplify drawing configuration
  QgsVectorSimplifyMethod::SimplifyHints simplifyHints = QgsVectorSimplifyMethod::NoSimplification;
  if ( mSimplifyDrawingGroupBox->isChecked() )
  {
    simplifyHints |= QgsVectorSimplifyMethod::GeometrySimplification;
    if ( mSimplifyDrawingSpinBox->value() > 1 ) simplifyHints |= QgsVectorSimplifyMethod::AntialiasingSimplification;
  }
  QgsVectorSimplifyMethod simplifyMethod = layer->simplifyMethod();
  simplifyMethod.setSimplifyHints( simplifyHints );
  simplifyMethod.setThreshold( mSimplifyDrawingSpinBox->value() );
  simplifyMethod.setForceLocalOptimization( !mSimplifyDrawingAtProvider->isChecked() );
  simplifyMethod.setMaximumScale( 1.0 / mSimplifyMaximumScaleComboBox->scale() );
  layer->setSimplifyMethod( simplifyMethod );

  if ( layer->rendererV2() )
    layer->rendererV2()->setForceRasterRender( mForceRasterCheckBox->isChecked() );

  mOldJoins = layer->vectorJoins();

  //save variables
  QgsExpressionContextUtils::setLayerVariables( layer, mVariableEditor->variablesInActiveScope() );

  // update symbology
  emit refreshLegend( layer->id() );

  layer->triggerRepaint();
  // notify the project we've made a change
  QgsProject::instance()->dirty( true );
}

void QgsVectorLayerProperties::onCancel()
{
  if ( mOldJoins != layer->vectorJoins() )
  {
    // need to undo changes in vector layer joins - they are applied directly to the layer (not in apply())
    // so other parts of the properties dialog can use the fields from the joined layers

    Q_FOREACH ( const QgsVectorJoinInfo& info, layer->vectorJoins() )
      layer->removeJoin( info.joinLayerId );

    Q_FOREACH ( const QgsVectorJoinInfo& info, mOldJoins )
      layer->addJoin( info );
  }
}

void QgsVectorLayerProperties::on_pbnQueryBuilder_clicked()
{
  // launch the query builder
  QgsQueryBuilder *qb = new QgsQueryBuilder( layer, this );

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
  QgsVectorDataProvider* pr = layer->dataProvider();
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
  return layer->metadata();
}

void QgsVectorLayerProperties::on_mLayerOrigNameLineEdit_textEdited( const QString& text )
{
  txtDisplayName->setText( layer->capitaliseLayerName( text ) );
}

void QgsVectorLayerProperties::on_mCrsSelector_crsChanged( QgsCoordinateReferenceSystem crs )
{
  layer->setCrs( crs );
}

void QgsVectorLayerProperties::loadDefaultStyle_clicked()
{
  QString msg;
  bool defaultLoadedFlag = false;

  if ( layer->dataProvider()->isSaveAndLoadStyleToDBSupported() )
  {
    QMessageBox askToUser;
    askToUser.setText( tr( "Load default style from: " ) );
    askToUser.setIcon( QMessageBox::Question );
    askToUser.addButton( tr( "Cancel" ), QMessageBox::RejectRole );
    askToUser.addButton( tr( "Local database" ), QMessageBox::NoRole );
    askToUser.addButton( tr( "Datasource database" ), QMessageBox::YesRole );

    switch ( askToUser.exec() )
    {
      case( 0 ):
        return;
        break;
      case( 2 ):
        msg = layer->loadNamedStyle( layer->styleURI(), defaultLoadedFlag );
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
        break;
      default:
        break;
    }
  }

  QString myMessage = layer->loadNamedStyle( layer->styleURI(), defaultLoadedFlag, true );
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
  if ( layer->dataProvider()->isSaveAndLoadStyleToDBSupported() )
  {
    QMessageBox askToUser;
    askToUser.setText( tr( "Save default style to: " ) );
    askToUser.setIcon( QMessageBox::Question );
    askToUser.addButton( tr( "Cancel" ), QMessageBox::RejectRole );
    askToUser.addButton( tr( "Local database" ), QMessageBox::NoRole );
    askToUser.addButton( tr( "Datasource database" ), QMessageBox::YesRole );

    switch ( askToUser.exec() )
    {
      case( 0 ):
        return;
        break;
      case( 2 ):
        layer->saveStyleToDatabase( "", "", true, "", errorMsg );
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
  errorMsg = layer->saveDefaultStyle( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    QMessageBox::warning( this, tr( "Default Style" ), errorMsg );
  }
}


void QgsVectorLayerProperties::loadStyle_clicked()
{
  QSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( "style/lastStyleDir", "." ).toString();

  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load layer properties from style file" ), myLastUsedDir,
                       tr( "QGIS Layer Style File" ) + " (*.qml);;" + tr( "SLD File" ) + " (*.sld)" );
  if ( myFileName.isNull() )
  {
    return;
  }

  QString myMessage;
  bool defaultLoadedFlag = false;

  if ( myFileName.endsWith( ".sld", Qt::CaseInsensitive ) )
  {
    // load from SLD
    myMessage = layer->loadSldStyle( myFileName, defaultLoadedFlag );
  }
  else
  {
    myMessage = layer->loadNamedStyle( myFileName, defaultLoadedFlag );
  }
  //reset if the default style was loaded ok only
  if ( defaultLoadedFlag )
  {
    syncToLayer();
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::information( this, tr( "Load Style" ), myMessage );
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

  saveStyleAs(( StyleType ) index );
}

void QgsVectorLayerProperties::saveStyleAs( StyleType styleType )
{
  QSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( "style/lastStyleDir", "." ).toString();

  QString format, extension;
  if ( styleType == DB )
  {
    QString infoWindowTitle = QObject::tr( "Save style to DB (%1)" ).arg( layer->providerType() );
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

      layer->saveStyleToDatabase( styleName, styleDesc, isDefault, uiFileContent, msgError );

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
      myMessage = layer->saveSldStyle( myOutputFileName, defaultLoadedFlag );
    }
    else
    {
      myMessage = layer->saveNamedStyle( myOutputFileName, defaultLoadedFlag );
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
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, layer );
}

void QgsVectorLayerProperties::showListOfStylesFromDatabase()
{
  QString errorMsg;
  QStringList ids, names, descriptions;

  //get the list of styles in the db
  int sectionLimit = layer->listStylesInDatabase( ids, names, descriptions, errorMsg );
  if ( !errorMsg.isNull() )
  {
    QMessageBox::warning( this, tr( "Error occured retrieving styles from database" ), errorMsg );
    return;
  }

  QgsLoadStyleFromDBDialog dialog;
  dialog.initializeLists( ids, names, descriptions, sectionLimit );

  if ( dialog.exec() == QDialog::Accepted )
  {
    QString selectedStyleId = dialog.getSelectedStyleId();

    QString qmlStyle = layer->getStyleFromDatabase( selectedStyleId, errorMsg );
    if ( !errorMsg.isNull() )
    {
      QMessageBox::warning( this, tr( "Error occured retrieving styles from database" ), errorMsg );
      return;
    }
    if ( layer->applyNamedStyle( qmlStyle, errorMsg ) )
    {
      syncToLayer();
    }
    else
    {
      QMessageBox::warning( this, tr( "Error occured retrieving styles from database" ),
                            tr( "The retrieved style is not a valid named style. Error message: %1" )
                            .arg( errorMsg ) );
    }

  }
}

void QgsVectorLayerProperties::on_mButtonAddJoin_clicked()
{
  if ( !layer )
    return;

  QList<QgsMapLayer*> joinedLayers;
  const QList< QgsVectorJoinInfo >& joins = layer->vectorJoins();
  for ( int i = 0; i < joins.size(); ++i )
  {
    joinedLayers.append( QgsMapLayerRegistry::instance()->mapLayer( joins[i].joinLayerId ) );
  }

  QgsJoinDialog d( layer, joinedLayers );
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
    layer->addJoin( info );
    addJoinToTreeWidget( info );
    pbnQueryBuilder->setEnabled( layer && layer->dataProvider() && layer->dataProvider()->supportsSubsetString() &&
                                 !layer->isEditable() );
    mFieldsPropertiesDialog->init();
  }
}

void QgsVectorLayerProperties::on_mButtonEditJoin_clicked()
{
  QTreeWidgetItem* currentJoinItem = mJoinTreeWidget->currentItem();
  if ( !layer || !currentJoinItem )
  {
    return;
  }

  QList<QgsMapLayer*> joinedLayers;
  QString joinLayerId = currentJoinItem->data( 0, Qt::UserRole ).toString();
  const QList< QgsVectorJoinInfo >& joins = layer->vectorJoins();
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

  QgsJoinDialog d( layer, joinedLayers );
  d.setJoinInfo( joins[j] );

  if ( d.exec() == QDialog::Accepted )
  {
    QgsVectorJoinInfo info = d.joinInfo();

    // remove old join
    layer->removeJoin( joinLayerId );
    int idx = mJoinTreeWidget->indexOfTopLevelItem( currentJoinItem );
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
    layer->addJoin( info );
    addJoinToTreeWidget( info, idx );

    pbnQueryBuilder->setEnabled( layer && layer->dataProvider() && layer->dataProvider()->supportsSubsetString() &&
                                 !layer->isEditable() );
    mFieldsPropertiesDialog->init();
  }
}

void QgsVectorLayerProperties::addJoinToTreeWidget( const QgsVectorJoinInfo& join, const int insertIndex )
{
  QTreeWidgetItem* joinItem = new QTreeWidgetItem();

  QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( join.joinLayerId ) );
  if ( !layer || !joinLayer )
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

  if ( join.targetFieldName.isEmpty() && join.targetFieldIndex >= 0 && join.targetFieldIndex < layer->fields().count() )
  {
    joinItem->setText( 2, layer->fields().field( join.targetFieldIndex ).name() );   //for compatibility with 1.x
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

void QgsVectorLayerProperties::on_mButtonRemoveJoin_clicked()
{
  QTreeWidgetItem* currentJoinItem = mJoinTreeWidget->currentItem();
  if ( !layer || !currentJoinItem )
  {
    return;
  }

  layer->removeJoin( currentJoinItem->data( 0, Qt::UserRole ).toString() );
  mJoinTreeWidget->takeTopLevelItem( mJoinTreeWidget->indexOfTopLevelItem( currentJoinItem ) );
  pbnQueryBuilder->setEnabled( layer && layer->dataProvider() && layer->dataProvider()->supportsSubsetString() &&
                               !layer->isEditable() );
  mFieldsPropertiesDialog->init();
}


void QgsVectorLayerProperties::updateSymbologyPage()
{

  //find out the type of renderer in the vectorlayer, create a dialog with these settings and add it to the form
  delete mRendererDialog;
  mRendererDialog = 0;

  if ( layer->rendererV2() )
  {
    mRendererDialog = new QgsRendererV2PropertiesDialog( layer, QgsStyleV2::defaultStyle(), true );

    // display the menu to choose the output format (fix #5136)
    mActionSaveStyleAs->setText( tr( "Save Style" ) );
    mActionSaveStyleAs->setMenu( mSaveAsMenu );
    QObject::disconnect( mActionSaveStyleAs, SIGNAL( triggered() ), this, SLOT( saveStyleAs_clicked() ) );
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

void QgsVectorLayerProperties::on_pbnUpdateExtents_clicked()
{
  layer->updateExtents();
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
  if ( !( layer->dataProvider()->capabilities() & QgsVectorDataProvider::SimplifyGeometries ) )
  {
    mSimplifyDrawingAtProvider->setEnabled( false );
  }
  else
  {
    mSimplifyDrawingAtProvider->setEnabled( checked );
  }
}
