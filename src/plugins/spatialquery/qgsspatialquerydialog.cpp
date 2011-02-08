/***************************************************************************
                          qgsspatialquerydialog.cpp
                             -------------------
    begin                : Dec 29, 2009
    copyright            : (C) 2009 by Diego Moreira And Luiz Motta
    email                : moreira.geo at gmail.com And motta.luiz at gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id: qgsspatialquerydialog.cpp 13441 2010-05-08 19:07:12Z jef $ */

#include <QMessageBox>
#include <QDateTime>
#include <QPushButton>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"

#include "qgsspatialquerydialog.h"
#include "qgsspatialquery.h"
#include "qgsrubberselectid.h"
#include "qgsmngprogressbar.h"

QgsSpatialQueryDialog::QgsSpatialQueryDialog( QWidget* parent, QgisInterface* iface ): QDialog( parent )
{
  setupUi( this );

  mLayerReference = mLayerTarget = NULL;
  mIface = iface;
  mRubberSelectId = new QgsRubberSelectId( iface->mapCanvas() );

  initGui();
  connectAll();

  mMsgLayersLessTwo = tr( "The spatial query requires at least two layers" );

} // QgsSpatialQueryDialog::QgsSpatialQueryDialog( QWidget* parent, QgisInterface* iface )

QgsSpatialQueryDialog::~QgsSpatialQueryDialog()
{
  disconnectAll();
  delete mRubberSelectId;
  mMapIdVectorLayers.clear();
  mFeatureResult.clear();
  mFeatureInvalidTarget.clear();
  mFeatureInvalidReference.clear();

} // QgsSpatialQueryDialog::~QgsSpatialQueryDialog()

void QgsSpatialQueryDialog::show()
{
  QDialog::show();
  adjustSize();
} // void QgsSpatialQueryDialog::show()

void QgsSpatialQueryDialog::messageLayersLessTwo()
{
  QString msgLayersLessTwo = tr( "The spatial query requires at least two layers" );
  QMessageBox::warning( 0, tr( "Insufficient number of layers" ), msgLayersLessTwo, QMessageBox::Ok );
}

void QgsSpatialQueryDialog::disconnectQGis()
{
  disconnectAll();

} // void QgsSpatialQueryDialog::unload()

void QgsSpatialQueryDialog::initGui()
{
  showLogProcessing( false );
  setLayoutResultInvalid( false );

  buttonBoxMain->button( QDialogButtonBox::Close )->hide();

  populateTargetLayerComboBox();
  if ( targetLayerComboBox->count() > 1 )
  {
    setLayer( true, 0 );
    evaluateCheckBox( true );
    populateReferenceLayerComboBox();
    setLayer( false, 0 );
    evaluateCheckBox( false );
    populateOperationComboBox();
  }
  else
  {
    buttonBoxMain->setEnabled( false );
    textEditStatus->append( mMsgLayersLessTwo );
  }

} // QgsSpatialQueryDialog::initGui()

void QgsSpatialQueryDialog::setColorRubberSelect()
{
  mRGBRubberSelect[0] = 255 - QgsProject::instance()->readNumEntry( "Gui", "/SelectionColorRedPart", 255 );
  mRGBRubberSelect[1] = 255 - QgsProject::instance()->readNumEntry( "Gui", "/SelectionColorGreenPart", 255 );
  mRGBRubberSelect[2] = 255 - QgsProject::instance()->readNumEntry( "Gui", "/SelectionColorBluePart", 0 );
} // void QgsSpatialQueryDialog::setColorRubberSelectId()

void QgsSpatialQueryDialog::setLayer( bool isTarget, int index )
{
  if ( isTarget )
  {
    if ( mLayerTarget )
    {
      disconnect( mLayerTarget, SIGNAL( selectionChanged() ),
                  this, SLOT( signal_layerTarget_selectionFeaturesChanged() ) );
    }
    mLayerTarget = getLayerFromCombobox( isTarget, index );
    connect( mLayerTarget, SIGNAL( selectionChanged() ),
             this, SLOT( signal_layerTarget_selectionFeaturesChanged() ) );
  }
  else
  {
    if ( mLayerReference )
    {
      disconnect( mLayerReference, SIGNAL( selectionChanged() ),
                  this, SLOT( signal_layerReference_selectionFeaturesChanged() ) );
    }
    mLayerReference = getLayerFromCombobox( isTarget, index );
    connect( mLayerReference, SIGNAL( selectionChanged() ),
             this, SLOT( signal_layerReference_selectionFeaturesChanged() ) );
  }

} // void QgsSpatialQueryDialog::setLayer(bool isTarget, int index)

