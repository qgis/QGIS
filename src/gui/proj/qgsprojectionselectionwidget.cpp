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
#include "qgsrecentcoordinatereferencesystemsmodel.h"
#include "qgsdatums.h"

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif


///@cond PRIVATE
StandardCoordinateReferenceSystemsModel::StandardCoordinateReferenceSystemsModel( QObject *parent )
  : QAbstractItemModel( parent )
  , mProjectCrs( QgsProject::instance()->crs() )
{
#ifdef ENABLE_MODELTEST
  new ModelTest( this, this );
#endif

  const QgsSettings settings;
  mDefaultCrs = QgsCoordinateReferenceSystem( settings.value( QStringLiteral( "/projections/defaultProjectCrs" ), geoEpsgCrsAuthId(), QgsSettings::App ).toString() );

  connect( QgsApplication::coordinateReferenceSystemRegistry(), &QgsCoordinateReferenceSystemRegistry::userCrsChanged, this, [ = ]
  {
    mCurrentCrs.updateDefinition();
    mLayerCrs.updateDefinition();
    mProjectCrs.updateDefinition();
    mDefaultCrs.updateDefinition();
  } );
}

Qt::ItemFlags StandardCoordinateReferenceSystemsModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    return Qt::ItemFlags();
  }

  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant StandardCoordinateReferenceSystemsModel::data( const QModelIndex &index, int role ) const
{
  const QgsProjectionSelectionWidget::CrsOption option = optionForIndex( index );
  if ( option == QgsProjectionSelectionWidget::CrsOption::Invalid )
    return QVariant();

  const QgsCoordinateReferenceSystem crs = StandardCoordinateReferenceSystemsModel::crs( index );
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
      switch ( option )
      {
        case QgsProjectionSelectionWidget::ProjectCrs:
          return tr( "Project CRS: %1" ).arg( crs.userFriendlyIdentifier() );
        case QgsProjectionSelectionWidget::DefaultCrs:
          return tr( "Default CRS: %1" ).arg( crs.userFriendlyIdentifier() );
        case QgsProjectionSelectionWidget::LayerCrs:
          return tr( "Layer CRS: %1" ).arg( crs.userFriendlyIdentifier() );
        case QgsProjectionSelectionWidget::CrsNotSet:
          return mNotSetText;
        case QgsProjectionSelectionWidget::CurrentCrs:
          return QgsProjectionSelectionWidget::crsOptionText( crs );
        case QgsProjectionSelectionWidget::Invalid:
        case QgsProjectionSelectionWidget::RecentCrs:
          break;
      }
      break;

    case RoleCrs:
      return crs;

    case RoleOption:
      return static_cast< int >( option );

    default:
      break;
  }

  return QVariant();
}

int StandardCoordinateReferenceSystemsModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return 5;
}

int StandardCoordinateReferenceSystemsModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

QModelIndex StandardCoordinateReferenceSystemsModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( row < 0 || row >= rowCount() || column != 0 || parent.isValid() )
    return QModelIndex();

  return createIndex( row, column );
}

QModelIndex StandardCoordinateReferenceSystemsModel::parent( const QModelIndex & ) const
{
  return QModelIndex();
}

QgsCoordinateReferenceSystem StandardCoordinateReferenceSystemsModel::crs( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QgsCoordinateReferenceSystem();

  const QgsProjectionSelectionWidget::CrsOption option = optionForIndex( index );
  switch ( option )
  {
    case QgsProjectionSelectionWidget::ProjectCrs:
      return mProjectCrs;
    case QgsProjectionSelectionWidget::DefaultCrs:
      return mDefaultCrs;
    case QgsProjectionSelectionWidget::CurrentCrs:
      return mCurrentCrs;
    case QgsProjectionSelectionWidget::LayerCrs:
      return mLayerCrs;
    case QgsProjectionSelectionWidget::Invalid:
    case QgsProjectionSelectionWidget::CrsNotSet:
    case QgsProjectionSelectionWidget::RecentCrs:
      return QgsCoordinateReferenceSystem();
  }
  BUILTIN_UNREACHABLE
}

