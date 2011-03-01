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
/*  $Id: qgsspatialquerydialog.cpp 15141 2011-02-08 13:34:43Z jef $ */

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
} // void QgsSpatialQueryDialog::messageLayersLessTwo()

void QgsSpatialQueryDialog::disconnectQGis()
{
  disconnectAll();

} // void QgsSpatialQueryDialog::disconnectQGis()

void QgsSpatialQueryDialog::initGui()
{
  mRubberSelectId->setStyle(250, 0, 0, 2); // Same identify

  showLogProcessing( false );
  setLayoutResultInvalid( false );

  bbMain->button( QDialogButtonBox::Close )->hide();

  populateCbTargetLayer();
  if ( cbTargetLayer->count() > 1 )
  {
    setLayer( true, 0 );
    evaluateCheckBox( true );
    populateCbReferenceLayer();
    setLayer( false, 0 );
    evaluateCheckBox( false );
    populateCbOperation();
  }
  else
  {
    bbMain->setEnabled( false );
    teStatus->append( mMsgLayersLessTwo );
  }

} // QgsSpatialQueryDialog::initGui()

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
  QgsVectorLayer* lyr = NULL;
  QCheckBox* checkbox = NULL;
  if ( isTarget )
  {
    lyr = mLayerTarget;
    checkbox = ckbUsingSelectedTarget;
  }
  else
  {
    lyr = mLayerReference;
    checkbox = ckbUsingSelectedReference;
  }
  int selectedCount = lyr->selectedFeatureCount();
  bool isCheckBoxValid = ( lyr != NULL &&  selectedCount > 0 );
  checkbox->setChecked( isCheckBoxValid );
  checkbox->setEnabled( isCheckBoxValid );
  QString textCheckBox  = isCheckBoxValid
                          ? tr( "%n selected geometries", "selected geometries", selectedCount )
                          : tr( "Selected geometries" );
  checkbox->setText( textCheckBox );

} // void QgsSpatialQueryDialog::evaluateCheckBox(bool isTarget)

void QgsSpatialQueryDialog::evaluateButtonSelected()
{
  QSet < int > selectedTarget = mLayerTarget->selectedFeaturesIds();
  // pbSelectResultTargetAdd and pbSelectResultTargetRemove => disable
  if( selectedTarget.isEmpty() )
  {
    pbSelectResultTargetAdd->setEnabled( false );
    pbSelectResultTargetRemove->setEnabled( false );
    return;
  }
  // pbSelectResultTargetAdd
  selectedTarget.contains( mFeatureResult ) || mFeatureResult.contains( selectedTarget )
      ? pbSelectResultTargetAdd->setEnabled( false )
      : pbSelectResultTargetAdd->setEnabled( true );
  // pbSelectResultTargetRemove
  selectedTarget.intersect( mFeatureResult ).isEmpty()
      ? pbSelectResultTargetRemove->setEnabled( false )
      : pbSelectResultTargetRemove->setEnabled( true );
} // void QgsSpatialQueryDialog::evaluateButtonSelected()

void QgsSpatialQueryDialog::runQuery()
{
  bbMain->setEnabled( false );
  MngProgressBar* pb = new MngProgressBar( pgbStatus );
  QgsSpatialQuery* spatialQuery = new QgsSpatialQuery( pb );
  if ( ckbUsingSelectedTarget->isChecked() )
  {
    spatialQuery->setSelectedFeaturesTarget( true );
  }
  if ( ckbUsingSelectedReference->isChecked() )
  {
    spatialQuery->setSelectedFeaturesReference( true );
  }
  pgbStatus->setTextVisible( true );
  mFeatureResult.clear();
  mFeatureInvalidTarget.clear();
  mFeatureInvalidReference.clear();

  int currentItem = cbOperantion->currentIndex();
  bool isOk;
  int operation = cbOperantion->itemData( currentItem ).toInt( &isOk );
  spatialQuery->runQuery( mFeatureResult, mFeatureInvalidTarget, mFeatureInvalidReference, operation, mLayerTarget, mLayerReference );
  delete spatialQuery;
  delete pb;

  pgbStatus->setTextVisible( false );
  bbMain->setEnabled( true );
  setLayoutOperationVisible( false );
  pgbStatus->hide();
  bbMain->button( QDialogButtonBox::Close )->show();
  bbMain->button( QDialogButtonBox::Cancel )->hide();
  bbMain->button( QDialogButtonBox::Ok )->hide();
} // void QgsSpatialQueryDialog::runQuery()