void QgsSpatialQueryDialog::evaluateCheckBox( bool isTarget )
{
  QgsVectorLayer* layer = NULL;
  QCheckBox* checkbox = NULL;
  if ( isTarget )
  {
    layer = mLayerTarget;
    checkbox = usingSelectedTargetCheckBox;
  }
  else
  {
    layer = mLayerReference;
    checkbox = usingSelectedReferenceCheckBox;
  }
  int selectedCount = layer->selectedFeatureCount();
  bool isCheckBoxValid = ( layer != NULL &&  selectedCount > 0 );
  checkbox->setChecked( isCheckBoxValid );
  checkbox->setEnabled( isCheckBoxValid );
  QString textCheckBox  = isCheckBoxValid
                          ? tr( "%n selected geometries", "selected geometries", selectedCount )
                          : tr( "Selected geometries" );
  checkbox->setText( textCheckBox );

} // void QgsSpatialQueryDialog::evaluateCheckBox(bool isTarget)

void QgsSpatialQueryDialog::runQuery()
{
  buttonBoxMain->setEnabled( false );
  MngProgressBar* pb = new MngProgressBar( progressBarStatus );
  QgsSpatialQuery* spatialQuery = new QgsSpatialQuery( pb );
  if ( usingSelectedTargetCheckBox->isChecked() )
  {
    spatialQuery->setSelectedFeaturesTarget( true );
  }
  if ( usingSelectedReferenceCheckBox->isChecked() )
  {
    spatialQuery->setSelectedFeaturesReference( true );
  }
  progressBarStatus->setTextVisible( true );
  mFeatureResult.clear();
  mFeatureInvalidTarget.clear();
  mFeatureInvalidReference.clear();

  int currentItem = operantionComboBox->currentIndex();
  bool isOk;
  int operation = operantionComboBox->itemData( currentItem ).toInt( &isOk );
  spatialQuery->runQuery( mFeatureResult, mFeatureInvalidTarget, mFeatureInvalidReference, operation, mLayerTarget, mLayerReference );
  delete spatialQuery;
  delete pb;

  progressBarStatus->setTextVisible( false );
  buttonBoxMain->setEnabled( true );
  setLayoutOperationVisible( false );
  progressBarStatus->hide();
  buttonBoxMain->button( QDialogButtonBox::Close )->show();
  buttonBoxMain->button( QDialogButtonBox::Cancel )->hide();
  buttonBoxMain->button( QDialogButtonBox::Ok )->hide();
} // void QgsSpatialQueryDialog::runQuery()

void QgsSpatialQueryDialog::setLayoutOperationVisible( bool show )
{
  grpTargetGroupBox->setVisible( show );
  grpReferenceGroupBox->setVisible( show );
  grpOperationGroupBox->setVisible( show );
} // void QgsSpatialQueryDialog::setLayoutOperationVisible( bool show )

void QgsSpatialQueryDialog::setLayoutResultInvalid( bool show )
{
  grpResult->setVisible( show );
  grpInvalid->setVisible( show );
  ckboxLogProcessing->setVisible( show );
  labelInfo->setVisible( show );
} // void QgsSpatialQueryDialog::setLayoutResultInvalid( bool show )

void QgsSpatialQueryDialog::showLogProcessing( bool hasShow )
{
  static int heightDialogNoStatus = 0;

  hasShow ? textEditStatus->show() : textEditStatus->hide();
  adjustSize();

  if ( ! hasShow )
  {
    if ( heightDialogNoStatus == 0 )
    {
      heightDialogNoStatus = geometry().height();
    }
    else
    {
      setGeometry( geometry().x(), geometry().y(),
                   geometry().width(), heightDialogNoStatus );
    }
  }

} // void QgsSpatialQueryDialog::showLogProcessing(bool hasShow)

