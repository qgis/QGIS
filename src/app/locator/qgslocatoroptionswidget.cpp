/***************************************************************************
                         qgslocatoroptionswidget.cpp
                         ---------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QApplication>
#include <QToolButton>
#include <QHBoxLayout>

#include "qgslocatoroptionswidget.h"

#include "qgsapplication.h"
#include "qgslocatorwidget.h"
#include "qgssettings.h"


QgsLocatorOptionsWidget::QgsLocatorOptionsWidget( QgsLocatorWidget *locator, QWidget *parent )
  : QTreeView( parent )
  , mLocatorWidget( locator )
  , mLocator( locator->locator() )
{

  mModel = new QgsLocatorFiltersModel( mLocator, this );
  setModel( mModel );

  header()->setStretchLastSection( false );
  header()->setSectionResizeMode( QgsLocatorFiltersModel::Name, QHeaderView::Stretch );

  setEditTriggers( QAbstractItemView::AllEditTriggers );
  setAlternatingRowColors( true );
  setSelectionMode( QAbstractItemView::NoSelection );

  // add the config button
  for ( int row = 0; row < mModel->rowCount(); ++row )
  {
    const QModelIndex index = mModel->index( row, QgsLocatorFiltersModel::Config );
    QWidget *bt = mModel->configButton( index, this );
    if ( bt )
    {
      setIndexWidget( index, bt );
    }
  }
}

void QgsLocatorOptionsWidget::commitChanges()
{
  mModel->commitChanges();
  mLocatorWidget->invalidateResults();
}

void QgsLocatorOptionsWidget::dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles )
{
  for ( int row = topLeft.row(); row < bottomRight.row(); ++row )
  {
    const QModelIndex index = mModel->index( row, QgsLocatorFiltersModel::Config );
    if ( !indexWidget( index ) )
    {
      QWidget *bt = mModel->configButton( index, this );
      if ( bt )
      {
        setIndexWidget( index, bt );
      }
    }
  }
  QTreeView::dataChanged( topLeft, bottomRight, roles );
}

//
// QgsLocatorFiltersModel
//

#define HIDDEN_FILTER_OFFSET 1

QgsLocatorFiltersModel::QgsLocatorFiltersModel( QgsLocator *locator, QObject *parent )
  : QAbstractTableModel( parent )
  , mLocator( locator )
{
  mIconSize = std::floor( std::max( Qgis::UI_SCALE_FACTOR * QgsApplication::fontMetrics().height() * 1.1, 24.0 ) );
  mRowSize = std::floor( std::max( Qgis::UI_SCALE_FACTOR * QgsApplication::fontMetrics().height() * 1.1 * 4 / 3, 32.0 ) );
}

QWidget *QgsLocatorFiltersModel::configButton( const QModelIndex &index, QWidget *parent ) const
{
  if ( !index.isValid() )
    return nullptr;

  QgsLocatorFilter *filter = filterForIndex( index );
  if ( filter && filter->hasConfigWidget() )
  {
    // use a layout to get the button center aligned
    QWidget *w = new QWidget( parent );
    QToolButton *bt = new QToolButton( );
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->addWidget( bt );
    w->setLayout( layout );

    connect( bt, &QToolButton::clicked, this, [ = ]() {filter->openConfigWidget( bt );} );
    bt->setMaximumSize( mIconSize, mIconSize );
    bt->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    bt->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/settings.svg" ) ) );
    return w;
  }
  else
  {
    return nullptr;
  }
}

int QgsLocatorFiltersModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mLocator->filters().count() - HIDDEN_FILTER_OFFSET;
}

int QgsLocatorFiltersModel::columnCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return 5;
}

QVariant QgsLocatorFiltersModel::data( const QModelIndex &index, int role ) const
{
  if ( index.parent().isValid() )
    return QVariant();
  if ( !index.isValid() || index.row() < 0 || index.column() < 0 ||
       index.row() >= rowCount( QModelIndex() ) || index.column() >= columnCount( QModelIndex() ) )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case Name:
          return filterForIndex( index )->displayName();

        case Prefix:
          return mPrefixes.value( filterForIndex( index ), filterForIndex( index )->activePrefix() );

        case Active:
        case Default:
        case Config:
          return QVariant();
      }
      break;
    }

    case Qt::CheckStateRole:
      switch ( index.column() )
      {
        case Name:
        case Prefix:
        case Config:
          return QVariant();

        case Active:
        {
          QgsLocatorFilter *filter = filterForIndex( index );
          if ( mEnabledChanges.contains( filter ) )
            return mEnabledChanges.value( filter ) ? Qt::Checked : Qt::Unchecked;
          else
            return filterForIndex( index )->enabled() ? Qt::Checked : Qt::Unchecked;
        }

        case Default:
          QgsLocatorFilter *filter = filterForIndex( index );
          if ( mDefaultChanges.contains( filter ) )
            return mDefaultChanges.value( filter ) ? Qt::Checked : Qt::Unchecked;
          else
            return filterForIndex( index )->useWithoutPrefix() ? Qt::Checked : Qt::Unchecked;
      }
      break;

    case Qt::SizeHintRole:
      return QSize( mRowSize, mRowSize );

    case Qt::TextAlignmentRole:
      if ( index.column() == Config )
        return static_cast<Qt::Alignment::Int>( Qt::AlignCenter );
      break;

  }

  return QVariant();
}

bool QgsLocatorFiltersModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || index.parent().isValid() || index.row() < 0 || index.column() < 0 ||
       index.row() >= rowCount( QModelIndex() ) || index.column() >= columnCount( QModelIndex() ) )
    return false;

  switch ( role )
  {
    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case Name:
        case Active:
        case Default:
          return false;

        case Prefix:
        {
          const QString prefix = value.toString();
          if ( !prefix.isEmpty() )
          {
            mPrefixes.insert( filterForIndex( index ), prefix );
          }
          else
          {
            // reset to the native prefix
            mPrefixes.insert( filterForIndex( index ), filterForIndex( index )->prefix() );
          }
          emit dataChanged( index, index );
          return true;
        }
      }
      return false;
    }

    case Qt::CheckStateRole:
    {
      const bool checked = static_cast< Qt::CheckState >( value.toInt() ) == Qt::Checked;
      switch ( index.column() )
      {
        case Name:
        case Prefix:
          return false;

        case Active:
        {
          mEnabledChanges.insert( filterForIndex( index ), checked );
          emit dataChanged( index, index );
          return true;
        }

        case Default:
        {
          mDefaultChanges.insert( filterForIndex( index ), checked );
          emit dataChanged( index, index );
          return true;
        }
      }
    }
  }
  return false;
}

Qt::ItemFlags QgsLocatorFiltersModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() || index.parent().isValid() || index.row() < 0 || index.column() < 0 ||
       index.row() >= rowCount( QModelIndex() ) || index.column() >= columnCount( QModelIndex() ) )
    return QAbstractTableModel::flags( index );

  Qt::ItemFlags flags = QAbstractTableModel::flags( index );
  switch ( index.column() )
  {
    case Name:
    case Config:
      break;

    case Prefix:
      flags = flags | Qt::ItemIsEditable;
      break;

    case Active:
    case Default:
      flags = flags | Qt::ItemIsUserCheckable;
      break;
  }

  return flags;
}

QVariant QgsLocatorFiltersModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    switch ( section )
    {
      case Name:
        return tr( "Filter" );

      case Prefix:
        return tr( "Prefix" );

      case Active:
        return tr( "Enabled" );

      case Default:
        return tr( "Default" );

      case Config:
        return tr( "Configuration" );
    }
  }

  return QVariant();
}

void QgsLocatorFiltersModel::commitChanges()
{
  QHash< QgsLocatorFilter *, QString >::const_iterator itp = mPrefixes.constBegin();
  for ( ; itp != mPrefixes.constEnd(); ++itp )
  {
    QgsLocatorFilter *filter = itp.key();
    const QString activePrefix = itp.value();
    if ( !activePrefix.isEmpty() && activePrefix != filter->prefix() )
    {
      filter->setActivePrefix( activePrefix );
      QgsLocator::settingsLocatorFilterPrefix.setValue( activePrefix, filter->name() );
    }
    else
    {
      filter->setActivePrefix( QString() );
      QgsLocator::settingsLocatorFilterPrefix.remove( filter->name() );
    }
  }
  QHash< QgsLocatorFilter *, bool >::const_iterator it = mEnabledChanges.constBegin();
  for ( ; it != mEnabledChanges.constEnd(); ++it )
  {
    QgsLocatorFilter *filter = it.key();
    QgsLocator::settingsLocatorFilterEnabled.setValue( it.value(), filter->name() );
    filter->setEnabled( it.value() );
  }
  it = mDefaultChanges.constBegin();
  for ( ; it != mDefaultChanges.constEnd(); ++it )
  {
    QgsLocatorFilter *filter = it.key();
    QgsLocator::settingsLocatorFilterDefault.setValue( it.value(), filter->name() );
    filter->setUseWithoutPrefix( it.value() );
  }
}

QgsLocatorFilter *QgsLocatorFiltersModel::filterForIndex( const QModelIndex &index ) const
{
  return mLocator->filters().at( index.row() + HIDDEN_FILTER_OFFSET );
}