void QgsSpatialQueryDialog::setLayoutOperationVisible( bool show )
{
  grpTargetGroupBox->setVisible( show );
  grpReferenceGroupBox->setVisible( show );
  grpOperationGroupBox->setVisible( show );
} // void QgsSpatialQueryDialog::setLayoutOperationVisible( bool show )

void QgsSpatialQueryDialog::setLayoutResultInvalid( bool show )
{
  twResultInvalid->setVisible( show );
  ckbZoomItem->setVisible( show );
  ckbLogProcessing->setVisible( show );
  teStatus->setVisible( false ); // Never show
} // void QgsSpatialQueryDialog::setLayoutResultInvalid( bool show )

void QgsSpatialQueryDialog::showLogProcessing( bool hasShow )
{
  static int heightDialogNoStatus = 0;

  teStatus->setVisible( hasShow );
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
  QString msg = tr( "Begin at [%L1]" ).arg( datetimeStart->toString() );
  teStatus->append( msg );
  teStatus->append( "" );
  msg = QString( "%1" ).arg( getDescriptionLayerShow( true ) );
  teStatus->append( msg );
  msg = tr( "< %1 >" ).arg( cbOperantion->currentText() );
  teStatus->append( msg );
  msg = QString( "%1" ).arg( getDescriptionLayerShow( false ) );
  teStatus->append( msg );
  msg = tr( "Total of features =  %1" ).arg( mFeatureResult.size() );
  teStatus->append( msg );
  teStatus->append( "" );
  teStatus->append( tr("Total of invalid features:") );
  teStatus->append( getDescriptionInvalidFeaturesShow( true ) );
  teStatus->append( getDescriptionInvalidFeaturesShow( false ) );
  teStatus->append( "" );
  double timeProcess = ( double )datetimeStart->secsTo( *datetimeEnd ) / 60.0;
  msg = tr( "Finish at [%L1] (processing time %L2 minutes)" ).arg( datetimeEnd->toString() ).arg( timeProcess, 0, 'f', 2 );
  teStatus->append( msg );

  ckbLogProcessing->setChecked( false );

  lbResultTarget->setText( mLayerTarget->name() );
  lbInvalidTarget->setText( mLayerTarget->name() );
  lbInvalidReference->setText( mLayerReference->name() );

  QString formatLabel( tr("Feature IDs(%1)"));
  lbFIDresultTarget->setText( formatLabel.arg( mFeatureResult.size() ) );
  lbFIDinvalidTarget->setText( formatLabel.arg( mFeatureInvalidTarget.size() ) );
  lbFIDinvalidReference->setText( formatLabel.arg( mFeatureInvalidReference.size() ) );

  setLabelButtonSelected(lbSelected, mLayerTarget, pbSelectedSubsetLayer);

  // Result target
  if ( mFeatureResult.size() > 0 )
  {
    populateLwFeature( lwResultFeatureTarget, mFeatureResult );
    evaluateCheckBox( true );
    on_lwResultFeatureTarget_currentItemChanged( lwResultFeatureTarget->currentItem() );
    lwResultFeatureTarget->setEnabled( true );
    // Button
    pbSelectResultTarget->setEnabled( true );
    evaluateButtonSelected();
    ckbZoomItem->setEnabled( true ); // Show first the Result
  }
  else
  {
    pbSelectResultTarget->setEnabled( false );
    pbSelectResultTargetAdd->setEnabled( false );
    pbSelectResultTargetRemove->setEnabled( false );
    clearLwFeature( lwResultFeatureTarget );
    lwResultFeatureTarget->setEnabled( false );
    ckbZoomItem->setEnabled( false );
  }
  // Invalid target
  if ( mFeatureInvalidTarget.size() > 0 )
  {
    pbSelectInvalidTarget->setEnabled( true );
    populateLwFeature( lwInvalidFeatureTarget, mFeatureInvalidTarget );
    lwInvalidFeatureTarget->setEnabled( true );
  }
  else
  {
    pbSelectInvalidTarget->setEnabled( false );
    clearLwFeature( lwInvalidFeatureTarget );
    lwInvalidFeatureTarget->setEnabled( false );
  }
  // Invalid reference
  if ( mFeatureInvalidReference.size() > 0 )
  {
    pbSelectInvalidReference->setEnabled( true );
    populateLwFeature( lwInvalidFeatureReference, mFeatureInvalidReference );
    lwInvalidFeatureReference->setEnabled( true );
  }
  else
  {
    pbSelectInvalidReference->setEnabled( false );
    clearLwFeature( lwInvalidFeatureReference );
    lwInvalidFeatureReference->setEnabled( false );
  }
  setLayoutResultInvalid( true );
  adjustSize();
} // void QgsSpatialQueryDialog::showResultQuery(QDateTime *datetimeStart, QDateTime *datetimeEnd)