void QgsSpatialQueryDialog::showResultQuery( QDateTime *datetimeStart, QDateTime *datetimeEnd )
{
  // Report processing
  QString msg = tr( "<<-- Begin at [%L1] --" ).arg( datetimeStart->toString() );
  textEditStatus->append( msg );
  msg = tr( "Query:" );
  textEditStatus->append( msg );
  msg = QString( "- %1" ).arg( getDescriptionLayerShow( true ) );
  textEditStatus->append( msg );
  msg = tr( "< %1 >" ).arg( operantionComboBox->currentText() );
  textEditStatus->append( msg );
  msg = QString( "- %1" ).arg( getDescriptionLayerShow( false ) );
  textEditStatus->append( msg );
  msg = tr( "Result: %1 features" ).arg( mFeatureResult.size() );
  textEditStatus->append( msg );
  double timeProcess = ( double )datetimeStart->secsTo( *datetimeEnd ) / 60.0;
  msg = tr( "-- Finish at [%L1] (processing time %L2 minutes) -->>" ).arg( datetimeEnd->toString() ).arg( timeProcess, 0, 'f', 2 );
  textEditStatus->append( msg );


  mRubberSelectId->reset();

  QString formatLabel("%1(%2)");
  resultTargetLabel->setText( formatLabel.arg( mLayerTarget->name() ).arg( mFeatureResult.size() ) );
  invalidTargetLabel->setText( formatLabel.arg( mLayerTarget->name() ).arg( mFeatureInvalidTarget.size() ) );
  invalidReferenceLabel->setText( formatLabel.arg( mLayerReference->name() ).arg( mFeatureInvalidReference.size() ) );

  // Result target
  if ( mFeatureResult.size() > 0 )
  {
    pushButtonSelectResultTarget->setEnabled(true);
    populateFeatureListWidget( resultFeatureTargetListWidget, mFeatureResult );
    evaluateCheckBox( true );
    on_resultFeatureTargetListWidget_currentItemChanged( resultFeatureTargetListWidget->currentItem() );
  }
  else
  {
    pushButtonSelectResultTarget->setEnabled(false);
    clearFeatureListWidget(resultFeatureTargetListWidget);
  }
  // Invalid target
  if ( mFeatureInvalidTarget.size() > 0 )
  {
    pushButtonSelectInvalidTarget->setEnabled(true);
    populateFeatureListWidget( invalidFeatureTargetListWidget, mFeatureInvalidTarget, false );
  }
  else
  {
    pushButtonSelectInvalidTarget->setEnabled(false);
    clearFeatureListWidget(invalidFeatureTargetListWidget);
  }
  // Invalid reference
  if ( mFeatureInvalidReference.size() > 0 )
  {
    pushButtonSelectInvalidReference->setEnabled(true);
    populateFeatureListWidget( invalidFeatureReferenceListWidget, mFeatureInvalidReference, false );
  }
  else
  {
    pushButtonSelectInvalidReference->setEnabled(false);
    clearFeatureListWidget(invalidFeatureReferenceListWidget);
  }
  setLayoutResultInvalid( true );
  adjustSize();
} // void QgsSpatialQueryDialog::showResultQuery(QDateTime *datetimeStart, QDateTime *datetimeEnd)

QString QgsSpatialQueryDialog::getDescriptionLayerShow( bool isTarget )
{
  QgsVectorLayer* layer = NULL;
  QCheckBox * checkBox = NULL;
  if ( isTarget )
  {
    layer = mLayerTarget;
    checkBox = usingSelectedTargetCheckBox;
  }
  else
  {
    layer = mLayerReference;
    checkBox = usingSelectedReferenceCheckBox;
  }

  QString sDescFeatures = checkBox->isChecked()
                          ? tr( "%1 of %2" ).arg( layer->selectedFeatureCount() ).arg( layer->featureCount() )
                          : tr( "all = %1" ).arg( layer->featureCount() );

  return QString( "%1 (%2)" ).arg( layer->name() ).arg( sDescFeatures );

} // QString QgsSpatialQueryDialog::getDescriptionLayerShow(bool isTarget)

