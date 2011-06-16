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

#include <QCheckBox>
#include <QDoubleValidator>
#include <QComboBox>
#include <QLineEdit>
#include <QDockWidget>
#include <QPushButton>


class QgsSnappingDock : public QDockWidget
{
  public:
    QgsSnappingDock( const QString & title, QWidget * parent = 0, Qt::WindowFlags flags = 0 )
        : QDockWidget( title, parent, flags )
    {
      setObjectName( "Snapping and Digitizing Options" ); // set object name so the position can be saved
    }

    virtual void closeEvent( QCloseEvent *e )
    {
      Q_UNUSED( e );
      // deleteLater();
    }

};

QgsSnappingDialog::QgsSnappingDialog( QWidget* parent, QgsMapCanvas* canvas ): QDialog( parent ), mMapCanvas( canvas ), mDock( 0 )
{
  setupUi( this );

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
  }
  else
  {
    connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( apply() ) );
    connect( mButtonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  }
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer * ) ), this, SLOT( addLayer( QgsMapLayer * ) ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( layerWillBeRemoved( QString ) ) );
  connect( cbxEnableTopologicalEditingCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( on_cbxEnableTopologicalEditingCheckBox_stateChanged( int ) ) );

  mLayerTreeWidget->clear();

  QMap< QString, QgsMapLayer *> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap< QString, QgsMapLayer *>::iterator it;
  for ( it = mapLayers.begin(); it != mapLayers.end() ; ++it )
  {
    addLayer( it.value() );
  }

  mLayerTreeWidget->setHeaderLabels( QStringList() << "" );
  mLayerTreeWidget->resizeColumnToContents( 0 );
  mLayerTreeWidget->setColumnWidth( 1, 200 );  //hardcoded for now
  mLayerTreeWidget->setColumnWidth( 2, 200 );  //hardcoded for now
  mLayerTreeWidget->resizeColumnToContents( 3 );
  mLayerTreeWidget->resizeColumnToContents( 4 );
  mLayerTreeWidget->setSortingEnabled( true );

  // read the digitizing settings
  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
  if ( topologicalEditing != 0 )
  {
    cbxEnableTopologicalEditingCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    cbxEnableTopologicalEditingCheckBox->setCheckState( Qt::Unchecked );
  }
}

QgsSnappingDialog::QgsSnappingDialog()
{
}

QgsSnappingDialog::~QgsSnappingDialog()
{
}

void QgsSnappingDialog::on_cbxEnableTopologicalEditingCheckBox_stateChanged( int state )
{
  int topologicalEditingEnabled = ( state == Qt::Checked ) ? 1 : 0;
  QgsProject::instance()->writeEntry( "Digitizing", "/TopologicalEditing", topologicalEditingEnabled );
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

    toleranceList << QString::number( qobject_cast<QLineEdit*>( mLayerTreeWidget->itemWidget( currentItem, 3 ) )->text().toDouble(), 'f' );
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
}

void QgsSnappingDialog::show()
{
  if ( mDock )
    mDock->setVisible( true );
  else
    QDialog::show();
}

void QgsSnappingDialog::addLayer( QgsMapLayer * theMapLayer )
{
  QgsVectorLayer *currentVectorLayer = qobject_cast<QgsVectorLayer *>( theMapLayer );
  if ( !currentVectorLayer || currentVectorLayer->geometryType() == QGis::NoGeometry )
    return;

  QSettings myQsettings;
  bool myDockFlag = myQsettings.value( "/qgis/dockSnapping", false ).toBool();

  double defaultSnappingTolerance = myQsettings.value( "/qgis/digitizing/default_snapping_tolerance", 0 ).toDouble();
  int defaultSnappingUnit = myQsettings.value( "/qgis/digitizing/default_snapping_tolerance_unit", 0 ).toInt();
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
  else //to vertex and segment
  {
    defaultSnappingStringIdx = 2;
  }

  bool layerIdListOk, enabledListOk, toleranceListOk, toleranceUnitListOk, snapToListOk, avoidIntersectionListOk;
  QStringList layerIdList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingList", &layerIdListOk );
  QStringList enabledList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingEnabledList", &enabledListOk );
  QStringList toleranceList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceList", & toleranceListOk );
  QStringList toleranceUnitList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceUnitList", & toleranceUnitListOk );
  QStringList snapToList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnapToList", &snapToListOk );
  QStringList avoidIntersectionsList = QgsProject::instance()->readListEntry( "Digitizing", "/AvoidIntersectionsList", &avoidIntersectionListOk );

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
  QLineEdit *leTolerance = new QLineEdit( mLayerTreeWidget );
  QDoubleValidator *validator = new QDoubleValidator( leTolerance );
  leTolerance->setValidator( validator );
  leTolerance->setText( QString::number( defaultSnappingTolerance, 'f' ) );

  mLayerTreeWidget->setItemWidget( item, 3, leTolerance );

  //snap to vertex/ snap to segment
  QComboBox *cbxUnits = new QComboBox( mLayerTreeWidget );
  cbxUnits->insertItem( 0, tr( "map units" ) );
  cbxUnits->insertItem( 1, tr( "pixels" ) );
  cbxUnits->setCurrentIndex( defaultSnappingUnit );
  mLayerTreeWidget->setItemWidget( item, 4, cbxUnits );

  QCheckBox *cbxAvoidIntersection = 0;
  if ( currentVectorLayer->geometryType() == QGis::Polygon )
  {
    cbxAvoidIntersection = new QCheckBox( mLayerTreeWidget );
    mLayerTreeWidget->setItemWidget( item, 5, cbxAvoidIntersection );
  }

  if ( myDockFlag )
  {
    connect( cbxEnable, SIGNAL( stateChanged( int ) ), this, SLOT( apply() ) );
    connect( cbxSnapTo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( apply() ) );
    connect( leTolerance, SIGNAL( textEdited( const QString ) ), this, SLOT( apply() ) );
    connect( cbxUnits, SIGNAL( currentIndexChanged( int ) ), this, SLOT( apply() ) );

    if ( cbxAvoidIntersection )
    {
      connect( cbxAvoidIntersection, SIGNAL( stateChanged( int ) ), this, SLOT( apply() ) );
    }
  }

  int idx = layerIdList.indexOf( currentVectorLayer->id() );
  if ( idx < 0 )
  {
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
  leTolerance->setText( QString::number( toleranceList[idx].toDouble(), 'f' ) );
  cbxUnits->setCurrentIndex( toleranceUnitList[idx].toInt() );
  if ( cbxAvoidIntersection )
  {
    cbxAvoidIntersection->setChecked( avoidIntersectionsList.contains( currentVectorLayer->id() ) );
  }
}

void QgsSnappingDialog::layerWillBeRemoved( QString theLayerId )
{
  QTreeWidgetItem *item = 0;

  for ( int i = 0; i < mLayerTreeWidget->topLevelItemCount(); ++i )
  {
    item = mLayerTreeWidget->topLevelItem( i );
    if ( item && item->data( 0, Qt::UserRole ).toString() == theLayerId )
      break;
    item = 0;
  }

  if ( item )
    delete item;
}