QgsProjectionSelectionWidget::CrsOption StandardCoordinateReferenceSystemsModel::optionForIndex( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QgsProjectionSelectionWidget::Invalid;

  const int row = index.row();
  switch ( row )
  {
    case 0:
      return QgsProjectionSelectionWidget::CrsNotSet;
    case 1:
      return QgsProjectionSelectionWidget::CurrentCrs;
    case 2:
      return QgsProjectionSelectionWidget::ProjectCrs;
    case 3:
      return QgsProjectionSelectionWidget::DefaultCrs;
    case 4:
      return QgsProjectionSelectionWidget::LayerCrs;
    default:
      break;
  }

  return QgsProjectionSelectionWidget::Invalid;
}

QModelIndex StandardCoordinateReferenceSystemsModel::indexForOption( QgsProjectionSelectionWidget::CrsOption option ) const
{
  int row = 0;
  switch ( option )
  {
    case QgsProjectionSelectionWidget::Invalid:
    case QgsProjectionSelectionWidget::RecentCrs:
      return QModelIndex();
    case QgsProjectionSelectionWidget::CrsNotSet:
      row = 0;
      break;
    case QgsProjectionSelectionWidget::CurrentCrs:
      row = 1;
      break;
    case QgsProjectionSelectionWidget::ProjectCrs:
      row = 2;
      break;
    case QgsProjectionSelectionWidget::DefaultCrs:
      row = 3;
      break;
    case QgsProjectionSelectionWidget::LayerCrs:
      row = 4;
      break;
  }

  return index( row, 0, QModelIndex() );
}

void StandardCoordinateReferenceSystemsModel::setLayerCrs( const QgsCoordinateReferenceSystem &crs )
{
  mLayerCrs = crs;
  const QModelIndex index = indexForOption( QgsProjectionSelectionWidget::LayerCrs );
  emit dataChanged( index, index );
}

void StandardCoordinateReferenceSystemsModel::setCurrentCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCurrentCrs = crs;
  const QModelIndex index = indexForOption( QgsProjectionSelectionWidget::LayerCrs );
  emit dataChanged( index, index );
}

void StandardCoordinateReferenceSystemsModel::setNotSetText( const QString &text )
{
  mNotSetText = text;
  const QModelIndex index = indexForOption( QgsProjectionSelectionWidget::CrsNotSet );
  emit dataChanged( index, index );
}

//
// CombinedCoordinateReferenceSystemsModel
//

CombinedCoordinateReferenceSystemsModel::CombinedCoordinateReferenceSystemsModel( QObject *parent )
  : QConcatenateTablesProxyModel( parent )
  , mStandardModel( new StandardCoordinateReferenceSystemsModel( this ) )
  , mRecentModel( new QgsRecentCoordinateReferenceSystemsProxyModel( this ) )
{
  addSourceModel( mStandardModel );
  addSourceModel( mRecentModel );
}

void CombinedCoordinateReferenceSystemsModel::setNotSetText( const QString &text )
{
  mStandardModel->setNotSetText( text );
}

QString CombinedCoordinateReferenceSystemsModel::notSetText() const
{
  return mStandardModel->notSetText();
}

QgsCoordinateReferenceSystem CombinedCoordinateReferenceSystemsModel::currentCrs() const
{
  return mStandardModel->currentCrs();
}

//
// CombinedCoordinateReferenceSystemsProxyModel
//
CombinedCoordinateReferenceSystemsProxyModel::CombinedCoordinateReferenceSystemsProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
  , mModel( new CombinedCoordinateReferenceSystemsModel( this ) )
{
  mVisibleOptions.setFlag( QgsProjectionSelectionWidget::CurrentCrs, true );
  mVisibleOptions.setFlag( QgsProjectionSelectionWidget::ProjectCrs, true );
  mVisibleOptions.setFlag( QgsProjectionSelectionWidget::DefaultCrs, true );

  setSourceModel( mModel );
  setDynamicSortFilter( true );
}

bool CombinedCoordinateReferenceSystemsProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  const QModelIndex sourceIndex = mModel->index( sourceRow, 0, sourceParent );

  const QgsCoordinateReferenceSystem crs = mModel->data( sourceIndex, StandardCoordinateReferenceSystemsModel::RoleCrs ).value< QgsCoordinateReferenceSystem >();
  if ( !mFilteredCrs.isEmpty() && !mFilteredCrs.contains( crs ) )
    return false;

  switch ( crs.type() )
  {
    case Qgis::CrsType::Unknown:
    case Qgis::CrsType::Other:
      break;

    case Qgis::CrsType::Geodetic:
    case Qgis::CrsType::Geocentric:
    case Qgis::CrsType::Geographic2d:
    case Qgis::CrsType::Geographic3d:
    case Qgis::CrsType::Projected:
    case Qgis::CrsType::Temporal:
    case Qgis::CrsType::Engineering:
    case Qgis::CrsType::Bound:
    case Qgis::CrsType::DerivedProjected:
      if ( !mFilters.testFlag( QgsCoordinateReferenceSystemProxyModel::Filter::FilterHorizontal ) )
        return false;
      break;

    case Qgis::CrsType::Vertical:
      if ( !mFilters.testFlag( QgsCoordinateReferenceSystemProxyModel::Filter::FilterVertical ) )
        return false;
      break;

    case Qgis::CrsType::Compound:
      if ( !mFilters.testFlag( QgsCoordinateReferenceSystemProxyModel::Filter::FilterCompound ) )
        return false;
      break;
  }

  const QVariant optionInt = mModel->data( sourceIndex, StandardCoordinateReferenceSystemsModel::RoleOption );
  if ( optionInt.isValid() )
  {
    if ( optionInt.toInt() > 0 )
    {
      const QgsProjectionSelectionWidget::CrsOption option = static_cast< QgsProjectionSelectionWidget::CrsOption >( optionInt.toInt() );
      if ( !mVisibleOptions.testFlag( option ) )
        return false;

      // specific logic for showing/hiding options:
      switch ( option )
      {
        case QgsProjectionSelectionWidget::Invalid:
          break;

        case QgsProjectionSelectionWidget::LayerCrs:
        case QgsProjectionSelectionWidget::ProjectCrs:
          // only show these options if the crs is valid
          return crs.isValid();

        case QgsProjectionSelectionWidget::CurrentCrs:
          // hide invalid current CRS value option only if "not set" option is shown
          return crs.isValid() || !mVisibleOptions.testFlag( QgsProjectionSelectionWidget::CrsNotSet );

        case QgsProjectionSelectionWidget::DefaultCrs:
        case QgsProjectionSelectionWidget::RecentCrs:
        case QgsProjectionSelectionWidget::CrsNotSet:
          // always shown
          break;
      }
      return true;
    }
  }
  else
  {
    // a recent crs
    // these are only shown if they aren't duplicates of a standard item already shown in the list
    for ( QgsProjectionSelectionWidget::CrsOption standardOption :
          {
            QgsProjectionSelectionWidget::CrsOption::CurrentCrs,
            QgsProjectionSelectionWidget::CrsOption::DefaultCrs,
            QgsProjectionSelectionWidget::CrsOption::LayerCrs,
            QgsProjectionSelectionWidget::CrsOption::ProjectCrs
          } )
    {
      const QModelIndexList standardItemIndex = mModel->match( mModel->index( 0, 0 ), StandardCoordinateReferenceSystemsModel::RoleOption, static_cast< int >( standardOption ) );
      if ( standardItemIndex.empty() )
        continue;

      const QgsCoordinateReferenceSystem standardItemCrs = mModel->data( standardItemIndex.at( 0 ), StandardCoordinateReferenceSystemsModel::RoleCrs ).value< QgsCoordinateReferenceSystem >();
      if ( standardItemCrs == crs && filterAcceptsRow( standardItemIndex.at( 0 ).row(), QModelIndex() ) )
        return false;
    }
  }

  return true;
}

void CombinedCoordinateReferenceSystemsProxyModel::setLayerCrs( const QgsCoordinateReferenceSystem &crs )
{
  mModel->standardModel()->setLayerCrs( crs );
  invalidateFilter();
}

void CombinedCoordinateReferenceSystemsProxyModel::setCurrentCrs( const QgsCoordinateReferenceSystem &crs )
{
  mModel->standardModel()->setCurrentCrs( crs );
  invalidateFilter();
}

void CombinedCoordinateReferenceSystemsProxyModel::setFilters( QgsCoordinateReferenceSystemProxyModel::Filters filters )
{
  mFilters = filters;
  invalidateFilter();
}

QgsCoordinateReferenceSystemProxyModel::Filters CombinedCoordinateReferenceSystemsProxyModel::filters() const
{
  return mFilters;
}

void CombinedCoordinateReferenceSystemsProxyModel::setFilteredCrs( const QList<QgsCoordinateReferenceSystem> &crses )
{
  mFilteredCrs = crses;
  invalidateFilter();
}

