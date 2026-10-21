/***************************************************************************
    qgsdatetimewithtimezonewidget.cpp
     --------------------------------------
    Date                 : October 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatetimewithtimezonewidget.h"

#include <mutex>

#include "qgsapplication.h"
#include "qgsdatetimeedit.h"
#include "qgsfilterlineedit.h"
#include "qgsgui.h"
#include "qgspanelwidget.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QListView>
#include <QString>
#include <QTimeZone>
#include <QToolButton>

#include "moc_qgsdatetimewithtimezonewidget.cpp"

using namespace Qt::StringLiterals;

//
// QgsTimeZoneItemModel
//

std::vector< QgsTimeZoneItemModel::TimeZoneItem > QgsTimeZoneItemModel::sItems;

QgsTimeZoneItemModel::QgsTimeZoneItemModel( QObject *parent )
  : QAbstractItemModel( parent )
{
  static std::once_flag initialized;
  std::call_once( initialized, [] {
    const QDateTime now = QDateTime::currentDateTime();
    const QList<QByteArray> zones = QTimeZone::availableTimeZoneIds();
    sItems.reserve( zones.size() );
    for ( const QByteArray &zoneId : std::as_const( zones ) )
    {
      const QTimeZone timeZone( zoneId );
      if ( !timeZone.isValid() )
        continue;

      const int offset = timeZone.offsetFromUtc( now );
      const QString displayString = displayStringForTimeZone( timeZone );

      TimeZoneItem item { zoneId, displayString, timeZone, offset };
      sItems.emplace_back( item );
    }
  } );
}

int QgsTimeZoneItemModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return static_cast< int>( sItems.size() );
}

int QgsTimeZoneItemModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

QVariant QgsTimeZoneItemModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() < 0 || index.row() >= static_cast< int >( sItems.size() ) )
    return QVariant();

  const TimeZoneItem &item = sItems[index.row()];
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
      return item.displayString;

    case static_cast< int >( CustomRole::TimeZoneId ):
      return item.id;

    case static_cast< int >( CustomRole::TimeZone ):
      return QVariant::fromValue( item.timeZone );

    case static_cast< int >( CustomRole::UtcOffset ):
      return item.utcOffset;

    default:
      break;
  }

  return QVariant();
}

QModelIndex QgsTimeZoneItemModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount() )
  {
    return QModelIndex();
  }

  if ( !parent.isValid() )
  {
    return createIndex( row, column, nullptr );
  }

  return QModelIndex();
}

QModelIndex QgsTimeZoneItemModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index )
  return QModelIndex();
}

QModelIndex QgsTimeZoneItemModel::indexFromTimeZone( const QTimeZone &timeZone ) const
{
  if ( !timeZone.isValid() )
    return QModelIndex();
  const QString id = timeZone.id();
  const QModelIndex startIndex = index( 0, 0 );
  const QModelIndexList matches = match( startIndex, static_cast< int >( CustomRole::TimeZoneId ), id, 1 );
  return matches.isEmpty() ? QModelIndex() : matches.at( 0 );
}

QString QgsTimeZoneItemModel::displayStringForTimeZone( const QTimeZone &timeZone, QTimeZone::NameType nameType )
{
  const QDateTime now = QDateTime::currentDateTime();
  const int offset = timeZone.offsetFromUtc( now );

  return u"%1: %2 (%3)"_s.arg( offsetToString( offset ), QString( timeZone.id() ), timeZone.displayName( now, nameType ) );
}

QString QgsTimeZoneItemModel::offsetToString( int seconds )
{
  const int offsetInMinutes = std::abs( seconds ) / 60;
  const int hours = offsetInMinutes / 60;
  const int minutes = offsetInMinutes % 60;
  return tr( "UTC%1%2:%3" ).arg( seconds >= 0 ? '+' : '-' ).arg( hours, 2, 10, '0' ).arg( minutes, 2, 10, '0' );
}

//
// QgsTimeZoneSortFilterModel
//

QgsTimeZoneSortFilterModel::QgsTimeZoneSortFilterModel( QgsTimeZoneItemModel *source, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mSourceModel( source )
{
  setSourceModel( mSourceModel );
  setDynamicSortFilter( true );
  sort( 0 );
}

QModelIndex QgsTimeZoneSortFilterModel::indexFromTimeZone( const QTimeZone &timeZone ) const
{
  return mapFromSource( mSourceModel->indexFromTimeZone( timeZone ) );
}

void QgsTimeZoneSortFilterModel::setFilterString( const QString &filter )
{
  mFilterString = filter;
  invalidateFilter();
}

bool QgsTimeZoneSortFilterModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  const QString itemText = sourceModel()->data( sourceModel()->index( sourceRow, 0, sourceParent ) ).toString();

  if ( !mFilterString.trimmed().isEmpty() )
  {
    if ( !itemText.contains( mFilterString, Qt::CaseInsensitive ) )
      return false;
  }

  return true;
}

bool QgsTimeZoneSortFilterModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  const int leftOffset = sourceModel()->data( left, static_cast< int >( QgsTimeZoneItemModel::CustomRole::UtcOffset ) ).toInt();
  const int rightOffset = sourceModel()->data( right, static_cast< int >( QgsTimeZoneItemModel::CustomRole::UtcOffset ) ).toInt();
  if ( leftOffset != rightOffset )
  {
    return leftOffset < rightOffset;
  }

  const QString leftText = sourceModel()->data( left, Qt::DisplayRole ).toString();
  const QString rightText = sourceModel()->data( right, Qt::DisplayRole ).toString();
  if ( leftText.isEmpty() )
    return true;
  if ( rightText.isEmpty() )
    return false;

  return QString::localeAwareCompare( leftText, rightText ) < 0;
}

//
// QgsTimeZoneWidget
//

QgsTimeZoneWidget::QgsTimeZoneWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  mModel = new QgsTimeZoneItemModel( this );
  mProxyModel = new QgsTimeZoneSortFilterModel( mModel, this );

  mFilterEdit = new QgsFilterLineEdit();
  mFilterEdit->setShowClearButton( true );
  mFilterEdit->setShowSearchIcon( true );
  mFilterEdit->setPlaceholderText( "Search…" );

  mListView = new QListView();
  mListView->setModel( mProxyModel );
  mListView->setUniformItemSizes( true );

  auto layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mFilterEdit );
  layout->addWidget( mListView, 1 );

  setLayout( layout );

  connect( mFilterEdit, &QgsFilterLineEdit::textEdited, mProxyModel, &QgsTimeZoneSortFilterModel::setFilterString );
  connect( mListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] { emit timeZoneChanged( timeZone() ); } );

  connect( mListView, &QListView::doubleClicked, this, &QgsTimeZoneWidget::itemDoubleClicked );
}

QTimeZone QgsTimeZoneWidget::timeZone() const
{
  const QModelIndexList list = mListView->selectionModel()->selectedIndexes().toList();
  if ( list.isEmpty() )
    return QTimeZone();

  const QModelIndex currentIndex = list.at( 0 );
  if ( !currentIndex.isValid() )
    return QTimeZone();

  return mProxyModel->data( currentIndex, static_cast< int>( QgsTimeZoneItemModel::CustomRole::TimeZone ) ).value< QTimeZone >();
}

void QgsTimeZoneWidget::setTimeZone( const QTimeZone &zone )
{
  const QModelIndex index = mProxyModel->indexFromTimeZone( zone );
  if ( !index.isValid() )
  {
    mListView->clearSelection();
  }
  else
  {
    mListView->selectionModel()->select( index, QItemSelectionModel::SelectionFlag::ClearAndSelect );
    mListView->scrollTo( index );
  }
}

void QgsTimeZoneWidget::itemDoubleClicked( const QModelIndex &index )
{
  emit timeZoneDoubleClicked( mProxyModel->data( index, static_cast< int>( QgsTimeZoneItemModel::CustomRole::TimeZone ) ).value< QTimeZone >() );
}

///@cond PRIVATE

//
// StandardTimeZoneModel
//

StandardTimeZoneModel::StandardTimeZoneModel( QObject *parent )
  : QAbstractItemModel( parent )
{}

QVariant StandardTimeZoneModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() < 0 || index.row() >= rowCount() )
    return QVariant();

  const QgsTimeZoneComboBox::StandardItem item = itemForIndex( index );
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
      switch ( item )
      {
        case QgsTimeZoneComboBox::StandardItem::Invalid:
          return tr( "Not set" );

        case QgsTimeZoneComboBox::StandardItem::Current:
          return QgsTimeZoneItemModel::displayStringForTimeZone( mCurrentTimeZone, QTimeZone::NameType::ShortName );

        case QgsTimeZoneComboBox::StandardItem::System:
          return tr( "System: %1" ).arg( QgsTimeZoneItemModel::displayStringForTimeZone( QTimeZone::systemTimeZone(), QTimeZone::NameType::ShortName ) );
      }
      break;

    case static_cast< int >( StandardTimeZoneModel::CustomRole::TimeZone ):
      switch ( item )
      {
        case QgsTimeZoneComboBox::StandardItem::Invalid:
          return QVariant::fromValue( QTimeZone() );

        case QgsTimeZoneComboBox::StandardItem::Current:
          return QVariant::fromValue( mCurrentTimeZone );

        case QgsTimeZoneComboBox::StandardItem::System:
          return QVariant::fromValue( QTimeZone::systemTimeZone() );
      }
      break;

    default:
      break;
  }

  return QVariant();
}

int StandardTimeZoneModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return 3;
}

int StandardTimeZoneModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

QModelIndex StandardTimeZoneModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( row < 0 || row >= rowCount() || column != 0 || parent.isValid() )
    return QModelIndex();

  return createIndex( row, column );
}

QModelIndex StandardTimeZoneModel::parent( const QModelIndex & ) const
{
  return QModelIndex();
}

QgsTimeZoneComboBox::StandardItem StandardTimeZoneModel::itemForIndex( const QModelIndex &index ) const
{
  switch ( index.row() )
  {
    case 0:
      return QgsTimeZoneComboBox::StandardItem::Invalid;

    case 1:
      return QgsTimeZoneComboBox::StandardItem::Current;

    case 2:
      return QgsTimeZoneComboBox::StandardItem::System;

    default:
      return QgsTimeZoneComboBox::StandardItem::Invalid;
  }
}

QModelIndex StandardTimeZoneModel::indexForItem( QgsTimeZoneComboBox::StandardItem item ) const
{
  switch ( item )
  {
    case QgsTimeZoneComboBox::StandardItem::Invalid:
      return index( 0, 0, QModelIndex() );
    case QgsTimeZoneComboBox::StandardItem::Current:
      return index( 1, 0, QModelIndex() );
    case QgsTimeZoneComboBox::StandardItem::System:
      return index( 2, 0, QModelIndex() );
  }
  BUILTIN_UNREACHABLE
}

void StandardTimeZoneModel::setCurrentTimeZone( const QTimeZone &timeZone )
{
  mCurrentTimeZone = timeZone;
  emit dataChanged( index( 1, 0, QModelIndex() ), index( 1, 0, QModelIndex() ) );
}

//
// StandardTimeZoneProxyModel
//
StandardTimeZoneProxyModel::StandardTimeZoneProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
  mModel = new StandardTimeZoneModel( this );
  mItems = QgsTimeZoneComboBox::StandardItem::Invalid | QgsTimeZoneComboBox::StandardItem::System;

  setSourceModel( mModel );
  setDynamicSortFilter( true );
}

void StandardTimeZoneProxyModel::setDisplayedItems( QgsTimeZoneComboBox::StandardItems items )
{
  mItems = items;
  invalidateFilter();
}

void StandardTimeZoneProxyModel::setCurrentTimeZone( const QTimeZone &timeZone )
{
  mModel->setCurrentTimeZone( timeZone );
  mItems.setFlag( QgsTimeZoneComboBox::StandardItem::Current, timeZone.isValid() );
  invalidateFilter();
}

QModelIndex StandardTimeZoneProxyModel::indexForItem( QgsTimeZoneComboBox::StandardItem item ) const
{
  return mapFromSource( mModel->indexForItem( item ) );
}

bool StandardTimeZoneProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  const QgsTimeZoneComboBox::StandardItem item = mModel->itemForIndex( mModel->index( sourceRow, 0, sourceParent ) );
  return mItems.testFlag( item );
}


///@endcond

//
// QgsTimeZoneComboBox
//

QgsTimeZoneComboBox::QgsTimeZoneComboBox( QWidget *parent )
  : QWidget( parent )
  , mDialogTitle( tr( "Time Zone Selector" ) )
{
  mComboBox = new QComboBox();
  mTimeZoneModel = new StandardTimeZoneProxyModel( this );
  mComboBox->setModel( mTimeZoneModel );

  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );

  layout->addWidget( mComboBox, 1 );

  mButton = new QToolButton( this );
  mButton->setIcon( QgsApplication::getThemeIcon( u"mActionSetTimeZone.svg"_s ) );
  mButton->setToolTip( tr( "Select Time Zone" ) );
  layout->addWidget( mButton );

  setFocusPolicy( Qt::StrongFocus );
  setFocusProxy( mButton );

  setLayout( layout );

  connect( mComboBox, qOverload< int >( &QComboBox::currentIndexChanged ), this, [this] {
    if ( !mBlockSignals )
    {
      emit timeZoneChanged( timeZone() );
    }
  } );

  connect( mButton, &QToolButton::clicked, this, &QgsTimeZoneComboBox::selectTimeZone );
}

QTimeZone QgsTimeZoneComboBox::timeZone() const
{
  return mTimeZoneModel->data( mTimeZoneModel->index( mComboBox->currentIndex(), 0 ), static_cast< int >( StandardTimeZoneModel::CustomRole::TimeZone ) ).value< QTimeZone >();
}

void QgsTimeZoneComboBox::setDialogTitle( const QString &title )
{
  mDialogTitle = title;
}

QString QgsTimeZoneComboBox::dialogTitle() const
{
  return mDialogTitle;
}

void QgsTimeZoneComboBox::setTimeZone( const QTimeZone &zone )
{
  const QTimeZone oldTimeZone = timeZone();
  mBlockSignals++;
  mTimeZoneModel->setCurrentTimeZone( zone );
  if ( zone.isValid() )
  {
    mComboBox->setCurrentIndex( mTimeZoneModel->indexForItem( QgsTimeZoneComboBox::StandardItem::Current ).row() );
  }
  else
  {
    mComboBox->setCurrentIndex( mTimeZoneModel->indexForItem( QgsTimeZoneComboBox::StandardItem::Invalid ).row() );
  }
  mBlockSignals--;
  if ( timeZone() != oldTimeZone )
  {
    emit timeZoneChanged( timeZone() );
  }
}

void QgsTimeZoneComboBox::selectTimeZone()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );

  if ( panel && panel->dockMode() )
  {
    mActivePanel = new QgsTimeZoneWidget( this );
    mActivePanel->setTimeZone( timeZone() );
    mActivePanel->setPanelTitle( mDialogTitle );

    connect( mActivePanel, &QgsTimeZoneWidget::timeZoneChanged, this, [this] {
      if ( mIgnorePanelSignals )
        return;

      const QTimeZone timeZone = mActivePanel->timeZone();

      mIgnorePanelSignals++;
      setTimeZone( timeZone );
      mIgnorePanelSignals--;

      emit timeZoneChanged( timeZone );
    } );
    panel->openPanel( mActivePanel );
  }
  else
  {
    QgsTimeZoneDialog dlg( this, QgsGuiUtils::ModalDialogFlags );
    dlg.setWindowTitle( mDialogTitle );
    dlg.setTimeZone( timeZone() );
    if ( dlg.exec() )
    {
      setTimeZone( dlg.timeZone() );
    }
  }
}

//
// QgsTimeZoneDialog
//

QgsTimeZoneDialog::QgsTimeZoneDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  QVBoxLayout *vlayout = new QVBoxLayout();

  mWidget = new QgsTimeZoneWidget();
  vlayout->addWidget( mWidget, 1 );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsTimeZoneDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsTimeZoneDialog::reject );

  vlayout->addWidget( mButtonBox );

  setLayout( vlayout );

  QgsGui::enableAutoGeometryRestore( this );

  //apply selected time zone upon double-click on item
  connect( mWidget, &QgsTimeZoneWidget::timeZoneDoubleClicked, this, &QgsTimeZoneDialog::accept );
}

QTimeZone QgsTimeZoneDialog::timeZone() const
{
  return mWidget->timeZone();
}

void QgsTimeZoneDialog::setTimeZone( const QTimeZone &zone )
{
  mWidget->setTimeZone( zone );
}

//
// QgsDateTimeWithTimeZoneWidget
//

QgsDateTimeWithTimeZoneWidget::QgsDateTimeWithTimeZoneWidget( QWidget *parent )
  : QWidget( parent )
{
  auto layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  mDateTimeEdit = new QgsDateTimeEdit();
  layout->addWidget( mDateTimeEdit, 1 );
  mTimeZoneCombo = new QgsTimeZoneComboBox();
  layout->addWidget( mTimeZoneCombo, 2 );
  setLayout( layout );

  connect( mTimeZoneCombo, &QgsTimeZoneComboBox::timeZoneChanged, this, [this] {
    if ( mBlockSignals )
      return;

    emit dateTimeChanged( dateTime() );
  } );

  connect( mDateTimeEdit, &QgsDateTimeEdit::dateTimeChanged, this, [this] {
    if ( mBlockSignals )
      return;

    emit dateTimeChanged( dateTime() );
  } );
}

QgsDateTimeEdit *QgsDateTimeWithTimeZoneWidget::dateTimeEdit()
{
  return mDateTimeEdit;
}

QgsTimeZoneComboBox *QgsDateTimeWithTimeZoneWidget::timeZoneComboBox()
{
  return mTimeZoneCombo;
}

void QgsDateTimeWithTimeZoneWidget::setDialogTitle( const QString &title )
{
  mTimeZoneCombo->setDialogTitle( title );
}

QString QgsDateTimeWithTimeZoneWidget::dialogTitle() const
{
  return mTimeZoneCombo->dialogTitle();
}

QDateTime QgsDateTimeWithTimeZoneWidget::dateTime() const
{
  QDateTime res = mDateTimeEdit->dateTime();
  res.setTimeZone( mTimeZoneCombo->timeZone() );
  return res;
}

void QgsDateTimeWithTimeZoneWidget::setDateTime( const QDateTime &dateTime )
{
  const QDateTime current = QgsDateTimeWithTimeZoneWidget::dateTime();
  mBlockSignals++;
  mDateTimeEdit->setDateTime( dateTime );
  mTimeZoneCombo->setTimeZone( dateTime.timeZone() );
  mBlockSignals--;
  if ( current != dateTime )
  {
    emit dateTimeChanged( dateTime );
  }
}
