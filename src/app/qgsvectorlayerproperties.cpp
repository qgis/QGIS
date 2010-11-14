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
/* $Id$ */

#include <memory>
#include <limits>

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsattributeactiondialog.h"
#include "qgsapplydialog.h"
#include "qgscontexthelp.h"
#include "qgscontinuouscolordialog.h"
#include "qgscoordinatetransform.h"
#include "qgsfieldcalculator.h"
#include "qgsgraduatedsymboldialog.h"
#include "qgslabeldialog.h"
#include "qgslabel.h"
#include "qgsgenericprojectionselector.h"
#include "qgslogger.h"
#include "qgspluginmetadata.h"
#include "qgspluginregistry.h"
#include "qgsproject.h"
#include "qgssinglesymboldialog.h"
#include "qgsuniquevaluedialog.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerproperties.h"
#include "qgsconfig.h"
#include "qgsvectordataprovider.h"
#include "qgsvectoroverlayplugin.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"

#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QComboBox>
#include <QCheckBox>

#include "qgsrendererv2propertiesdialog.h"
#include "qgsstylev2.h"
#include "qgssymbologyv2conversion.h"


QgsVectorLayerProperties::QgsVectorLayerProperties(
  QgsVectorLayer *lyr,
  QWidget * parent,
  Qt::WFlags fl
)
    : QDialog( parent, fl )
    , layer( lyr )
    , mMetadataFilled( false )
    , mRendererDialog( 0 )
{
  setupUi( this );
  setupEditTypes();

  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( apply() ) );
  connect( mAddAttributeButton, SIGNAL( clicked() ), this, SLOT( addAttribute() ) );
  connect( mDeleteAttributeButton, SIGNAL( clicked() ), this, SLOT( deleteAttribute() ) );

  connect( mToggleEditingButton, SIGNAL( clicked() ), this, SLOT( toggleEditing() ) );
  connect( this, SIGNAL( toggleEditing( QgsMapLayer* ) ),
           QgisApp::instance(), SLOT( toggleEditing( QgsMapLayer* ) ) );

  connect( layer, SIGNAL( editingStarted() ), this, SLOT( editingToggled() ) );
  connect( layer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );
  connect( layer, SIGNAL( attributeAdded( int ) ), this, SLOT( attributeAdded( int ) ) );
  connect( layer, SIGNAL( attributeDeleted( int ) ), this, SLOT( attributeDeleted( int ) ) );

  mAddAttributeButton->setIcon( QgisApp::getThemeIcon( "/mActionNewAttribute.png" ) );
  mDeleteAttributeButton->setIcon( QgisApp::getThemeIcon( "/mActionDeleteAttribute.png" ) );
  mToggleEditingButton->setIcon( QgisApp::getThemeIcon( "/mActionToggleEditing.png" ) );
  mCalculateFieldButton->setIcon( QgisApp::getThemeIcon( "/mActionCalculateField.png" ) );

  connect( btnUseNewSymbology, SIGNAL( clicked() ), this, SLOT( useNewSymbology() ) );

  // Create the Label dialog tab
  QVBoxLayout *layout = new QVBoxLayout( labelOptionsFrame );
  layout->setMargin( 0 );
  labelDialog = new QgsLabelDialog( layer->label(), labelOptionsFrame );
  layout->addWidget( labelDialog );
  labelOptionsFrame->setLayout( layout );
  connect( labelDialog, SIGNAL( labelSourceSet() ), this, SLOT( setLabelCheckBox() ) );

  // Create the Actions dialog tab
  QVBoxLayout *actionLayout = new QVBoxLayout( actionOptionsFrame );
  actionLayout->setMargin( 0 );
  const QgsFieldMap &fields = layer->pendingFields();
  actionDialog = new QgsAttributeActionDialog( layer->actions(), fields, actionOptionsFrame );
  actionLayout->addWidget( actionDialog );

  reset();

  if ( layer->dataProvider() )//enable spatial index button group if supported by provider
  {
    int capabilities = layer->dataProvider()->capabilities();
    if ( !( capabilities&QgsVectorDataProvider::CreateSpatialIndex ) )
    {
      pbnIndex->setEnabled( false );
    }
  }

  updateButtons();

  leSpatialRefSys->setText( layer->srs().toProj4() );
  leSpatialRefSys->setCursorPosition( 0 );

  leEditForm->setText( layer->editForm() );
  leEditFormInit->setText( layer->editFormInit() );

  connect( sliderTransparency, SIGNAL( valueChanged( int ) ), this, SLOT( sliderTransparency_valueChanged( int ) ) );

  //for each overlay plugin create a new tab
  int position;
  QList<QgsVectorOverlayPlugin*> overlayPluginList = overlayPlugins();
  QList<QgsVectorOverlayPlugin*>::const_iterator it = overlayPluginList.constBegin();

  for ( ; it != overlayPluginList.constEnd(); ++it )
  {
    QgsApplyDialog* d = ( *it )->dialog( lyr );
    position = tabWidget->insertTab( tabWidget->count(), qobject_cast<QDialog*>( d ), QgisApp::getThemeIcon( "propertyicons/diagram.png" ), tr( "Overlay"));
    tabWidget->setCurrentIndex( position ); //ugly, but otherwise the properties dialog is a mess
    mOverlayDialogs.push_back( d );
  }

  tabWidget->setCurrentIndex( 0 );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/VectorLayerProperties/geometry" ).toByteArray() );
  tabWidget->setCurrentIndex( settings.value( "/Windows/VectorLayerProperties/row" ).toInt() );

  setWindowTitle( tr( "Layer Properties - %1" ).arg( layer->name() ) );
} // QgsVectorLayerProperties ctor

