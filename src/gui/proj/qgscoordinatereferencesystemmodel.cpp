/***************************************************************************
                             qgscoordinatereferencesystemmodel.h
                             -------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
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
#include "qgscoordinatereferencesystemmodel.h"
#include "moc_qgscoordinatereferencesystemmodel.cpp"
#include "qgscoordinatereferencesystemutils.h"
#include "qgsapplication.h"
#include "qgsstringutils.h"

#include <QFont>

QgsCoordinateReferenceSystemModel::QgsCoordinateReferenceSystemModel( QObject *parent )
  : QAbstractItemModel( parent )
  , mRootNode( std::make_unique<QgsCoordinateReferenceSystemModelGroupNode>( QString(), QIcon(), QString() ) )
{
  mCrsDbRecords = QgsApplication::coordinateReferenceSystemRegistry()->crsDbRecords();

  rebuild();

  connect( QgsApplication::coordinateReferenceSystemRegistry(), &QgsCoordinateReferenceSystemRegistry::userCrsAdded, this, &QgsCoordinateReferenceSystemModel::userCrsAdded );
  connect( QgsApplication::coordinateReferenceSystemRegistry(), &QgsCoordinateReferenceSystemRegistry::userCrsRemoved, this, &QgsCoordinateReferenceSystemModel::userCrsRemoved );
  connect( QgsApplication::coordinateReferenceSystemRegistry(), &QgsCoordinateReferenceSystemRegistry::userCrsChanged, this, &QgsCoordinateReferenceSystemModel::userCrsChanged );
}

Qt::ItemFlags QgsCoordinateReferenceSystemModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    return Qt::ItemFlags();
  }

  QgsCoordinateReferenceSystemModelNode *n = index2node( index );
  if ( !n )
    return Qt::ItemFlags();

  switch ( n->nodeType() )
  {
    case QgsCoordinateReferenceSystemModelNode::NodeGroup:
      return index.column() == 0 ? Qt::ItemIsEnabled : Qt::ItemFlags();
    case QgsCoordinateReferenceSystemModelNode::NodeCrs:
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  }
  BUILTIN_UNREACHABLE
}

QVariant QgsCoordinateReferenceSystemModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsCoordinateReferenceSystemModelNode *n = index2node( index );
  if ( !n )
    return QVariant();

  if ( role == static_cast<int>( CustomRole::NodeType ) )
    return n->nodeType();

  switch ( n->nodeType() )
  {
    case QgsCoordinateReferenceSystemModelNode::NodeGroup:
    {
      QgsCoordinateReferenceSystemModelGroupNode *groupNode = qgis::down_cast<QgsCoordinateReferenceSystemModelGroupNode *>( n );
      switch ( role )
      {
        case Qt::DecorationRole:
          switch ( index.column() )
          {
            case 0:
              return groupNode->icon();
            default:
              break;
          }
          break;

        case Qt::DisplayRole:
        case Qt::ToolTipRole:
          switch ( index.column() )
          {
            case 0:
              return groupNode->name();

            default:
              break;
          }
          break;

        case Qt::FontRole:
        {
          QFont font;
          font.setItalic( true );
          if ( groupNode->parent() == mRootNode.get() )
          {
            font.setBold( true );
          }
          return font;
        }

        case static_cast<int>( CustomRole::GroupId ):
          return groupNode->id();
      }
      return QVariant();
    }
    case QgsCoordinateReferenceSystemModelNode::NodeCrs:
    {
      QgsCoordinateReferenceSystemModelCrsNode *crsNode = qgis::down_cast<QgsCoordinateReferenceSystemModelCrsNode *>( n );
      switch ( role )
      {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
          switch ( index.column() )
          {
            case 0:
              return crsNode->record().description;

            case 1:
            {
              if ( crsNode->record().authName == QLatin1String( "CUSTOM" ) )
                return QString();
              return QStringLiteral( "%1:%2" ).arg( crsNode->record().authName, crsNode->record().authId );
            }

            default:
              break;
          }
          break;

        case static_cast<int>( CustomRole::Name ):
          return crsNode->record().description;

        case static_cast<int>( CustomRole::AuthId ):
          if ( !crsNode->record().authId.isEmpty() )
            return QStringLiteral( "%1:%2" ).arg( crsNode->record().authName, crsNode->record().authId );
          else
            return QVariant();

        case static_cast<int>( CustomRole::Deprecated ):
          return crsNode->record().deprecated;

        case static_cast<int>( CustomRole::Type ):
          return QVariant::fromValue( crsNode->record().type );

        case static_cast<int>( CustomRole::Wkt ):
          return crsNode->wkt();

        case static_cast<int>( CustomRole::Proj ):
          return crsNode->proj();

        case static_cast<int>( CustomRole::Group ):
          return crsNode->group();

        case static_cast<int>( CustomRole::Projection ):
          return crsNode->projection();

        default:
          break;
      }
    }
  }
  return QVariant();
}

QVariant QgsCoordinateReferenceSystemModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal )
  {
    switch ( role )
    {
      case Qt::DisplayRole:
        switch ( section )
        {
          case 0:
            return tr( "Coordinate Reference System" );
          case 1:
            return tr( "Authority ID" );
          default:
            break;
        }
        break;

      default:
        break;
    }
  }
  return QVariant();
}

int QgsCoordinateReferenceSystemModel::rowCount( const QModelIndex &parent ) const
{
  QgsCoordinateReferenceSystemModelNode *n = index2node( parent );
  if ( !n )
    return 0;

  return n->children().count();
}

int QgsCoordinateReferenceSystemModel::columnCount( const QModelIndex & ) const
{
  return 2;
}

QModelIndex QgsCoordinateReferenceSystemModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  QgsCoordinateReferenceSystemModelNode *n = index2node( parent );
  if ( !n )
    return QModelIndex(); // have no children

  return createIndex( row, column, n->children().at( row ) );
}

QModelIndex QgsCoordinateReferenceSystemModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() )
    return QModelIndex();

  if ( QgsCoordinateReferenceSystemModelNode *n = index2node( child ) )
  {
    return indexOfParentTreeNode( n->parent() ); // must not be null
  }
  else
  {
    Q_ASSERT( false ); // no other node types!
    return QModelIndex();
  }
}

QModelIndex QgsCoordinateReferenceSystemModel::authIdToIndex( const QString &authid ) const
{
  const QModelIndex startIndex = index( 0, 0 );
  const QModelIndexList hits = match( startIndex, static_cast<int>( CustomRole::AuthId ), authid, 1, Qt::MatchRecursive );
  return hits.value( 0 );
}

void QgsCoordinateReferenceSystemModel::rebuild()
{
  beginResetModel();

  mRootNode->deleteChildren();

  for ( const QgsCrsDbRecord &record : std::as_const( mCrsDbRecords ) )
  {
    addRecord( record );
  }

  const QList<QgsCoordinateReferenceSystemRegistry::UserCrsDetails> userCrsList = QgsApplication::coordinateReferenceSystemRegistry()->userCrsList();
  for ( const QgsCoordinateReferenceSystemRegistry::UserCrsDetails &details : userCrsList )
  {
    QgsCrsDbRecord userRecord;
    userRecord.authName = QStringLiteral( "USER" );
    userRecord.authId = QString::number( details.id );
    userRecord.description = details.name;

    addRecord( userRecord );
  }

  endResetModel();
}

void QgsCoordinateReferenceSystemModel::userCrsAdded( const QString &id )
{
  const QList<QgsCoordinateReferenceSystemRegistry::UserCrsDetails> userCrsList = QgsApplication::coordinateReferenceSystemRegistry()->userCrsList();
  for ( const QgsCoordinateReferenceSystemRegistry::UserCrsDetails &details : userCrsList )
  {
    if ( QStringLiteral( "USER:%1" ).arg( details.id ) == id )
    {
      QgsCrsDbRecord userRecord;
      userRecord.authName = QStringLiteral( "USER" );
      userRecord.authId = QString::number( details.id );
      userRecord.description = details.name;

      QgsCoordinateReferenceSystemModelGroupNode *group = mRootNode->getChildGroupNode( QStringLiteral( "USER" ) );
      if ( !group )
      {
        std::unique_ptr<QgsCoordinateReferenceSystemModelGroupNode> newGroup = std::make_unique<QgsCoordinateReferenceSystemModelGroupNode>(
          tr( "User-defined" ),
          QgsApplication::getThemeIcon( QStringLiteral( "/user.svg" ) ), QStringLiteral( "USER" )
        );
        beginInsertRows( QModelIndex(), mRootNode->children().length(), mRootNode->children().length() );
        mRootNode->addChildNode( newGroup.get() );
        endInsertRows();
        group = newGroup.release();
      }

      const QModelIndex parentGroupIndex = node2index( group );

      beginInsertRows( parentGroupIndex, group->children().size(), group->children().size() );
      QgsCoordinateReferenceSystemModelCrsNode *crsNode = addRecord( userRecord );
      crsNode->setProj( details.proj );
      crsNode->setWkt( details.wkt );
      endInsertRows();
      break;
    }
  }
}

void QgsCoordinateReferenceSystemModel::userCrsRemoved( long id )
{
  QgsCoordinateReferenceSystemModelGroupNode *group = mRootNode->getChildGroupNode( QStringLiteral( "USER" ) );
  if ( group )
  {
    for ( int row = 0; row < group->children().size(); ++row )
    {
      if ( QgsCoordinateReferenceSystemModelCrsNode *crsNode = dynamic_cast<QgsCoordinateReferenceSystemModelCrsNode *>( group->children().at( row ) ) )
      {
        if ( crsNode->record().authId == QString::number( id ) )
        {
          const QModelIndex parentIndex = node2index( group );
          beginRemoveRows( parentIndex, row, row );
          delete group->takeChild( crsNode );
          endRemoveRows();
          return;
        }
      }
    }
  }
}

void QgsCoordinateReferenceSystemModel::userCrsChanged( const QString &id )
{
  QgsCoordinateReferenceSystemModelGroupNode *group = mRootNode->getChildGroupNode( QStringLiteral( "USER" ) );
  if ( group )
  {
    for ( int row = 0; row < group->children().size(); ++row )
    {
      if ( QgsCoordinateReferenceSystemModelCrsNode *crsNode = dynamic_cast<QgsCoordinateReferenceSystemModelCrsNode *>( group->children().at( row ) ) )
      {
        if ( QStringLiteral( "USER:%1" ).arg( crsNode->record().authId ) == id )
        {
          // treat a change as a remove + add operation
          const QModelIndex parentIndex = node2index( group );
          beginRemoveRows( parentIndex, row, row );
          delete group->takeChild( crsNode );
          endRemoveRows();

          userCrsAdded( id );
          return;
        }
      }
    }
  }
}

QgsCoordinateReferenceSystemModelCrsNode *QgsCoordinateReferenceSystemModel::addRecord( const QgsCrsDbRecord &record )
{
  QgsCoordinateReferenceSystemModelGroupNode *parentNode = mRootNode.get();
  std::unique_ptr<QgsCoordinateReferenceSystemModelCrsNode> crsNode = std::make_unique<QgsCoordinateReferenceSystemModelCrsNode>( record );

  QString groupName;
  QString groupId;
  QIcon groupIcon;
  if ( record.authName == QLatin1String( "USER" ) )
  {
    groupName = tr( "User-defined" );
    groupId = QStringLiteral( "USER" );
    groupIcon = QgsApplication::getThemeIcon( QStringLiteral( "/user.svg" ) );
  }
  else if ( record.authName == QLatin1String( "CUSTOM" ) )
  {
    // the group is guaranteed to exist at this point
    groupId = QStringLiteral( "CUSTOM" );
  }
  else
  {
    groupId = qgsEnumValueToKey( record.type );
    switch ( record.type )
    {
      case Qgis::CrsType::Unknown:
        break;
      case Qgis::CrsType::Geodetic:
        groupName = tr( "Geodetic" );
        groupIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconProjectionEnabled.svg" ) );
        break;
      case Qgis::CrsType::Geocentric:
        groupName = tr( "Geocentric" );
        groupIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconProjectionEnabled.svg" ) );
        break;
      case Qgis::CrsType::Geographic2d:
        groupName = tr( "Geographic (2D)" );
        groupIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconProjectionEnabled.svg" ) );
        break;

      case Qgis::CrsType::Geographic3d:
        groupName = tr( "Geographic (3D)" );
        groupIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconProjectionEnabled.svg" ) );
        break;

      case Qgis::CrsType::Vertical:
        groupName = tr( "Vertical" );
        break;

      case Qgis::CrsType::Projected:
      case Qgis::CrsType::DerivedProjected:
        groupName = tr( "Projected" );
        groupIcon = QgsApplication::getThemeIcon( QStringLiteral( "/transformed.svg" ) );
        break;

      case Qgis::CrsType::Compound:
        groupName = tr( "Compound" );
        break;

      case Qgis::CrsType::Temporal:
        groupName = tr( "Temporal" );
        break;

      case Qgis::CrsType::Engineering:
        groupName = tr( "Engineering" );
        break;

      case Qgis::CrsType::Bound:
        groupName = tr( "Bound" );
        break;

      case Qgis::CrsType::Other:
        groupName = tr( "Other" );
        break;
    }
  }
  crsNode->setGroup( groupName );

  if ( QgsCoordinateReferenceSystemModelGroupNode *group = parentNode->getChildGroupNode( groupId ) )
  {
    parentNode = group;
  }
  else
  {
    std::unique_ptr<QgsCoordinateReferenceSystemModelGroupNode> newGroup = std::make_unique<QgsCoordinateReferenceSystemModelGroupNode>( groupName, groupIcon, groupId );
    parentNode->addChildNode( newGroup.get() );
    parentNode = newGroup.release();
  }

  if ( ( record.authName != QLatin1String( "USER" ) && record.authName != QLatin1String( "CUSTOM" ) ) && ( record.type == Qgis::CrsType::Projected || record.type == Qgis::CrsType::DerivedProjected ) )
  {
    QString projectionName = QgsCoordinateReferenceSystemUtils::translateProjection( record.projectionAcronym );
    if ( projectionName.isEmpty() )
      projectionName = tr( "Other" );
    else
      crsNode->setProjection( projectionName );

    if ( QgsCoordinateReferenceSystemModelGroupNode *group = parentNode->getChildGroupNode( record.projectionAcronym ) )
    {
      parentNode = group;
    }
    else
    {
      std::unique_ptr<QgsCoordinateReferenceSystemModelGroupNode> newGroup = std::make_unique<QgsCoordinateReferenceSystemModelGroupNode>( projectionName, QIcon(), record.projectionAcronym );
      parentNode->addChildNode( newGroup.get() );
      parentNode = newGroup.release();
    }
  }

  parentNode->addChildNode( crsNode.get() );
  return crsNode.release();
}

QModelIndex QgsCoordinateReferenceSystemModel::addCustomCrs( const QgsCoordinateReferenceSystem &crs )
{
  QgsCrsDbRecord userRecord;
  userRecord.authName = QStringLiteral( "CUSTOM" );
  userRecord.description = crs.description().isEmpty() ? tr( "Custom CRS" ) : crs.description();
  userRecord.type = crs.type();

  QgsCoordinateReferenceSystemModelGroupNode *group = mRootNode->getChildGroupNode( QStringLiteral( "CUSTOM" ) );
  if ( !group )
  {
    std::unique_ptr<QgsCoordinateReferenceSystemModelGroupNode> newGroup = std::make_unique<QgsCoordinateReferenceSystemModelGroupNode>(
      tr( "Custom" ),
      QgsApplication::getThemeIcon( QStringLiteral( "/user.svg" ) ), QStringLiteral( "CUSTOM" )
    );
    beginInsertRows( QModelIndex(), mRootNode->children().length(), mRootNode->children().length() );
    mRootNode->addChildNode( newGroup.get() );
    endInsertRows();
    group = newGroup.release();
  }

  const QModelIndex parentGroupIndex = node2index( group );

  const int newRow = group->children().size();
  beginInsertRows( parentGroupIndex, newRow, newRow );
  QgsCoordinateReferenceSystemModelCrsNode *node = addRecord( userRecord );
  node->setWkt( crs.toWkt( Qgis::CrsWktVariant::Preferred ) );
  node->setProj( crs.toProj() );
  endInsertRows();

  return index( newRow, 0, parentGroupIndex );
}

QgsCoordinateReferenceSystemModelNode *QgsCoordinateReferenceSystemModel::index2node( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mRootNode.get();

  return reinterpret_cast<QgsCoordinateReferenceSystemModelNode *>( index.internalPointer() );
}

QModelIndex QgsCoordinateReferenceSystemModel::node2index( QgsCoordinateReferenceSystemModelNode *node ) const
{
  if ( !node || !node->parent() )
    return QModelIndex(); // this is the only root item -> invalid index

  QModelIndex parentIndex = node2index( node->parent() );

  int row = node->parent()->children().indexOf( node );
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
}

QModelIndex QgsCoordinateReferenceSystemModel::indexOfParentTreeNode( QgsCoordinateReferenceSystemModelNode *parentNode ) const
{
  Q_ASSERT( parentNode );

  QgsCoordinateReferenceSystemModelNode *grandParentNode = parentNode->parent();
  if ( !grandParentNode )
    return QModelIndex(); // root node -> invalid index

  int row = grandParentNode->children().indexOf( parentNode );
  Q_ASSERT( row >= 0 );

  return createIndex( row, 0, parentNode );
}

///@cond PRIVATE
QgsCoordinateReferenceSystemModelNode::~QgsCoordinateReferenceSystemModelNode()
{
  qDeleteAll( mChildren );
}

QgsCoordinateReferenceSystemModelNode *QgsCoordinateReferenceSystemModelNode::takeChild( QgsCoordinateReferenceSystemModelNode *node )
{
  return mChildren.takeAt( mChildren.indexOf( node ) );
}

void QgsCoordinateReferenceSystemModelNode::addChildNode( QgsCoordinateReferenceSystemModelNode *node )
{
  if ( !node )
    return;

  Q_ASSERT( !node->mParent );
  node->mParent = this;

  mChildren.append( node );
}

void QgsCoordinateReferenceSystemModelNode::deleteChildren()
{
  qDeleteAll( mChildren );
  mChildren.clear();
}

QgsCoordinateReferenceSystemModelGroupNode *QgsCoordinateReferenceSystemModelNode::getChildGroupNode( const QString &id )
{
  for ( QgsCoordinateReferenceSystemModelNode *node : std::as_const( mChildren ) )
  {
    if ( node->nodeType() == NodeGroup )
    {
      QgsCoordinateReferenceSystemModelGroupNode *groupNode = qgis::down_cast<QgsCoordinateReferenceSystemModelGroupNode *>( node );
      if ( groupNode && groupNode->id() == id )
        return groupNode;
    }
  }
  return nullptr;
}

QgsCoordinateReferenceSystemModelGroupNode::QgsCoordinateReferenceSystemModelGroupNode( const QString &name, const QIcon &icon, const QString &id )
  : mId( id )
  , mName( name )
  , mIcon( icon )
{
}

QgsCoordinateReferenceSystemModelCrsNode::QgsCoordinateReferenceSystemModelCrsNode( const QgsCrsDbRecord &record )
  : mRecord( record )
{
}
///@endcond


//
// QgsCoordinateReferenceSystemProxyModel
//

QgsCoordinateReferenceSystemProxyModel::QgsCoordinateReferenceSystemProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
  , mModel( new QgsCoordinateReferenceSystemModel( this ) )
{
  setSourceModel( mModel );
  setDynamicSortFilter( true );
  setSortLocaleAware( true );
  setFilterCaseSensitivity( Qt::CaseInsensitive );
  setRecursiveFilteringEnabled( true );
  sort( 0 );
}

QgsCoordinateReferenceSystemModel *QgsCoordinateReferenceSystemProxyModel::coordinateReferenceSystemModel()
{
  return mModel;
}

const QgsCoordinateReferenceSystemModel *QgsCoordinateReferenceSystemProxyModel::coordinateReferenceSystemModel() const
{
  return mModel;
}

void QgsCoordinateReferenceSystemProxyModel::setFilters( QgsCoordinateReferenceSystemProxyModel::Filters filters )
{
  if ( mFilters == filters )
    return;

  mFilters = filters;
  invalidateFilter();
}

void QgsCoordinateReferenceSystemProxyModel::setFilterString( const QString &filter )
{
  mFilterString = filter;
  invalidateFilter();
}

void QgsCoordinateReferenceSystemProxyModel::setFilterAuthIds( const QSet<QString> &filter )
{
  if ( mFilterAuthIds == filter )
    return;

  mFilterAuthIds.clear();
  mFilterAuthIds.reserve( filter.size() );
  for ( const QString &id : filter )
  {
    mFilterAuthIds.insert( id.toUpper() );
  }
  invalidateFilter();
}

void QgsCoordinateReferenceSystemProxyModel::setFilterDeprecated( bool filter )
{
  if ( mFilterDeprecated == filter )
    return;

  mFilterDeprecated = filter;
  invalidateFilter();
}

bool QgsCoordinateReferenceSystemProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  if ( mFilterString.trimmed().isEmpty() && !mFilters && !mFilterDeprecated && mFilterAuthIds.isEmpty() )
    return true;

  const QModelIndex sourceIndex = mModel->index( sourceRow, 0, sourceParent );
  const QgsCoordinateReferenceSystemModelNode::NodeType nodeType = static_cast<QgsCoordinateReferenceSystemModelNode::NodeType>( sourceModel()->data( sourceIndex, static_cast<int>( QgsCoordinateReferenceSystemModel::CustomRole::NodeType ) ).toInt() );
  switch ( nodeType )
  {
    case QgsCoordinateReferenceSystemModelNode::NodeGroup:
      return false;
    case QgsCoordinateReferenceSystemModelNode::NodeCrs:
      break;
  }

  const bool deprecated = sourceModel()->data( sourceIndex, static_cast<int>( QgsCoordinateReferenceSystemModel::CustomRole::Deprecated ) ).toBool();
  if ( mFilterDeprecated && deprecated )
    return false;

  if ( mFilters )
  {
    const Qgis::CrsType type = sourceModel()->data( sourceIndex, static_cast<int>( QgsCoordinateReferenceSystemModel::CustomRole::Type ) ).value<Qgis::CrsType>();
    switch ( type )
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
        if ( !mFilters.testFlag( Filter::FilterHorizontal ) )
          return false;
        break;

      case Qgis::CrsType::Vertical:
        if ( !mFilters.testFlag( Filter::FilterVertical ) )
          return false;
        break;

      case Qgis::CrsType::Compound:
        if ( !mFilters.testFlag( Filter::FilterCompound ) )
          return false;
        break;
    }
  }

  const QString authid = sourceModel()->data( sourceIndex, static_cast<int>( QgsCoordinateReferenceSystemModel::CustomRole::AuthId ) ).toString();
  if ( !mFilterAuthIds.isEmpty() )
  {
    if ( !mFilterAuthIds.contains( authid.toUpper() ) )
      return false;
  }

  if ( !mFilterString.trimmed().isEmpty() )
  {
    const QString name = sourceModel()->data( sourceIndex, static_cast<int>( QgsCoordinateReferenceSystemModel::CustomRole::Name ) ).toString();
    QString candidate = name;
    const QString groupName = sourceModel()->data( sourceIndex, static_cast<int>( QgsCoordinateReferenceSystemModel::CustomRole::Group ) ).toString();
    if ( !groupName.isEmpty() )
      candidate += ' ' + groupName;
    const QString projectionName = sourceModel()->data( sourceIndex, static_cast<int>( QgsCoordinateReferenceSystemModel::CustomRole::Projection ) ).toString();
    if ( !projectionName.isEmpty() )
      candidate += ' ' + projectionName;

    if ( !( QgsStringUtils::containsByWord( candidate, mFilterString )
            || authid.contains( mFilterString, Qt::CaseInsensitive ) ) )
      return false;
  }
  return true;
}

bool QgsCoordinateReferenceSystemProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  QgsCoordinateReferenceSystemModelNode::NodeType leftType = static_cast<QgsCoordinateReferenceSystemModelNode::NodeType>( sourceModel()->data( left, static_cast<int>( QgsCoordinateReferenceSystemModel::CustomRole::NodeType ) ).toInt() );
  QgsCoordinateReferenceSystemModelNode::NodeType rightType = static_cast<QgsCoordinateReferenceSystemModelNode::NodeType>( sourceModel()->data( right, static_cast<int>( QgsCoordinateReferenceSystemModel::CustomRole::NodeType ) ).toInt() );

  if ( leftType != rightType )
  {
    if ( leftType == QgsCoordinateReferenceSystemModelNode::NodeGroup )
      return true;
    else if ( rightType == QgsCoordinateReferenceSystemModelNode::NodeGroup )
      return false;
  }

  const QString leftStr = sourceModel()->data( left ).toString().toLower();
  const QString rightStr = sourceModel()->data( right ).toString().toLower();

  if ( leftType == QgsCoordinateReferenceSystemModelNode::NodeGroup )
  {
    // both are groups -- ensure USER group comes last, and CUSTOM group comes first
    const QString leftGroupId = sourceModel()->data( left, static_cast<int>( QgsCoordinateReferenceSystemModel::CustomRole::GroupId ) ).toString();
    const QString rightGroupId = sourceModel()->data( left, static_cast<int>( QgsCoordinateReferenceSystemModel::CustomRole::GroupId ) ).toString();
    if ( leftGroupId == QLatin1String( "USER" ) )
      return false;
    if ( rightGroupId == QLatin1String( "USER" ) )
      return true;

    if ( leftGroupId == QLatin1String( "CUSTOM" ) )
      return true;
    if ( rightGroupId == QLatin1String( "CUSTOM" ) )
      return false;
  }

  // default sort is alphabetical order
  return QString::localeAwareCompare( leftStr, rightStr ) < 0;
}