void CombinedCoordinateReferenceSystemsProxyModel::setOption( QgsProjectionSelectionWidget::CrsOption option, bool enabled )
{
  mVisibleOptions.setFlag( option, enabled );
  invalidateFilter();
}

///@endcond PRIVATE


QgsProjectionSelectionWidget::QgsProjectionSelectionWidget( QWidget *parent,
    QgsCoordinateReferenceSystemProxyModel::Filters filters )
  : QWidget( parent )
  , mDialogTitle( tr( "Coordinate Reference System Selector" ) )
{
  mCrsComboBox = new QgsHighlightableComboBox( this );
  mCrsComboBox->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Preferred );

  mModel = new CombinedCoordinateReferenceSystemsProxyModel( this );
  mModel->setFilters( filters );
  mCrsComboBox->setModel( mModel );

  const int labelMargin = static_cast< int >( std::round( mCrsComboBox->fontMetrics().horizontalAdvance( 'X' ) ) );
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  setLayout( layout );

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
}

QgsCoordinateReferenceSystem QgsProjectionSelectionWidget::crs() const
{
  return mModel->data( mModel->index( mCrsComboBox->currentIndex(), 0 ), StandardCoordinateReferenceSystemsModel::RoleCrs ).value< QgsCoordinateReferenceSystem >();
}

void QgsProjectionSelectionWidget::setOptionVisible( const QgsProjectionSelectionWidget::CrsOption option, const bool visible )
{
  switch ( option )
  {
    case QgsProjectionSelectionWidget::LayerCrs:
    case QgsProjectionSelectionWidget::ProjectCrs:
    case QgsProjectionSelectionWidget::DefaultCrs:
    case QgsProjectionSelectionWidget::CurrentCrs:
    {
      mModel->setOption( option, visible );
      updateTooltip();
      return;
    }
    case QgsProjectionSelectionWidget::RecentCrs:
      //recently used CRS option cannot be readded
      return;
    case QgsProjectionSelectionWidget::CrsNotSet:
    {
      mModel->setOption( CrsNotSet, visible );

      if ( !visible )
      {
        setOptionVisible( CurrentCrs, true );
      }
      else
      {
        if ( !mModel->combinedModel()->currentCrs().isValid() )
          whileBlocking( mCrsComboBox )->setCurrentIndex( 0 );
      }
      updateTooltip();

      return;
    }
    case Invalid:
      return;
  }
}

void QgsProjectionSelectionWidget::setNotSetText( const QString &text )
{
  mModel->combinedModel()->setNotSetText( text );
}

void QgsProjectionSelectionWidget::setMessage( const QString &text )
{
  mMessage = text;
}

bool QgsProjectionSelectionWidget::optionVisible( QgsProjectionSelectionWidget::CrsOption option ) const
{
  const QModelIndexList matches = mModel->match( mModel->index( 0, 0 ), StandardCoordinateReferenceSystemsModel::Role::RoleOption, static_cast< int >( option ) );
  return !matches.empty();
}

void QgsProjectionSelectionWidget::selectCrs()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  const QList< QgsCoordinateReferenceSystem > filteredCrses = mModel->filteredCrs();

  QSet< QString > ogcFilter;
  ogcFilter.reserve( filteredCrses.size( ) );
  for ( const QgsCoordinateReferenceSystem &crs : std::as_const( filteredCrses ) )
  {
    ogcFilter << crs.authid();
  }

  if ( panel && panel->dockMode() )
  {
    mActivePanel = new QgsCrsSelectionWidget( this, mModel->filters() );
    if ( !ogcFilter.isEmpty() )
      mActivePanel->setOgcWmsCrsFilter( ogcFilter );
    if ( !mMessage.isEmpty() )
      mActivePanel->setMessage( mMessage );
    mActivePanel->setCrs( crs() );

    if ( !mModel->combinedModel()->notSetText().isEmpty() )
      mActivePanel->setNotSetText( mModel->combinedModel()->notSetText() );

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
      mCrsComboBox->setCurrentIndex( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs, StandardCoordinateReferenceSystemsModel::Role::RoleOption ) );
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
    QgsProjectionSelectionDialog dlg( this, QgsGuiUtils::ModalDialogFlags, mModel->filters() );
    if ( !mMessage.isEmpty() )
      dlg.setMessage( mMessage );
    if ( !ogcFilter.isEmpty() )
      dlg.setOgcWmsCrsFilter( ogcFilter );
    dlg.setCrs( crs() );
    dlg.setWindowTitle( mDialogTitle );

    if ( !mModel->combinedModel()->notSetText().isEmpty() )
      dlg.setNotSetText( mModel->combinedModel()->notSetText() );

    if ( optionVisible( QgsProjectionSelectionWidget::CrsOption::CrsNotSet ) )
    {
      dlg.setShowNoProjection( true );
    }
    dlg.setRequireValidSelection();

    if ( dlg.exec() )
    {
      mCrsComboBox->blockSignals( true );
      mCrsComboBox->setCurrentIndex( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs, StandardCoordinateReferenceSystemsModel::Role::RoleOption ) );
      mCrsComboBox->blockSignals( false );
      const QgsCoordinateReferenceSystem crs = dlg.crs();
      // setCrs will emit crsChanged for us
      setCrs( crs );
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
  mModel->setFilteredCrs( crses );
}

