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

#include <QMessageBox>
#include <QDateTime>
#include <QPushButton>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"

#include "qgsspatialquerydialog.h"
#include "qgsspatialquery.h"
#include "qgsrubberselectid.h"
#include "qgsmngprogressbar.h"

QgsSpatialQueryDialog::QgsSpatialQueryDialog( QWidget* parent, QgisInterface* iface ): QDialog( parent )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "SpatialQuery/geometry" ).toByteArray() );

  mLayerReference = mLayerTarget = nullptr;
  mIface = iface;
  mRubberSelectId = new QgsRubberSelectId( iface->mapCanvas() );

  initGui();
  connectAll();
} // QgsSpatialQueryDialog::QgsSpatialQueryDialog( QWidget* parent, QgisInterface* iface )

QgsSpatialQueryDialog::~QgsSpatialQueryDialog()
{
  QSettings settings;
  settings.setValue( "SpatialQuery/geometry", saveGeometry() );

  disconnectAll();
  delete mRubberSelectId;
  mMapIdVectorLayers.clear();
  mFeatureResult.clear();
  mFeatureInvalidTarget.clear();
  mFeatureInvalidReference.clear();
} // QgsSpatialQueryDialog::~QgsSpatialQueryDialog()

bool QgsSpatialQueryDialog::hasPossibleQuery( QString &msg )
{
  // Count the number of vector layer
  QMap <QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
  QMapIterator <QString, QgsMapLayer*> item( layers );
  QgsMapLayer * mapLayer = nullptr;
  QgsVectorLayer * lyr = nullptr;
  unsigned int totalVector = 0;
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
    totalVector++;
  }
  // check is possible query
  if ( totalVector < 2 )
  {
    msg = tr( "The spatial query requires at least two vector layers" );
    return false;
  }
  else
  {
    return true;
  }
} // bool QgsSpatialQueryDialog::hasPossibleQuery( QString &msg )

void QgsSpatialQueryDialog::initGui()
{
  mRubberSelectId->setStyle( 250, 0, 0, 2 ); // Same identify
  visibleResult( false );
  populateTypeItems();
  populateCbTargetLayer();
  if ( cbTargetLayer->count() > 1 )
  {
    setLayer( true, 0 );
    setSelectedGui();
    evaluateCheckBoxLayer( true );
    populateCbReferenceLayer();
    setLayer( false, 0 );
    evaluateCheckBoxLayer( false );
    populateCbOperation();
  }
  else
  {
    bbMain->button( QDialogButtonBox::Apply )->hide();
  }
  populateCbResulFor(); // Depend if Target is selected
  adjustSize();
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

void QgsSpatialQueryDialog::evaluateCheckBoxLayer( bool isTarget )
{
  QgsVectorLayer* lyr = nullptr;
  QCheckBox* checkbox = nullptr;
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
  int selectedCount = lyr ? lyr->selectedFeatureCount() : 0;
  bool isCheckBoxValid = selectedCount > 0;
  checkbox->setChecked( isCheckBoxValid );
  checkbox->setEnabled( isCheckBoxValid );
  QString textCheckBox  = isCheckBoxValid
                          ? tr( "%n selected geometries", "selected geometries", selectedCount )
                          : tr( "Selected geometries" );
  checkbox->setText( textCheckBox );
} // void QgsSpatialQueryDialog::evaluateCheckBoxLayer(bool isTarget)

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

  int currentItem = cbOperation->currentIndex();
  int operation = cbOperation->itemData( currentItem ).toInt();
  spatialQuery->runQuery( mFeatureResult, mFeatureInvalidTarget, mFeatureInvalidReference, operation, mLayerTarget, mLayerReference );
  delete spatialQuery;
  delete pb;

  bbMain->setEnabled( true );
} // void QgsSpatialQueryDialog::runQuery()