void QgsSpatialQueryDialog::setLabelButtonSelected( QLabel *lb,  QgsVectorLayer* lyr, QPushButton *pb)
{
  QString formatLabel("%1 of %2 selected features");
  int numSelected = lyr->selectedFeatureCount();
  lb->setText( formatLabel.arg( numSelected ).arg( lyr->featureCount() ) );
  numSelected > 0 ? pb->setEnabled( true ) : pb->setEnabled( false );
}

QString QgsSpatialQueryDialog::getSubsetSelected( QgsVectorLayer* lyr )
{
  QSet< int > selected = lyr->selectedFeaturesIds();
  if( selected.size() == 0 )
  {
    return QString("");
  }
  QSetIterator <int>item( selected );
  QStringList lstFID;
  while ( item.hasNext() )
  {
    lstFID.append( QString::number( item.next() ) );
  }
  QString qFormat("FID in (%1)");
  QString qReturn  = qFormat.arg( lstFID.join(",") );
  lstFID.clear();
  return qReturn;
} // QString QgsSpatialQueryDialog::getSubsetSelected( QgsVectorLayer* lyr )

QString QgsSpatialQueryDialog::getDescriptionLayerShow( bool isTarget )
{
  QgsVectorLayer* lyr = NULL;
  QCheckBox * checkBox = NULL;
  if ( isTarget )
  {
    lyr = mLayerTarget;
    checkBox = ckbUsingSelectedTarget;
  }
  else
  {
    lyr = mLayerReference;
    checkBox = ckbUsingSelectedReference;
  }

  QString sDescFeatures = checkBox->isChecked()
                          ? tr( "%1 of %2" ).arg( lyr->selectedFeatureCount() ).arg( lyr->featureCount() )
                          : tr( "all = %1" ).arg( lyr->featureCount() );

  return QString( "%1 (%2)" ).arg( lyr->name() ).arg( sDescFeatures );

} // QString QgsSpatialQueryDialog::getDescriptionLayerShow(bool isTarget)

QString QgsSpatialQueryDialog::getDescriptionInvalidFeaturesShow( bool isTarget )
{

  QgsVectorLayer* lyr = NULL;
  QCheckBox* checkBox = NULL;
  int totalInvalid = 0;
  if ( isTarget )
  {
    lyr = mLayerTarget;
    checkBox = ckbUsingSelectedTarget;
    totalInvalid = mFeatureInvalidTarget.size();
  }
  else
  {
    lyr = mLayerReference;
    checkBox = ckbUsingSelectedReference;
    totalInvalid = mFeatureInvalidReference.size();
  }


  QString sDescFeatures = checkBox->isChecked()
                          ? tr( "%1 of %2(selected features)" ).arg( totalInvalid ).arg( lyr->selectedFeatureCount() )
                          : tr( "%1 of %2" ).arg( totalInvalid ).arg( lyr->featureCount() );

  return QString( "%1: %2" ).arg( lyr->name() ).arg( sDescFeatures );

} // QString QgsSpatialQueryDialog::getDescriptionInvalidFeatures(bool isTarget)

