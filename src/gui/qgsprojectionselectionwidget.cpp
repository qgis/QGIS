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
#include "qgshighlightablecombobox.h"
#include "qgscoordinatereferencesystemregistry.h"
#include "qgsdatums.h"

QgsProjectionSelectionWidget::QgsProjectionSelectionWidget( QWidget *parent )
  : QWidget( parent )
  , mDialogTitle( tr( "Select CRS" ) )
{

  mCrsComboBox = new QgsHighlightableComboBox( this );
  mCrsComboBox->addItem( tr( "invalid projection" ), QgsProjectionSelectionWidget::CurrentCrs );
  mCrsComboBox->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Preferred );

  const int labelMargin = static_cast< int >( std::round( mCrsComboBox->fontMetrics().horizontalAdvance( 'X' ) ) );
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  setLayout( layout );

  mProjectCrs = QgsProject::instance()->crs();
  addProjectCrsOption();

  const QgsSettings settings;
  mDefaultCrs = QgsCoordinateReferenceSystem( settings.value( QStringLiteral( "/projections/defaultProjectCrs" ), geoEpsgCrsAuthId(), QgsSettings::App ).toString() );
  if ( mDefaultCrs.authid() != mProjectCrs.authid() )
  {
    //only show default CRS option if it's different to the project CRS, avoids
    //needlessly cluttering the widget
    addDefaultCrsOption();
  }

  addRecentCrs();

  layout->addWidget( mCrsComboBox, 1 );

  // bit of fiddlyness here -- we want the initial spacing to only be visible
  // when the warning label is shown, so it's embedded inside mWarningLabel
  // instead of outside it
  mWarningLabelContainer = new QWidget();
  QHBoxLayout *warningLayout = new QHBoxLayout();
  warningLayout->setContentsMargins( 0, 0, 0, 0 );
  mWarningLabel = new QLabel();
  const QIcon icon = QgsApplication::getThemeIcon( QStringLiteral( "mIconWarning.svg" ) );
  const int size = static_cast< int >( std::max( 24.0, mCrsComboBox->minimumSize().height() * 0.5 ) );
  mWarningLabel->setPixmap( icon.pixmap( icon.actualSize( QSize( size, size ) ) ) );
  warningLayout->insertSpacing( 0, labelMargin / 2 );
  warningLayout->insertWidget( 1, mWarningLabel );
  mWarningLabelContainer->setLayout( warningLayout );
  layout->addWidget( mWarningLabelContainer );
  mWarningLabelContainer->hide();

  layout->addSpacing( labelMargin / 2 );

  mButton = new QToolButton( this );
  mButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionSetProjection.svg" ) ) );
  mButton->setToolTip( tr( "Select CRS" ) );
  layout->addWidget( mButton );

  setFocusPolicy( Qt::StrongFocus );
  setFocusProxy( mButton );
  setAcceptDrops( true );

  connect( mButton, &QToolButton::clicked, this, &QgsProjectionSelectionWidget::selectCrs );
  connect( mCrsComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsProjectionSelectionWidget::comboIndexChanged );

  connect( QgsApplication::coordinateReferenceSystemRegistry(), &QgsCoordinateReferenceSystemRegistry::userCrsChanged, this, [ = ]
  {
    mCrs.updateDefinition();
    mLayerCrs.updateDefinition();
    mProjectCrs.updateDefinition();
    mDefaultCrs.updateDefinition();
  } );
}

QgsCoordinateReferenceSystem QgsProjectionSelectionWidget::crsAtIndex( int index ) const
{
  switch ( static_cast< CrsOption >( mCrsComboBox->itemData( index ).toInt() ) )
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
      const long srsid = mCrsComboBox->itemData( index, Qt::UserRole + 1 ).toLongLong();
      const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromSrsId( srsid );
      return crs;
    }
    case QgsProjectionSelectionWidget::CrsNotSet:
      return QgsCoordinateReferenceSystem();
  }
  return mCrs;
}

QgsCoordinateReferenceSystem QgsProjectionSelectionWidget::crs() const
{
  return crsAtIndex( mCrsComboBox->currentIndex() );
}

void QgsProjectionSelectionWidget::setOptionVisible( const QgsProjectionSelectionWidget::CrsOption option, const bool visible )
{
  const int optionIndex = mCrsComboBox->findData( option );

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
  const int optionIndex = mCrsComboBox->findData( CrsNotSet );
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
  const int optionIndex = mCrsComboBox->findData( option );
  return optionIndex >= 0;
}