void QgsSpatialQueryDialog::showResultQuery( QDateTime *datetimeStart, QDateTime *datetimeEnd )
{
  static int countQuery = 0;
  // Report processing
  countQuery++;
  QString msg = tr( "%1)Query" ).arg( countQuery );
  teStatus->append( msg );
  msg = tr( "Begin at %L1" ).arg( datetimeStart->toString() );
  teStatus->append( msg );
  teStatus->append( "" );
  msg = QString( "%1" ).arg( getDescriptionLayerShow( true ) );
  teStatus->append( msg );
  msg = tr( "< %1 >" ).arg( cbOperation->currentText() );
  teStatus->append( msg );
  msg = QString( "%1" ).arg( getDescriptionLayerShow( false ) );
  teStatus->append( msg );
  msg = tr( "Total of features = %1" ).arg( mFeatureResult.size() );
  teStatus->append( msg );
  teStatus->append( "" );
  teStatus->append( tr( "Total of invalid features:" ) );
  teStatus->append( getDescriptionInvalidFeaturesShow( true ) );
  teStatus->append( getDescriptionInvalidFeaturesShow( false ) );
  teStatus->append( "" );
  double timeProcess = ( double )datetimeStart->secsTo( *datetimeEnd ) / 60.0;
  msg = tr( "Finish at %L1 (processing time %L2 minutes)" ).arg( datetimeEnd->toString() ).arg( timeProcess, 0, 'f', 2 );
  teStatus->append( msg );
  teStatus->append( "" );

  ckbLogProcessing->setChecked( false );
  QVariant item = QVariant::fromValue(( int )itemsResult );
  int index = cbTypeItems->findData( item );
  cbTypeItems->setCurrentIndex( index );
  on_cbTypeItems_currentIndexChanged( index );

  // Result target
  if ( !mFeatureResult.isEmpty() )
  {
    // Select features
    TypeResultFor typeResultFor = ( TypeResultFor ) cbResultFor->itemData( cbResultFor->currentIndex() ).toInt();
    switch ( typeResultFor )
    {
      case selectedNew:
        mLayerTarget->selectByIds( mFeatureResult );
        break;
      case selectedAdd:
        mLayerTarget->selectByIds( mLayerTarget->selectedFeaturesIds() + mFeatureResult );
        break;
      case selectedRemove:
        mLayerTarget->selectByIds( mLayerTarget->selectedFeaturesIds() - mFeatureResult );
        break;
      default:
        return;
    }
  }
} // void QgsSpatialQueryDialog::showResultQuery(QDateTime *datetimeStart, QDateTime *datetimeEnd)

QString QgsSpatialQueryDialog::getSubsetFIDs( const QgsFeatureIds *fids, const QString& fieldFID )
{
  if ( fids->isEmpty() )
  {
    return QString();
  }
  QSetIterator <QgsFeatureId> item( *fids );
  QStringList lstFID;
  while ( item.hasNext() )
  {
    lstFID.append( FID_TO_STRING( item.next() ) );
  }
  QString qFormat( "%1 in (%2)" );
  QString qReturn  = qFormat.arg( fieldFID, lstFID.join( "," ) );
  lstFID.clear();
  return qReturn;
} // QString QgsSpatialQueryDialog::getSubsetFIDs( const QgsFeatureIds *fids, QString fieldFID )

QgsSpatialQueryDialog::TypeVerifyCreateSubset QgsSpatialQueryDialog::verifyCreateSubset( QString &msg, QString &fieldFID )
{
  QString providerType = mLayerTarget->providerType().toUpper();
  // OGR
  if ( providerType  == "OGR" )
  {
    fieldFID = QString( "FID" );
    return verifyOk;
  }
  // Database Postgis and Spatialite
  if ( providerType  == "POSTGRES" || providerType  == "SPATIALITE" )
  {
    fieldFID = mLayerTarget->dataProvider()->fields().at( 0 ).name();
    msg = tr( "Using the field \"%1\" for subset" ).arg( fieldFID );
    return verifyTry;
  }
  msg = tr( "Sorry! Only this providers are enable: OGR, POSTGRES and SPATIALITE." );
  return verifyImpossible;
} // TypeVerifyCreateSubset QgsSpatialQueryDialog::verifyCreateSubset(QString &msg, QString &fieldFID)

bool QgsSpatialQueryDialog::addLayerSubset( const QString& name, const QString& subset )
{
  QgsVectorLayer *addLyr = new QgsVectorLayer( mLayerTarget->source(), name, mLayerTarget->providerType() );
  if ( ! addLyr->setSubsetString( subset ) )
  {
    delete addLyr;
    return false;
  }
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << addLyr );
  return true;
} // bool QgsSpatialQueryDialog::addLayerSubset( QString name, QString subset )