void QgsSpatialQueryDialog::connectAll()
{
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ),
           this, SLOT( signal_qgis_layerWasAdded( QgsMapLayer* ) ) ) ;
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ),
           this, SLOT( signal_qgis_layerWillBeRemoved( QString ) ) );
  connect( ckboxLogProcessing, SIGNAL( clicked( bool ) ),
           this, SLOT( on_ckboxLogProcessing_clicked( bool ) ) );

} // QgsSpatialQueryDialog::connectAll()

void QgsSpatialQueryDialog::disconnectAll()
{
  disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ),
              this, SLOT( signal_qgis_layerWasAdded( QgsMapLayer* ) ) ) ;
  disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ),
              this, SLOT( signal_qgis_layerWillBeRemoved( QString ) ) );

  if ( mLayerTarget )
  {
    disconnect( mLayerTarget, SIGNAL( selectionChanged() ),
                this, SLOT( signal_layerTarget_selectionFeaturesChanged() ) );

  }
  if ( mLayerReference )
  {
    disconnect( mLayerReference, SIGNAL( selectionChanged() ),
                this, SLOT( signal_layerReference_selectionFeaturesChanged() ) );
  }

} // QgsSpatialQueryDialog::disconnectAll()

void QgsSpatialQueryDialog::reject()
{
  disconnectAll();

  mRubberSelectId->reset();
  mLayerTarget = mLayerReference = NULL;
  mFeatureResult.clear();
  mFeatureInvalidTarget.clear();
  mFeatureInvalidReference.clear();
  mMapIdVectorLayers.clear();

  QDialog::reject();

} // QgsSpatialQueryDialog::reject()

QgsVectorLayer * QgsSpatialQueryDialog::getLayerFromCombobox( bool isTarget, int index )
{
  QVariant data = isTarget
                  ? targetLayerComboBox->itemData( index )
                  : referenceLayerComboBox->itemData( index );
  QgsVectorLayer* lyr = static_cast<QgsVectorLayer*>( data.value<void *>() );
  return lyr;

} // QgsVectorLayer * QgsSpatialQueryDialog::getLayerFromCombobox(bool isTarget, int index)

QIcon QgsSpatialQueryDialog::getIconTypeGeometry( QGis::GeometryType geomType )
{
  QString theName;
  if ( geomType == QGis::Point )
  {
    theName = "/mIconPointLayer.png";
  }
  else if ( geomType == QGis::Line )
  {
    theName = "/mIconLineLayer.png";
  }
  else // Polygon
  {
    theName = "/mIconPolygonLayer.png";
  }
  // Copy from qgisapp.cpp
  QString myPreferredPath = QgsApplication::activeThemePath() + QDir::separator() + theName;
  QString myDefaultPath = QgsApplication::defaultThemePath() + QDir::separator() + theName;
  if ( QFile::exists( myPreferredPath ) )
  {
    return QIcon( myPreferredPath );
  }
  else if ( QFile::exists( myDefaultPath ) )
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QIcon( myDefaultPath );
  }
  else
  {
    return QIcon();
  }

} // QIcon QgsSpatialQueryDialog::getIconTypeGeometry(int typeGeometry)

void QgsSpatialQueryDialog::addLayerCombobox( bool isTarget, QgsVectorLayer* vectorLayer )
{
  QVariant item = QVariant::fromValue(( void * )vectorLayer );
  QComboBox * cmb = isTarget ? targetLayerComboBox : referenceLayerComboBox;
  int idNew = cmb->count();
  QIcon icon = getIconTypeGeometry( vectorLayer->geometryType() );
  cmb->addItem( icon, vectorLayer->name(), item );
  cmb->setItemData( idNew, QVariant( vectorLayer->source() ), Qt::ToolTipRole );

} // void QgsSpatialQueryDialog::removeLayerCombobox(bool isTarget, QgsVectorLayer* vectorLayer)

int QgsSpatialQueryDialog::getIndexLayerCombobox( bool isTarget, QgsVectorLayer* vectorLayer )
{
  QVariant item = QVariant::fromValue(( void * )vectorLayer );
  QComboBox * cmb = isTarget ? targetLayerComboBox : referenceLayerComboBox;
  return cmb->findData( item );

} //

