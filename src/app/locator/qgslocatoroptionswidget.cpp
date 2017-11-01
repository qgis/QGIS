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

#include "qgslocatoroptionswidget.h"
#include "qgslocatorwidget.h"
#include "qgssettings.h"

QgsLocatorOptionsWidget::QgsLocatorOptionsWidget( QgsLocatorWidget *locator, QWidget *parent )
  : QWidget( parent )
  , mLocatorWidget( locator )
  , mLocator( locator->locator() )
{
  setupUi( this );

  mModel = new QgsLocatorFiltersModel( mLocator, this );
  mFiltersTreeView->setModel( mModel );

  mFiltersTreeView->header()->setStretchLastSection( false );
  mFiltersTreeView->header()->setSectionResizeMode( 0, QHeaderView::Stretch );

  mConfigureFilterButton->setEnabled( false );
  connect( mFiltersTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [ = ]( const QItemSelection & selected, const QItemSelection & )
  {
    if ( selected.count() == 0 || selected.at( 0 ).indexes().count() == 0 )
    {
      mConfigureFilterButton->setEnabled( false );
    }
    else
    {
      QModelIndex sel = selected.at( 0 ).indexes().at( 0 );
      QgsLocatorFilter *filter = mModel->filterForIndex( sel );
      mConfigureFilterButton->setEnabled( filter->hasConfigWidget() );
    }
  } );
  connect( mConfigureFilterButton, &QPushButton::clicked, this, &QgsLocatorOptionsWidget::configureCurrentFilter );
}

void QgsLocatorOptionsWidget::commitChanges()
{
  mModel->commitChanges();
  mLocatorWidget->invalidateResults();
}

void QgsLocatorOptionsWidget::configureCurrentFilter()
{
  auto selected = mFiltersTreeView->selectionModel()->selection();
  if ( selected.count() == 0 || selected.at( 0 ).indexes().count() == 0 )
  {
    return;
  }
  else
  {
    QModelIndex sel = selected.at( 0 ).indexes().at( 0 );
    QgsLocatorFilter *filter = mModel->filterForIndex( sel );
    if ( filter )
      filter->openConfigWidget();
  }
}


//
// QgsLocatorFiltersModel
//

#define HIDDEN_FILTER_OFFSET 1

QgsLocatorFiltersModel::QgsLocatorFiltersModel( QgsLocator *locator, QObject *parent )
  : QAbstractTableModel( parent )
  , mLocator( locator )
{
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

  return 4;
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
          return filterForIndex( index )->prefix();

        case Active:
        case Default:
          return QVariant();
      }
      break;
    }

    case Qt::CheckStateRole:
      switch ( index.column() )
      {
        case Name:
        case Prefix:
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
    case Qt::CheckStateRole:
    {
      bool checked = static_cast< Qt::CheckState >( value.toInt() ) == Qt::Checked;
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
    case Prefix:
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
    }
  }

  return QVariant();
}

void QgsLocatorFiltersModel::commitChanges()
{
  QgsSettings settings;

  QHash< QgsLocatorFilter *, bool >::const_iterator it = mEnabledChanges.constBegin();
  for ( ; it != mEnabledChanges.constEnd(); ++it )
  {
    QgsLocatorFilter *filter = it.key();
    settings.setValue( QStringLiteral( "locator_filters/enabled_%1" ).arg( filter->name() ), it.value(), QgsSettings::Section::Gui );
    filter->setEnabled( it.value() );
  }
  it = mDefaultChanges.constBegin();
  for ( ; it != mDefaultChanges.constEnd(); ++it )
  {
    QgsLocatorFilter *filter = it.key();
    settings.setValue( QStringLiteral( "locator_filters/default_%1" ).arg( filter->name() ), it.value(), QgsSettings::Section::Gui );
    filter->setUseWithoutPrefix( it.value() );
  }
}

QgsLocatorFilter *QgsLocatorFiltersModel::filterForIndex( const QModelIndex &index ) const
{
  return mLocator->filters().at( index.row() + HIDDEN_FILTER_OFFSET );
}