void QgsSpatialQueryDialog::connectAll()
{
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ),
           this, SLOT( signal_qgis_layerWasAdded( QgsMapLayer* ) ) ) ;
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ),
           this, SLOT( signal_qgis_layerWillBeRemoved( QString ) ) );
  connect( ckbLogProcessing, SIGNAL( clicked( bool ) ),
           this, SLOT( on_ckbLogProcessing_clicked( bool ) ) );

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
                  ? cbTargetLayer->itemData( index )
                  : cbReferenceLayer->itemData( index );
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

void QgsSpatialQueryDialog::addCbLayer( bool isTarget, QgsVectorLayer* lyr )
{
  QVariant item = QVariant::fromValue(( void * )lyr );
  QComboBox * cmb = isTarget ? cbTargetLayer : cbReferenceLayer;
  int idNew = cmb->count();
  QIcon icon = getIconTypeGeometry( lyr->geometryType() );
  cmb->addItem( icon, lyr->name(), item );
  cmb->setItemData( idNew, QVariant( lyr->source() ), Qt::ToolTipRole );

} // void QgsSpatialQueryDialog::removeLayerCombobox(bool isTarget, QgsVectorLayer* lyr)

int QgsSpatialQueryDialog::getCbIndexLayer( bool isTarget, QgsVectorLayer* lyr )
{
  QVariant item = QVariant::fromValue(( void * )lyr );
  QComboBox * cmb = isTarget ? cbTargetLayer : cbReferenceLayer;
  return cmb->findData( item );

} //

void QgsSpatialQueryDialog::removeLayer( bool isTarget, QgsVectorLayer* lyr )
{
  QComboBox * cmb = isTarget ? cbTargetLayer : cbReferenceLayer;
  cmb->blockSignals( true );
  // Remove Combobox
  int index = getCbIndexLayer( isTarget, lyr );
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
  if ( lyr == lyrThis )
  {
    lyrThis = NULL;
    if ( cmb->count() > 0 )
    {
      cmb->setCurrentIndex( 0 );
      setLayer( isTarget, 0 );
      evaluateCheckBox( isTarget );
      mRubberSelectId->reset();
      QString lbFID("Feature IDs(0)");
      if ( isTarget )
      {
        // Result
        clearLwFeature( lwResultFeatureTarget );
        lbFIDresultTarget->setText( lbFID );
        pbSelectResultTarget->setEnabled( false );
        pbSelectResultTargetAdd->setEnabled( false );
        pbSelectResultTargetRemove->setEnabled( false );
        // Invalid
        clearLwFeature( lwInvalidFeatureTarget );
        lbFIDinvalidTarget->setText( lbFID );
        pbSelectInvalidTarget->setEnabled( false );
      }
      else
      {
        clearLwFeature( lwInvalidFeatureReference );
        lbFIDinvalidReference->setText( lbFID );
        pbSelectInvalidReference->setEnabled( false );
      }
    }
  }
  cmb->blockSignals( false );

} // void QgsSpatialQueryDialog::removeLayer(bool isTarget, QgsVectorLayer* lyr)

void QgsSpatialQueryDialog::populateCbTargetLayer()
{
  cbTargetLayer->blockSignals( true );

  QMap <QString, QgsMapLayer*> map = QgsMapLayerRegistry::instance()->mapLayers();
  QMapIterator <QString, QgsMapLayer*> item( map );
  QgsMapLayer * mapLayer = NULL;
  QgsVectorLayer * lyr = NULL;
  QString layerId;
  while ( item.hasNext() )
  {
    item.next();
    mapLayer = item.value();
    if ( mapLayer->type() != QgsMapLayer::VectorLayer )
    {
      continue;
    }
    lyr = qobject_cast<QgsVectorLayer *>( mapLayer );
    if ( !lyr )
    {
      continue;
    }

    addCbLayer( true, lyr );
    mMapIdVectorLayers.insert( lyr->getLayerID(), lyr );
  }
  cbTargetLayer->setCurrentIndex( 0 );
  cbTargetLayer->blockSignals( false );

} // void QgsSpatialQueryDialog::populateCbTargetLayer()