void QgsSpatialQueryDialog::removeLayer( bool isTarget, QgsVectorLayer* lyrRemove )
{
  QComboBox * cmb = isTarget ? targetLayerComboBox : referenceLayerComboBox;
  cmb->blockSignals( true );
  // Remove Combobox
  int index = getIndexLayerCombobox( isTarget, lyrRemove );
  if ( index > -1 )
  {
    cmb->removeItem( index );
  }
  else
  {
    return;
  }
  // Set Layers (Target or Reference)
  QgsVectorLayer* lyrThis = mLayerTarget;
  if ( !isTarget )
  {
    lyrThis = mLayerReference;
  }
  if ( lyrRemove == lyrThis )
  {
    lyrThis = NULL;
    if ( cmb->count() > 0 )
    {
      cmb->setCurrentIndex( 0 );
      setLayer( isTarget, 0 );
      evaluateCheckBox( isTarget );
      if ( isTarget )
      {
        clearFeatureListWidget( resultFeatureTargetListWidget );
        resultTargetLabel->setText( tr( "Total" ) + ": 0" );
        mRubberSelectId->reset();
      }
    }
  }
  cmb->blockSignals( false );

} // void QgsSpatialQueryDialog::removeLayer(bool isTarget, QgsVectorLayer* lyrRemove)

void QgsSpatialQueryDialog::populateTargetLayerComboBox()
{
  targetLayerComboBox->blockSignals( true );

  QMap <QString, QgsMapLayer*> map = QgsMapLayerRegistry::instance()->mapLayers();
  QMapIterator <QString, QgsMapLayer*> item( map );
  QgsMapLayer * mapLayer = NULL;
  QgsVectorLayer * vectorLayer = NULL;
  QString layerId;
  while ( item.hasNext() )
  {
    item.next();
    mapLayer = item.value();
    if ( mapLayer->type() != QgsMapLayer::VectorLayer )
    {
      continue;
    }
    vectorLayer = qobject_cast<QgsVectorLayer *>( mapLayer );
    if ( !vectorLayer )
    {
      continue;
    }

    addLayerCombobox( true, vectorLayer );
    mMapIdVectorLayers.insert( vectorLayer->getLayerID(), vectorLayer );
  }
  targetLayerComboBox->setCurrentIndex( 0 );
  targetLayerComboBox->blockSignals( false );

} // void QgsSpatialQueryDialog::populateTargetLayerComboBox()

void QgsSpatialQueryDialog::populateReferenceLayerComboBox()
{
  referenceLayerComboBox->blockSignals( true );
  referenceLayerComboBox->clear();

  // Populate new values and Set current item keeping the previous value
  QString itemText;
  QVariant itemData;
  QIcon itemIcon;
  QgsVectorLayer * itemLayer = NULL;
  int idNew = 0;
  for ( int id = 0; id < targetLayerComboBox->count(); id++ )
  {
    itemText = targetLayerComboBox->itemText( id );
    itemData = targetLayerComboBox->itemData( id );
    itemIcon = targetLayerComboBox->itemIcon( id );
    itemLayer = static_cast<QgsVectorLayer *>( itemData.value<void *>() );
    if ( itemLayer == mLayerTarget )
    {
      continue;
    }
    referenceLayerComboBox->addItem( itemIcon, itemText, itemData );
    referenceLayerComboBox->setItemData( idNew, QVariant( itemLayer->source() ), Qt::ToolTipRole );
    idNew++;
  }
  int idCurrent = getIndexLayerCombobox( false, mLayerReference );
  if ( idCurrent == -1 )
  {
    idCurrent = 0;
  }
  referenceLayerComboBox->setCurrentIndex( idCurrent );
  referenceLayerComboBox->blockSignals( false );

} // QgsSpatialQueryDialog::populateReferenceLayerComboBox()