QString QgsSpatialQueryDialog::getDescriptionLayerShow( bool isTarget )
{
  QgsVectorLayer* lyr = nullptr;
  QCheckBox * checkBox = nullptr;
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

  return QString( "%1 (%2)" ).arg( lyr->name(), sDescFeatures );
} // QString QgsSpatialQueryDialog::getDescriptionLayerShow(bool isTarget)

QString QgsSpatialQueryDialog::getDescriptionInvalidFeaturesShow( bool isTarget )
{
  QgsVectorLayer* lyr = nullptr;
  QCheckBox* checkBox = nullptr;
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

  return QString( "%1: %2" ).arg( lyr->name(), sDescFeatures );
} // QString QgsSpatialQueryDialog::getDescriptionInvalidFeatures(bool isTarget)

void QgsSpatialQueryDialog::connectAll()
{
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ),
           this, SLOT( signal_qgis_layerWasAdded( QgsMapLayer* ) ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ),
           this, SLOT( signal_qgis_layerWillBeRemoved( QString ) ) );
  connect( ckbLogProcessing, SIGNAL( clicked( bool ) ),
           this, SLOT( on_ckbLogProcessing_clicked( bool ) ) );
} // QgsSpatialQueryDialog::connectAll()

void QgsSpatialQueryDialog::disconnectAll()
{
  disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ),
              this, SLOT( signal_qgis_layerWasAdded( QgsMapLayer* ) ) );
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
  mLayerTarget = mLayerReference = nullptr;
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
    theName = "/mIconPointLayer.svg";
  }
  else if ( geomType == QGis::Line )
  {
    theName = "/mIconLineLayer.svg";
  }
  else // Polygon
  {
    theName = "/mIconPolygonLayer.svg";
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
}

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
    lyrThis = nullptr;
    if ( cmb->count() > 0 )
    {
      cmb->setCurrentIndex( 0 );
      setLayer( isTarget, 0 );
      evaluateCheckBoxLayer( isTarget );
      if ( isTarget )
      {
        if ( gbResultQuery->isVisible() )
        {
          visibleResult( false );
        }
      }
    }
  }
  cmb->blockSignals( false );
} // void QgsSpatialQueryDialog::removeLayer(bool isTarget, QgsVectorLayer* lyr)

void QgsSpatialQueryDialog::populateCbResulFor()
{
  cbResultFor->blockSignals( true );
  cbResultFor->clear();
  QVariant item;
  item = QVariant::fromValue(( int )selectedNew );
  cbResultFor->addItem( tr( "Create new selection" ), item );
  if ( mLayerTarget->selectedFeatureCount() == 0 )
  {
    return;
  }
  if ( ! ckbUsingSelectedTarget->isChecked() )
  {
    item = QVariant::fromValue(( int )selectedAdd );
    cbResultFor->addItem( tr( "Add to current selection" ), item );
  }
  item = QVariant::fromValue(( int )selectedRemove );
  cbResultFor->addItem( tr( "Remove from current selection" ), item );
  cbResultFor->blockSignals( false );
} // void QgsSpatialQueryDialog::populateCbResulFor()

void QgsSpatialQueryDialog::populateTypeItems()
{
  QVariant item;
  cbTypeItems->blockSignals( true );
  item = QVariant::fromValue(( int )itemsResult );
  cbTypeItems->addItem( tr( "Result query" ), item );
  item = QVariant::fromValue(( int )itemsInvalidTarget );
  cbTypeItems->addItem( tr( "Invalid source" ), item );
  item = QVariant::fromValue(( int )itemsInvalidReference );
  cbTypeItems->addItem( tr( "Invalid reference" ), item );
  cbTypeItems->blockSignals( false );
}