void QgsSpatialQueryDialog::populateCbReferenceLayer()
{
  cbReferenceLayer->blockSignals( true );
  cbReferenceLayer->clear();

  // Populate new values and Set current item keeping the previous value
  QString itemText;
  QVariant itemData;
  QIcon itemIcon;
  QgsVectorLayer * lyr = NULL;
  int idNew = 0;
  for ( int id = 0; id < cbTargetLayer->count(); id++ )
  {
    itemText = cbTargetLayer->itemText( id );
    itemData = cbTargetLayer->itemData( id );
    itemIcon = cbTargetLayer->itemIcon( id );
    lyr = static_cast<QgsVectorLayer *>( itemData.value<void *>() );
    if ( lyr == mLayerTarget )
    {
      continue;
    }
    cbReferenceLayer->addItem( itemIcon, itemText, itemData );
    cbReferenceLayer->setItemData( idNew, QVariant( lyr->source() ), Qt::ToolTipRole );
    idNew++;
  }
  int idCurrent = getCbIndexLayer( false, mLayerReference );
  if ( idCurrent == -1 )
  {
    idCurrent = 0;
  }
  cbReferenceLayer->setCurrentIndex( idCurrent );
  cbReferenceLayer->blockSignals( false );

} // QgsSpatialQueryDialog::populateCbReferenceLayer()

void QgsSpatialQueryDialog::populateCbOperation()
{
  cbOperantion->blockSignals( true );

  if ( mLayerTarget == NULL || mLayerReference == NULL )
  {
    cbOperantion->clear();
    cbOperantion->blockSignals( true );
  }

  QVariant currentValueItem;
  bool isStartEmpty = false;
  if ( cbOperantion->count() == 0 )
  {
    isStartEmpty = true;
  }
  else
  {
    currentValueItem = cbOperantion->itemData( cbOperantion->currentIndex() );
  }

  // Populate new values
  QMap<QString, int> * map = QgsSpatialQuery::getTypesOperations( mLayerTarget, mLayerReference );
  QMapIterator <QString, int> item( *map );
  cbOperantion->clear();
  while ( item.hasNext() )
  {
    item.next();
    cbOperantion->addItem( item.key(), QVariant( item.value() ) );
  }
  delete map;

  // Set current item keeping the previous value
  int idCurrent = 0;
  if ( !isStartEmpty )
  {
    idCurrent = cbOperantion->findData( currentValueItem );
    if ( idCurrent == -1 )
    {
      idCurrent = 0;
    }
  }
  cbOperantion->setCurrentIndex( idCurrent );
  cbOperantion->blockSignals( false );

} // QgsSpatialQueryDialog::populatecbOperantion()

void QgsSpatialQueryDialog::populateLwFeature( QListWidget *lw, QSet<int> & setFeatures )
{
  lw->blockSignals( true );
  lw->clear();
  QSetIterator <int>item( setFeatures );
  QListWidgetItem *lwItem = NULL;
  while ( item.hasNext() )
  {
    lwItem = new QListWidgetItem(lw);
    QVariant fid  = QVariant( item.next() );
    lwItem->setData( Qt::UserRole, fid ); // Data
    lwItem->setData( Qt::DisplayRole, fid ); // Label
    lw->addItem( lwItem );
  }
  lw->sortItems();
  lw->setCurrentRow(0);
  lw->blockSignals( false );
} // void populateLwFeature( QListWidget *lw, QSet<int> & setFeatures )

void QgsSpatialQueryDialog::clearLwFeature( QListWidget *listWidget )
{
  listWidget->blockSignals( true );
  listWidget->clear();
  listWidget->blockSignals( false );
} // void QgsSpatialQueryDialog::clearLwFeature( QListWidget *listWidget )