void QgsSpatialQueryDialog::populateOperationComboBox()
{
  operantionComboBox->blockSignals( true );

  if ( mLayerTarget == NULL || mLayerReference == NULL )
  {
    operantionComboBox->clear();
    operantionComboBox->blockSignals( true );
  }

  QVariant currentValueItem;
  bool isStartEmpty = false;
  if ( operantionComboBox->count() == 0 )
  {
    isStartEmpty = true;
  }
  else
  {
    currentValueItem = operantionComboBox->itemData( operantionComboBox->currentIndex() );
  }

  // Populate new values
  QMap<QString, int> * map = QgsSpatialQuery::getTypesOperations( mLayerTarget, mLayerReference );
  QMapIterator <QString, int> item( *map );
  operantionComboBox->clear();
  while ( item.hasNext() )
  {
    item.next();
    operantionComboBox->addItem( item.key(), QVariant( item.value() ) );
  }
  delete map;

  // Set current item keeping the previous value
  int idCurrent = 0;
  if ( !isStartEmpty )
  {
    idCurrent = operantionComboBox->findData( currentValueItem );
    if ( idCurrent == -1 )
    {
      idCurrent = 0;
    }
  }
  operantionComboBox->setCurrentIndex( idCurrent );
  operantionComboBox->blockSignals( false );

} // QgsSpatialQueryDialog::populateOperantionComboBox()

void QgsSpatialQueryDialog::populateFeatureListWidget( QListWidget *listWidget, QSet<int> & setFeatures, bool hasSetRow)
{
  listWidget->blockSignals( true );
  listWidget->clear();
  listWidget->setEnabled( false );

  QSetIterator <int>item( setFeatures );
  while ( item.hasNext() )
  {
    listWidget->addItem( QString::number( item.next() ) );
  }
  listWidget->setEnabled( true );
  if (hasSetRow)
  {
    listWidget->setCurrentRow( 0 );
  }
  listWidget->blockSignals( false );

} // void populateFeatureListWidget( QListWidget *listWidget, QSet<int> & setFeatures )

void QgsSpatialQueryDialog::clearFeatureListWidget( QListWidget *listWidget )
{
   listWidget->blockSignals( true );
   listWidget->clear();
   listWidget->blockSignals( false );
} // void QgsSpatialQueryDialog::clearFeatureListWidget( QListWidget *listWidget )

void QgsSpatialQueryDialog::changeFeatureListWidget( QListWidget *listWidget, QgsVectorLayer* vectorLayer, const QString& currentText )
{
    listWidget->setEnabled( false );
    bool ok;
    int id = currentText.toInt( &ok );
    showRubberFeature( vectorLayer, id );
    listWidget->setEnabled( true );
    listWidget->setFocus();
} // void QgsSpatialQueryDialog::changeFeatureListWidget( QListWidget *listWidget, QgsVectorLayer* layer, const QString& currentText )

void QgsSpatialQueryDialog::showRubberFeature( QgsVectorLayer* vectorLayer, int id )
{
  mRubberSelectId->reset();

  Qt::CursorShape shapeCurrent = cursor().shape();

  QCursor c;
  c.setShape( Qt::WaitCursor );
  setCursor( c );

  mRubberSelectId->addFeature( vectorLayer, id );
  mRubberSelectId->show();

  c.setShape( shapeCurrent );
  setCursor( c );
} // void QgsSpatialQueryDialog::showRubberFeature( QgsVectorLayer* vectorLayer, int id )


//! Slots for signs of Dialog
void QgsSpatialQueryDialog::on_buttonBoxMain_accepted()
{
  if ( ! mLayerReference )
  {
    QMessageBox::warning( 0, tr( "Missing reference layer" ), tr( "Select reference layer!" ), QMessageBox::Ok );
    return;
  }
  if ( ! mLayerTarget )
  {
    QMessageBox::warning( 0, tr( "Missing target layer" ), tr( "Select target layer!" ), QMessageBox::Ok );
    return;
  }

  QDateTime datetimeStart = QDateTime::currentDateTime();
  runQuery();
  QDateTime datetimeEnd = QDateTime::currentDateTime();
  showResultQuery( &datetimeStart, &datetimeEnd );
  adjustSize();
} // QgsSpatialQueryDialog::on_buttonBoxMain_accepted()

void QgsSpatialQueryDialog::on_buttonBoxMain_rejected()
{
  if ( grpResult->isHidden() )
  {
    reject();
  }
  else
  {
    setLayoutResultInvalid( false );
    setLayoutOperationVisible( true );
    progressBarStatus->show();
    buttonBoxMain->button( QDialogButtonBox::Close )->hide();
    buttonBoxMain->button( QDialogButtonBox::Cancel )->show();
    buttonBoxMain->button( QDialogButtonBox::Ok )->show();
  }
  adjustSize();
} // void QgsSpatialQueryDialog::on_buttonBoxMain_rejected()

