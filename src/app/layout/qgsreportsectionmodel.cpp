/***************************************************************************
    qgsreportsectionmodel.cpp
    ---------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawso
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsreportsectionmodel.h"

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

QgsReportSectionModel::QgsReportSectionModel( QgsReport *report, QObject *parent )
  : QAbstractItemModel( parent )
  , mReport( report )
{
}

Qt::ItemFlags QgsReportSectionModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return 0;

  return QAbstractItemModel::flags( index );
}

QVariant QgsReportSectionModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsAbstractReportSection *section = sectionForIndex( index );
  if ( !section )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    {
      switch ( index.column() )
      {
        case 0:
          return section->description();
        default:
          return QVariant();
      }
      break;
    }

    case Qt::TextAlignmentRole:
    {
      return ( index.column() == 2 || index.column() == 3 ) ? Qt::AlignRight : Qt::AlignLeft;
    }

    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case 0:
          return section->type();

        default:
          return QVariant();
      }
      break;
    }

    default:
      return QVariant();
  }

  return QVariant();
}

QVariant QgsReportSectionModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section <= 0 )
  {
    QStringList lst;
    lst << tr( "Section" );
    return lst[section];
  }

  return QVariant();
}

int QgsReportSectionModel::rowCount( const QModelIndex &parent ) const
{
  QgsAbstractReportSection *parentSection = nullptr;
  if ( parent.column() > 0 )
    return 0;

  if ( !parent.isValid() )
    parentSection = mReport;
  else
    parentSection = sectionForIndex( parent );

  return parentSection->childCount();
}

int QgsReportSectionModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

QModelIndex QgsReportSectionModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  QgsAbstractReportSection *parentSection = nullptr;

  if ( !parent.isValid() )
    parentSection = mReport;
  else
    parentSection = sectionForIndex( parent );

  QgsAbstractReportSection *childSection = parentSection->childSection( row );
  if ( childSection )
    return createIndex( row, column, childSection );
  else
    return QModelIndex();
}

QModelIndex QgsReportSectionModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  QgsAbstractReportSection *childSection = sectionForIndex( index );
  QgsAbstractReportSection *parentSection = childSection->parentSection();

  if ( parentSection == mReport )
    return QModelIndex();

  return createIndex( parentSection->row(), 0, parentSection );
}

bool QgsReportSectionModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  QgsAbstractReportSection *section = sectionForIndex( index );
  ( void )section;
  ( void )value;

  if ( role != Qt::EditRole )
    return false;

  switch ( index.column() )
  {
    case 0:
      return false;

    default:
      return false;
  }

  emit dataChanged( index, index );
  return true;
}

QgsAbstractReportSection *QgsReportSectionModel::sectionForIndex( const QModelIndex &index ) const
{
  return static_cast<QgsAbstractReportSection *>( index.internalPointer() );
}

bool QgsReportSectionModel::removeRows( int row, int count, const QModelIndex &parent )
{
  QgsAbstractReportSection *parentSection = sectionForIndex( parent );

  if ( row < 0 || row >= parentSection->childCount() )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );

  for ( int i = 0; i < count; i++ )
  {
    if ( row < parentSection->childCount() )
    {
      parentSection->removeChildAt( row );
    }
  }

  endRemoveRows();

  return true;
}

void QgsReportSectionModel::addSection( const QModelIndex &parent, std::unique_ptr<QgsAbstractReportSection> section )
{
  QgsAbstractReportSection *parentSection = sectionForIndex( parent );
  if ( !parentSection )
    return;

  beginInsertRows( parent, parentSection->childCount(), parentSection->childCount() );
  parentSection->appendChild( section.release() );
  endInsertRows();
}