void QgsSpatialQueryDialog::populateCbTargetLayer()
{
  cbTargetLayer->blockSignals( true );

  QMap <QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
  QMapIterator <QString, QgsMapLayer*> item( layers );
  QgsMapLayer * mapLayer = nullptr;
  QgsVectorLayer * lyr = nullptr;
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
    mMapIdVectorLayers.insert( lyr->id(), lyr );
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
  QgsVectorLayer * lyr = nullptr;
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
  QVariant currentValueItem;
  bool isStartEmpty = false;
  if ( cbOperation->count() == 0 )
  {
    isStartEmpty = true;
  }
  else
  {
    currentValueItem = cbOperation->itemData( cbOperation->currentIndex() );
  }

  // Populate new values
  QMap<QString, int> * map = QgsSpatialQuery::getTypesOperations( mLayerTarget, mLayerReference );
  QMapIterator <QString, int> item( *map );
  cbOperation->blockSignals( true );
  cbOperation->clear();
  while ( item.hasNext() )
  {
    item.next();
    cbOperation->addItem( item.key(), QVariant( item.value() ) );
  }
  delete map;

  // Set current item keeping the previous value
  int idCurrent = 0;
  if ( !isStartEmpty )
  {
    idCurrent = cbOperation->findData( currentValueItem );
    if ( idCurrent == -1 )
    {
      idCurrent = 0;
    }
  }
  cbOperation->setCurrentIndex( idCurrent );
  cbOperation->blockSignals( false );
} // QgsSpatialQueryDialog::populatecbOperation()

void QgsSpatialQueryDialog::setSelectedGui()
{
  int selectedFeat = mLayerTarget->selectedFeatureCount();
  int totalFeat = mLayerTarget->featureCount();
  QString formatLabel( tr( "%1 of %2 selected by \"%3\"" ) );
  if ( ! mIsSelectedOperator )
  {
    mSourceSelected = tr( "user" );
  }
  lbStatusSelected->setText( formatLabel.arg( selectedFeat ).arg( totalFeat ).arg( mSourceSelected ) );
  mIsSelectedOperator = false;
  pbCreateLayerSelected->setEnabled( selectedFeat > 0 );
} // void QgsSpatialQueryDialog::setSelectedGui()

void QgsSpatialQueryDialog::changeLwFeature( QgsVectorLayer* lyr, QgsFeatureId fid )
{
  lwFeatures->setEnabled( false ); // The showRubberFeature can be slow
  showRubberFeature( lyr, fid );
  // Zoom
  if ( ckbZoomItem->isChecked() )
  {
    zoomFeature( lyr, fid );
  }
  lwFeatures->setEnabled( true );
  lwFeatures->setFocus();
} // void QgsSpatialQueryDialog::changeLwFeature( QgsVectorLayer* lyr, QgsFeatureId fid )

