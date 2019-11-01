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
#include "qgsprojectionselectiondialog.h"
#include "qgsproject.h"
#include "qgssettings.h"

QgsProjectionSelectionWidget::QgsProjectionSelectionWidget( QWidget *parent )
  : QWidget( parent )
{


  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 6 );
  setLayout( layout );

  mCrsComboBox = new QComboBox( this );
  mCrsComboBox->addItem( tr( "invalid projection" ), QgsProjectionSelectionWidget::CurrentCrs );
  mCrsComboBox->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Preferred );

  mProjectCrs = QgsProject::instance()->crs();
  addProjectCrsOption();

  QgsSettings settings;
  mDefaultCrs = QgsCoordinateReferenceSystem( settings.value( QStringLiteral( "/projections/defaultProjectCrs" ), geoEpsgCrsAuthId(), QgsSettings::App ).toString() );
  if ( mDefaultCrs.authid() != mProjectCrs.authid() )
  {
    //only show default CRS option if it's different to the project CRS, avoids
    //needlessly cluttering the widget
    addDefaultCrsOption();
  }

  addRecentCrs();

  layout->addWidget( mCrsComboBox );

  mButton = new QToolButton( this );
  mButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionSetProjection.svg" ) ) );
  mButton->setToolTip( tr( "Select CRS" ) );
  layout->addWidget( mButton );

  setFocusPolicy( Qt::StrongFocus );
  setFocusProxy( mButton );

  connect( mButton, &QToolButton::clicked, this, &QgsProjectionSelectionWidget::selectCrs );
  connect( mCrsComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsProjectionSelectionWidget::comboIndexChanged );
}

QgsCoordinateReferenceSystem QgsProjectionSelectionWidget::crs() const
{
  switch ( ( CrsOption )mCrsComboBox->currentData().toInt() )
  {
    case QgsProjectionSelectionWidget::LayerCrs:
      return mLayerCrs;
    case QgsProjectionSelectionWidget::ProjectCrs:
      return mProjectCrs;
    case QgsProjectionSelectionWidget::DefaultCrs:
      return mDefaultCrs;
    case QgsProjectionSelectionWidget::CurrentCrs:
      return mCrs;
    case QgsProjectionSelectionWidget::RecentCrs:
    {
      long srsid = mCrsComboBox->currentData( Qt::UserRole + 1 ).toLongLong();
      QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromSrsId( srsid );
      return crs;
    }
    case QgsProjectionSelectionWidget::CrsNotSet:
      return QgsCoordinateReferenceSystem();
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
      {
        addCurrentCrsOption();
        return;
      }
      case QgsProjectionSelectionWidget::RecentCrs:
        //recently used CRS option cannot be readded
        return;
      case QgsProjectionSelectionWidget::CrsNotSet:
      {
        addNotSetOption();

        if ( optionVisible( CurrentCrs ) && !mCrs.isValid() )
        {
          // hide invalid option if not set option is shown
          setOptionVisible( CurrentCrs, false );
        }

        return;
      }
    }
  }
  else if ( !visible && optionIndex >= 0 )
  {
    //remove CRS option
    mCrsComboBox->removeItem( optionIndex );

    if ( option == CrsNotSet )
    {
      setOptionVisible( CurrentCrs, true );
    }
  }
}

void QgsProjectionSelectionWidget::setNotSetText( const QString &text )
{
  mNotSetText = text;
  int optionIndex = mCrsComboBox->findData( CrsNotSet );
  if ( optionIndex >= 0 )
  {
    mCrsComboBox->setItemText( optionIndex, mNotSetText );
  }
}

void QgsProjectionSelectionWidget::setMessage( const QString &text )
{
  mMessage = text;
}

bool QgsProjectionSelectionWidget::optionVisible( QgsProjectionSelectionWidget::CrsOption option ) const
{
  int optionIndex = mCrsComboBox->findData( option );
  return optionIndex >= 0;
}

void QgsProjectionSelectionWidget::selectCrs()
{
  //find out crs id of current proj4 string
  QgsProjectionSelectionDialog dlg( this );
  if ( !mMessage.isEmpty() )
    dlg.setMessage( mMessage );
  dlg.setCrs( mCrs );

  if ( optionVisible( QgsProjectionSelectionWidget::CrsOption::CrsNotSet ) )
  {
    dlg.setShowNoProjection( true );
  }

  if ( dlg.exec() )
  {
    mCrsComboBox->blockSignals( true );
    mCrsComboBox->setCurrentIndex( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ) );
    mCrsComboBox->blockSignals( false );
    QgsCoordinateReferenceSystem crs = dlg.crs();
    setCrs( crs );
    emit crsChanged( crs );
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }
}

void QgsProjectionSelectionWidget::addNotSetOption()
{
  mCrsComboBox->insertItem( 0, mNotSetText, QgsProjectionSelectionWidget::CrsNotSet );
  if ( !mCrs.isValid() )
    whileBlocking( mCrsComboBox )->setCurrentIndex( 0 );
}

