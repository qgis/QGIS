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
#include <QCheckBox>
#include <QDoubleValidator>
#include <QComboBox>
#include <QLineEdit>
#include <QDockWidget>


class QgsSnappingDock : public QDockWidget
{
  public:
    QgsSnappingDock( const QString & title, QWidget * parent = 0, Qt::WindowFlags flags = 0 )
        : QDockWidget( title, parent, flags )
    {
      setObjectName( "Snapping Options" ); // set object name so the position can be saved
    }

    virtual void closeEvent( QCloseEvent * ev )
    {
      //deleteLater();
    }

};

QgsSnappingDialog::QgsSnappingDialog( QWidget* parent, QgsMapCanvas* canvas ): QDialog( parent ), mMapCanvas( canvas ), mDock( 0 )
{
  setupUi( this );

  QSettings myQsettings;
  bool myDockFlag = myQsettings.value( "/qgis/dockSnapping", false ).toBool();
  if ( myDockFlag )
  {
    mDock = new QgsSnappingDock( tr( "Snapping Options" ), QgisApp::instance() );
    mDock->setAllowedAreas( Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea );
    mDock->setWidget( this );
    connect( this, SIGNAL( destroyed() ), mDock, SLOT( close() ) );
    QgisApp::instance()->addDockWidget( Qt::BottomDockWidgetArea, mDock );
    mButtonBox->setVisible( false );
  }
  else
  {
    connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( apply() ) );
  }
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer * ) ), this, SLOT( update() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( update() ) );

  update();

  mLayerTreeWidget->setHeaderLabels( QStringList() << "" );
  mLayerTreeWidget->resizeColumnToContents( 0 );
  mLayerTreeWidget->setColumnWidth( 1, 200 );  //hardcoded for now
  mLayerTreeWidget->setColumnWidth( 2, 200 );  //hardcoded for now
  mLayerTreeWidget->resizeColumnToContents( 3 );
  mLayerTreeWidget->resizeColumnToContents( 4 );
  mLayerTreeWidget->setSortingEnabled( true );
}

QgsSnappingDialog::QgsSnappingDialog()
{
}

QgsSnappingDialog::~QgsSnappingDialog()
{
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


void QgsSnappingDialog::update()
{
  if ( !mMapCanvas )
    return;

  QSettings myQsettings;
  bool myDockFlag = myQsettings.value( "/qgis/dockSnapping", false ).toBool();

  double defaultSnapppingTolerance = myQsettings.value( "/qgis/digitizing/default_snapping_tolerance", 0 ).toDouble();
  int defaultSnapppingUnit = myQsettings.value( "/qgis/digitizing/default_snapping_tolerance_unit", 0 ).toInt();
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

  bool layerIdListOk, enabledListOk, toleranceListOk, toleranceUnitListOk, snapToListOk;
  QStringList layerIdList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingList", &layerIdListOk );
  QStringList enabledList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingEnabledList", &enabledListOk );
  QStringList toleranceList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceList", & toleranceListOk );
  QStringList toleranceUnitList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceUnitList", & toleranceUnitListOk );
  QStringList snapToList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnapToList", &snapToListOk );

  mLayerTreeWidget->clear();

  for ( int i = 0; i < mMapCanvas->layerCount(); ++i )
  {
    QgsVectorLayer *currentVectorLayer = qobject_cast<QgsVectorLayer *>( mMapCanvas->layer( i ) );
    if ( !currentVectorLayer )
      continue;

    //snap to layer yes/no
    QTreeWidgetItem *item = new QTreeWidgetItem( mLayerTreeWidget );

    QCheckBox *cbxEnable = new QCheckBox( mLayerTreeWidget );
    mLayerTreeWidget->setItemWidget( item, 0, cbxEnable );
    item->setData( 0, Qt::UserRole, currentVectorLayer->getLayerID() );

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
    leTolerance->setText( QString::number( defaultSnapppingTolerance, 'f' ) );

    mLayerTreeWidget->setItemWidget( item, 3, leTolerance );

    //snap to vertex/ snap to segment
    QComboBox *cbxUnits = new QComboBox( mLayerTreeWidget );
    cbxUnits->insertItem( 0, tr( "map units" ) );
    cbxUnits->insertItem( 1, tr( "pixels" ) );
    cbxUnits->setCurrentIndex( defaultSnapppingUnit );
    mLayerTreeWidget->setItemWidget( item, 4, cbxUnits );

    int idx = layerIdList.indexOf( currentVectorLayer->getLayerID() );
    if ( idx < 0 )
    {
      // no settings for this layer yet
      continue;
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
  }

  if ( myDockFlag )
  {
    for ( int i = 0; i < mLayerTreeWidget->topLevelItemCount(); ++i )
    {
      QTreeWidgetItem *item = mLayerTreeWidget->topLevelItem( i );
      connect( mLayerTreeWidget->itemWidget( item, 0 ), SIGNAL( stateChanged( int ) ), this, SLOT( apply() ) );
      connect( mLayerTreeWidget->itemWidget( item, 2 ), SIGNAL( currentIndexChanged( int ) ), this, SLOT( apply() ) );
      connect( mLayerTreeWidget->itemWidget( item, 3 ), SIGNAL( textEdited( const QString ) ), this, SLOT( apply() ) );
      connect( mLayerTreeWidget->itemWidget( item, 4 ), SIGNAL( currentIndexChanged( int ) ), this, SLOT( apply() ) );
    }
  }
}

void QgsSnappingDialog::apply()
{
  QStringList layerIdList;
  QStringList snapToList;
  QStringList toleranceList;
  QStringList enabledList;
  QStringList toleranceUnitList;

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
  }

  QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnappingList", layerIdList );
  QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnapToList", snapToList );
  QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnappingToleranceList", toleranceList );
  QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnappingToleranceUnitList", toleranceUnitList );
  QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnappingEnabledList", enabledList );
}

void QgsSnappingDialog::show()
{
  if ( mDock )
    mDock->setVisible( true );
  else
    QDialog::show();
}