void QgsSpatialQueryDialog::zoomFeature( QgsVectorLayer* lyr, QgsFeatureId fid )
{
  static QgsVectorLayer* lyrCheck = nullptr;
  static bool hasMsg = false;
  if ( ! lyrCheck || lyrCheck != lyr )
  {
    lyrCheck = lyr;
    hasMsg = true;
  }
  else
  {
    hasMsg = false;
  }

  QgsFeature feat;
  if ( !lyr->getFeatures( QgsFeatureRequest().setFilterFid( fid ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( feat ) )
  {
    return;
  }
  if ( !feat.constGeometry() )
  {
    return;
  }
  // Set system reference
  QgsCoordinateReferenceSystem srsSource = lyr->dataProvider()->crs();
  QgsCoordinateReferenceSystem srcMapcanvas = mIface->mapCanvas()->mapSettings().destinationCrs();
  if ( ! srsSource.isValid() )
  {
    if ( hasMsg )
    {
      QString crsMapcanvas = srcMapcanvas.authid();
      bool isFly = mIface->mapCanvas()->mapSettings().hasCrsTransformEnabled();
      QString msgFly = tr( "Map \"%1\" \"on the fly\" transformation." ).arg( isFly ? tr( "enable" ) : tr( "disable" ) );
      QString msg = tr( "Coordinate reference system(CRS) of\n\"%1\" is invalid(see CRS of provider)." ).arg( lyr->name() );
      msg.append( tr( "\n\nCRS of map is %1.\n%2." ).arg( crsMapcanvas, msgFly ) );
      msg.append( "\n\nUsing CRS of map for all features!" );

      QMessageBox::warning( this, tr( "Zoom to feature" ), msg, QMessageBox::Ok );
    }
    mIface->mapCanvas()->setExtent( feat.constGeometry()->boundingBox() );
  }
  else if ( srsSource == srcMapcanvas )
  {
    mIface->mapCanvas()->setExtent( feat.constGeometry()->boundingBox() );
  }
  else
  {
    QgsCoordinateTransform * coordTransform =  new QgsCoordinateTransform( srsSource, srcMapcanvas );
    QgsRectangle rectExtent = coordTransform->transform( feat.constGeometry()->boundingBox() );
    delete coordTransform;
    mIface->mapCanvas()->setExtent( rectExtent );
  }
  mIface->mapCanvas()->refresh();
} // void QgsSpatialQueryDialog::zoomFeature( QgsVectorLayer* lyr, QgsFeatureId fid )

void QgsSpatialQueryDialog::showRubberFeature( QgsVectorLayer* lyr, QgsFeatureId id )
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

void QgsSpatialQueryDialog::apply()
{
  if ( ! mLayerReference )
  {
    QMessageBox::warning( nullptr, tr( "Missing reference layer" ), tr( "Select reference layer!" ), QMessageBox::Ok );
    return;
  }
  if ( ! mLayerTarget )
  {
    QMessageBox::warning( nullptr, tr( "Missing target layer" ), tr( "Select target layer!" ), QMessageBox::Ok );
    return;
  }

  pgbStatus->setVisible( true );
  QDateTime datetimeStart = QDateTime::currentDateTime();
  mSourceSelected = cbResultFor->currentText();
  mIsSelectedOperator = true;
  runQuery();
  QDateTime datetimeEnd = QDateTime::currentDateTime();
  if ( mFeatureResult.isEmpty() )
  {
    mIsSelectedOperator = false;
  }
  showResultQuery( &datetimeStart, &datetimeEnd );
  visibleResult( true );
} // void QgsSpatialQueryDialog::apply()

void QgsSpatialQueryDialog::visibleResult( bool show )
{
  blockSignals( true );
  if ( !show )
  {
    mRubberSelectId->reset();
  }
  leSpace->setVisible( show );
  pgbStatus->setVisible( show );
  gbResultQuery->setVisible( show );
  gbSelected->setVisible( show );
  ckbLogProcessing->setVisible( show );
  teStatus->setVisible( false );
  pgbStatus->setVisible( !show );
  blockSignals( false );
  adjustSize();
} // void QgsSpatialQueryDialog::visibleResult( bool show )

//! Slots for signs of Dialog
void QgsSpatialQueryDialog::on_bbMain_clicked( QAbstractButton * button )
{
  switch ( bbMain->buttonRole( button ) )
  {
    case QDialogButtonBox::ApplyRole:
      apply();
      break;
    case QDialogButtonBox::DestructiveRole:
    case QDialogButtonBox::RejectRole:
      reject();
      break;
    default:
      return;
  }
} // QgsSpatialQueryDialog::on_bbMain_accepted()

void QgsSpatialQueryDialog::on_pbCreateLayerItems_clicked()
{
  TypeItems typeItem = ( TypeItems ) cbTypeItems->itemData( cbTypeItems->currentIndex() ).toInt();
  QgsFeatureIds *fids = nullptr;
  switch ( typeItem )
  {
    case itemsResult:
      fids = &mFeatureResult;
      break;
    case itemsInvalidTarget:
      fids = &mFeatureInvalidTarget;
      break;
    case itemsInvalidReference:
      fids = &mFeatureInvalidReference;
      break;
    default:
      return;
  }
  QString title = tr( "Create new layer from items" );
  QString msg;
  QString fieldFID;
  TypeVerifyCreateSubset verify = verifyCreateSubset( msg, fieldFID );
  if ( verify == verifyImpossible )
  {
    QMessageBox::critical( this, title, msg, QMessageBox::Ok );
    return;
  }
  if ( verify == verifyTry )
  {
    QMessageBox::warning( this, title, msg, QMessageBox::Ok );
  }

  QString subset = getSubsetFIDs( fids, fieldFID );
  QString name = QString( "%1 < %2 > %3" ).arg( mLayerTarget->name(), cbOperation->currentText(), mLayerReference->name() );
  if ( ! addLayerSubset( name, subset ) )
  {
    msg = tr( "The query from \"%1\" using \"%2\" in field not possible." ).arg( mLayerTarget->name(), fieldFID );
    QMessageBox::critical( this, title, msg, QMessageBox::Ok );
  }
} // void QgsSpatialQueryDialog::on_pbCreateLayerItems_clicked()

void QgsSpatialQueryDialog::on_pbCreateLayerSelected_clicked()
{
  const QgsFeatureIds *fids = & ( mLayerTarget->selectedFeaturesIds() );
  QString title = tr( "Create new layer from selected" );
  QString msg;
  QString fieldFID;
  TypeVerifyCreateSubset verify = verifyCreateSubset( msg, fieldFID );
  if ( verify == verifyImpossible )
  {
    QMessageBox::critical( this, title, msg, QMessageBox::Ok );
    return;
  }
  if ( verify == verifyTry )
  {
    QMessageBox::warning( this, title, msg, QMessageBox::Ok );
  }

  QString subset = getSubsetFIDs( fids, fieldFID );
  QString name = QString( "%1 selected" ).arg( mLayerTarget->name() );
  if ( ! addLayerSubset( name, subset ) )
  {
    msg = tr( "The query from \"%1\" using \"%2\" in field not possible." ).arg( mLayerTarget->name(), fieldFID );
    QMessageBox::critical( this, title, msg, QMessageBox::Ok );
  }
} // void QgsSpatialQueryDialog::on_pbCreateLayerSelected_clicked()

void QgsSpatialQueryDialog::on_cbTargetLayer_currentIndexChanged( int index )
{
  // Add old target layer in reference combobox
  addCbLayer( false, mLayerTarget );

  // Set target layer
  setLayer( true, index );
  evaluateCheckBoxLayer( true );
  setSelectedGui();

  // Remove new target layer in reference combobox
  removeLayer( false, mLayerTarget );

  populateCbOperation();

  if ( gbResultQuery->isVisible() )
  {
    visibleResult( false );
  }
} // QgsSpatialQueryDialog::on_cbTargetLayer_currentIndexChanged(int index)

void QgsSpatialQueryDialog::on_cbReferenceLayer_currentIndexChanged( int index )
{
  setLayer( false, index );
  evaluateCheckBoxLayer( false );

  populateCbOperation();

  if ( gbResultQuery->isVisible() )
  {
    visibleResult( false );
  }
} // QgsSpatialQueryDialog::on_cbReferenceLayer_currentIndexChanged(int index);

void QgsSpatialQueryDialog::on_cbTypeItems_currentIndexChanged( int index )
{
  // Get Value type Item
  QVariant qtypItem = cbTypeItems->itemData( index );
  TypeItems typeItem = ( TypeItems ) qtypItem.toInt();

  QgsFeatureIds *setItems = nullptr;
  int totalFeat = mLayerTarget->featureCount();
  switch ( typeItem )
  {
    case itemsResult:
      setItems = &mFeatureResult;
      break;
    case itemsInvalidTarget:
      setItems = &mFeatureInvalidTarget;
      break;
    case itemsInvalidReference:
      setItems = &mFeatureInvalidReference;
      totalFeat = mLayerReference->featureCount();
      break;
    default:
      return;
  }

  lwFeatures->blockSignals( true );
  lwFeatures->clear();
  int totalItens = setItems->size();
  if ( totalItens > 0 )
  {
    // Populate lwFeatures
    QSetIterator <QgsFeatureId> item( *setItems );
    QListWidgetItem *lwItem = nullptr;
    while ( item.hasNext() )
    {
      lwItem = new QListWidgetItem( lwFeatures );
      QVariant fid  = QVariant( item.next() );
      lwItem->setData( Qt::UserRole, fid ); // Data
      lwItem->setData( Qt::DisplayRole, fid ); // Label
      lwFeatures->addItem( lwItem );
    }
    lwFeatures->sortItems();
    lwFeatures->blockSignals( false );
    lwFeatures->setCurrentRow( 0 ); // Has signal/slot for change current item in ListWidget
  }
  else
  {
    mRubberSelectId->reset();
    lwFeatures->blockSignals( false );
  }
  // Set lbStatusItems and pbCreateLayer
  QString formatLabel( tr( "%1 of %2 identified" ) );
  lbStatusItems->setText( formatLabel.arg( totalItens ).arg( totalFeat ) );
  pbCreateLayerItems->setEnabled( totalItens > 0 );
  ckbZoomItem->setEnabled( totalItens > 0 );
}

void QgsSpatialQueryDialog::on_cbResultFor_currentIndexChanged()
{
  if ( gbResultQuery->isVisible() )
  {
    visibleResult( false );
  }
} // void QgsSpatialQueryDialog::on_cbResultFor_currentIndexChanged()

void QgsSpatialQueryDialog::on_cbOperation_currentIndexChanged()
{
  if ( gbResultQuery->isVisible() )
  {
    visibleResult( false );
  }
} // void QgsSpatialQueryDialog::on_cbOperation_currentIndexChanged()

void QgsSpatialQueryDialog::on_lwFeatures_currentItemChanged( QListWidgetItem * item )
{
  TypeItems typeItem = ( TypeItems )( cbTypeItems->itemData( cbTypeItems->currentIndex() ).toInt() );
  QgsVectorLayer *lyr = typeItem == itemsInvalidReference ? mLayerReference : mLayerTarget;
  changeLwFeature( lyr, STRING_TO_FID( item->data( Qt::UserRole ).toString() ) );
} // void QgsSpatialQueryDialog::on_lwFeatures_currentItemChanged( QListWidgetItem * item )

void QgsSpatialQueryDialog::on_ckbUsingSelectedTarget_toggled()
{
  populateCbResulFor();
} // void QgsSpatialQueryDialog::on_ckbUsingSelectedTarget_clicked( bool checked )

void QgsSpatialQueryDialog::on_ckbLogProcessing_clicked( bool checked )
{
  teStatus->setVisible( checked );
  adjustSize();
} // void QgsSpatialQueryDialog::on_ckbLogProcessing_clicked(bool checked)

void QgsSpatialQueryDialog::on_ckbZoomItem_clicked( bool checked )
{
  if ( checked )
  {
    if ( lwFeatures->count() > 0 )
    {
      QgsFeatureId fid = STRING_TO_FID( lwFeatures->currentItem()->data( Qt::UserRole ).toString() );
      TypeItems typeItem = ( TypeItems )( cbTypeItems->itemData( cbTypeItems->currentIndex() ).toInt() );
      QgsVectorLayer *lyr = typeItem == itemsInvalidReference ? mLayerReference : mLayerTarget;
      zoomFeature( lyr, fid );
    }
  }
} // QgsSpatialQueryDialog::on_ckbZoomItem_clicked( bool checked )

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
  addCbLayer( true, lyr );
  if ( cbTargetLayer->count() > 1 && bbMain->button( QDialogButtonBox::Apply )->isHidden() )
  {
    bbMain->button( QDialogButtonBox::Apply )->show();
    cbOperation->setEnabled( true );
    cbResultFor->setEnabled( true );
  }
  addCbLayer( false, lyr );
  mMapIdVectorLayers.insert( lyr->id(), lyr );
} // QgsSpatialQueryDialog::signal_qgis_layerWasAdded(QgsMapLayer* mapLayer)

void QgsSpatialQueryDialog::signal_qgis_layerWillBeRemoved( const QString& idLayer )
{
  // If Frozen: the QGis can be: Exit, Add Project, New Project
  if ( mIface->mapCanvas()->isFrozen() )
  {
    reject();
  }
  // idLayer = QgsMapLayer::getLayerID()
  // Get Pointer layer removed
  QMap<QString, QgsVectorLayer *>::const_iterator i = mMapIdVectorLayers.constFind( idLayer );
  if ( i == mMapIdVectorLayers.constEnd() )
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

  if ( cbTargetLayer->count() < 2 )
  {
    bbMain->button( QDialogButtonBox::Apply )->hide();
    cbOperation->setEnabled( false );
    cbResultFor->setEnabled( false );
    if ( gbResultQuery->isVisible() )
    {
      visibleResult( false );
    }

    mLayerReference = nullptr;
    if ( cbTargetLayer->count() < 1 )
    {
      mLayerTarget = nullptr;
    }
  }
  else if ( mLayerTarget )
  {
    populateCbOperation();
  }
} // QgsSpatialQueryDialog::signal_qgis_layerWillBeRemoved(QString idLayer)

//! Slots for signals of Layers (Target or Reference)
void QgsSpatialQueryDialog::signal_layerTarget_selectionFeaturesChanged()
{
  evaluateCheckBoxLayer( true );
  setSelectedGui();
  adjustSize();
} // void QgsSpatialQueryDialog::signal_layerTarget_selectionFeaturesChanged()

void QgsSpatialQueryDialog::signal_layerReference_selectionFeaturesChanged()
{
  evaluateCheckBoxLayer( false );
} // void QgsSpatialQueryDialog::signal_layerReference_selectionFeaturesChanged()