void QgsProjectionSelectionWidget::selectCrs()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );

  QSet< QString > ogcFilter;
  ogcFilter.reserve( mFilter.size( ) );
  for ( const QgsCoordinateReferenceSystem &crs : std::as_const( mFilter ) )
  {
    ogcFilter << crs.authid();
  }

  if ( panel && panel->dockMode() )
  {
    mActivePanel = new QgsCrsSelectionWidget( this );
    if ( !ogcFilter.isEmpty() )
      mActivePanel->setOgcWmsCrsFilter( ogcFilter );
    if ( !mMessage.isEmpty() )
      mActivePanel->setMessage( mMessage );
    mActivePanel->setCrs( mCrs );

    if ( !mNotSetText.isEmpty() )
      mActivePanel->setNotSetText( mNotSetText );

    mActivePanel->setPanelTitle( mDialogTitle );

    if ( optionVisible( QgsProjectionSelectionWidget::CrsOption::CrsNotSet ) )
    {
      mActivePanel->setShowNoCrs( true );
    }

    connect( mActivePanel, &QgsCrsSelectionWidget::crsChanged, this, [ this ]
    {
      if ( mIgnorePanelSignals )
        return;

      if ( !mActivePanel->hasValidSelection() )
        return;

      mCrsComboBox->blockSignals( true );
      mCrsComboBox->setCurrentIndex( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ) );
      mCrsComboBox->blockSignals( false );
      const QgsCoordinateReferenceSystem crs = mActivePanel->crs();

      mIgnorePanelSignals++;
      setCrs( crs );
      mIgnorePanelSignals--;

      emit crsChanged( crs );
    } );
    panel->openPanel( mActivePanel );
  }
  else
  {
    QgsProjectionSelectionDialog dlg( this );
    if ( !mMessage.isEmpty() )
      dlg.setMessage( mMessage );
    if ( !ogcFilter.isEmpty() )
      dlg.setOgcWmsCrsFilter( ogcFilter );
    dlg.setCrs( mCrs );
    dlg.setWindowTitle( mDialogTitle );

    if ( !mNotSetText.isEmpty() )
      dlg.setNotSetText( mNotSetText );

    if ( optionVisible( QgsProjectionSelectionWidget::CrsOption::CrsNotSet ) )
    {
      dlg.setShowNoProjection( true );
    }
    dlg.setRequireValidSelection();

    if ( dlg.exec() )
    {
      mCrsComboBox->blockSignals( true );
      mCrsComboBox->setCurrentIndex( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ) );
      mCrsComboBox->blockSignals( false );
      const QgsCoordinateReferenceSystem crs = dlg.crs();
      setCrs( crs );
      emit crsChanged( crs );
    }
    else
    {
      QApplication::restoreOverrideCursor();
    }
  }
}

void QgsProjectionSelectionWidget::dragEnterEvent( QDragEnterEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
  {
    event->ignore();
    return;
  }

  if ( mapLayerFromMimeData( event->mimeData() ) )
  {
    // dragged an acceptable layer, phew
    event->setDropAction( Qt::CopyAction );
    event->accept();
    mCrsComboBox->setHighlighted( true );
    update();
  }
  else
  {
    event->ignore();
  }
}

void QgsProjectionSelectionWidget::dragLeaveEvent( QDragLeaveEvent *event )
{
  if ( mCrsComboBox->isHighlighted() )
  {
    event->accept();
    mCrsComboBox->setHighlighted( false );
    update();
  }
  else
  {
    event->ignore();
  }
}

void QgsProjectionSelectionWidget::dropEvent( QDropEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
  {
    event->ignore();
    return;
  }

  if ( QgsMapLayer *layer = mapLayerFromMimeData( event->mimeData() ) )
  {
    // dropped a map layer
    setFocus( Qt::MouseFocusReason );
    event->setDropAction( Qt::CopyAction );
    event->accept();

    if ( layer->crs().isValid() )
      setCrs( layer->crs() );
  }
  else
  {
    event->ignore();
  }
  mCrsComboBox->setHighlighted( false );
  update();
}

QString QgsProjectionSelectionWidget::sourceEnsemble() const
{
  return mSourceEnsemble;
}

