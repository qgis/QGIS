/***************************************************************************
    qgsprojectstoredobjectmanagermodel.cpp
    --------------------
    Date                 : January 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectstoredobjectmanagermodel.h"
#include "moc_qgsprojectstoredobjectmanagermodel.cpp"
#include <QMessageBox>
#include <QIcon>

//
// QgsProjectStoredObjectManagerModelBase
//

QgsProjectStoredObjectManagerModelBase::QgsProjectStoredObjectManagerModelBase( QObject *parent )
  : QAbstractListModel( parent )
{

}

int QgsProjectStoredObjectManagerModelBase::rowCount( const QModelIndex &parent ) const
{
  return rowCountInternal( parent );
}

QVariant QgsProjectStoredObjectManagerModelBase::data( const QModelIndex &index, int role ) const
{
  return dataInternal( index, role );
}

bool QgsProjectStoredObjectManagerModelBase::setData( const QModelIndex &index, const QVariant &value, int role )
{
  return setDataInternal( index, value, role );
}

Qt::ItemFlags QgsProjectStoredObjectManagerModelBase::flags( const QModelIndex &index ) const
{
  return flagsInternal( index );
}

///@cond PRIVATE
void QgsProjectStoredObjectManagerModelBase::objectAboutToBeAdded( const QString &name )
{
  objectAboutToBeAddedInternal( name );
}

void QgsProjectStoredObjectManagerModelBase::objectAboutToBeRemoved( const QString &name )
{
  objectAboutToBeRemovedInternal( name );
}

void QgsProjectStoredObjectManagerModelBase::objectAdded( const QString &name )
{
  objectAddedInternal( name );
}

void QgsProjectStoredObjectManagerModelBase::objectRemoved( const QString &name )
{
  objectRemovedInternal( name );
}
///@endcond

//
// QgsProjectStoredObjectManagerModel
//
template<class T>
QgsProjectStoredObjectManagerModel<T>::QgsProjectStoredObjectManagerModel( QgsAbstractProjectStoredObjectManager<T> *manager, QObject *parent )
  : QgsProjectStoredObjectManagerModelBase( parent )
  , mObjectManager( manager )
{
  connect( mObjectManager, &QgsProjectStoredObjectManagerBase::objectAboutToBeAdded, this, &QgsProjectStoredObjectManagerModel::objectAboutToBeAdded );
  connect( mObjectManager, &QgsProjectStoredObjectManagerBase::objectAdded, this, &QgsProjectStoredObjectManagerModel::objectAdded );
  connect( mObjectManager, &QgsProjectStoredObjectManagerBase::objectAboutToBeRemoved, this, &QgsProjectStoredObjectManagerModel::objectAboutToBeRemoved );
  connect( mObjectManager, &QgsProjectStoredObjectManagerBase::objectRemoved, this, &QgsProjectStoredObjectManagerModel::objectRemoved );
}

///@cond PRIVATE
template<class T>
int QgsProjectStoredObjectManagerModel<T>::rowCountInternal( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return ( mObjectManager ? mObjectManager->objects().count() : 0 ) + ( mAllowEmpty ? 1 : 0 );
}

template<class T>
QVariant QgsProjectStoredObjectManagerModel<T>::dataInternal( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  const bool isEmpty = index.row() == 0 && mAllowEmpty;
  const int objectRow = mAllowEmpty ? index.row() - 1 : index.row();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
      return !isEmpty && mObjectManager ? mObjectManager->objects().at( objectRow )->name() : QVariant();

    case static_cast< int >( CustomRole::Object ):
    {
      if ( isEmpty || !mObjectManager )
        return QVariant();
      return objectToVariant( mObjectManager->objects().at( objectRow ) );
    }

    case Qt::DecorationRole:
    {
      return isEmpty || !mObjectManager ? QIcon() : mObjectManager->objects().at( objectRow )->icon();
    }

    default:
      return QVariant();
  }
}

template<class T>
bool QgsProjectStoredObjectManagerModel<T>::setDataInternal( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
  {
    return false;
  }
  if ( index.row() >= mObjectManager->objects().count() )
  {
    return false;
  }

  if ( index.row() == 0 && mAllowEmpty )
    return false;

  if ( value.toString().isEmpty() )
    return false;

  T *object = objectFromIndex( index );
  if ( !object )
    return false;

  //has name changed?
  bool changed = object->name() != value.toString();
  if ( !changed )
    return true;

  //check if name already exists
  QStringList existingNames;
  const QList< T * > objects = mObjectManager->objects();
  for ( T *l : objects )
  {
    existingNames << l->name();
  }
  if ( existingNames.contains( value.toString() ) )
  {
    //name exists!
    QMessageBox::warning( nullptr, tr( "Rename Layout" ), tr( "There is already a layout named “%1”." ).arg( value.toString() ) );
    return false;
  }

  object->setName( value.toString() );
  return true;
}

template<class T>
Qt::ItemFlags QgsProjectStoredObjectManagerModel<T>::flagsInternal( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractListModel::flags( index );
#if 0 // double-click is now used for opening the object
  if ( index.isValid() )
  {
    return flags | Qt::ItemIsEditable;
  }
  else
  {
    return flags;
  }
#endif
  return flags;
}

template<class T>
void QgsProjectStoredObjectManagerModel<T>::objectAboutToBeAddedInternal( const QString & )
{
  int row = mObjectManager->objects().count() + ( mAllowEmpty ? 1 : 0 );
  beginInsertRows( QModelIndex(), row, row );
}

template<class T>
void QgsProjectStoredObjectManagerModel<T>::objectAboutToBeRemovedInternal( const QString &name )
{
  T *l = mObjectManager->objectByName( name );
  int row = mObjectManager->objects().indexOf( l ) + ( mAllowEmpty ? 1 : 0 );
  if ( row >= 0 )
    beginRemoveRows( QModelIndex(), row, row );
}

template<class T>
void QgsProjectStoredObjectManagerModel<T>::objectAddedInternal( const QString & )
{
  endInsertRows();
}

template<class T>
void QgsProjectStoredObjectManagerModel<T>::objectRemovedInternal( const QString & )
{
  endRemoveRows();
}

template<class T>
void QgsProjectStoredObjectManagerModel<T>::objectRenamedInternal( T *object, const QString & )
{
  int row = mObjectManager->objects().indexOf( object ) + ( mAllowEmpty ? 1 : 0 );
  QModelIndex index = createIndex( row, 0 );
  emit dataChanged( index, index, QVector<int>() << Qt::DisplayRole );
}

template<class T>
QVariant QgsProjectStoredObjectManagerModel<T>::objectToVariant( T *object ) const
{
  if ( T *l = dynamic_cast< T * >( object ) )
    return QVariant::fromValue( l );
  return QVariant();
}


///@endcond

template<class T>
T *QgsProjectStoredObjectManagerModel<T>::objectFromIndex( const QModelIndex &index ) const
{
  if ( index.row() == 0 && mAllowEmpty )
    return nullptr;

  if ( T *l = qobject_cast< T * >( qvariant_cast<QObject *>( data( index, static_cast< int >( CustomRole::Object ) ) ) ) )
    return l;
  else
    return nullptr;
}

template<class T>
QModelIndex QgsProjectStoredObjectManagerModel<T>::indexFromObject( T *object ) const
{
  if ( !mObjectManager )
  {
    return QModelIndex();
  }

  const int r = mObjectManager->objects().indexOf( object );
  if ( r < 0 )
    return QModelIndex();

  QModelIndex idx = index( mAllowEmpty ? r + 1 : r, 0, QModelIndex() );
  if ( idx.isValid() )
  {
    return idx;
  }

  return QModelIndex();
}

template<class T>
void QgsProjectStoredObjectManagerModel<T>::setAllowEmptyObject( bool allowEmpty )
{
  if ( allowEmpty == mAllowEmpty )
    return;

  if ( allowEmpty )
  {
    beginInsertRows( QModelIndex(), 0, 0 );
    mAllowEmpty = true;
    endInsertRows();
  }
  else
  {
    beginRemoveRows( QModelIndex(), 0, 0 );
    mAllowEmpty = false;
    endRemoveRows();
  }
}


#include "qgsmasterlayoutinterface.h"
#include "qgsprintlayout.h"
#include "qgsreport.h"

///@cond PRIVATE
template<>
QVariant QgsProjectStoredObjectManagerModel<QgsMasterLayoutInterface>::objectToVariant( QgsMasterLayoutInterface *object ) const
{
  if ( QgsLayout *l = dynamic_cast< QgsLayout * >( object ) )
    return QVariant::fromValue( l );
  else if ( QgsReport *r = dynamic_cast< QgsReport * >( object ) )
    return QVariant::fromValue( r );
  return QVariant();
}

template<>
QgsMasterLayoutInterface *QgsProjectStoredObjectManagerModel<QgsMasterLayoutInterface>::objectFromIndex( const QModelIndex &index ) const
{
  if ( index.row() == 0 && mAllowEmpty )
    return nullptr;

  if ( QgsPrintLayout *l = qobject_cast< QgsPrintLayout * >( qvariant_cast<QObject *>( data( index, static_cast< int >( CustomRole::Object ) ) ) ) )
    return l;
  else if ( QgsReport *r = qobject_cast< QgsReport * >( qvariant_cast<QObject *>( data( index, static_cast< int >( CustomRole::Object ) ) ) ) )
    return r;
  else
    return nullptr;
}

template class QgsProjectStoredObjectManagerModel<QgsMasterLayoutInterface>;

///@endcond

//
// QgsProjectStoredObjectManagerProxyModelBase
//

QgsProjectStoredObjectManagerProxyModelBase::QgsProjectStoredObjectManagerProxyModelBase( QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setDynamicSortFilter( true );
  sort( 0 );
  setSortCaseSensitivity( Qt::CaseInsensitive );
}

bool QgsProjectStoredObjectManagerProxyModelBase::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  const QString leftText = sourceModel()->data( left, Qt::DisplayRole ).toString();
  const QString rightText = sourceModel()->data( right, Qt::DisplayRole ).toString();
  if ( leftText.isEmpty() )
    return true;
  if ( rightText.isEmpty() )
    return false;

  return QString::localeAwareCompare( leftText, rightText ) < 0;
}

bool QgsProjectStoredObjectManagerProxyModelBase::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  return filterAcceptsRowInternal( sourceRow, sourceParent );
}

void QgsProjectStoredObjectManagerProxyModelBase::setFilterString( const QString &filter )
{
  mFilterString = filter;
  invalidateFilter();
}

bool QgsProjectStoredObjectManagerProxyModelBase::filterAcceptsRowInternal( int, const QModelIndex & ) const
{
  return true;
}

//
// QgsProjectStoredObjectManagerProxyModel
//

template<class T>
QgsProjectStoredObjectManagerProxyModel<T>::QgsProjectStoredObjectManagerProxyModel( QObject *parent )
  : QgsProjectStoredObjectManagerProxyModelBase( parent )
{

}

template<class T>
bool QgsProjectStoredObjectManagerProxyModel<T>::filterAcceptsRowInternal( int sourceRow, const QModelIndex &sourceParent ) const
{
  QgsProjectStoredObjectManagerModel<T> *model = dynamic_cast< QgsProjectStoredObjectManagerModel<T> * >( sourceModel() );
  if ( !model )
    return false;

  T *object = model->objectFromIndex( model->index( sourceRow, 0, sourceParent ) );
  if ( !object )
    return model->allowEmptyObject();

  if ( !mFilterString.trimmed().isEmpty() )
  {
    if ( !object->name().contains( mFilterString, Qt::CaseInsensitive ) )
      return false;
  }

  return true;
}

///@cond PRIVATE
template class QgsProjectStoredObjectManagerProxyModel<QgsMasterLayoutInterface>;
///@endcond