void QgsSpatialQueryDialog::changeLwFeature( QListWidget *listWidget, QgsVectorLayer* lyr, int fid )
{
  listWidget->setEnabled( false ); // The showRubberFeature can be slow
  showRubberFeature( lyr, fid );
  // Zoom
  if( ckbZoomItem->isChecked() )
  {
    zoomFeatureTarget(lyr, fid);
  }
  listWidget->setEnabled( true );
  listWidget->setFocus();
} // void QgsSpatialQueryDialog::changeLwFeature( QListWidget *listWidget, QgsVectorLayer* lyr, int fid )

void QgsSpatialQueryDialog::zoomFeatureTarget(QgsVectorLayer* lyr, int fid)
{
  QgsFeature feat;
  if ( !lyr->featureAtId( fid, feat, true, false ) )
  {
    return;
  }
  if ( !feat.geometry() )
  {
    return;
  }
  // Set system reference
  QgsCoordinateReferenceSystem srsSource = lyr->srs();
  QgsCoordinateReferenceSystem srcMapcanvas = mIface->mapCanvas()->mapRenderer()->destinationSrs();
  if( srsSource == srcMapcanvas)
  {
     mIface->mapCanvas()->setExtent( feat.geometry()->boundingBox() );
  }
  else
  {
    QgsCoordinateTransform * coordTransform =  new QgsCoordinateTransform( srsSource, srcMapcanvas );
    QgsRectangle rectExtent = coordTransform->transform( feat.geometry()->boundingBox() );
    delete coordTransform;
    mIface->mapCanvas()->setExtent( rectExtent );
  }
  mIface->mapCanvas()->refresh();
} // void QgsSpatialQueryDialog::zoomFeatureTarget(QgsVectorLayer* lyr, int fid)

void QgsSpatialQueryDialog::showRubberFeature( QgsVectorLayer* lyr, int id )
{
  mRubberSelectId->reset();

  Qt::CursorShape shapeCurrent = cursor().shape();

  QCursor c;
  c.setShape( Qt::WaitCursor );
  setCursor( c );

  mRubberSelectId->addFeature( lyr, id );
  mRubberSelectId->show();

  c.setShape( shapeCurrent );
  setCursor( c );
} // void QgsSpatialQueryDialog::showRubberFeature( QgsVectorLayer* lyr, int id )

//! Slots for signs of Dialog
void QgsSpatialQueryDialog::on_bbMain_accepted()
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
} // QgsSpatialQueryDialog::on_bbMain_accepted()

void QgsSpatialQueryDialog::on_bbMain_rejected()
{
  if ( twResultInvalid->isHidden() )
  {
    reject();
  }
  else
  {
    mRubberSelectId->reset();
    setLayoutResultInvalid( false );
    setLayoutOperationVisible( true );
    pgbStatus->show();
    bbMain->button( QDialogButtonBox::Close )->hide();
    bbMain->button( QDialogButtonBox::Cancel )->show();
    bbMain->button( QDialogButtonBox::Ok )->show();
  }
  adjustSize();
} // void QgsSpatialQueryDialog::on_bbMain_rejected()

void QgsSpatialQueryDialog::on_cbTargetLayer_currentIndexChanged( int index )
{
  // Add old target layer in reference combobox
  addCbLayer( false, mLayerTarget );

  // Set target layer
  setLayer( true, index );
  evaluateCheckBox( true );

  // Remove new target layer in reference combobox
  removeLayer( false, mLayerTarget );

  populateCbOperation();

} // QgsSpatialQueryDialog::on_cbTargetLayer_currentIndexChanged(int index)

void QgsSpatialQueryDialog::on_cbReferenceLayer_currentIndexChanged( int index )
{
  setLayer( false, index );
  evaluateCheckBox( false );

  populateCbOperation();

} // QgsSpatialQueryDialog::on_cbReferenceLayer_currentIndexChanged(int index);

