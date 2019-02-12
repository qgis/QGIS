/***************************************************************************
    qgsreportsectionmodel.cpp
    ---------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
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
#include "functional"
#include "qgsguiutils.h"

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
    return nullptr;

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

    case Qt::DecorationRole:
      switch ( index.column() )
      {
        case 0:
        {
          QIcon icon = section->icon();

          if ( section == mEditedSection )
          {
            const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
            QPixmap pixmap( icon.pixmap( iconSize, iconSize ) );

            QPainter painter( &pixmap );
            painter.drawPixmap( 0, 0, iconSize, iconSize, QgsApplication::getThemePixmap( QStringLiteral( "/mActionToggleEditing.svg" ) ) );
            painter.end();

            return QIcon( pixmap );
          }
          else
          {
            return icon;
          }
        }

        default:
          return QVariant();
      }
      break;

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
  if ( !parent.isValid() )
    return 1; // report

  QgsAbstractReportSection *parentSection = sectionForIndex( parent );
  return parentSection ? parentSection->childCount() : 0;
}

bool QgsReportSectionModel::hasChildren( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
    return true; // root item: its children are top level items

  QgsAbstractReportSection *parentSection = sectionForIndex( parent );
  return parentSection && parentSection->childCount() > 0;
}

int QgsReportSectionModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

QModelIndex QgsReportSectionModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  QgsAbstractReportSection *parentSection = sectionForIndex( parent );
  if ( parentSection )
  {
    QgsAbstractReportSection *item = parentSection->childSections().value( row, nullptr );
    return item ? createIndex( row, column, item ) : QModelIndex();
  }
  else
  {
    if ( row == 0 )
      return createIndex( row, column, nullptr );
    else
      return QModelIndex();
  }
}

QModelIndex QgsReportSectionModel::parent( const QModelIndex &index ) const
{
  QgsAbstractReportSection *childSection = sectionForIndex( index );
  if ( !childSection )
    return QModelIndex();

  QgsAbstractReportSection *parentSection = childSection->parentSection();

  if ( !parentSection )
    return QModelIndex();
  else
    return createIndex( parentSection->row(), 0, parentSection != mReport ? parentSection : nullptr );
}

QgsAbstractReportSection *QgsReportSectionModel::sectionForIndex( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return nullptr;

  if ( !index.internalPointer() ) // top level item
    return mReport; // IMPORTANT - QgsReport uses multiple inheritance, so cannot static cast the void*!

  return static_cast<QgsAbstractReportSection *>( index.internalPointer() );
}

QModelIndex QgsReportSectionModel::indexForSection( QgsAbstractReportSection *section ) const
{
  if ( !section )
    return QModelIndex();

  std::function< QModelIndex( const QModelIndex &parent, QgsAbstractReportSection *section ) > findIndex = [&]( const QModelIndex & parent, QgsAbstractReportSection * section )->QModelIndex
  {
    for ( int row = 0; row < rowCount( parent ); ++row )
    {
      QModelIndex current = index( row, 0, parent );
      if ( sectionForIndex( current ) == section )
        return current;

      QModelIndex checkChildren = findIndex( current, section );
      if ( checkChildren.isValid() )
        return checkChildren;
    }
    return QModelIndex();
  };

  return findIndex( QModelIndex(), section );
}

void QgsReportSectionModel::setEditedSection( QgsAbstractReportSection *section )
{
  QModelIndex oldSection;
  if ( mEditedSection )
  {
    oldSection = indexForSection( mEditedSection );
  }

  mEditedSection = section;
  if ( oldSection.isValid() )
    emit dataChanged( oldSection, oldSection, QVector<int>() << Qt::DecorationRole );

  if ( mEditedSection )
  {
    QModelIndex newSection = indexForSection( mEditedSection );
    emit dataChanged( newSection, newSection, QVector<int>() << Qt::DecorationRole );
  }

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
    parentSection = mReport;

  beginInsertRows( parent, parentSection->childCount(), parentSection->childCount() );
  parentSection->appendChild( section.release() );
  endInsertRows();
}

