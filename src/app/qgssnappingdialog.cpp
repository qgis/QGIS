/***************************************************************************
                              qgssnappingdialog.cpp
                              ---------------------
  begin                : June 11, 2007
  copyright            : (C) 2007 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssnappingdialog.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"
#include "qgisapp.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgsdockwidget.h"

#include <QCheckBox>
#include <QDoubleValidator>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleSpinBox>

QgsSnappingDialog::QgsSnappingDialog( QWidget* parent, QgsMapCanvas* canvas )
    : QDialog( parent )
    , mMapCanvas( canvas )
    , mDock( nullptr )
{
  setupUi( this );

  mDefaultSnapToComboBox->insertItem( 0, tr( "To vertex" ), "to vertex" );
  mDefaultSnapToComboBox->insertItem( 1, tr( "To segment" ), "to segment" );
  mDefaultSnapToComboBox->insertItem( 2, tr( "To vertex and segment" ), "to vertex and segment" );
  mDefaultSnapToComboBox->insertItem( 3, tr( "Off" ), "off" );

  QSettings myQsettings;
  bool myDockFlag = myQsettings.value( "/qgis/dockSnapping", false ).toBool();
  if ( myDockFlag )
  {
    mDock = new QgsSnappingDock( tr( "Snapping and Digitizing Options" ), QgisApp::instance() );
    mDock->setAllowedAreas( Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea );
    mDock->setWidget( this );
    connect( this, SIGNAL( destroyed() ), mDock, SLOT( close() ) );
    QgisApp::instance()->addDockWidget( Qt::BottomDockWidgetArea, mDock );
    mButtonBox->setVisible( false );

    connect( mSnapModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( apply() ) );
    connect( mDefaultSnapToComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( apply() ) );
    connect( mDefaultSnappingToleranceSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( apply() ) );
    connect( mDefaultSnappingToleranceComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( apply() ) );
  }
  else
  {
    connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( apply() ) );
    connect( mButtonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  }
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer * > ) ), this, SLOT( addLayers( QList<QgsMapLayer * > ) ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( layersWillBeRemoved( QStringList ) ) );
  connect( mSnapModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onSnappingModeIndexChanged( int ) ) );
  connect( cbxEnableTopologicalEditingCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( on_cbxEnableTopologicalEditingCheckBox_stateChanged( int ) ) );
  connect( cbxEnableIntersectionSnappingCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( on_cbxEnableIntersectionSnappingCheckBox_stateChanged( int ) ) );

  reload();

  QMap< QString, QgsMapLayer *> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap< QString, QgsMapLayer *>::iterator it;
  for ( it = mapLayers.begin(); it != mapLayers.end() ; ++it )
  {
    addLayer( it.value() );
  }

  mLayerTreeWidget->setHeaderLabels( QStringList() << "" );
  mLayerTreeWidget->setSortingEnabled( true );

  connect( QgsProject::instance(), SIGNAL( snapSettingsChanged() ), this, SLOT( reload() ) );
  connect( QgisApp::instance(), SIGNAL( newProject() ), this, SLOT( initNewProject() ) );
  connect( QgisApp::instance(), SIGNAL( projectRead() ), this, SLOT( reload() ) );
}

QgsSnappingDialog::QgsSnappingDialog()
    : mMapCanvas( nullptr )
    , mDock( nullptr )
{
}

QgsSnappingDialog::~QgsSnappingDialog()
{
}


void QgsSnappingDialog::reload()
{
  setSnappingMode();

  int idx;
  QSettings settings;
  QString snapType = settings.value( "/qgis/digitizing/default_snap_mode", "off" ).toString();
  snapType = QgsProject::instance()->readEntry( "Digitizing", "/DefaultSnapType", snapType );
  if ( snapType == "to segment" )
    idx = 1;
  else if ( snapType == "to vertex and segment" )
    idx = 2;
  else if ( snapType == "to vertex" )
    idx = 0;
  else // off
    idx = 3;
  mDefaultSnapToComboBox->blockSignals( true );
  mDefaultSnapToComboBox->setCurrentIndex( idx );
  mDefaultSnapToComboBox->blockSignals( false );

  double tolerance = settings.value( "/qgis/digitizing/default_snapping_tolerance", 0 ).toDouble();
  tolerance = QgsProject::instance()->readDoubleEntry( "Digitizing", "/DefaultSnapTolerance", tolerance );
  mDefaultSnappingToleranceSpinBox->blockSignals( true );
  mDefaultSnappingToleranceSpinBox->setValue( tolerance );
  mDefaultSnappingToleranceSpinBox->blockSignals( false );

  int unit = settings.value( "/qgis/digitizing/default_snapping_tolerance_unit", QgsTolerance::ProjectUnits ).toInt();
  unit = QgsProject::instance()->readNumEntry( "Digitizing", "/DefaultSnapToleranceUnit", unit );
  mDefaultSnappingToleranceComboBox->blockSignals( true );
  mDefaultSnappingToleranceComboBox->setCurrentIndex( unit == QgsTolerance::Pixels ? 1 : 0 );
  mDefaultSnappingToleranceComboBox->blockSignals( false );

  mLayerTreeWidget->clear();

  QMap< QString, QgsMapLayer *> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap< QString, QgsMapLayer *>::iterator it;
  for ( it = mapLayers.begin(); it != mapLayers.end() ; ++it )
  {
    addLayer( it.value() );
  }

  setTopologicalEditingState();
  setIntersectionSnappingState();
}

void QgsSnappingDialog::on_cbxEnableTopologicalEditingCheckBox_stateChanged( int state )
{
  QgsProject::instance()->writeEntry( "Digitizing", "/TopologicalEditing", state == Qt::Checked );
  setTopologicalEditingState();
}

void QgsSnappingDialog::on_cbxEnableIntersectionSnappingCheckBox_stateChanged( int state )
{
  QgsProject::instance()->writeEntry( "Digitizing", "/IntersectionSnapping", state == Qt::Checked );
}

void QgsSnappingDialog::onSnappingModeIndexChanged( int index )
{
  if ( index == 0 || index == 1 )
    mStackedWidget->setCurrentIndex( 0 );
  else
    mStackedWidget->setCurrentIndex( 1 );
}

void QgsSnappingDialog::initNewProject()
{
  QgsProject::instance()->writeEntry( "Digitizing", "/SnappingMode", QString( "current_layer" ) );

  QSettings settings;
  QString snapType = settings.value( "/qgis/digitizing/default_snap_mode", "off" ).toString();
  QgsProject::instance()->writeEntry( "Digitizing", "/DefaultSnapType", snapType );
  double tolerance = settings.value( "/qgis/digitizing/default_snapping_tolerance", 0 ).toDouble();
  QgsProject::instance()->writeEntry( "Digitizing", "/DefaultSnapTolerance", tolerance );
  int unit = settings.value( "/qgis/digitizing/default_snapping_tolerance_unit", QgsTolerance::ProjectUnits ).toInt();
  QgsProject::instance()->writeEntry( "Digitizing", "/DefaultSnapToleranceUnit", unit );

  reload();
  emitProjectSnapSettingsChanged();
}

void QgsSnappingDialog::closeEvent( QCloseEvent* event )
{
  QDialog::closeEvent( event );

  if ( !mDock )
  {
    QSettings settings;
    settings.setValue( "/Windows/BetterSnapping/geometry", saveGeometry() );
  }
}

void QgsSnappingDialog::apply()
{
  QString snapMode;
  switch ( mSnapModeComboBox->currentIndex() )
  {
    case 0:
      snapMode = "current_layer";
      break;
    case 1:
      snapMode = "all_layers";
      break;
    default:
      snapMode = "advanced";
      break;
  }
  QgsProject::instance()->writeEntry( "Digitizing", "/SnappingMode", snapMode );

  QString snapType = mDefaultSnapToComboBox->itemData( mDefaultSnapToComboBox->currentIndex() ).toString();
  QgsProject::instance()->writeEntry( "Digitizing", "/DefaultSnapType", snapType );
  QgsProject::instance()->writeEntry( "Digitizing", "/DefaultSnapTolerance", mDefaultSnappingToleranceSpinBox->value() );
  QgsProject::instance()->writeEntry( "Digitizing", "/DefaultSnapToleranceUnit", mDefaultSnappingToleranceComboBox->currentIndex() == 1 ? QgsTolerance::Pixels : QgsTolerance::ProjectUnits );


  QStringList layerIdList;
  QStringList snapToList;
  QStringList toleranceList;
  QStringList enabledList;
  QStringList toleranceUnitList;
  QStringList avoidIntersectionList;

  for ( int i = 0; i < mLayerTreeWidget->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem *currentItem = mLayerTreeWidget->topLevelItem( i );
    if ( !currentItem )
    {
      continue;
    }

    layerIdList << currentItem->data( 0, Qt::UserRole ).toString();
    enabledList << ( qobject_cast<QCheckBox*>( mLayerTreeWidget->itemWidget( currentItem, 0 ) )->isChecked() ? "enabled" : "disabled" );

    QString snapToItemText = qobject_cast<QComboBox*>( mLayerTreeWidget->itemWidget( currentItem, 2 ) )->currentText();
    if ( snapToItemText == tr( "to vertex" ) )
    {
      snapToList << "to_vertex";
    }
    else if ( snapToItemText == tr( "to segment" ) )
    {
      snapToList << "to_segment";
    }
    else //to vertex and segment
    {
      snapToList << "to_vertex_and_segment";
    }

    toleranceList << QString::number( qobject_cast<QDoubleSpinBox*>( mLayerTreeWidget->itemWidget( currentItem, 3 ) )->value(), 'f' );
    toleranceUnitList << QString::number( qobject_cast<QComboBox*>( mLayerTreeWidget->itemWidget( currentItem, 4 ) )->currentIndex() );

    QCheckBox *cbxAvoidIntersection = qobject_cast<QCheckBox*>( mLayerTreeWidget->itemWidget( currentItem, 5 ) );
    if ( cbxAvoidIntersection && cbxAvoidIntersection->isChecked() )
    {
      avoidIntersectionList << currentItem->data( 0, Qt::UserRole ).toString();
    }
  }

  QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnappingList", layerIdList );
  QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnapToList", snapToList );
  QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnappingToleranceList", toleranceList );
  QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnappingToleranceUnitList", toleranceUnitList );
  QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnappingEnabledList", enabledList );
  QgsProject::instance()->writeEntry( "Digitizing", "/AvoidIntersectionsList", avoidIntersectionList );

  emitProjectSnapSettingsChanged();
}

void QgsSnappingDialog::emitProjectSnapSettingsChanged()
{
  disconnect( QgsProject::instance(), SIGNAL( snapSettingsChanged() ), this, SLOT( reload() ) );
  connect( this, SIGNAL( snapSettingsChanged() ), QgsProject::instance(), SIGNAL( snapSettingsChanged() ) );

  emit snapSettingsChanged();

  disconnect( this, SIGNAL( snapSettingsChanged() ), QgsProject::instance(), SIGNAL( snapSettingsChanged() ) );
  connect( QgsProject::instance(), SIGNAL( snapSettingsChanged() ), this, SLOT( reload() ) );
}

void QgsSnappingDialog::show()
{
  setTopologicalEditingState();
  setIntersectionSnappingState();
  if ( mDock )
    mDock->setVisible( true );
  else
    QDialog::show();

  mLayerTreeWidget->resizeColumnToContents( 0 );
  mLayerTreeWidget->resizeColumnToContents( 1 );
  mLayerTreeWidget->resizeColumnToContents( 2 );
  mLayerTreeWidget->resizeColumnToContents( 3 );
  mLayerTreeWidget->resizeColumnToContents( 4 );
}

void QgsSnappingDialog::addLayers( const QList<QgsMapLayer *>& layers )
{
  Q_FOREACH ( QgsMapLayer* layer, layers )
  {
    addLayer( layer );
  }
}

void QgsSnappingDialog::addLayer( QgsMapLayer *theMapLayer )
{
  QgsVectorLayer *currentVectorLayer = qobject_cast<QgsVectorLayer *>( theMapLayer );
  if ( !currentVectorLayer || currentVectorLayer->geometryType() == QGis::NoGeometry )
    return;

  QSettings myQsettings;
  bool myDockFlag = myQsettings.value( "/qgis/dockSnapping", false ).toBool();
  double defaultSnappingTolerance = myQsettings.value( "/qgis/digitizing/default_snapping_tolerance", 0 ).toDouble();
  int defaultSnappingUnit = myQsettings.value( "/qgis/digitizing/default_snapping_tolerance_unit", QgsTolerance::ProjectUnits ).toInt();
  QString defaultSnappingString = myQsettings.value( "/qgis/digitizing/default_snap_mode", "to vertex" ).toString();

  int defaultSnappingStringIdx = 0;
  if ( defaultSnappingString == "to vertex" )
  {
    defaultSnappingStringIdx = 0;
  }
  else if ( defaultSnappingString == "to segment" )
  {
    defaultSnappingStringIdx = 1;
  }
  else
  {
    // to vertex and segment
    defaultSnappingStringIdx = 2;
  }

  bool layerIdListOk, enabledListOk, toleranceListOk, toleranceUnitListOk, snapToListOk, avoidIntersectionListOk;
  QStringList layerIdList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingList", QStringList(), &layerIdListOk );
  QStringList enabledList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingEnabledList", QStringList(), &enabledListOk );
  QStringList toleranceList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceList", QStringList(), & toleranceListOk );
  QStringList toleranceUnitList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceUnitList", QStringList(), &toleranceUnitListOk );
  QStringList snapToList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnapToList", QStringList(), &snapToListOk );
  QStringList avoidIntersectionsList = QgsProject::instance()->readListEntry( "Digitizing", "/AvoidIntersectionsList", QStringList(), &avoidIntersectionListOk );

  //snap to layer yes/no
  QTreeWidgetItem *item = new QTreeWidgetItem( mLayerTreeWidget );

  QCheckBox *cbxEnable = new QCheckBox( mLayerTreeWidget );
  mLayerTreeWidget->setItemWidget( item, 0, cbxEnable );
  item->setData( 0, Qt::UserRole, currentVectorLayer->id() );
  item->setText( 1, currentVectorLayer->name() );

  //snap to vertex/ snap to segment
  QComboBox *cbxSnapTo = new QComboBox( mLayerTreeWidget );
  cbxSnapTo->insertItem( 0, tr( "to vertex" ) );
  cbxSnapTo->insertItem( 1, tr( "to segment" ) );
  cbxSnapTo->insertItem( 2, tr( "to vertex and segment" ) );
  cbxSnapTo->setCurrentIndex( defaultSnappingStringIdx );
  mLayerTreeWidget->setItemWidget( item, 2, cbxSnapTo );

  //snapping tolerance
  QDoubleSpinBox* sbTolerance = new QDoubleSpinBox( mLayerTreeWidget );
  sbTolerance->setRange( 0., 100000000. );
  sbTolerance->setDecimals( 5 );
  sbTolerance->setValue( defaultSnappingTolerance );

  mLayerTreeWidget->setItemWidget( item, 3, sbTolerance );

  //snap to vertex/ snap to segment
  QComboBox *cbxUnits = new QComboBox( mLayerTreeWidget );
  cbxUnits->insertItem( 0, tr( "layer units" ) );
  cbxUnits->insertItem( 1, tr( "pixels" ) );
  cbxUnits->insertItem( 2, tr( "map units" ) );
  cbxUnits->setCurrentIndex( defaultSnappingUnit );
  mLayerTreeWidget->setItemWidget( item, 4, cbxUnits );

  QCheckBox *cbxAvoidIntersection = nullptr;
  if ( currentVectorLayer->geometryType() == QGis::Polygon )
  {
    cbxAvoidIntersection = new QCheckBox( mLayerTreeWidget );
    mLayerTreeWidget->setItemWidget( item, 5, cbxAvoidIntersection );
  }

  //resize treewidget columns
  for ( int i = 0 ; i < 4 ; ++i )
  {
    mLayerTreeWidget->resizeColumnToContents( i );
  }

  int idx = layerIdList.indexOf( currentVectorLayer->id() );
  if ( idx < 0 )
  {
    if ( myDockFlag )
    {
      connect( cbxEnable, SIGNAL( stateChanged( int ) ), this, SLOT( apply() ) );
      connect( cbxSnapTo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( apply() ) );
      connect( sbTolerance, SIGNAL( valueChanged( double ) ), this, SLOT( apply() ) );
      connect( cbxUnits, SIGNAL( currentIndexChanged( int ) ), this, SLOT( apply() ) );

      if ( cbxAvoidIntersection )
      {
        connect( cbxAvoidIntersection, SIGNAL( stateChanged( int ) ), this, SLOT( apply() ) );
      }
      setTopologicalEditingState();
      setIntersectionSnappingState();
    }

    cbxEnable->setChecked( defaultSnappingString != "off" );

    // no settings for this layer yet
    return;
  }

  cbxEnable->setChecked( enabledList[ idx ] == "enabled" );

  int snappingStringIdx = 0;
  if ( snapToList[idx] == "to_vertex" )
  {
    snappingStringIdx = 0;
  }
  else if ( snapToList[idx] == "to_segment" )
  {
    snappingStringIdx = 1;
  }
  else //to vertex and segment
  {
    snappingStringIdx = 2;
  }

  cbxSnapTo->setCurrentIndex( snappingStringIdx );
  sbTolerance->setValue( toleranceList[idx].toDouble() );
  cbxUnits->setCurrentIndex( toleranceUnitList[idx].toInt() );
  if ( cbxAvoidIntersection )
  {
    cbxAvoidIntersection->setChecked( avoidIntersectionsList.contains( currentVectorLayer->id() ) );
  }

  if ( myDockFlag )
  {
    connect( cbxEnable, SIGNAL( stateChanged( int ) ), this, SLOT( apply() ) );
    connect( cbxSnapTo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( apply() ) );
    connect( sbTolerance, SIGNAL( valueChanged( double ) ), this, SLOT( apply() ) );
    connect( cbxUnits, SIGNAL( currentIndexChanged( int ) ), this, SLOT( apply() ) );

    if ( cbxAvoidIntersection )
    {
      connect( cbxAvoidIntersection, SIGNAL( stateChanged( int ) ), this, SLOT( apply() ) );
    }

    setTopologicalEditingState();
    setIntersectionSnappingState();
  }
}

void QgsSnappingDialog::layersWillBeRemoved( const QStringList& thelayers )
{
  Q_FOREACH ( const QString& theLayerId, thelayers )
  {
    QTreeWidgetItem *item = nullptr;

    for ( int i = 0; i < mLayerTreeWidget->topLevelItemCount(); ++i )
    {
      item = mLayerTreeWidget->topLevelItem( i );
      if ( item && item->data( 0, Qt::UserRole ).toString() == theLayerId )
        break;
      item = nullptr;
    }

    if ( item )
      delete item;
  }
  apply();
}

void QgsSnappingDialog::setTopologicalEditingState()
{
  // read the digitizing settings
  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
  cbxEnableTopologicalEditingCheckBox->blockSignals( true );
  cbxEnableTopologicalEditingCheckBox->setChecked( topologicalEditing );
  cbxEnableTopologicalEditingCheckBox->blockSignals( false );
}

void QgsSnappingDialog::setIntersectionSnappingState()
{
  // read the digitizing settings
  int intersectionSnapping = QgsProject::instance()->readNumEntry( "Digitizing", "/IntersectionSnapping", 0 );
  cbxEnableIntersectionSnappingCheckBox->blockSignals( true );
  cbxEnableIntersectionSnappingCheckBox->setChecked( intersectionSnapping );
  cbxEnableIntersectionSnappingCheckBox->blockSignals( false );
}

void QgsSnappingDialog::setSnappingMode()
{
  mSnapModeComboBox->blockSignals( true );
  QString snapMode = QgsProject::instance()->readEntry( "Digitizing", "/SnappingMode" );
  if ( snapMode == "current_layer" )
    mSnapModeComboBox->setCurrentIndex( 0 );
  else if ( snapMode == "all_layers" )
    mSnapModeComboBox->setCurrentIndex( 1 );
  else // "advanced" or empty (backward compatibility)
    mSnapModeComboBox->setCurrentIndex( 2 );
  onSnappingModeIndexChanged( mSnapModeComboBox->currentIndex() );
  mSnapModeComboBox->blockSignals( false );
}


//
// QgsSnappingDock
//

QgsSnappingDock::QgsSnappingDock( const QString& title, QWidget* parent, Qt::WindowFlags flags )
    : QgsDockWidget( title, parent, flags )
{
  setObjectName( "Snapping and Digitizing Options" ); // set object name so the position can be saved
}

void QgsSnappingDock::closeEvent( QCloseEvent* e )
{
  Q_UNUSED( e );
  // deleteLater();
}