void QgsSpatialQueryDialog::on_lwResultFeatureTarget_currentItemChanged( QListWidgetItem * item )
{
  bool isOk;
  int fid = item->data(Qt::UserRole).toInt( &isOk );
  changeLwFeature( lwResultFeatureTarget, mLayerTarget, fid );
} // void QgsSpatialQueryDialog::on_lwResultFeatureTarget_currentItemChanged( QListWidgetItem * item )

void QgsSpatialQueryDialog::on_twResultInvalid_currentChanged ( int index )
{
  void ( QgsSpatialQueryDialog::* on_lw_currentItemChanged )( QListWidgetItem * ) = NULL;
  QListWidget *lw = NULL;
  switch (index)
  {
    case 0: // Result target
      on_lw_currentItemChanged = &QgsSpatialQueryDialog::on_lwResultFeatureTarget_currentItemChanged;
      lw = lwResultFeatureTarget;
      break;
    case 1: // Invalid target
      on_lw_currentItemChanged = &QgsSpatialQueryDialog::on_lwInvalidFeatureTarget_currentItemChanged;
      lw = lwInvalidFeatureTarget;
      break;
    case 2: // Invalid reference
      on_lw_currentItemChanged = &QgsSpatialQueryDialog::on_lwInvalidFeatureReference_currentItemChanged;
      lw = lwInvalidFeatureReference;
      break;
    default:
      return;
  }
  if( lw->count() > 0)
  {
    ckbZoomItem->setEnabled( true );
    ( this->*on_lw_currentItemChanged )( lw->currentItem() );
  }
  else
  {
    ckbZoomItem->setEnabled( false );
  }
} // void QgsSpatialQueryDialog::on_twResultInvalid_currentChanged ( int index )

void QgsSpatialQueryDialog::on_lwInvalidFeatureTarget_currentItemChanged( QListWidgetItem * item )
{
  bool isOk;
  int fid = item->data(Qt::UserRole).toInt( &isOk );
  changeLwFeature( lwInvalidFeatureTarget, mLayerTarget, fid );
} // void QgsSpatialQueryDialog::on_lwInvalidFeatureTarget_currentItemChanged( QListWidgetItem * item )

void QgsSpatialQueryDialog::on_lwInvalidFeatureReference_currentItemChanged( QListWidgetItem * item )
{
  bool isOk;
  int fid = item->data(Qt::UserRole).toInt( &isOk );
  changeLwFeature( lwInvalidFeatureReference, mLayerReference, fid );
} // void QgsSpatialQueryDialog::on_lwInvalidFeatureReference_currentItemChanged( QListWidgetItem * item )

void QgsSpatialQueryDialog::on_ckbLogProcessing_clicked( bool checked )
{
  showLogProcessing( checked );
  adjustSize();

} // void QgsSpatialQueryDialog::on_ckbLogProcessing_clicked(bool checked)

void QgsSpatialQueryDialog::on_ckbZoomItem_clicked( bool checked )
{
  if( checked )
  {
    QListWidget *lw  = NULL;
    QgsVectorLayer *lyr = NULL;
    int index = twResultInvalid->currentIndex();
    switch (index)
    {
      case 0: // Result target
        lyr = mLayerTarget;
        lw = lwResultFeatureTarget;
        break;
      case 1: // Invalid target
        lyr = mLayerTarget;
        lw = lwInvalidFeatureTarget;
        break;
      case 2: // Invalid reference
        lyr = mLayerReference;
        lw = lwInvalidFeatureReference;
        break;
      default:
        return;
    }
    if( lw->count() > 0 )
    {
      bool ok;
      int fid = lw->currentItem()->data(Qt::UserRole).toInt(&ok);
      zoomFeatureTarget(lyr, fid);
    }
  }
} // QgsSpatialQueryDialog::on_ckbZoomItem_clicked( bool checked )

void QgsSpatialQueryDialog::on_pbSelectResultTarget_clicked()
{
  mLayerTarget->setSelectedFeatures( mFeatureResult );
} // void QgsSpatialQueryDialog::on_pbSelectResultTarget_clicked()