void QgsSpatialQueryDialog::on_targetLayerComboBox_currentIndexChanged( int index )
{
  // Add old target layer in reference combobox
  addLayerCombobox( false, mLayerTarget );

  // Set target layer
  setLayer( true, index );
  evaluateCheckBox( true );

  // Remove new target layer in reference combobox
  removeLayer( false, mLayerTarget );

  populateOperationComboBox();

} // QgsSpatialQueryDialog::on_targetLayerComboBox_currentIndexChanged(int index)

void QgsSpatialQueryDialog::on_referenceLayerComboBox_currentIndexChanged( int index )
{
  setLayer( false, index );
  evaluateCheckBox( false );

  populateOperationComboBox();

} // QgsSpatialQueryDialog::on_referenceLayerComboBox_currentIndexChanged(int index);

void QgsSpatialQueryDialog::on_resultFeatureTargetListWidget_itemClicked( QListWidgetItem * item )
{
  if ( mCurrentFeatureWidget != FW_Result )
  {
    mCurrentFeatureWidget = FW_Result;
    on_resultFeatureTargetListWidget_currentItemChanged(item);
  }
} // void QgsSpatialQueryDialog::on_resultFeatureTargetListWidget_itemClicked( QListWidgetItem * item )

void  QgsSpatialQueryDialog::on_resultFeatureTargetListWidget_currentItemChanged( QListWidgetItem * item )
{
  if ( mCurrentFeatureWidget != FW_Result )
  {
    mCurrentFeatureWidget = FW_Result;
  }
  mRubberSelectId->setColor( mRGBRubberSelect[0], mRGBRubberSelect[1], mRGBRubberSelect[2], 0.5, 2 );
  changeFeatureListWidget( resultFeatureTargetListWidget, mLayerTarget, item->text() );
} // void  QgsSpatialQueryDialog::on_resultFeatureTargetListWidget_currentItemChanged( QListWidgetItem * item )

void QgsSpatialQueryDialog::on_invalidFeatureTargetListWidget_itemClicked( QListWidgetItem * item )
{
  if ( mCurrentFeatureWidget != FW_InvalidTarget )
  {
    mCurrentFeatureWidget = FW_InvalidTarget;
    on_invalidFeatureTargetListWidget_currentItemChanged(item);
  }
} // void QgsSpatialQueryDialog::on_invalidFeatureTargetListWidget_itemClicked( QListWidgetItem * item )

void  QgsSpatialQueryDialog::on_invalidFeatureTargetListWidget_currentItemChanged( QListWidgetItem * item )
{
  if ( mCurrentFeatureWidget != FW_InvalidTarget )
  {
    mCurrentFeatureWidget = FW_InvalidTarget;
  }
  mRubberSelectId->setColor( 255, 0, 0, 0.5, 2 ); // RED
  changeFeatureListWidget( invalidFeatureTargetListWidget, mLayerTarget, item->text() );
} // void  QgsSpatialQueryDialog::on_invalidFeatureTargetListWidget_currentItemChanged( QListWidgetItem * item )

void QgsSpatialQueryDialog::on_invalidFeatureReferenceListWidget_itemClicked( QListWidgetItem * item )
{
  if ( mCurrentFeatureWidget != FW_InvalidRefence )
  {
    mCurrentFeatureWidget = FW_InvalidRefence;
    on_invalidFeatureReferenceListWidget_currentItemChanged(item);
  }
} // void QgsSpatialQueryDialog::on_invalidFeatureReferenceListWidget_itemClicked( QListWidgetItem * item )

void  QgsSpatialQueryDialog::on_invalidFeatureReferenceListWidget_currentItemChanged( QListWidgetItem * item )
{
  if ( mCurrentFeatureWidget != FW_InvalidRefence )
  {
    mCurrentFeatureWidget = FW_InvalidRefence;
  }
  mRubberSelectId->setColor( 255, 0, 0, 0.5, 2 ); // RED
  changeFeatureListWidget( invalidFeatureReferenceListWidget, mLayerReference, item->text() );
} // void  QgsSpatialQueryDialog::on_invalidFeatureReferenceListWidget_currentItemChanged( QListWidgetItem * item )

