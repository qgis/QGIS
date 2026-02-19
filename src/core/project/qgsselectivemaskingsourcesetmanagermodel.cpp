/***************************************************************************
    qgsselectivemaskingsourcesetmanagermodel.cpp
    --------------------
    Date                 : January 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsselectivemaskingsourcesetmanagermodel.h"

#include "qgsproject.h"
#include "qgsselectivemaskingsourceset.h"
#include "qgsselectivemaskingsourcesetmanager.h"

#include "moc_qgsselectivemaskingsourcesetmanagermodel.cpp"

//
// QgsSelectiveMaskingSourceSetManagerModel
//

QgsSelectiveMaskingSourceSetManagerModel::QgsSelectiveMaskingSourceSetManagerModel( QgsSelectiveMaskingSourceSetManager *manager, QObject *parent )
  : QgsProjectStoredObjectManagerModel( manager, parent )
{
  connect( manager, &QgsSelectiveMaskingSourceSetManager::setRenamed, this, [this]( const QString &, const QString & newName )
  {
    QgsSelectiveMaskingSourceSet *sourceSet = mObjectManager->objectByName( newName );
    objectRenamedInternal( sourceSet, newName );
  } );
}

QgsSelectiveMaskingSourceSet *QgsSelectiveMaskingSourceSetManagerModel::setFromIndex( const QModelIndex &index ) const
{
  return objectFromIndex( index );
}

QModelIndex QgsSelectiveMaskingSourceSetManagerModel::indexFromSet( QgsSelectiveMaskingSourceSet *set ) const
{
  return indexFromObject( set );
}

QVariant QgsSelectiveMaskingSourceSetManagerModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  const bool isEmpty = index.row() == 0 && allowEmptyObject();
  if ( isEmpty )
  {
    switch ( role )
    {
      case Qt::DisplayRole:
        return tr( "Custom" );

      case Qt::FontRole:
      {
        QFont font;
        font.setItalic( true );
        return font;
      }

      case Qt::ForegroundRole:
      {
        QBrush brush = QgsProjectStoredObjectManagerModel::data( index, role ).value< QBrush >();
        QColor fadedTextColor = brush.color();
        fadedTextColor.setAlpha( 128 );
        brush.setColor( fadedTextColor );
        return brush;
      }

      default:
        break;
    }

    return QgsProjectStoredObjectManagerModel::data( index, role );
  }
  else if ( role == static_cast< int >( CustomRole::SetId ) )
  {
    if ( QgsSelectiveMaskingSourceSet *set = setFromIndex( index ) )
    {
      return set->id();
    }
    return QVariant();
  }
  else
  {
    return QgsProjectStoredObjectManagerModel::data( index, role );
  }
}


//
// QgsSelectiveMaskingSourceSetManagerProxyModel
//

QgsSelectiveMaskingSourceSetManagerProxyModel::QgsSelectiveMaskingSourceSetManagerProxyModel( QObject *parent )
  : QgsProjectStoredObjectManagerProxyModel( parent )
{
}