void QgsSpatialQueryDialog::on_pbSelectResultTargetAdd_clicked()
{
  mLayerTarget->setSelectedFeatures( mLayerTarget->selectedFeaturesIds() + mFeatureResult );
} // void QgsSpatialQueryDialog::on_pbSelectResultTargetAdd_clicked()

void QgsSpatialQueryDialog::on_pbSelectResultTargetRemove_clicked()
{
  mLayerTarget->setSelectedFeatures( mLayerTarget->selectedFeaturesIds() - mFeatureResult );
} // void QgsSpatialQueryDialog::on_pbSelectResultTargetRemove_clicked()

void QgsSpatialQueryDialog::on_pbSelectedSubsetLayer_clicked()
{
  mLayerTarget->setSubsetString( getSubsetSelected( mLayerTarget ) );
  mLayerTarget->removeSelection();
  mIface->mapCanvas()->refresh();
} // void QgsSpatialQueryDialog::on_pbSelectedSubsetLayer_clicked()

void QgsSpatialQueryDialog::on_pbSelectInvalidTarget_clicked()
{
  mLayerTarget->setSelectedFeatures( mFeatureInvalidTarget );
} // void QgsSpatialQueryDialog::on_pbSelectInvalidTarget_clicked()

void QgsSpatialQueryDialog::on_pbSelectInvalidReference_clicked()
{
  mLayerReference->setSelectedFeatures( mFeatureInvalidReference );
} // void QgsSpatialQueryDialog::on_pbSelectInvalidReference_clicked()


//! Slots for signs of QGIS
void QgsSpatialQueryDialog::signal_qgis_layerWasAdded( QgsMapLayer* mapLayer )
{
  if ( mapLayer->type() != QgsMapLayer::VectorLayer )
  {
    return;
  }
  QgsVectorLayer * lyr = qobject_cast<QgsVectorLayer *>( mapLayer );
  if ( !lyr )
  {
    return;
  }
<<<<<<< HEAD
  addCbLayer( true, vectorLayer );
  addCbLayer( false, vectorLayer );
  mMapIdVectorLayers.insert( vectorLayer->getLayerID(), vectorLayer );
=======
  addCbLayer( true, lyr );
  addCbLayer( false, lyr );
  mMapIdVectorLayers.insert( lyr->getLayerID(), lyr );

>>>>>>> Updates from Luiz for spatial query plugin
  // Verify is can enable buttonBox
  if ( !bbMain->button( QDialogButtonBox::Ok )->isEnabled() && cbTargetLayer->count() > 1 )
  {
    bbMain->button( QDialogButtonBox::Ok )->setEnabled( true );
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
  QgsVectorLayer *lyr = i.value();
  removeLayer( true, lyr ); // set new target if need
  removeLayer( false, lyr ); // set new reference if need
  if ( mLayerTarget && getCbIndexLayer( cbReferenceLayer, mLayerTarget ) > -1 )
  {
    removeLayer( false, mLayerTarget );
  }

  populateCbOperation();

  if ( cbTargetLayer->count() < 2 )
  {
    bbMain->button( QDialogButtonBox::Ok )->setEnabled( false );
    teStatus->append( mMsgLayersLessTwo );
  }

} // QgsSpatialQueryDialog::signal_qgis_layerWillBeRemoved(QString idLayer)

//! Slots for signals of Layers (Target or Reference)
void QgsSpatialQueryDialog::signal_layerTarget_selectionFeaturesChanged()
{
  evaluateCheckBox( true );
  evaluateButtonSelected();
  setLabelButtonSelected(lbSelected, mLayerTarget, pbSelectedSubsetLayer);
} // void QgsSpatialQueryDialog::signal_layerTarget_selectionFeaturesChanged()

void QgsSpatialQueryDialog::signal_layerReference_selectionFeaturesChanged()
{
  evaluateCheckBox( false );

} // void QgsSpatialQueryDialog::signal_layerReference_selectionFeaturesChanged()

void QgsSpatialQueryDialog::MsgDEBUG( QString sMSg )
{
  QMessageBox::warning( 0, tr( "DEBUG" ), sMSg, QMessageBox::Ok );
}