void QgsVectorLayerProperties::loadRows()
{
  const QgsFieldMap &fields = layer->pendingFields();

  tblAttributes->clear();

  tblAttributes->setColumnCount( attrColCount );
  tblAttributes->setRowCount( fields.size() );
  tblAttributes->setHorizontalHeaderItem( attrIdCol, new QTableWidgetItem( tr( "Id" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrNameCol, new QTableWidgetItem( tr( "Name" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrTypeCol, new QTableWidgetItem( tr( "Type" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrLengthCol, new QTableWidgetItem( tr( "Length" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrPrecCol, new QTableWidgetItem( tr( "Precision" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrCommentCol, new QTableWidgetItem( tr( "Comment" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrEditTypeCol, new QTableWidgetItem( tr( "Edit widget" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrAliasCol, new QTableWidgetItem( tr( "Alias" ) ) );

  tblAttributes->horizontalHeader()->setResizeMode(1,QHeaderView::Stretch);
  tblAttributes->setSelectionBehavior( QAbstractItemView::SelectRows );
  tblAttributes->setSelectionMode( QAbstractItemView::MultiSelection );

  int row = 0;
  for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); it++, row++ )
    setRow( row, it.key(), it.value() );

  tblAttributes->resizeColumnsToContents();
}

void QgsVectorLayerProperties::setRow( int row, int idx, const QgsField &field )
{
  tblAttributes->setItem( row, attrIdCol, new QTableWidgetItem( QString::number( idx ) ) );
  tblAttributes->setItem( row, attrNameCol, new QTableWidgetItem( field.name() ) );
  tblAttributes->setItem( row, attrTypeCol, new QTableWidgetItem( field.typeName() ) );
  tblAttributes->setItem( row, attrLengthCol, new QTableWidgetItem( QString::number( field.length() ) ) );
  tblAttributes->setItem( row, attrPrecCol, new QTableWidgetItem( QString::number( field.precision() ) ) );
  tblAttributes->setItem( row, attrCommentCol, new QTableWidgetItem( field.comment() ) );

  for ( int i = 0; i < attrEditTypeCol; i++ )
    tblAttributes->item( row, i )->setFlags( tblAttributes->item( row, i )->flags() & ~Qt::ItemIsEditable );

  QPushButton *pb = new QPushButton( editTypeButtonText( layer->editType( idx ) ) );
  tblAttributes->setCellWidget( row, attrEditTypeCol, pb );
  connect( pb, SIGNAL( pressed() ), this, SLOT( attributeTypeDialog( ) ) );
  mButtonMap.insert( idx, pb );

  //set the alias for the attribute
  tblAttributes->setItem( row, attrAliasCol, new QTableWidgetItem( layer->attributeAlias( idx ) ) );
}

QgsVectorLayerProperties::~QgsVectorLayerProperties()
{
  disconnect( labelDialog, SIGNAL( labelSourceSet() ), this, SLOT( setLabelCheckBox() ) );

  QSettings settings;
  settings.setValue( "/Windows/VectorLayerProperties/geometry", saveGeometry() );
  settings.setValue( "/Windows/VectorLayerProperties/row", tabWidget->currentIndex() );
}

void QgsVectorLayerProperties::attributeTypeDialog( )
{
  QPushButton *pb = qobject_cast<QPushButton *>( sender() );
  if ( !pb )
    return;

  int index = mButtonMap.key( pb, -1 );
  if ( index == -1 )
    return;

  QgsAttributeTypeDialog attributeTypeDialog( layer );

  if ( mValueMaps.contains( index ) )
  {
    attributeTypeDialog.setValueMap( mValueMaps[index] );
  }
  else
  {
    attributeTypeDialog.setValueMap( QMap<QString, QVariant>() );
  }

  if ( mRanges.contains( index ) )
  {
    attributeTypeDialog.setRange( mRanges[index] );
  }
  else
  {
    attributeTypeDialog.setRange( QgsVectorLayer::RangeData( 0, 5, 1 ) );
  }

  if ( mEditTypeMap.contains( index ) )
  {
    attributeTypeDialog.setIndex( index, mEditTypeMap[index] );
  }
  else
  {
    attributeTypeDialog.setIndex( index );
  }

  if ( mCheckedStates.contains( index ) )
  {
    attributeTypeDialog.setCheckedState( mCheckedStates[index].first, mCheckedStates[index].second );
  }

  if ( !attributeTypeDialog.exec() )
    return;

  QgsVectorLayer::EditType editType = attributeTypeDialog.editType();

  mEditTypeMap.insert( index, editType );

  QString buttonText;
  switch ( editType )
  {
    case QgsVectorLayer::ValueMap:
      mValueMaps.insert( index, attributeTypeDialog.valueMap() );
      break;
    case QgsVectorLayer::EditRange:
    case QgsVectorLayer::SliderRange:
    case QgsVectorLayer::DialRange:
      mRanges.insert( index, attributeTypeDialog.rangeData() );
      break;
    case QgsVectorLayer::CheckBox:
      mCheckedStates.insert( index, attributeTypeDialog.checkedState() );
    default:
      break;
  }

  pb->setText( editTypeButtonText( editType ) );
}


void QgsVectorLayerProperties::toggleEditing()
{
  emit toggleEditing( layer );

  pbnQueryBuilder->setEnabled( layer && layer->dataProvider() && layer->dataProvider()->supportsSubsetString() && !layer->isEditable() );
  if ( layer->isEditable() )
  {
    pbnQueryBuilder->setToolTip( tr( "Stop editing mode to enable this." ) );
  }
}

void QgsVectorLayerProperties::attributeAdded( int idx )
{
  const QgsFieldMap &fields = layer->pendingFields();
  int row = tblAttributes->rowCount();
  tblAttributes->insertRow( row );
  setRow( row, idx, fields[idx] );
  tblAttributes->setCurrentCell( row, idx );
}


void QgsVectorLayerProperties::attributeDeleted( int idx )
{
  for ( int i = 0; i < tblAttributes->rowCount(); i++ )
  {
    if ( tblAttributes->item( i, 0 )->text().toInt() == idx )
    {
      tblAttributes->removeRow( i );
      break;
    }
  }
}

void QgsVectorLayerProperties::addAttribute()
{
  QgsAddAttrDialog dialog( layer, this );
  if ( dialog.exec() == QDialog::Accepted )
  {
    layer->beginEditCommand( "Attribute added" );
    if ( !addAttribute( dialog.field() ) )
    {
      layer->destroyEditCommand();
      QMessageBox::information( this, tr( "Name conflict" ), tr( "The attribute could not be inserted. The name already exists in the table." ) );
    }
    else
    {
      layer->endEditCommand();
    }
  }
}

bool QgsVectorLayerProperties::addAttribute( const QgsField &field )
{
  QgsDebugMsg( "inserting attribute " + field.name() + " of type " + field.typeName() );
  layer->beginEditCommand( tr( "Added attribute" ) );
  if ( layer->addAttribute( field ) )
  {
    layer->endEditCommand();
    return true;
  }
  else
  {
    layer->destroyEditCommand();
    return false;
  }
}

void QgsVectorLayerProperties::deleteAttribute()
{
  QList<QTableWidgetItem*> items = tblAttributes->selectedItems();
  QList<int> idxs;

  for ( QList<QTableWidgetItem*>::const_iterator it = items.begin(); it != items.end(); it++ )
  {
    if (( *it )->column() == 0 )
      idxs << ( *it )->text().toInt();
  }
  for ( QList<int>::const_iterator it = idxs.begin(); it != idxs.end(); it++ )
  {
    layer->beginEditCommand( tr( "Deleted attribute" ) );
    layer->deleteAttribute( *it );
    layer->endEditCommand();
  }
}

void QgsVectorLayerProperties::editingToggled()
{
  if ( layer->isEditable() )
    loadRows();

  updateButtons();
}

void QgsVectorLayerProperties::updateButtons()
{
  if ( layer->isEditable() )
  {
    int cap = layer->dataProvider()->capabilities();
    mAddAttributeButton->setEnabled( cap & QgsVectorDataProvider::AddAttributes );
    mDeleteAttributeButton->setEnabled( cap & QgsVectorDataProvider::DeleteAttributes );
    mCalculateFieldButton->setEnabled( cap & QgsVectorDataProvider::ChangeAttributeValues );
    mToggleEditingButton->setChecked( true );
  }
  else
  {
    mAddAttributeButton->setEnabled( false );
    mDeleteAttributeButton->setEnabled( false );
    mToggleEditingButton->setChecked( false );
    mCalculateFieldButton->setEnabled( false );
  }
}

void QgsVectorLayerProperties::sliderTransparency_valueChanged( int theValue )
{
  //set the transparency percentage label to a suitable value
  int myInt = static_cast < int >(( theValue / 255.0 ) * 100 );  //255.0 to prevent integer division
  lblTransparencyPercent->setText( tr( "Transparency: %1%" ).arg( myInt ) );
}//sliderTransparency_valueChanged

void QgsVectorLayerProperties::setLabelCheckBox()
{
  labelCheckBox->setCheckState( Qt::Checked );
}

void QgsVectorLayerProperties::alterLayerDialog( const QString & dialogString )
{

  widgetStackRenderers->removeWidget( mRendererDialog );
  delete mRendererDialog;
  mRendererDialog = 0;
  if ( dialogString == tr( "Single Symbol" ) )
  {
    mRendererDialog = new QgsSingleSymbolDialog( layer );
  }
  else if ( dialogString == tr( "Graduated Symbol" ) )
  {
    mRendererDialog = new QgsGraduatedSymbolDialog( layer );
  }
  else if ( dialogString == tr( "Continuous Color" ) )
  {
    mRendererDialog = new QgsContinuousColorDialog( layer );
  }
  else if ( dialogString == tr( "Unique Value" ) )
  {
    mRendererDialog = new QgsUniqueValueDialog( layer );
  }
  widgetStackRenderers->addWidget( mRendererDialog );
  widgetStackRenderers->setCurrentWidget( mRendererDialog );
}

void QgsVectorLayerProperties::setLegendType( QString type )
{
  legendtypecombobox->setItemText( legendtypecombobox->currentIndex(), type );
}

void QgsVectorLayerProperties::setDisplayField( QString name )
{
  displayFieldComboBox->setItemText( displayFieldComboBox->currentIndex(), name );
}

//! @note in raster props, this method is called sync()
void QgsVectorLayerProperties::reset( void )
{
  QObject::disconnect( tblAttributes, SIGNAL( cellChanged( int, int ) ), this, SLOT( on_tblAttributes_cellChanged( int, int ) ) );

  // populate the general information
  txtDisplayName->setText( layer->name() );
  pbnQueryBuilder->setWhatsThis( tr( "This button opens the query "
                                     "builder and allows you to create a subset of features to display on "
                                     "the map canvas rather than displaying all features in the layer" ) );
  txtSubsetSQL->setWhatsThis( tr( "The query used to limit the features in the "
                                  "layer is shown here. To enter or modify the query, click on the Query Builder button" ) );

  //see if we are dealing with a pg layer here
  grpSubset->setEnabled( true );
  txtSubsetSQL->setText( layer->subsetString() );
  // if the user is allowed to type an adhoc query, the app will crash if the query
  // is bad. For this reason, the sql box is disabled and the query must be built
  // using the query builder, either by typing it in by hand or using the buttons, etc
  // on the builder. If the ability to enter a query directly into the box is required,
  // a mechanism to check it must be implemented.
  txtSubsetSQL->setEnabled( false );
  pbnQueryBuilder->setEnabled( layer && layer->dataProvider() && layer->dataProvider()->supportsSubsetString() && !layer->isEditable() );
  if ( layer->isEditable() )
  {
    pbnQueryBuilder->setToolTip( tr( "Stop editing mode to enable this." ) );
  }

  //get field list for display field combo
  const QgsFieldMap& myFields = layer->pendingFields();
  for ( QgsFieldMap::const_iterator it = myFields.begin(); it != myFields.end(); ++it )
  {
    displayFieldComboBox->addItem( it->name() );
  }
  displayFieldComboBox->setCurrentIndex( displayFieldComboBox->findText(
                                           layer->displayField() ) );

  // set up the scale based layer visibility stuff....
  chkUseScaleDependentRendering->setChecked( layer->hasScaleBasedVisibility() );
  leMinimumScale->setText( QString::number( layer->minimumScale(), 'f' ) );
  leMinimumScale->setValidator( new QDoubleValidator( 0, std::numeric_limits<float>::max(), 1000, this ) );
  leMaximumScale->setText( QString::number( layer->maximumScale(), 'f' ) );
  leMaximumScale->setValidator( new QDoubleValidator( 0, std::numeric_limits<float>::max(), 1000, this ) );

  // symbology initialization
  if ( legendtypecombobox->count() == 0 )
  {
    legendtypecombobox->addItem( tr( "Single Symbol" ) );
    if ( myFields.size() > 0 )
    {
      legendtypecombobox->addItem( tr( "Graduated Symbol" ) );
      legendtypecombobox->addItem( tr( "Continuous Color" ) );
      legendtypecombobox->addItem( tr( "Unique Value" ) );
    }
  }

  // load appropriate symbology page (V1 or V2)
  updateSymbologyPage();

  // reset fields in label dialog
  layer->label()->setFields( layer->pendingFields() );

  actionDialog->init();
  labelDialog->init();
  labelCheckBox->setChecked( layer->hasLabelsEnabled() );
  labelOptionsFrame->setEnabled( layer->hasLabelsEnabled() );
  //set the transparency slider
  sliderTransparency->setValue( 255 - layer->getTransparency() );
  //update the transparency percentage label
  sliderTransparency_valueChanged( 255 - layer->getTransparency() );

  loadRows();
  QObject::connect( tblAttributes, SIGNAL( cellChanged( int, int ) ), this, SLOT( on_tblAttributes_cellChanged( int, int ) ) );
} // reset()


//
// methods reimplemented from qt designer base class
//

QMap< QgsVectorLayer::EditType, QString > QgsVectorLayerProperties::editTypeMap;

void QgsVectorLayerProperties::setupEditTypes()
{
  if ( !editTypeMap.isEmpty() )
    return;

  editTypeMap.insert( QgsVectorLayer::LineEdit, tr( "Line edit" ) );
  editTypeMap.insert( QgsVectorLayer::UniqueValues, tr( "Unique values" ) );
  editTypeMap.insert( QgsVectorLayer::UniqueValuesEditable, tr( "Unique values editable" ) );
  editTypeMap.insert( QgsVectorLayer::Classification, tr( "Classification" ) );
  editTypeMap.insert( QgsVectorLayer::ValueMap, tr( "Value map" ) );
  editTypeMap.insert( QgsVectorLayer::EditRange, tr( "Edit range" ) );
  editTypeMap.insert( QgsVectorLayer::SliderRange, tr( "Slider range" ) );
  editTypeMap.insert( QgsVectorLayer::DialRange, tr( "Dial range" ) );
  editTypeMap.insert( QgsVectorLayer::FileName, tr( "File name" ) );
  editTypeMap.insert( QgsVectorLayer::Enumeration, tr( "Enumeration" ) );
  editTypeMap.insert( QgsVectorLayer::Immutable, tr( "Immutable" ) );
  editTypeMap.insert( QgsVectorLayer::Hidden, tr( "Hidden" ) );
  editTypeMap.insert( QgsVectorLayer::CheckBox, tr( "Checkbox" ) );
  editTypeMap.insert( QgsVectorLayer::TextEdit, tr( "Text edit" ) );
  editTypeMap.insert( QgsVectorLayer::Calendar, tr( "Calendar" ) );
}

QString QgsVectorLayerProperties::editTypeButtonText( QgsVectorLayer::EditType type )
{
  return editTypeMap[ type ];
}

QgsVectorLayer::EditType QgsVectorLayerProperties::editTypeFromButtonText( QString text )
{
  return editTypeMap.key( text );
}

void QgsVectorLayerProperties::apply()
{
  //
  // Set up sql subset query if applicable
  //
  grpSubset->setEnabled( true );

  if ( txtSubsetSQL->toPlainText() != layer->subsetString() )
  {
    // set the subset sql for the layer
    layer->setSubsetString( txtSubsetSQL->toPlainText() );
    mMetadataFilled = false;
  }

  // set up the scale based layer visibility stuff....
  layer->toggleScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  layer->setMinimumScale( leMinimumScale->text().toFloat() );
  layer->setMaximumScale( leMaximumScale->text().toFloat() );

  // update the display field
  layer->setDisplayField( displayFieldComboBox->currentText() );

  layer->setEditForm( leEditForm->text() );
  layer->setEditFormInit( leEditFormInit->text() );

  actionDialog->apply();

  labelDialog->apply();
  layer->enableLabels( labelCheckBox->isChecked() );
  layer->setLayerName( displayName() );

  for ( int i = 0; i < tblAttributes->rowCount(); i++ )
  {
    int idx = tblAttributes->item( i, attrIdCol )->text().toInt();

    QPushButton *pb = qobject_cast<QPushButton *>( tblAttributes->cellWidget( i, attrEditTypeCol ) );
    if ( !pb )
      continue;

    QgsVectorLayer::EditType editType = editTypeFromButtonText( pb->text() );
    layer->setEditType( idx, editType );

    if ( editType == QgsVectorLayer::ValueMap )
    {
      if ( mValueMaps.contains( idx ) )
      {
        QMap<QString, QVariant> &map = layer->valueMap( idx );
        map.clear();
        map = mValueMaps[idx];
      }
    }
    else if ( editType == QgsVectorLayer::EditRange ||
              editType == QgsVectorLayer::SliderRange ||
              editType == QgsVectorLayer::DialRange )
    {
      if ( mRanges.contains( idx ) )
      {
        layer->range( idx ) = mRanges[idx];
      }
    }
    else if ( editType == QgsVectorLayer::CheckBox )
    {
      if ( mCheckedStates.contains( idx ) )
      {
        layer->setCheckedState( idx, mCheckedStates[idx].first, mCheckedStates[idx].second );
      }
    }
  }

  if ( layer->isUsingRendererV2() )
  {
    QgsRendererV2PropertiesDialog* dlg =
      static_cast<QgsRendererV2PropertiesDialog*>( widgetStackRenderers->currentWidget() );
    dlg->apply();
  }
  else
  {
    QgsSingleSymbolDialog *sdialog =
      qobject_cast < QgsSingleSymbolDialog * >( widgetStackRenderers->currentWidget() );
    QgsGraduatedSymbolDialog *gdialog =
      qobject_cast < QgsGraduatedSymbolDialog * >( widgetStackRenderers->currentWidget() );
    QgsContinuousColorDialog *cdialog =
      qobject_cast < QgsContinuousColorDialog * >( widgetStackRenderers->currentWidget() );
    QgsUniqueValueDialog* udialog =
      qobject_cast< QgsUniqueValueDialog * >( widgetStackRenderers->currentWidget() );

    if ( sdialog )
    {
      sdialog->apply();
    }
    else if ( gdialog )
    {
      gdialog->apply();
    }
    else if ( cdialog )
    {
      cdialog->apply();
    }
    else if ( udialog )
    {
      udialog->apply();
    }
    layer->setTransparency( static_cast < unsigned int >( 255 - sliderTransparency->value() ) );

  }

  //apply overlay dialogs
  for ( QList<QgsApplyDialog*>::iterator it = mOverlayDialogs.begin(); it != mOverlayDialogs.end(); ++it )
  {
    ( *it )->apply();
  }

  // update symbology
  emit refreshLegend( layer->getLayerID(), false );

  //no need to delete the old one, maplayer will do it if needed
  layer->setCacheImage( 0 );

  layer->triggerRepaint();
  // notify the project we've made a change
  QgsProject::instance()->dirty( true );

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
  QString myMetadata = "<html><body>";
  myMetadata += "<table width=\"100%\">";

  //-------------

  myMetadata += "<tr class=\"glossy\"><td>";
  myMetadata += tr( "General:" );
  myMetadata += "</td></tr>";

  // data comment
  if ( !( layer->dataComment().isEmpty() ) )
  {
    myMetadata += "<tr><td>";
    myMetadata += tr( "Layer comment: %1" ).arg( layer->dataComment() );
    myMetadata += "</td></tr>";
  }

  //storage type
  myMetadata += "<tr><td>";
  myMetadata += tr( "Storage type of this layer: %1" ).arg( layer->storageType() );
  myMetadata += "</td></tr>";

  // data source
  myMetadata += "<tr><td>";
  myMetadata += tr( "Source for this layer: %1" ).arg( layer->publicSource() );
  myMetadata += "</td></tr>";

  //geom type

  QGis::GeometryType type = layer->geometryType();

  if ( type < 0 || type > QGis::Polygon )
  {
    QgsDebugMsg( "Invalid vector type" );
  }
  else
  {
    QString typeString( QGis::qgisVectorGeometryType[layer->geometryType()] );

    myMetadata += "<tr><td>";
    myMetadata += tr( "Geometry type of the features in this layer: %1" ).arg( typeString );
    myMetadata += "</td></tr>";
  }


  //feature count
  myMetadata += "<tr><td>";
  myMetadata += tr( "The number of features in this layer: %1" ).arg( layer->featureCount() );
  myMetadata += "</td></tr>";
  //capabilities
  myMetadata += "<tr><td>";
  myMetadata += tr( "Editing capabilities of this layer: %1" ).arg( layer->capabilitiesString() );
  myMetadata += "</td></tr>";

  //-------------

  QgsRectangle myExtent = layer->extent();
  myMetadata += "<tr class=\"glossy\"><td>";
  myMetadata += tr( "Extents:" );
  myMetadata += "</td></tr>";
  //extents in layer cs  TODO...maybe make a little nested table to improve layout...
  myMetadata += "<tr><td>";

  // Try to be a bit clever over what number format we use for the
  // extents. Some people don't like it using scientific notation when the
  // numbers get large, but for small numbers this is the more practical
  // option (so we can't force the format to 'f' for all values).
  // The scheme:
  // - for all numbers with more than 5 digits, force non-scientific notation
  // and 2 digits after the decimal point.
  // - for all smaller numbers let the OS decide which format to use (it will
  // generally use non-scientific unless the number gets much less than 1).

  QString xMin, yMin, xMax, yMax;
  double changeoverValue = 99999; // The 'largest' 5 digit number
  if ( fabs( myExtent.xMinimum() ) > changeoverValue )
  {
    xMin = QString( "%1" ).arg( myExtent.xMinimum(), 0, 'f', 2 );
  }
  else
  {
    xMin = QString( "%1" ).arg( myExtent.xMinimum() );
  }

  if ( fabs( myExtent.yMinimum() ) > changeoverValue )
  {
    yMin = QString( "%1" ).arg( myExtent.yMinimum(), 0, 'f', 2 );
  }
  else
  {
    yMin = QString( "%1" ).arg( myExtent.yMinimum() );
  }

  if ( fabs( myExtent.xMaximum() ) > changeoverValue )
  {
    xMax = QString( "%1" ).arg( myExtent.xMaximum(), 0, 'f', 2 );
  }
  else
  {
    xMax = QString( "%1" ).arg( myExtent.xMaximum() );
  }

  if ( fabs( myExtent.yMaximum() ) > changeoverValue )
  {
    yMax = QString( "%1" ).arg( myExtent.yMaximum(), 0, 'f', 2 );
  }
  else
  {
    yMax = QString( "%1" ).arg( myExtent.yMaximum() );
  }

  myMetadata += tr( "In layer spatial reference system units : " )
                + tr( "xMin,yMin %1,%2 : xMax,yMax %3,%4" )
                .arg( xMin ).arg( yMin ).arg( xMax ).arg( yMax );
  myMetadata += "</td></tr>";

  //extents in project cs

  try
  {
#if 0
    // TODO: currently disabled, will revisit later [MD]
    QgsRectangle myProjectedExtent = coordinateTransform->transformBoundingBox( layer->extent() );
    myMetadata += "<tr><td>";
    myMetadata += tr( "In project spatial reference system units : " )
                  + tr( "xMin,yMin %1,%2 : xMax,yMax %3,%4" )
                  .arg( myProjectedExtent.xMinimum() )
                  .arg( myProjectedExtent.yMinimum() )
                  .arg( myProjectedExtent.xMaximum() )
                  .arg( myProjectedExtent.yMaximum() );
    myMetadata += "</td></tr>";
#endif

    //
    // Display layer spatial ref system
    //
    myMetadata += "<tr class=\"glossy\"><td>";
    myMetadata += tr( "Layer Spatial Reference System:" );
    myMetadata += "</td></tr>";
    myMetadata += "<tr><td>";
    myMetadata += layer->srs().toProj4().replace( QRegExp( "\"" ), " \"" );
    myMetadata += "</td></tr>";

    //
    // Display project (output) spatial ref system
    //
#if 0
    // TODO: disabled for now, will revisit later [MD]
    myMetadata += "<tr><td bgcolor=\"gray\">";
    myMetadata += tr( "Project (Output) Spatial Reference System:" );
    myMetadata += "</td></tr>";
    myMetadata += "<tr><td>";
    myMetadata += coordinateTransform->destCRS().toProj4().replace( QRegExp( "\"" ), " \"" );
    myMetadata += "</td></tr>";
#endif
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsDebugMsg( cse.what() );

    myMetadata += "<tr><td>";
    myMetadata += tr( "In project spatial reference system units : " )
                  + tr( "(Invalid transformation of layer extents)" );
    myMetadata += "</td></tr>";

  }

#if 0
  //
  // Add the info about each field in the attribute table
  //
  myMetadata += "<tr class=\"glossy\"><td>";
  myMetadata += tr( "Attribute field info:" );
  myMetadata += "</td></tr>";
  myMetadata += "<tr><td>";

  // Start a nested table in this trow
  myMetadata += "<table width=\"100%\">";
  myMetadata += "<tr><th>";
  myMetadata += tr( "Field" );
  myMetadata += "</th>";
  myMetadata += "<th>";
  myMetadata += tr( "Type" );
  myMetadata += "</th>";
  myMetadata += "<th>";
  myMetadata += tr( "Length" );
  myMetadata += "</th>";
  myMetadata += "<th>";
  myMetadata += tr( "Precision" );
  myMetadata += "</th>";
  myMetadata += "<th>";
  myMetadata += tr( "Comment" );
  myMetadata += "</th>";

  //get info for each field by looping through them
  const QgsFieldMap& myFields = layer->pendingFields();
  for ( QgsFieldMap::const_iterator it = myFields.begin(); it != myFields.end(); ++it )
  {
    const QgsField& myField = *it;

    myMetadata += "<tr><td>";
    myMetadata += myField.name();
    myMetadata += "</td>";
    myMetadata += "<td>";
    myMetadata += myField.typeName();
    myMetadata += "</td>";
    myMetadata += "<td>";
    myMetadata += QString( "%1" ).arg( myField.length() );
    myMetadata += "</td>";
    myMetadata += "<td>";
    myMetadata += QString( "%1" ).arg( myField.precision() );
    myMetadata += "</td>";
    myMetadata += "<td>";
    myMetadata += QString( "%1" ).arg( myField.comment() );
    myMetadata += "</td></tr>";
  }

  //close field list
  myMetadata += "</table>"; //end of nested table
#endif

  myMetadata += "</td></tr>"; //end of stats container table row
  //
  // Close the table
  //

  myMetadata += "</table>";

  myMetadata += "</body></html>";
  return myMetadata;
}



void QgsVectorLayerProperties::on_pbnChangeSpatialRefSys_clicked()
{
  QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector( this );
  mySelector->setMessage();
  mySelector->setSelectedCrsId( layer->srs().srsid() );
  if ( mySelector->exec() )
  {
    QgsCoordinateReferenceSystem srs( mySelector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );
    layer->setCrs( srs );
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }
  delete mySelector;

  leSpatialRefSys->setText( layer->srs().toProj4() );
  leSpatialRefSys->setCursorPosition( 0 );
}

void QgsVectorLayerProperties::on_pbnLoadDefaultStyle_clicked()
{
  bool defaultLoadedFlag = false;
  QString myMessage = layer->loadDefaultStyle( defaultLoadedFlag );
  //reset if the default style was loaded ok only
  if ( defaultLoadedFlag )
  {
    // all worked ok so no need to inform user
    reset();
  }
  else
  {
    //something went wrong - let them know why
    QMessageBox::information( this, tr( "Default Style" ), myMessage );
  }
}

void QgsVectorLayerProperties::on_pbnSaveDefaultStyle_clicked()
{
  apply(); // make sure the qml to save is uptodate

  // a flag passed by reference
  bool defaultSavedFlag = false;
  // after calling this the above flag will be set true for success
  // or false if the save operation failed
  QString myMessage = layer->saveDefaultStyle( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    //only raise the message if something went wrong
    QMessageBox::information( this, tr( "Default Style" ), myMessage );
  }
}


void QgsVectorLayerProperties::on_pbnLoadStyle_clicked()
{
  QSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( "style/lastStyleDir", "." ).toString();
  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load layer properties from style file (.qml)" ), myLastUsedDir, tr( "QGIS Layer Style File (*.qml)" ) );
  if ( myFileName.isNull() )
  {
    return;
  }

  bool defaultLoadedFlag = false;
  QString myMessage = layer->loadNamedStyle( myFileName, defaultLoadedFlag );
  //reset if the default style was loaded ok only
  if ( defaultLoadedFlag )
  {
    reset();
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::information( this, tr( "Saved Style" ), myMessage );
  }

  QFileInfo myFI( myFileName );
  QString myPath = myFI.path();
  myQSettings.setValue( "style/lastStyleDir", myPath );
}


void QgsVectorLayerProperties::on_pbnSaveStyleAs_clicked()
{
  QSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( "style/lastStyleDir", "." ).toString();
  QString myOutputFileName = QFileDialog::getSaveFileName( this, tr( "Save layer properties as style file (.qml)" ), myLastUsedDir, tr( "QGIS Layer Style File (*.qml)" ) );
  if ( myOutputFileName.isNull() ) //dialog canceled
  {
    return;
  }

  apply(); // make sure the qml to save is uptodate

  //ensure the user never omitted the extension from the file name
  if ( !myOutputFileName.endsWith( ".qml", Qt::CaseInsensitive ) )
  {
    myOutputFileName += ".qml";
  }

  bool defaultLoadedFlag = false;
  QString myMessage = layer->saveNamedStyle( myOutputFileName, defaultLoadedFlag );
  //reset if the default style was loaded ok only
  if ( defaultLoadedFlag )
  {
    reset();
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

void QgsVectorLayerProperties::on_tblAttributes_cellChanged( int row, int column )
{
  if ( column == attrAliasCol && layer ) //only consider attribute aliases in this function
  {
    int idx = tblAttributes->item( row, attrIdCol )->text().toInt();

    const QgsFieldMap &fields = layer->pendingFields();

    if ( !fields.contains( idx ) )
    {
      return; // index must be wrong
    }

    QTableWidgetItem *aliasItem = tblAttributes->item( row, column );
    if ( aliasItem )
    {
      layer->addAttributeAlias( idx, aliasItem->text() );
    }
  }
}

void QgsVectorLayerProperties::on_mCalculateFieldButton_clicked()
{
  if ( !layer )
  {
    return;
  }

  QgsFieldCalculator calc( layer );
  calc.exec();
}

void QgsVectorLayerProperties::on_pbnSelectEditForm_clicked()
{
  QSettings myQSettings;
  QString lastUsedDir = myQSettings.value( "style/lastUIDir", "." ).toString();
  QString uifilename = QFileDialog::getOpenFileName( this, tr( "Select edit form" ), lastUsedDir, tr( "UI file (*.ui)" ) );

  if ( uifilename.isNull() )
    return;

  leEditForm->setText( uifilename );
}

QList<QgsVectorOverlayPlugin*> QgsVectorLayerProperties::overlayPlugins() const
{
  QList<QgsVectorOverlayPlugin*> pluginList;

  QgisPlugin* thePlugin = 0;
  QgsVectorOverlayPlugin* theOverlayPlugin = 0;

  QList<QgsPluginMetadata*> pluginData = QgsPluginRegistry::instance()->pluginData();
  for ( QList<QgsPluginMetadata*>::iterator it = pluginData.begin(); it != pluginData.end(); ++it )
  {
    if ( *it )
    {
      thePlugin = ( *it )->plugin();
      if ( thePlugin && thePlugin->type() == QgisPlugin::VECTOR_OVERLAY )
      {
        theOverlayPlugin = dynamic_cast<QgsVectorOverlayPlugin *>( thePlugin );
        if ( theOverlayPlugin )
        {
          pluginList.push_back( theOverlayPlugin );
        }
      }
    }
  }

  return pluginList;
}

void QgsVectorLayerProperties::setUsingNewSymbology( bool useNewSymbology )
{
  if ( useNewSymbology )
  {
    QgsSymbologyV2Conversion::rendererV1toV2( layer );
  }
  else
  {
    QgsSymbologyV2Conversion::rendererV2toV1( layer );
  }

  // update GUI!
  updateSymbologyPage();
}

void QgsVectorLayerProperties::useNewSymbology()
{
  int res = QMessageBox::question( this, tr( "Symbology" ),
                                   tr( "Do you wish to use the new symbology implementation for this layer?" ),
                                   QMessageBox::Yes | QMessageBox::No );

  if ( res != QMessageBox::Yes )
    return;

  setUsingNewSymbology( true );
}

void QgsVectorLayerProperties::updateSymbologyPage()
{

  //find out the type of renderer in the vectorlayer, create a dialog with these settings and add it to the form
  delete mRendererDialog;
  mRendererDialog = 0;

  bool v2 = layer->isUsingRendererV2();

  // hide unused widgets
  legendtypecombobox->setVisible( !v2 );
  legendtypelabel->setVisible( !v2 );
  lblTransparencyPercent->setVisible( !v2 );
  sliderTransparency->setVisible( !v2 );
  btnUseNewSymbology->setVisible( !v2 );

  if ( v2 )
  {
    mRendererDialog = new QgsRendererV2PropertiesDialog( layer, QgsStyleV2::defaultStyle(), true );

    connect( mRendererDialog, SIGNAL( useNewSymbology( bool ) ), this, SLOT( setUsingNewSymbology( bool ) ) );
  }
  else if ( layer->renderer() )
  {
    QString rtype = layer->renderer()->name();
    if ( rtype == "Single Symbol" )
    {
      mRendererDialog = new QgsSingleSymbolDialog( layer );
      legendtypecombobox->setCurrentIndex( 0 );
    }
    else if ( rtype == "Graduated Symbol" )
    {
      mRendererDialog = new QgsGraduatedSymbolDialog( layer );
      legendtypecombobox->setCurrentIndex( 1 );
    }
    else if ( rtype == "Continuous Color" )
    {
      mRendererDialog = new QgsContinuousColorDialog( layer );
      legendtypecombobox->setCurrentIndex( 2 );
    }
    else if ( rtype == "Unique Value" )
    {
      mRendererDialog = new QgsUniqueValueDialog( layer );
      legendtypecombobox->setCurrentIndex( 3 );
    }

    QObject::connect( legendtypecombobox, SIGNAL( activated( const QString & ) ), this,
                      SLOT( alterLayerDialog( const QString & ) ) );
  }
  else
  {
    if ( tabWidget->currentIndex() == 0 )
    {
      tabWidget->setCurrentIndex( 1 );
    }

    tabWidget->setTabEnabled( 0, true ); // hide symbology item
  }

  if ( mRendererDialog )
  {
    widgetStackRenderers->addWidget( mRendererDialog );
    widgetStackRenderers->setCurrentWidget( mRendererDialog );
  }
}

void QgsVectorLayerProperties::on_tabWidget_currentChanged( int index )
{
  if ( index != 4 || mMetadataFilled )
    return;

  //set the metadata contents (which can be expensive)
  QString myStyle = QgsApplication::reportStyleSheet();
  teMetadata->clear();
  teMetadata->document()->setDefaultStyleSheet( myStyle );
  teMetadata->setHtml( metadata() );
  mMetadataFilled = true;
}
