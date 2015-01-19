/***************************************************************************
    qgsprojectionselectionwidget.cpp
     --------------------------------------
    Date                 : 05.01.2015
    Copyright            : (C) 2015 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QHBoxLayout>


#include "qgsprojectionselectionwidget.h"
#include "qgsapplication.h"
#include "qgsgenericprojectionselector.h"
#include "qgsproject.h"
#include <QSettings>

QgsProjectionSelectionWidget::QgsProjectionSelectionWidget( QWidget *parent ) :
    QWidget( parent )
{
  mDialog = new QgsGenericProjectionSelector( this );

  QHBoxLayout* layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  setLayout( layout );

  mCrsComboBox = new QComboBox( this );
  mCrsComboBox->addItem( tr( "invalid projection" ), QgsProjectionSelectionWidget::CurrentCrs );

  if ( QgsProject::instance()->readNumEntry( "SpatialRefSys", "/ProjectionsEnabled", 0 ) )
  {
    //only show project CRS if OTF reprojection is enabled - otherwise the
    //CRS stored in the project can be misleading
    QString projectCrsString = QgsProject::instance()->readEntry( "SpatialRefSys", "/ProjectCrs" );
    mProjectCrs.createFromOgcWmsCrs( projectCrsString );
    addProjectCrsOption();
  }

  QSettings settings;
  QString defCrsString = settings.value( "/Projections/projectDefaultCrs", GEO_EPSG_CRS_AUTHID ).toString();
  mDefaultCrs.createFromOgcWmsCrs( defCrsString );
  if ( mDefaultCrs.authid() != mProjectCrs.authid() )
  {
    //only show default CRS option if it's different to the project CRS, avoids
    //needlessly cluttering the widget
    addDefaultCrsOption();
  }

  layout->addWidget( mCrsComboBox );

  mButton = new QToolButton( this );
  mButton->setIcon( QgsApplication::getThemeIcon( "mActionSetProjection.svg" ) );
  mButton->setToolTip( tr( "Select CRS" ) );
  layout->addWidget( mButton );

  setFocusPolicy( Qt::StrongFocus );
  setFocusProxy( mButton );

  connect( mButton, SIGNAL( clicked() ), this, SLOT( selectCrs() ) );
  connect( mCrsComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( comboIndexChanged( int ) ) );
}

QgsCoordinateReferenceSystem QgsProjectionSelectionWidget::crs() const
{
  switch (( CrsOption )mCrsComboBox->itemData( mCrsComboBox->currentIndex() ).toInt() )
  {
    case QgsProjectionSelectionWidget::LayerCrs:
      return mLayerCrs;
    case QgsProjectionSelectionWidget::ProjectCrs:
      return mProjectCrs ;
    case QgsProjectionSelectionWidget::DefaultCrs:
      return mDefaultCrs ;
    case QgsProjectionSelectionWidget::CurrentCrs:
      return mCrs;
  }
  return mCrs;
}

void QgsProjectionSelectionWidget::setOptionVisible( const QgsProjectionSelectionWidget::CrsOption option, const bool visible )
{
  int optionIndex = mCrsComboBox->findData( option );

  if ( visible && optionIndex < 0 )
  {
    //add missing CRS option
    switch ( option )
    {
      case QgsProjectionSelectionWidget::LayerCrs:
      {
        setLayerCrs( mLayerCrs );
        return;
      }
      case QgsProjectionSelectionWidget::ProjectCrs:
      {
        addProjectCrsOption();
        return;
      }
      case QgsProjectionSelectionWidget::DefaultCrs:
      {
        addDefaultCrsOption();
        return;
      }
      case QgsProjectionSelectionWidget::CurrentCrs:
        //current CRS option cannot be readded
        return;
    }
  }
  else if ( !visible && optionIndex >= 0 )
  {
    //remove CRS option
    mCrsComboBox->removeItem( optionIndex );
  }
}

void QgsProjectionSelectionWidget::selectCrs()
{
  //find out crs id of current proj4 string
  if ( mCrs.isValid() )
  {
    mDialog->setSelectedCrsId( mCrs.srsid() );
  }

  if ( mDialog->exec() )
  {
    mCrsComboBox->blockSignals( true );
    mCrsComboBox->setCurrentIndex( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ) );
    mCrsComboBox->blockSignals( false );
    QgsCoordinateReferenceSystem crs;
    crs.createFromOgcWmsCrs( mDialog->selectedAuthId() );
    setCrs( crs );
    emit crsChanged( crs );
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }
}

void QgsProjectionSelectionWidget::comboIndexChanged( int idx )
{
  switch (( CrsOption )mCrsComboBox->itemData( idx ).toInt() )
  {
    case QgsProjectionSelectionWidget::LayerCrs:
      emit crsChanged( mLayerCrs );
      break;
    case QgsProjectionSelectionWidget::ProjectCrs:
      emit crsChanged( mProjectCrs );
      break;
    case QgsProjectionSelectionWidget::CurrentCrs:
      emit crsChanged( mCrs );
      break;
    case QgsProjectionSelectionWidget::DefaultCrs:
      emit crsChanged( mDefaultCrs );
      break;
  }
}

void QgsProjectionSelectionWidget::setCrs( const QgsCoordinateReferenceSystem& crs )
{
  if ( crs.isValid() )
  {
    mCrsComboBox->setItemText( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ),
                               tr( "Selected CRS (%1, %2)" ).arg( crs.authid() ).arg( crs.description() ) );
    mCrsComboBox->blockSignals( true );
    mCrsComboBox->setCurrentIndex( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ) );
    mCrsComboBox->blockSignals( false );
  }
  else
  {
    mCrsComboBox->setItemText( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ),
                               tr( "invalid projection" ) );
  }
  mCrs = crs;
}

void QgsProjectionSelectionWidget::setLayerCrs( const QgsCoordinateReferenceSystem &crs )
{
  int layerItemIndex = mCrsComboBox->findData( QgsProjectionSelectionWidget::LayerCrs );
  if ( crs.isValid() )
  {
    if ( layerItemIndex > -1 )
    {
      mCrsComboBox->setItemText( layerItemIndex, tr( "Layer CRS (%1, %2)" ).arg( crs.authid() ).arg( crs.description() ) );
    }
    else
    {
      mCrsComboBox->addItem( tr( "Layer CRS (%1, %2)" ).arg( crs.authid() ).arg( crs.description() ), QgsProjectionSelectionWidget::LayerCrs );
    }
  }
  else
  {
    if ( layerItemIndex > -1 )
    {
      mCrsComboBox->removeItem( layerItemIndex );
    }
  }
  mLayerCrs = crs;
}

void QgsProjectionSelectionWidget::addProjectCrsOption()
{
  if ( mProjectCrs.isValid() )
  {
    mCrsComboBox->addItem( tr( "Project CRS (%1 - %2)" ).arg( mProjectCrs.authid() ).arg( mProjectCrs.description() ), QgsProjectionSelectionWidget::ProjectCrs );
  }
}

void QgsProjectionSelectionWidget::addDefaultCrsOption()
{
  mCrsComboBox->addItem( tr( "Default CRS (%1 - %2)" ).arg( mDefaultCrs.authid() ).arg( mDefaultCrs.description() ), QgsProjectionSelectionWidget::DefaultCrs );
}