void QgsProjectionSelectionWidget::comboIndexChanged( int idx )
{
  switch ( ( CrsOption )mCrsComboBox->itemData( idx ).toInt() )
  {
    case QgsProjectionSelectionWidget::LayerCrs:
      emit crsChanged( mLayerCrs );
      return;
    case QgsProjectionSelectionWidget::ProjectCrs:
      emit crsChanged( mProjectCrs );
      return;
    case QgsProjectionSelectionWidget::CurrentCrs:
      emit crsChanged( mCrs );
      return;
    case QgsProjectionSelectionWidget::DefaultCrs:
      emit crsChanged( mDefaultCrs );
      return;
    case QgsProjectionSelectionWidget::RecentCrs:
    {
      long srsid = mCrsComboBox->itemData( idx, Qt::UserRole + 1 ).toLongLong();
      QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromSrsId( srsid );
      emit crsChanged( crs );
      return;
    }
    case QgsProjectionSelectionWidget::CrsNotSet:
      emit cleared();
      emit crsChanged( QgsCoordinateReferenceSystem() );
      return;
  }
}

void QgsProjectionSelectionWidget::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( crs.isValid() )
  {
    if ( !optionVisible( QgsProjectionSelectionWidget::CurrentCrs ) )
      setOptionVisible( QgsProjectionSelectionWidget::CurrentCrs, true );
    mCrsComboBox->setItemText( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ),
                               crsOptionText( crs ) );
    mCrsComboBox->blockSignals( true );
    mCrsComboBox->setCurrentIndex( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ) );
    mCrsComboBox->blockSignals( false );
  }
  else
  {
    int crsNotSetIndex = mCrsComboBox->findData( QgsProjectionSelectionWidget::CrsNotSet );
    if ( crsNotSetIndex >= 0 )
    {
      mCrsComboBox->blockSignals( true );
      mCrsComboBox->setCurrentIndex( crsNotSetIndex );
      mCrsComboBox->blockSignals( false );
    }
    else
    {
      mCrsComboBox->setItemText( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ),
                                 crsOptionText( crs ) );
    }
  }
  if ( mCrs != crs )
  {
    mCrs = crs;
    emit crsChanged( crs );
  }
}

void QgsProjectionSelectionWidget::setLayerCrs( const QgsCoordinateReferenceSystem &crs )
{
  int layerItemIndex = mCrsComboBox->findData( QgsProjectionSelectionWidget::LayerCrs );
  if ( crs.isValid() )
  {
    if ( layerItemIndex > -1 )
    {
      mCrsComboBox->setItemText( layerItemIndex, tr( "Layer CRS: %1 - %2" ).arg( crs.authid(), crs.description() ) );
    }
    else
    {
      mCrsComboBox->insertItem( firstRecentCrsIndex(), tr( "Layer CRS: %1 - %2" ).arg( crs.authid(), crs.description() ), QgsProjectionSelectionWidget::LayerCrs );
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
    mCrsComboBox->addItem( tr( "Project CRS: %1 - %2" ).arg( mProjectCrs.authid(), mProjectCrs.description() ), QgsProjectionSelectionWidget::ProjectCrs );
  }
}

void QgsProjectionSelectionWidget::addDefaultCrsOption()
{
  mCrsComboBox->addItem( tr( "Default CRS: %1 - %2" ).arg( mDefaultCrs.authid(), mDefaultCrs.description() ), QgsProjectionSelectionWidget::DefaultCrs );
}

void QgsProjectionSelectionWidget::addCurrentCrsOption()
{
  int index = optionVisible( CrsNotSet ) ? 1 : 0;
  mCrsComboBox->insertItem( index, crsOptionText( mCrs ), QgsProjectionSelectionWidget::CurrentCrs );

}

QString QgsProjectionSelectionWidget::crsOptionText( const QgsCoordinateReferenceSystem &crs )
{
  if ( crs.isValid() )
    return tr( "%1 - %2" ).arg( crs.authid(), crs.description() );
  else
    return tr( "invalid projection" );
}

void QgsProjectionSelectionWidget::addRecentCrs()
{
  QStringList recentProjections = QgsCoordinateReferenceSystem::recentProjections();
  int i = 0;
  const auto constRecentProjections = recentProjections;
  for ( const QString &projection : constRecentProjections )
  {
    long srsid = projection.toLong();

    //check if already shown
    if ( crsIsShown( srsid ) )
    {
      continue;
    }

    i++;
    QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromSrsId( srsid );
    if ( crs.isValid() )
    {
      mCrsComboBox->addItem( tr( "%1 - %2" ).arg( crs.authid(), crs.description() ), QgsProjectionSelectionWidget::RecentCrs );
      mCrsComboBox->setItemData( mCrsComboBox->count() - 1, QVariant( ( long long )srsid ), Qt::UserRole + 1 );
    }
    if ( i >= 4 )
    {
      //limit to 4 recent projections to avoid clutter
      break;
    }
  }
}

bool QgsProjectionSelectionWidget::crsIsShown( const long srsid ) const
{
  return srsid == mLayerCrs.srsid() || srsid == mDefaultCrs.srsid() || srsid == mProjectCrs.srsid();
}

int QgsProjectionSelectionWidget::firstRecentCrsIndex() const
{
  for ( int i = 0; i < mCrsComboBox->count(); ++i )
  {
    if ( ( CrsOption )mCrsComboBox->itemData( i ).toInt() == RecentCrs )
    {
      return i;
    }
  }
  return -1;
}