void QgsSpatialQueryDialog::on_ckboxLogProcessing_clicked( bool checked )
{
  showLogProcessing( checked );
  adjustSize();

} // void QgsSpatialQueryDialog::on_ckboxLogProcessing_clicked(bool checked)

void QgsSpatialQueryDialog::on_pushButtonSelectResultTarget_clicked()
{
  mLayerTarget->setSelectedFeatures( mFeatureResult );
} // void QgsSpatialQueryDialog::on_pushButtonSelectResultTarget_clicked()

void QgsSpatialQueryDialog::on_pushButtonSelectInvalidTarget_clicked()
{
  mLayerTarget->setSelectedFeatures( mFeatureInvalidTarget );
} // void QgsSpatialQueryDialog::on_pushButtonSelectInvalidTarget_clicked()

void QgsSpatialQueryDialog::on_pushButtonSelectInvalidReference_clicked()
{
  mLayerReference->setSelectedFeatures( mFeatureInvalidReference );
} // void QgsSpatialQueryDialog::on_pushButtonSelectInvalidReference_clicked()


//! Slots for signs of QGIS
void QgsSpatialQueryDialog::signal_qgis_layerWasAdded( QgsMapLayer* mapLayer )
{
  if ( mapLayer->type() != QgsMapLayer::VectorLayer )
  {
    return;
  }
  QgsVectorLayer * vectorLayer = qobject_cast<QgsVectorLayer *>( mapLayer );
  if ( !vectorLayer )
  {
    return;
  }
  addLayerCombobox( true, vectorLayer );
  addLayerCombobox( false, vectorLayer );
  mMapIdVectorLayers.insert( vectorLayer->getLayerID(), vectorLayer );

  // Verify is can enable buttonBox
  if ( !buttonBoxMain->button( QDialogButtonBox::Ok )->isEnabled() && targetLayerComboBox->count() > 1 )
  {
    buttonBoxMain->button( QDialogButtonBox::Ok )->setEnabled( true );
  }

} // QgsSpatialQueryDialog::signal_qgis_layerWasAdded(QgsMapLayer* mapLayer)

void QgsSpatialQueryDialog::signal_qgis_layerWillBeRemoved( QString idLayer )
{
  // If Frozen: the QGis can be: Exit, Add Project, New Project
  if ( mIface->mapCanvas()->isFrozen() )
  {
    reject();
  }
  // idLayer = QgsMapLayer::getLayerID()
  // Get Pointer layer removed
  QMap<QString, QgsVectorLayer *>::const_iterator i = mMapIdVectorLayers.find( idLayer );
  if ( i == mMapIdVectorLayers.end() )
  {
    return;
  }
  mMapIdVectorLayers.remove( idLayer );
  QgsVectorLayer *vectorLayer = i.value();
  removeLayer( true, vectorLayer ); // set new target if need
  removeLayer( false, vectorLayer ); // set new reference if need
  if ( mLayerTarget && getIndexLayerCombobox( referenceLayerComboBox, mLayerTarget ) > -1 )
  {
    removeLayer( false, mLayerTarget );
  }

  populateOperationComboBox();

  if ( targetLayerComboBox->count() < 2 )
  {
    buttonBoxMain->button( QDialogButtonBox::Ok )->setEnabled( false );
    textEditStatus->append( mMsgLayersLessTwo );
  }

} // QgsSpatialQueryDialog::signal_qgis_layerWillBeRemoved(QString idLayer)

//! Slots for signals of Layers (Target or Reference)
void QgsSpatialQueryDialog::signal_layerTarget_selectionFeaturesChanged()
{
  evaluateCheckBox( true );

} // void QgsSpatialQueryDialog::signal_layerTarget_selectionFeaturesChanged()

void QgsSpatialQueryDialog::signal_layerReference_selectionFeaturesChanged()
{
  evaluateCheckBox( false );

} // void QgsSpatialQueryDialog::signal_layerReference_selectionFeaturesChanged()

void QgsSpatialQueryDialog::MsgDEBUG( QString sMSg )
{
  QMessageBox::warning( 0, tr( "DEBUG" ), sMSg, QMessageBox::Ok );
}