void QgsProjectionSelectionWidget::setDialogTitle( const QString &title )
{
  mDialogTitle = title;
}

QString QgsProjectionSelectionWidget::dialogTitle() const
{
  return mDialogTitle;
}

void QgsProjectionSelectionWidget::setFilter( const QList<QgsCoordinateReferenceSystem> &crses )
{
  mFilter = crses;


  for ( int i = mCrsComboBox->count() - 1; i >= 0; --i )
  {
    if ( !mFilter.contains( crsAtIndex( i ) ) )
      mCrsComboBox->removeItem( i );
  }
}

void QgsProjectionSelectionWidget::setSourceEnsemble( const QString &ensemble )
{
  if ( mSourceEnsemble == ensemble )
    return;

  mSourceEnsemble = ensemble;
  updateWarning();
}

bool QgsProjectionSelectionWidget::showAccuracyWarnings() const
{
  return mShowAccuracyWarnings;
}

void QgsProjectionSelectionWidget::setShowAccuracyWarnings( bool show )
{
  mShowAccuracyWarnings = show;
  if ( !mShowAccuracyWarnings )
    mWarningLabelContainer->hide();
  else
    updateWarning();
}

void QgsProjectionSelectionWidget::addNotSetOption()
{
  mCrsComboBox->insertItem( 0, mNotSetText, QgsProjectionSelectionWidget::CrsNotSet );
  if ( !mCrs.isValid() )
    whileBlocking( mCrsComboBox )->setCurrentIndex( 0 );
}

void QgsProjectionSelectionWidget::comboIndexChanged( int idx )
{
  switch ( static_cast< CrsOption >( mCrsComboBox->itemData( idx ).toInt() ) )
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
    case QgsProjectionSelectionWidget::RecentCrs:
    {
      const long srsid = mCrsComboBox->itemData( idx, Qt::UserRole + 1 ).toLongLong();
      const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromSrsId( srsid );
      emit crsChanged( crs );
      break;
    }
    case QgsProjectionSelectionWidget::CrsNotSet:
      emit cleared();
      emit crsChanged( QgsCoordinateReferenceSystem() );
      break;
  }
  updateTooltip();
}

