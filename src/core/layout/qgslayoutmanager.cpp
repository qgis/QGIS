/***************************************************************************
    qgslayoutmanager.cpp
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

#include "qgslayoutmanager.h"
#include "qgslayout.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgslayoutundostack.h"
#include "qgsprintlayout.h"
#include "qgsreport.h"
#include "qgscompositionconverter.h"
#include "qgsreadwritecontext.h"
#include "qgsstyleentityvisitor.h"
#include "qgsruntimeprofiler.h"
#include <QMessageBox>

QgsLayoutManager::QgsLayoutManager( QgsProject *project )
  : QObject( project )
  , mProject( project )
{

}

QgsLayoutManager::~QgsLayoutManager()
{
  clear();
}

bool QgsLayoutManager::addLayout( QgsMasterLayoutInterface *layout )
{
  if ( !layout || mLayouts.contains( layout ) )
    return false;

  // check for duplicate name
  for ( QgsMasterLayoutInterface *l : qgis::as_const( mLayouts ) )
  {
    if ( l->name() == layout->name() )
    {
      delete layout;
      return false;
    }
  }

  // ugly, but unavoidable for interfaces...
  if ( QgsPrintLayout *l = dynamic_cast< QgsPrintLayout * >( layout ) )
  {
    connect( l, &QgsPrintLayout::nameChanged, this, [this, l]( const QString & newName )
    {
      emit layoutRenamed( l, newName );
    } );
  }
  else if ( QgsReport *r = dynamic_cast< QgsReport * >( layout ) )
  {
    connect( r, &QgsReport::nameChanged, this, [this, r]( const QString & newName )
    {
      emit layoutRenamed( r, newName );
    } );
  }

  emit layoutAboutToBeAdded( layout->name() );
  mLayouts << layout;
  emit layoutAdded( layout->name() );
  mProject->setDirty( true );
  return true;
}

bool QgsLayoutManager::removeLayout( QgsMasterLayoutInterface *layout )
{
  if ( !layout )
    return false;

  if ( !mLayouts.contains( layout ) )
    return false;

  QString name = layout->name();
  emit layoutAboutToBeRemoved( name );
  mLayouts.removeAll( layout );
  delete layout;
  emit layoutRemoved( name );
  mProject->setDirty( true );
  return true;
}

void QgsLayoutManager::clear()
{
  const QList< QgsMasterLayoutInterface * > layouts = mLayouts;
  for ( QgsMasterLayoutInterface *l : layouts )
  {
    removeLayout( l );
  }
}

QList<QgsMasterLayoutInterface *> QgsLayoutManager::layouts() const
{
  return mLayouts;
}

QList<QgsPrintLayout *> QgsLayoutManager::printLayouts() const
{
  QList<QgsPrintLayout *> result;
  const QList<QgsMasterLayoutInterface *> _layouts( mLayouts );
  result.reserve( _layouts.size() );
  for ( const auto &layout : _layouts )
  {
    QgsPrintLayout *_item( dynamic_cast<QgsPrintLayout *>( layout ) );
    if ( _item )
      result.push_back( _item );
  }
  return result;
}

QgsMasterLayoutInterface *QgsLayoutManager::layoutByName( const QString &name ) const
{
  for ( QgsMasterLayoutInterface *l : mLayouts )
  {
    if ( l->name() == name )
      return l;
  }
  return nullptr;
}

bool QgsLayoutManager::readXml( const QDomElement &element, const QDomDocument &doc )
{
  clear();

  QDomElement layoutsElem = element;
  if ( element.tagName() != QStringLiteral( "Layouts" ) )
  {
    layoutsElem = element.firstChildElement( QStringLiteral( "Layouts" ) );
  }
  if ( layoutsElem.isNull() )
  {
    // handle legacy projects
    layoutsElem = doc.documentElement();
  }

  //restore each composer
  bool result = true;
  QDomNodeList composerNodes = element.elementsByTagName( QStringLiteral( "Composer" ) );
  QgsScopedRuntimeProfile profile( tr( "Loading QGIS 2.x compositions" ), QStringLiteral( "projectload" ) );
  for ( int i = 0; i < composerNodes.size(); ++i )
  {
    // This legacy title is the Composer "title" (that can be overridden by the Composition "name")
    QString legacyTitle = composerNodes.at( i ).toElement().attribute( QStringLiteral( "title" ) );
    // Convert compositions to layouts
    QDomNodeList compositionNodes = composerNodes.at( i ).toElement().elementsByTagName( QStringLiteral( "Composition" ) );
    for ( int j = 0; j < compositionNodes.size(); ++j )
    {
      std::unique_ptr< QgsPrintLayout > l( QgsCompositionConverter::createLayoutFromCompositionXml( compositionNodes.at( j ).toElement(), mProject ) );
      if ( l )
      {
        if ( l->name().isEmpty() )
          l->setName( legacyTitle );

        // some 2.x projects could end in a state where they had duplicated layout names. This is strictly forbidden in 3.x
        // so check for duplicate name in layouts already added
        int id = 2;
        bool isDuplicateName = false;
        QString originalName = l->name();
        do
        {
          isDuplicateName = false;
          for ( QgsMasterLayoutInterface *layout : qgis::as_const( mLayouts ) )
          {
            if ( l->name() == layout->name() )
            {
              isDuplicateName = true;
              break;
            }
          }
          if ( isDuplicateName )
          {
            l->setName( QStringLiteral( "%1 %2" ).arg( originalName ).arg( id ) );
            id++;
          }
        }
        while ( isDuplicateName );

        bool added = addLayout( l.release() );
        result = added && result;
      }
    }
  }

  QgsReadWriteContext context;
  context.setPathResolver( mProject->pathResolver() );

  profile.switchTask( tr( "Creating layouts" ) );

  // restore layouts
  const QDomNodeList layoutNodes = layoutsElem.childNodes();
  for ( int i = 0; i < layoutNodes.size(); ++i )
  {
    if ( layoutNodes.at( i ).nodeName() != QStringLiteral( "Layout" ) )
      continue;

    const QString layoutName = layoutNodes.at( i ).toElement().attribute( QStringLiteral( "name" ) );
    QgsScopedRuntimeProfile profile( layoutName, QStringLiteral( "projectload" ) );

    std::unique_ptr< QgsPrintLayout > l = qgis::make_unique< QgsPrintLayout >( mProject );
    l->undoStack()->blockCommands( true );
    if ( !l->readLayoutXml( layoutNodes.at( i ).toElement(), doc, context ) )
    {
      result = false;
      continue;
    }
    l->undoStack()->blockCommands( false );
    if ( !addLayout( l.release() ) )
    {
      result = false;
    }
  }
  //reports
  profile.switchTask( tr( "Creating reports" ) );
  const QDomNodeList reportNodes = element.elementsByTagName( QStringLiteral( "Report" ) );
  for ( int i = 0; i < reportNodes.size(); ++i )
  {
    const QString layoutName = reportNodes.at( i ).toElement().attribute( QStringLiteral( "name" ) );
    QgsScopedRuntimeProfile profile( layoutName, QStringLiteral( "projectload" ) );

    std::unique_ptr< QgsReport > r = qgis::make_unique< QgsReport >( mProject );
    if ( !r->readLayoutXml( reportNodes.at( i ).toElement(), doc, context ) )
    {
      result = false;
      continue;
    }
    if ( !addLayout( r.release() ) )
    {
      result = false;
    }
  }
  return result;
}

QDomElement QgsLayoutManager::writeXml( QDomDocument &doc ) const
{
  QDomElement layoutsElem = doc.createElement( QStringLiteral( "Layouts" ) );

  QgsReadWriteContext context;
  context.setPathResolver( mProject->pathResolver() );
  for ( QgsMasterLayoutInterface *l : mLayouts )
  {
    QDomElement layoutElem = l->writeLayoutXml( doc, context );
    layoutsElem.appendChild( layoutElem );
  }
  return layoutsElem;
}

QgsMasterLayoutInterface *QgsLayoutManager::duplicateLayout( const QgsMasterLayoutInterface *layout, const QString &newName )
{
  if ( !layout )
    return nullptr;

  std::unique_ptr< QgsMasterLayoutInterface > newLayout( layout->clone() );
  if ( !newLayout )
  {
    return nullptr;
  }

  newLayout->setName( newName );
  QgsMasterLayoutInterface *l = newLayout.get();
  if ( !addLayout( newLayout.release() ) )
  {
    return nullptr;
  }
  else
  {
    return l;
  }
}

QString QgsLayoutManager::generateUniqueTitle( QgsMasterLayoutInterface::Type type ) const
{
  QStringList names;
  names.reserve( mLayouts.size() );
  for ( QgsMasterLayoutInterface *l : mLayouts )
  {
    names << l->name();
  }
  QString name;
  int id = 1;
  while ( name.isEmpty() || names.contains( name ) )
  {
    switch ( type )
    {
      case QgsMasterLayoutInterface::PrintLayout:
        name = tr( "Layout %1" ).arg( id );
        break;
      case QgsMasterLayoutInterface::Report:
        name = tr( "Report %1" ).arg( id );
        break;
    }
    id++;
  }
  return name;
}

bool QgsLayoutManager::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mLayouts.empty() )
    return true;

  // NOTE: if visitEnter returns false it means "don't visit the layouts", not "abort all further visitations"
  if ( !visitor->visitEnter( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Layouts, QStringLiteral( "layouts" ), tr( "Layouts" ) ) ) )
    return true;

  for ( QgsMasterLayoutInterface *l : mLayouts )
  {
    if ( !l->layoutAccept( visitor ) )
      return false;
  }

  if ( !visitor->visitExit( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Layouts, QStringLiteral( "layouts" ), tr( "Layouts" ) ) ) )
    return false;

  return true;
}



//
// QgsLayoutManagerModel
//

QgsLayoutManagerModel::QgsLayoutManagerModel( QgsLayoutManager *manager, QObject *parent )
  : QAbstractListModel( parent )
  , mLayoutManager( manager )
{
  connect( mLayoutManager, &QgsLayoutManager::layoutAboutToBeAdded, this, &QgsLayoutManagerModel::layoutAboutToBeAdded );
  connect( mLayoutManager, &QgsLayoutManager::layoutAdded, this, &QgsLayoutManagerModel::layoutAdded );
  connect( mLayoutManager, &QgsLayoutManager::layoutAboutToBeRemoved, this, &QgsLayoutManagerModel::layoutAboutToBeRemoved );
  connect( mLayoutManager, &QgsLayoutManager::layoutRemoved, this, &QgsLayoutManagerModel::layoutRemoved );
  connect( mLayoutManager, &QgsLayoutManager::layoutRenamed, this, &QgsLayoutManagerModel::layoutRenamed );
}

int QgsLayoutManagerModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return ( mLayoutManager ? mLayoutManager->layouts().count() : 0 ) + ( mAllowEmpty ? 1 : 0 );
}

QVariant QgsLayoutManagerModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  const bool isEmpty = index.row() == 0 && mAllowEmpty;
  const int layoutRow = mAllowEmpty ? index.row() - 1 : index.row();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
      return !isEmpty && mLayoutManager ? mLayoutManager->layouts().at( layoutRow )->name() : QVariant();

    case LayoutRole:
    {
      if ( isEmpty || !mLayoutManager )
        return QVariant();
      else if ( QgsLayout *l = dynamic_cast< QgsLayout * >( mLayoutManager->layouts().at( layoutRow ) ) )
        return QVariant::fromValue( l );
      else if ( QgsReport *r = dynamic_cast< QgsReport * >( mLayoutManager->layouts().at( layoutRow ) ) )
        return QVariant::fromValue( r );
      else
        return QVariant();
    }

    case Qt::DecorationRole:
    {
      return isEmpty || !mLayoutManager ? QIcon() : mLayoutManager->layouts().at( layoutRow )->icon();
    }

    default:
      return QVariant();
  }
}

bool QgsLayoutManagerModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
  {
    return false;
  }
  if ( index.row() >= mLayoutManager->layouts().count() )
  {
    return false;
  }

  if ( index.row() == 0 && mAllowEmpty )
    return false;

  if ( value.toString().isEmpty() )
    return false;

  QgsMasterLayoutInterface *layout = layoutFromIndex( index );
  if ( !layout )
    return false;

  //has name changed?
  bool changed = layout->name() != value.toString();
  if ( !changed )
    return true;

  //check if name already exists
  QStringList layoutNames;
  const QList< QgsMasterLayoutInterface * > layouts = QgsProject::instance()->layoutManager()->layouts();
  for ( QgsMasterLayoutInterface *l : layouts )
  {
    layoutNames << l->name();
  }
  if ( layoutNames.contains( value.toString() ) )
  {
    //name exists!
    QMessageBox::warning( nullptr, tr( "Rename Layout" ), tr( "There is already a layout named “%1”." ).arg( value.toString() ) );
    return false;
  }

  layout->setName( value.toString() );
  return true;
}

Qt::ItemFlags QgsLayoutManagerModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractListModel::flags( index );
#if 0 // double-click is now used for opening the layout
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

QgsMasterLayoutInterface *QgsLayoutManagerModel::layoutFromIndex( const QModelIndex &index ) const
{
  if ( index.row() == 0 && mAllowEmpty )
    return nullptr;

  if ( QgsPrintLayout *l = qobject_cast< QgsPrintLayout * >( qvariant_cast<QObject *>( data( index, LayoutRole ) ) ) )
    return l;
  else if ( QgsReport *r = qobject_cast< QgsReport * >( qvariant_cast<QObject *>( data( index, LayoutRole ) ) ) )
    return r;
  else
    return nullptr;
}

QModelIndex QgsLayoutManagerModel::indexFromLayout( QgsMasterLayoutInterface *layout ) const
{
  if ( !mLayoutManager )
  {
    return QModelIndex();
  }

  const int r = mLayoutManager->layouts().indexOf( layout );
  if ( r < 0 )
    return QModelIndex();

  QModelIndex idx = index( mAllowEmpty ? r + 1 : r, 0, QModelIndex() );
  if ( idx.isValid() )
  {
    return idx;
  }

  return QModelIndex();
}

void QgsLayoutManagerModel::setAllowEmptyLayout( bool allowEmpty )
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

void QgsLayoutManagerModel::layoutAboutToBeAdded( const QString & )
{
  int row = mLayoutManager->layouts().count() + ( mAllowEmpty ? 1 : 0 );
  beginInsertRows( QModelIndex(), row, row );
}

void QgsLayoutManagerModel::layoutAboutToBeRemoved( const QString &name )
{
  QgsMasterLayoutInterface *l = mLayoutManager->layoutByName( name );
  int row = mLayoutManager->layouts().indexOf( l ) + ( mAllowEmpty ? 1 : 0 );
  if ( row >= 0 )
    beginRemoveRows( QModelIndex(), row, row );
}

void QgsLayoutManagerModel::layoutAdded( const QString & )
{
  endInsertRows();
}

void QgsLayoutManagerModel::layoutRemoved( const QString & )
{
  endRemoveRows();
}

void QgsLayoutManagerModel::layoutRenamed( QgsMasterLayoutInterface *layout, const QString & )
{
  int row = mLayoutManager->layouts().indexOf( layout ) + ( mAllowEmpty ? 1 : 0 );
  QModelIndex index = createIndex( row, 0 );
  emit dataChanged( index, index, QVector<int>() << Qt::DisplayRole );
}

//
// QgsLayoutManagerProxyModel
//

QgsLayoutManagerProxyModel::QgsLayoutManagerProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setDynamicSortFilter( true );
  sort( 0 );
  setSortCaseSensitivity( Qt::CaseInsensitive );
}

bool QgsLayoutManagerProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  const QString leftText = sourceModel()->data( left, Qt::DisplayRole ).toString();
  const QString rightText = sourceModel()->data( right, Qt::DisplayRole ).toString();
  if ( leftText.isEmpty() )
    return true;
  if ( rightText.isEmpty() )
    return false;

  return QString::localeAwareCompare( leftText, rightText ) < 0;
}

bool QgsLayoutManagerProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  QgsLayoutManagerModel *model = qobject_cast< QgsLayoutManagerModel * >( sourceModel() );
  if ( !model )
    return false;

  QgsMasterLayoutInterface *layout = model->layoutFromIndex( model->index( sourceRow, 0, sourceParent ) );
  if ( !layout )
    return model->allowEmptyLayout();

  if ( !mFilterString.trimmed().isEmpty() )
  {
    if ( !layout->name().contains( mFilterString, Qt::CaseInsensitive ) )
      return false;
  }

  switch ( layout->layoutType() )
  {
    case QgsMasterLayoutInterface::PrintLayout:
      return mFilters & FilterPrintLayouts;
    case QgsMasterLayoutInterface::Report:
      return mFilters & FilterReports;
  }
  return false;
}

QgsLayoutManagerProxyModel::Filters QgsLayoutManagerProxyModel::filters() const
{
  return mFilters;
}

void QgsLayoutManagerProxyModel::setFilters( Filters filters )
{
  mFilters = filters;
  invalidateFilter();
}

void QgsLayoutManagerProxyModel::setFilterString( const QString &filter )
{
  mFilterString = filter;
  invalidateFilter();
}