QgsCoordinateReferenceSystemProxyModel::Filters QgsProjectionSelectionWidget::filters() const
{
  return mModel->filters();
}

void QgsProjectionSelectionWidget::setFilters( QgsCoordinateReferenceSystemProxyModel::Filters filters )
{
  mModel->setFilters( filters );
  if ( mActivePanel )
    mActivePanel->setFilters( filters );
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

void QgsProjectionSelectionWidget::comboIndexChanged( int idx )
{
  const QgsCoordinateReferenceSystem crs = mModel->data( mModel->index( idx, 0 ), StandardCoordinateReferenceSystemsModel::RoleCrs ).value< QgsCoordinateReferenceSystem >();
  const QVariant optionData = mModel->data( mModel->index( idx, 0 ), StandardCoordinateReferenceSystemsModel::RoleOption );
  if ( !optionData.isValid() || static_cast< CrsOption >( optionData.toInt() ) != QgsProjectionSelectionWidget::CrsNotSet )
  {
    // RoleOption is only available for items from the standard coordinate reference system model, but we
    // are using a combined model which also has items from QgsRecentCoordinateReferenceSystemsModel
    emit crsChanged( crs );
  }
  else
  {
    emit cleared();
    emit crsChanged( QgsCoordinateReferenceSystem() );
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
  const QgsCoordinateReferenceSystem prevCrs = mModel->combinedModel()->currentCrs();
  mModel->setCurrentCrs( crs );

  if ( crs.isValid() )
  {
    setOptionVisible( QgsProjectionSelectionWidget::CurrentCrs, true );
    mCrsComboBox->blockSignals( true );
    mCrsComboBox->setCurrentIndex( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs, StandardCoordinateReferenceSystemsModel::Role::RoleOption ) );
    mCrsComboBox->blockSignals( false );
  }
  else
  {
    const int crsNotSetIndex = mCrsComboBox->findData( QgsProjectionSelectionWidget::CrsNotSet, StandardCoordinateReferenceSystemsModel::Role::RoleOption );
    if ( crsNotSetIndex >= 0 )
    {
      mCrsComboBox->blockSignals( true );
      mCrsComboBox->setCurrentIndex( crsNotSetIndex );
      mCrsComboBox->blockSignals( false );
    }
  }
  if ( mActivePanel && !mIgnorePanelSignals )
  {
    mIgnorePanelSignals++;
    mActivePanel->setCrs( crs );
    mIgnorePanelSignals--;
  }
  if ( prevCrs != crs )
  {
    emit crsChanged( crs );
  }
  updateTooltip();
}

void QgsProjectionSelectionWidget::setLayerCrs( const QgsCoordinateReferenceSystem &crs )
{
  mModel->setLayerCrs( crs );
}

QString QgsProjectionSelectionWidget::crsOptionText( const QgsCoordinateReferenceSystem &crs )
{
  if ( crs.isValid() )
    return crs.userFriendlyIdentifier();
  else
    return tr( "invalid projection" );
}

void QgsProjectionSelectionWidget::updateTooltip()
{
  const QgsCoordinateReferenceSystem c = crs();
  if ( c.isValid() )
    setToolTip( c.toWkt( Qgis::CrsWktVariant::Preferred, true ) );
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