void QgsProjectionSelectionWidget::updateWarning()
{
  if ( !mShowAccuracyWarnings )
  {
    if ( mWarningLabelContainer->isVisible() )
      mWarningLabelContainer->hide();
    return;
  }

  try
  {
    const double crsAccuracyWarningThreshold = QgsSettings().value( QStringLiteral( "/projections/crsAccuracyWarningThreshold" ), 0.0, QgsSettings::App ).toDouble();

    const QgsDatumEnsemble ensemble = crs().datumEnsemble();
    if ( !ensemble.isValid() || ensemble.name() == mSourceEnsemble || ( ensemble.accuracy() > 0 && ensemble.accuracy() < crsAccuracyWarningThreshold ) )
    {
      mWarningLabelContainer->hide();
    }
    else
    {
      mWarningLabelContainer->show();

      QString warning = QStringLiteral( "<p>" );

      QString id;
      if ( !ensemble.code().isEmpty() )
        id = QStringLiteral( "<i>%1</i> (%2:%3)" ).arg( ensemble.name(), ensemble.authority(), ensemble.code() );
      else
        id = QStringLiteral( "<i>%</i>”" ).arg( ensemble.name() );

      if ( ensemble.accuracy() > 0 )
      {
        warning = tr( "The selected CRS is based on %1, which has a limited accuracy of <b>at best %2 meters</b>." ).arg( id ).arg( ensemble.accuracy() );
      }
      else
      {
        warning = tr( "The selected CRS is based on %1, which has a limited accuracy." ).arg( id );
      }
      warning += QStringLiteral( "</p><p>" ) + tr( "Use an alternative CRS if accurate positioning is required." ) + QStringLiteral( "</p>" );

      const QList< QgsDatumEnsembleMember > members = ensemble.members();
      if ( !members.isEmpty() )
      {
        warning += QStringLiteral( "<p>" ) + tr( "%1 consists of the datums:" ).arg( ensemble.name() ) + QStringLiteral( "</p><ul>" );

        for ( const QgsDatumEnsembleMember &member : members )
        {
          if ( !member.code().isEmpty() )
            id = QStringLiteral( "%1 (%2:%3)" ).arg( member.name(), member.authority(), member.code() );
          else
            id = member.name();
          warning += QStringLiteral( "<li>%1</li>" ).arg( id );
        }

        warning += QLatin1String( "</ul>" );
      }

      mWarningLabel->setToolTip( warning );
    }
  }
  catch ( QgsNotSupportedException & )
  {
    mWarningLabelContainer->hide();
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
    const int crsNotSetIndex = mCrsComboBox->findData( QgsProjectionSelectionWidget::CrsNotSet );
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
  if ( mActivePanel && !mIgnorePanelSignals )
  {
    mIgnorePanelSignals++;
    mActivePanel->setCrs( crs );
    mIgnorePanelSignals--;
  }
  if ( mCrs != crs )
  {
    mCrs = crs;
    emit crsChanged( crs );
  }
  updateTooltip();
}

void QgsProjectionSelectionWidget::setLayerCrs( const QgsCoordinateReferenceSystem &crs )
{
  const int layerItemIndex = mCrsComboBox->findData( QgsProjectionSelectionWidget::LayerCrs );
  if ( crs.isValid() )
  {
    if ( layerItemIndex > -1 )
    {
      mCrsComboBox->setItemText( layerItemIndex, tr( "Layer CRS: %1" ).arg( crs.userFriendlyIdentifier() ) );
    }
    else
    {
      mCrsComboBox->insertItem( firstRecentCrsIndex(), tr( "Layer CRS: %1" ).arg( crs.userFriendlyIdentifier() ), QgsProjectionSelectionWidget::LayerCrs );
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
    mCrsComboBox->addItem( tr( "Project CRS: %1" ).arg( mProjectCrs.userFriendlyIdentifier() ), QgsProjectionSelectionWidget::ProjectCrs );
  }
}

void QgsProjectionSelectionWidget::addDefaultCrsOption()
{
  mCrsComboBox->addItem( tr( "Default CRS: %1" ).arg( mDefaultCrs.userFriendlyIdentifier() ), QgsProjectionSelectionWidget::DefaultCrs );
}

void QgsProjectionSelectionWidget::addCurrentCrsOption()
{
  const int index = optionVisible( CrsNotSet ) ? 1 : 0;
  mCrsComboBox->insertItem( index, crsOptionText( mCrs ), QgsProjectionSelectionWidget::CurrentCrs );

}

QString QgsProjectionSelectionWidget::crsOptionText( const QgsCoordinateReferenceSystem &crs )
{
  if ( crs.isValid() )
    return crs.userFriendlyIdentifier();
  else
    return tr( "invalid projection" );
}

void QgsProjectionSelectionWidget::addRecentCrs()
{
  const QList< QgsCoordinateReferenceSystem> recentProjections = QgsCoordinateReferenceSystem::recentCoordinateReferenceSystems();
  for ( const QgsCoordinateReferenceSystem &crs : recentProjections )
  {
    const long srsid = crs.srsid();

    //check if already shown
    if ( crsIsShown( srsid ) )
    {
      continue;
    }

    if ( crs.isValid() && ( mFilter.isEmpty() || mFilter.contains( crs ) ) )
    {
      mCrsComboBox->addItem( crs.userFriendlyIdentifier(), QgsProjectionSelectionWidget::RecentCrs );
      mCrsComboBox->setItemData( mCrsComboBox->count() - 1, QVariant( ( long long )srsid ), Qt::UserRole + 1 );
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
    if ( static_cast< CrsOption >( mCrsComboBox->itemData( i ).toInt() ) == RecentCrs )
    {
      return i;
    }
  }
  return -1;
}

void QgsProjectionSelectionWidget::updateTooltip()
{
  const QgsCoordinateReferenceSystem c = crs();
  if ( c.isValid() )
    setToolTip( c.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, true ) );
  else
    setToolTip( QString() );
  updateWarning();
}

QgsMapLayer *QgsProjectionSelectionWidget::mapLayerFromMimeData( const QMimeData *data ) const
{
  const QgsMimeDataUtils::UriList uriList = QgsMimeDataUtils::decodeUriList( data );
  for ( const QgsMimeDataUtils::Uri &u : uriList )
  {
    // is this uri from the current project?
    if ( QgsMapLayer *layer = u.mapLayer() )
    {
      return layer;
    }
  }
  return nullptr;
}

