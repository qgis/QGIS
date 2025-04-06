/***************************************************************************
    qgsattributesformmodel.cpp
    ---------------------
    begin                : March 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsactionmanager.h"
#include "qgsattributesformmodel.h"
#include "moc_qgsattributesformmodel.cpp"
#include "qgsgui.h"
#include "qgseditorwidgetregistry.h"
#include "qgsattributeeditoraction.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributeeditorrelation.h"
#include "qgsattributeeditorqmlelement.h"
#include "qgsattributeeditorhtmlelement.h"
#include "qgsattributeeditortextelement.h"
#include "qgsattributeeditorspacerelement.h"

#include <QMimeData>

/*
 * FieldConfig implementation
 */
QgsAttributesFormTreeData::FieldConfig::FieldConfig( QgsVectorLayer *layer, int idx )
{
  mAlias = layer->fields().at( idx ).alias();
  mDataDefinedProperties = layer->editFormConfig().dataDefinedFieldProperties( layer->fields().at( idx ).name() );
  mComment = layer->fields().at( idx ).comment();
  mEditable = !layer->editFormConfig().readOnly( idx );
  mLabelOnTop = layer->editFormConfig().labelOnTop( idx );
  mReuseLastValues = layer->editFormConfig().reuseLastValue( idx );
  mFieldConstraints = layer->fields().at( idx ).constraints();
  const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( layer, layer->fields().field( idx ).name() );
  mEditorWidgetType = setup.type();
  mEditorWidgetConfig = setup.config();
  mSplitPolicy = layer->fields().at( idx ).splitPolicy();
  mDuplicatePolicy = layer->fields().at( idx ).duplicatePolicy();
}

QgsAttributesFormTreeData::FieldConfig::operator QVariant()
{
  return QVariant::fromValue<QgsAttributesFormTreeData::FieldConfig>( *this );
}

/*
 * RelationEditorConfiguration implementation
 */
QgsAttributesFormTreeData::RelationEditorConfiguration::operator QVariant()
{
  return QVariant::fromValue<QgsAttributesFormTreeData::RelationEditorConfiguration>( *this );
}

Qgis::AttributeEditorContainerType QgsAttributesFormTreeData::DnDTreeNodeData::containerType() const
{
  return mContainerType;
}

void QgsAttributesFormTreeData::DnDTreeNodeData::setContainerType( Qgis::AttributeEditorContainerType type )
{
  mContainerType = type;
}

const QgsAttributeEditorElement::LabelStyle QgsAttributesFormTreeData::DnDTreeNodeData::labelStyle() const
{
  return mLabelStyle;
}

void QgsAttributesFormTreeData::DnDTreeNodeData::setLabelStyle( const QgsAttributeEditorElement::LabelStyle &labelStyle )
{
  mLabelStyle = labelStyle;
}

bool QgsAttributesFormTreeData::DnDTreeNodeData::showLabel() const
{
  return mShowLabel;
}

void QgsAttributesFormTreeData::DnDTreeNodeData::setShowLabel( bool showLabel )
{
  mShowLabel = showLabel;
}

QgsOptionalExpression QgsAttributesFormTreeData::DnDTreeNodeData::visibilityExpression() const
{
  return mVisibilityExpression;
}

void QgsAttributesFormTreeData::DnDTreeNodeData::setVisibilityExpression( const QgsOptionalExpression &visibilityExpression )
{
  mVisibilityExpression = visibilityExpression;
}

QgsOptionalExpression QgsAttributesFormTreeData::DnDTreeNodeData::collapsedExpression() const
{
  return mCollapsedExpression;
}

void QgsAttributesFormTreeData::DnDTreeNodeData::setCollapsedExpression( const QgsOptionalExpression &collapsedExpression )
{
  mCollapsedExpression = collapsedExpression;
}

QgsAttributesFormTreeData::RelationEditorConfiguration QgsAttributesFormTreeData::DnDTreeNodeData::relationEditorConfiguration() const
{
  return mRelationEditorConfiguration;
}

void QgsAttributesFormTreeData::DnDTreeNodeData::setRelationEditorConfiguration( const QgsAttributesFormTreeData::RelationEditorConfiguration &relationEditorConfiguration )
{
  mRelationEditorConfiguration = relationEditorConfiguration;
}

QgsAttributesFormTreeData::QmlElementEditorConfiguration QgsAttributesFormTreeData::DnDTreeNodeData::qmlElementEditorConfiguration() const
{
  return mQmlElementEditorConfiguration;
}

void QgsAttributesFormTreeData::DnDTreeNodeData::setQmlElementEditorConfiguration( const QmlElementEditorConfiguration &qmlElementEditorConfiguration )
{
  mQmlElementEditorConfiguration = qmlElementEditorConfiguration;
}


QgsAttributesFormTreeData::HtmlElementEditorConfiguration QgsAttributesFormTreeData::DnDTreeNodeData::htmlElementEditorConfiguration() const
{
  return mHtmlElementEditorConfiguration;
}

void QgsAttributesFormTreeData::DnDTreeNodeData::setHtmlElementEditorConfiguration( const HtmlElementEditorConfiguration &htmlElementEditorConfiguration )
{
  mHtmlElementEditorConfiguration = htmlElementEditorConfiguration;
}

QgsAttributesFormTreeData::SpacerElementEditorConfiguration QgsAttributesFormTreeData::DnDTreeNodeData::spacerElementEditorConfiguration() const
{
  return mSpacerElementEditorConfiguration;
}

void QgsAttributesFormTreeData::DnDTreeNodeData::setSpacerElementEditorConfiguration( SpacerElementEditorConfiguration spacerElementEditorConfiguration )
{
  mSpacerElementEditorConfiguration = spacerElementEditorConfiguration;
}

QColor QgsAttributesFormTreeData::DnDTreeNodeData::backgroundColor() const
{
  return mBackgroundColor;
}

void QgsAttributesFormTreeData::DnDTreeNodeData::setBackgroundColor( const QColor &backgroundColor )
{
  mBackgroundColor = backgroundColor;
}

QgsAttributesFormTreeData::TextElementEditorConfiguration QgsAttributesFormTreeData::DnDTreeNodeData::textElementEditorConfiguration() const
{
  return mTextElementEditorConfiguration;
}

void QgsAttributesFormTreeData::DnDTreeNodeData::setTextElementEditorConfiguration( const QgsAttributesFormTreeData::TextElementEditorConfiguration &textElementEditorConfiguration )
{
  mTextElementEditorConfiguration = textElementEditorConfiguration;
}

//
// QgsAttributesFormTreeNode implementation
//

QgsAttributesFormTreeNode::QgsAttributesFormTreeNode( QgsAttributesFormTreeData::AttributesFormTreeNodeType nodeType, const QString &name, const QString &displayName, QgsAttributesFormTreeNode *parent )
  : mName( name )
  , mDisplayName( !displayName.isEmpty() ? displayName : name )
  , mNodeType( nodeType )
  , mParent( parent )
{}

QgsAttributesFormTreeNode::QgsAttributesFormTreeNode( QgsAttributesFormTreeData::AttributesFormTreeNodeType nodeType, const QgsAttributesFormTreeData::DnDTreeNodeData &data, const QString &name, const QString &displayName, QgsAttributesFormTreeNode *parent )
  : mName( name )
  , mDisplayName( !displayName.isEmpty() ? displayName : name )
  , mNodeType( nodeType )
  , mNodeData( data )
  , mParent( parent )
{}

QgsAttributesFormTreeNode *QgsAttributesFormTreeNode::child( int row )
{
  if ( !mChildren.empty() && row >= 0 && row < childCount() )
    return mChildren.at( row ).get();

  return nullptr;
}

QgsAttributesFormTreeNode *QgsAttributesFormTreeNode::firstTopChild( const QgsAttributesFormTreeData::AttributesFormTreeNodeType nodeType, const QString &nodeId ) const
{
  if ( !mChildren.empty() && nodeId.trimmed().isEmpty() )
    return nullptr;

  // Search for first matching node by name
  const auto it = std::find_if( mChildren.cbegin(), mChildren.cend(), [nodeType, nodeId]( const std::unique_ptr< QgsAttributesFormTreeNode > &treeNode ) {
    return treeNode->type() == nodeType && treeNode->name() == nodeId;
  } );

  if ( it != mChildren.cend() )
    return it->get();

  return nullptr;
}

QgsAttributesFormTreeNode *QgsAttributesFormTreeNode::firstChildRecursive( const QgsAttributesFormTreeData::AttributesFormTreeNodeType &nodeType, const QString &nodeId ) const
{
  if ( !mChildren.empty() && nodeId.trimmed().isEmpty() )
    return nullptr;

  for ( const auto &child : std::as_const( mChildren ) )
  {
    if ( child->type() == nodeType && child->id() == nodeId )
      return child.get();

    if ( child->childCount() > 0 )
    {
      QgsAttributesFormTreeNode *node = child->firstChildRecursive( nodeType, nodeId );
      if ( node )
        return node;
    }
  }

  return nullptr;
}

int QgsAttributesFormTreeNode::childCount() const
{
  return int( mChildren.size() );
}

int QgsAttributesFormTreeNode::row() const
{
  if ( !mParent )
    return 0;

  const auto it = std::find_if( mParent->mChildren.cbegin(), mParent->mChildren.cend(), [this]( const std::unique_ptr< QgsAttributesFormTreeNode > &treeNode ) {
    return treeNode.get() == this;
  } );

  if ( it != mParent->mChildren.cend() )
    return ( int ) std::distance( mParent->mChildren.cbegin(), it );
  return -1;
}

void QgsAttributesFormTreeNode::addChildNode( std::unique_ptr< QgsAttributesFormTreeNode > &&node )
{
  if ( !node )
    return;

  if ( !node->mParent )
    node->mParent = this;

  mChildren.push_back( std::move( node ) );
}

void QgsAttributesFormTreeNode::insertChildNode( int position, std::unique_ptr< QgsAttributesFormTreeNode > &&node )
{
  if ( position < 0 || position > ( int ) mChildren.size() || !node )
    return;

  if ( !node->mParent )
    node->mParent = this;

  mChildren.insert( mChildren.begin() + position, std::move( node ) );
}

void QgsAttributesFormTreeNode::deleteChildAtIndex( int index )
{
  if ( index >= 0 && index < ( int ) mChildren.size() )
    mChildren.erase( mChildren.begin() + index );
}

void QgsAttributesFormTreeNode::deleteChildren()
{
  mChildren.clear();
}

QVariant QgsAttributesFormTreeNode::data( int role ) const
{
  switch ( role )
  {
    case QgsAttributesFormModel::NodeTypeRole:
      return mNodeType;
    case QgsAttributesFormModel::NodeDataRole:
      return QVariant::fromValue( mNodeData );
    case QgsAttributesFormModel::NodeNameRole:
      return mName;
    case QgsAttributesFormModel::NodeIdRole:
      return mNodeId;
    case QgsAttributesFormModel::NodeDisplayRole:
      return mDisplayName;
    case QgsAttributesFormModel::NodeFieldConfigRole:
      return QVariant::fromValue( mFieldConfigData );
    default:
      return QVariant();
  }
}

bool QgsAttributesFormTreeNode::setData( int role, const QVariant &value )
{
  switch ( role )
  {
    case QgsAttributesFormModel::NodeDataRole:
    {
      mNodeData = value.value< QgsAttributesFormTreeData::DnDTreeNodeData >();
      return true;
    }
    case QgsAttributesFormModel::NodeNameRole:
    {
      mName = value.toString();
      return true;
    }
    case QgsAttributesFormModel::NodeDisplayRole:
    {
      mDisplayName = value.toString();
      return true;
    }
    case QgsAttributesFormModel::NodeTypeRole:
    {
      mNodeType = static_cast<QgsAttributesFormTreeData::AttributesFormTreeNodeType>( value.toInt() );
      return true;
    }
    case QgsAttributesFormModel::NodeIdRole:
    {
      mNodeId = value.toString();
      return true;
    }
    case QgsAttributesFormModel::NodeFieldConfigRole:
    {
      mFieldConfigData = value.value< QgsAttributesFormTreeData::FieldConfig >();
      return true;
    }
    default:
      return false;
  }
}


//
// QgsAttributesFormModel implementation
//

QgsAttributesFormModel::QgsAttributesFormModel( QgsVectorLayer *layer, QgsProject *project, QObject *parent )
  : QAbstractItemModel( parent )
  , mRootNode( std::make_unique< QgsAttributesFormTreeNode >() )
  , mLayer( layer )
  , mProject( project )
{
}

QgsAttributesFormModel::~QgsAttributesFormModel() = default;

QgsAttributesFormTreeNode *QgsAttributesFormModel::nodeForIndex( const QModelIndex &index ) const
{
  if ( index.isValid() )
  {
    if ( auto *node = static_cast<QgsAttributesFormTreeNode *>( index.internalPointer() ) )
      return node;
  }
  return mRootNode.get();
}

int QgsAttributesFormModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() && parent.column() > 0 )
    return 0;

  const QgsAttributesFormTreeNode *parentNode = nodeForIndex( parent );

  return parentNode ? parentNode->childCount() : 0;
}

int QgsAttributesFormModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 1;
}

QModelIndex QgsAttributesFormModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  QgsAttributesFormTreeNode *parentNode = nodeForIndex( parent );
  if ( !parentNode )
    return QModelIndex();

  if ( QgsAttributesFormTreeNode *childNode = parentNode->child( row ) )
    return createIndex( row, column, childNode );

  return QModelIndex();
}

QModelIndex QgsAttributesFormModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  QgsAttributesFormTreeNode *childNode = nodeForIndex( index );
  QgsAttributesFormTreeNode *parentNode = childNode ? childNode->parent() : nullptr;

  return ( parentNode != mRootNode.get() && parentNode != nullptr )
           ? createIndex( parentNode->row(), 0, parentNode )
           : QModelIndex();
}

QModelIndex QgsAttributesFormModel::firstTopMatchingModelIndex( const QgsAttributesFormTreeData::AttributesFormTreeNodeType &nodeType, const QString &nodeId ) const
{
  QgsAttributesFormTreeNode *node = mRootNode->firstTopChild( nodeType, nodeId );
  return node ? createIndex( node->row(), 0, node ) : QModelIndex();
}

QModelIndex QgsAttributesFormModel::firstRecursiveMatchingModelIndex( const QgsAttributesFormTreeData::AttributesFormTreeNodeType &nodeType, const QString &nodeId ) const
{
  QgsAttributesFormTreeNode *node = mRootNode->firstChildRecursive( nodeType, nodeId );
  return node ? createIndex( node->row(), 0, node ) : QModelIndex();
}


//
// QgsAttributesAvailableWidgetsModel
//

QgsAttributesAvailableWidgetsModel::QgsAttributesAvailableWidgetsModel( QgsVectorLayer *layer, QgsProject *project, QObject *parent )
  : QgsAttributesFormModel( layer, project, parent )
{
}

Qt::ItemFlags QgsAttributesAvailableWidgetsModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::NoItemFlags;

  Qt::ItemFlags flags = Qt::ItemIsEnabled;

  const auto indexType = static_cast< QgsAttributesFormTreeData::AttributesFormTreeNodeType >( index.data( QgsAttributesFormModel::NodeTypeRole ).toInt() );
  if ( indexType != QgsAttributesFormTreeData::WidgetType )
  {
    flags = flags | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
  }

  return flags;
}

QVariant QgsAttributesAvailableWidgetsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  Q_UNUSED( section )
  return orientation == Qt::Horizontal && role == Qt::DisplayRole ? tr( "Available Widgets" ) : QVariant {};
}

void QgsAttributesAvailableWidgetsModel::populate()
{
  if ( !mLayer )
    return;

  beginResetModel();
  mRootNode->deleteChildren();

  // load Fields

  auto itemFields = std::make_unique< QgsAttributesFormTreeNode >( QgsAttributesFormTreeData::WidgetType, QStringLiteral( "Fields" ), tr( "Fields" ) );

  const QgsFields fields = mLayer->fields();
  for ( int i = 0; i < fields.size(); ++i )
  {
    const QgsField field = fields.at( i );
    auto itemData = QgsAttributesFormTreeData::DnDTreeNodeData();
    itemData.setShowLabel( true );

    QgsAttributesFormTreeData::FieldConfig cfg( mLayer, i );

    auto item = std::make_unique< QgsAttributesFormTreeNode >();
    item->setData( NodeFieldConfigRole, cfg );
    item->setData( NodeNameRole, field.name() );
    item->setData( NodeIdRole, field.name() ); // Field names act as ids
    item->setData( NodeTypeRole, QgsAttributesFormTreeData::Field );
    item->setData( NodeDataRole, itemData );
    item->setIcon( fields.iconForField( i, true ) );

    itemFields->addChildNode( std::move( item ) );
  }

  mRootNode->addChildNode( std::move( itemFields ) );

  //load Relations

  auto itemRelations = std::make_unique< QgsAttributesFormTreeNode >( QgsAttributesFormTreeData::WidgetType, QStringLiteral( "Relations" ), tr( "Relations" ) );

  const QList<QgsRelation> relations = mProject->relationManager()->referencedRelations( mLayer );

  for ( const QgsRelation &relation : relations )
  {
    QString name;
    const QgsPolymorphicRelation polymorphicRelation = relation.polymorphicRelation();
    if ( polymorphicRelation.isValid() )
    {
      name = QStringLiteral( "%1 (%2)" ).arg( relation.name(), polymorphicRelation.name() );
    }
    else
    {
      name = relation.name();
    }
    QgsAttributesFormTreeData::DnDTreeNodeData itemData = QgsAttributesFormTreeData::DnDTreeNodeData();
    itemData.setShowLabel( true );

    auto itemRelation = std::make_unique< QgsAttributesFormTreeNode >();
    itemRelation->setData( NodeTypeRole, QgsAttributesFormTreeData::Relation );
    itemRelation->setData( NodeNameRole, name );
    itemRelation->setData( NodeIdRole, relation.id() );
    itemRelation->setData( NodeDataRole, itemData );
    itemRelations->addChildNode( std::move( itemRelation ) );
  }

  mRootNode->addChildNode( std::move( itemRelations ) );

  // Load form actions

  auto itemActions = std::make_unique< QgsAttributesFormTreeNode >( QgsAttributesFormTreeData::WidgetType, QStringLiteral( "Actions" ), tr( "Actions" ) );
  const QList<QgsAction> actions { mLayer->actions()->actions() };

  for ( const auto &action : std::as_const( actions ) )
  {
    if ( action.isValid() && action.runable() && ( action.actionScopes().contains( QStringLiteral( "Feature" ) ) || action.actionScopes().contains( QStringLiteral( "Layer" ) ) ) )
    {
      const QString actionTitle { action.shortTitle().isEmpty() ? action.name() : action.shortTitle() };

      QgsAttributesFormTreeData::DnDTreeNodeData itemData = QgsAttributesFormTreeData::DnDTreeNodeData();
      itemData.setShowLabel( true );

      auto itemAction = std::make_unique< QgsAttributesFormTreeNode >();
      itemAction->setData( NodeIdRole, action.id().toString() );
      itemAction->setData( NodeTypeRole, QgsAttributesFormTreeData::Action );
      itemAction->setData( NodeNameRole, actionTitle );
      itemAction->setData( NodeDataRole, itemData );

      itemActions->addChildNode( std::move( itemAction ) );
    }
  }

  mRootNode->addChildNode( std::move( itemActions ) );

  // QML/HTML widgets

  auto itemOtherWidgets = std::make_unique< QgsAttributesFormTreeNode >( QgsAttributesFormTreeData::WidgetType, QStringLiteral( "Other" ), tr( "Other Widgets" ) );

  QgsAttributesFormTreeData::DnDTreeNodeData itemData = QgsAttributesFormTreeData::DnDTreeNodeData();
  itemData.setShowLabel( true );
  auto itemQml = std::make_unique< QgsAttributesFormTreeNode >( QgsAttributesFormTreeData::QmlWidget, itemData, QStringLiteral( "QML Widget" ), tr( "QML Widget" ) );
  itemOtherWidgets->addChildNode( std::move( itemQml ) );

  QgsAttributesFormTreeData::DnDTreeNodeData itemHtmlData = QgsAttributesFormTreeData::DnDTreeNodeData();
  itemHtmlData.setShowLabel( true );
  auto itemHtml = std::make_unique< QgsAttributesFormTreeNode >( QgsAttributesFormTreeData::HtmlWidget, itemHtmlData, QStringLiteral( "HTML Widget" ), tr( "HTML Widget" ) );
  itemOtherWidgets->addChildNode( std::move( itemHtml ) );

  QgsAttributesFormTreeData::DnDTreeNodeData itemTextData = QgsAttributesFormTreeData::DnDTreeNodeData();
  itemTextData.setShowLabel( true );
  auto itemText = std::make_unique< QgsAttributesFormTreeNode >( QgsAttributesFormTreeData::TextWidget, itemTextData, QStringLiteral( "Text Widget" ), tr( "Text Widget" ) );
  itemOtherWidgets->addChildNode( std::move( itemText ) );

  QgsAttributesFormTreeData::DnDTreeNodeData itemSpacerData = QgsAttributesFormTreeData::DnDTreeNodeData();
  itemTextData.setShowLabel( false );
  auto itemSpacer = std::make_unique< QgsAttributesFormTreeNode >( QgsAttributesFormTreeData::SpacerWidget, QStringLiteral( "Spacer Widget" ), tr( "Spacer Widget" ) );
  itemOtherWidgets->addChildNode( std::move( itemSpacer ) );

  mRootNode->addChildNode( std::move( itemOtherWidgets ) );

  endResetModel();
}

QVariant QgsAttributesAvailableWidgetsModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const QgsAttributesFormTreeNode *node = nodeForIndex( index );
  if ( !node )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      return node->displayName().isEmpty() ? node->name() : node->displayName();
    }

    case Qt::ToolTipRole:
    {
      if ( node->type() == QgsAttributesFormTreeData::AttributesFormTreeNodeType::Field )
      {
        const auto cfg = node->data( NodeFieldConfigRole ).value<QgsAttributesFormTreeData::FieldConfig>();
        if ( !cfg.mAlias.isEmpty() )
          return tr( "%1 (%2)" ).arg( node->name(), cfg.mAlias );
        else
          return node->name();
      }
      return QVariant();
    }

    case Qt::DecorationRole:
      return node->icon();

    case Qt::BackgroundRole:
    {
      if ( node->type() == QgsAttributesFormTreeData::AttributesFormTreeNodeType::WidgetType )
        return QBrush( Qt::lightGray );

      return QVariant();
    }

    case NodeDataRole:
    case NodeFieldConfigRole:
    case NodeNameRole:
    case NodeTypeRole:
    case NodeIdRole:
      return node->data( role );

    default:
      return QVariant();
  }
}

bool QgsAttributesAvailableWidgetsModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  QgsAttributesFormTreeNode *node = nodeForIndex( index );
  bool result = node->setData( role, value );

  if ( result )
    emit dataChanged( index, index, { role } );

  return result;
}

Qt::DropActions QgsAttributesAvailableWidgetsModel::supportedDragActions() const
{
  return Qt::CopyAction;
}

QStringList QgsAttributesAvailableWidgetsModel::mimeTypes() const
{
  return QStringList() << QStringLiteral( "application/x-qgsattributesformavailablewidgetsrelement" );
}

QMimeData *QgsAttributesAvailableWidgetsModel::mimeData( const QModelIndexList &indexes ) const
{
  if ( indexes.count() == 0 )
    return nullptr;

  const QStringList types = mimeTypes();
  if ( types.isEmpty() )
    return nullptr;

  QMimeData *data = new QMimeData();
  const QString format = types.at( 0 );
  QByteArray encoded;
  QDataStream stream( &encoded, QIODevice::WriteOnly );

  // Sort indexes since their order reflects selection order
  QModelIndexList sortedIndexes = indexes;
  std::sort( sortedIndexes.begin(), sortedIndexes.end() );

  for ( const QModelIndex &index : std::as_const( sortedIndexes ) )
  {
    if ( index.isValid() )
    {
      const QString nodeId = index.data( QgsAttributesFormModel::NodeIdRole ).toString();
      const QString nodeName = index.data( QgsAttributesFormModel::NodeNameRole ).toString();
      int nodeType = index.data( QgsAttributesFormModel::NodeTypeRole ).toInt();

      stream << nodeId << nodeType << nodeName;
    }
  }

  data->setData( format, encoded );
  return data;
}

QModelIndex QgsAttributesAvailableWidgetsModel::fieldContainer() const
{
  if ( mRootNode->childCount() > 0 )
  {
    const int row = 0;
    QgsAttributesFormTreeNode *node = mRootNode->child( row );
    if ( node && node->name() == QLatin1String( "Fields" ) && node->type() == QgsAttributesFormTreeData::WidgetType )
      return createIndex( row, 0, node );
  }
  return QModelIndex();
}

QModelIndex QgsAttributesAvailableWidgetsModel::relationContainer() const
{
  if ( mRootNode->childCount() > 1 )
  {
    const int row = 1;
    QgsAttributesFormTreeNode *node = mRootNode->child( row );
    if ( node && node->name() == QLatin1String( "Relations" ) && node->type() == QgsAttributesFormTreeData::WidgetType )
      return createIndex( row, 0, node );
  }
  return QModelIndex();
}

QModelIndex QgsAttributesAvailableWidgetsModel::fieldModelIndex( const QString &fieldName ) const
{
  if ( mRootNode->childCount() == 0 )
    return QModelIndex();

  QgsAttributesFormTreeNode *fieldNodes = mRootNode->child( 0 );
  if ( !fieldNodes || fieldNodes->name() != QLatin1String( "Fields" ) || fieldNodes->type() != QgsAttributesFormTreeData::WidgetType )
    return QModelIndex();

  QgsAttributesFormTreeNode *node = fieldNodes->firstTopChild( QgsAttributesFormTreeData::Field, fieldName );
  return node ? createIndex( node->row(), 0, node ) : QModelIndex();
}


//
// QgsAttributesFormLayoutModel
//

QgsAttributesFormLayoutModel::QgsAttributesFormLayoutModel( QgsVectorLayer *layer, QgsProject *project, QObject *parent )
  : QgsAttributesFormModel( layer, project, parent )
{
}

QVariant QgsAttributesFormLayoutModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  Q_UNUSED( section )
  return orientation == Qt::Horizontal && role == Qt::DisplayRole ? tr( "Form Layout" ) : QVariant {};
}

Qt::ItemFlags QgsAttributesFormLayoutModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemIsDropEnabled;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;

  QgsAttributesFormTreeNode *node = nodeForIndex( index );
  if ( node->type() == QgsAttributesFormTreeData::WidgetType || node->type() == QgsAttributesFormTreeData::Container )
    flags |= Qt::ItemIsDropEnabled;

  return flags;
}

void QgsAttributesFormLayoutModel::populate()
{
  if ( !mLayer )
    return;

  beginResetModel();
  mRootNode->deleteChildren();

  const auto editorElements = mLayer->editFormConfig().tabs();
  for ( QgsAttributeEditorElement *editorElement : editorElements )
  {
    loadAttributeEditorElementItem( editorElement, mRootNode.get() );
  }

  endResetModel();
}

void QgsAttributesFormLayoutModel::loadAttributeEditorElementItem( QgsAttributeEditorElement *const editorElement, QgsAttributesFormTreeNode *parent, const int position )
{
  auto setCommonProperties = [editorElement]( QgsAttributesFormTreeData::DnDTreeNodeData &itemData ) {
    itemData.setShowLabel( editorElement->showLabel() );
    itemData.setLabelStyle( editorElement->labelStyle() );
    itemData.setHorizontalStretch( editorElement->horizontalStretch() );
    itemData.setVerticalStretch( editorElement->verticalStretch() );
  };

  auto editorItem = std::make_unique< QgsAttributesFormTreeNode >();

  switch ( editorElement->type() )
  {
    case Qgis::AttributeEditorType::Field:
    {
      QgsAttributesFormTreeData::DnDTreeNodeData itemData = QgsAttributesFormTreeData::DnDTreeNodeData();
      setCommonProperties( itemData );

      editorItem->setData( NodeNameRole, editorElement->name() );
      editorItem->setData( NodeIdRole, editorElement->name() ); // Field names act as ids
      editorItem->setData( NodeTypeRole, QgsAttributesFormTreeData::Field );
      editorItem->setData( NodeDataRole, itemData );

      const int fieldIndex = mLayer->fields().indexOf( editorElement->name() );
      if ( fieldIndex != -1 )
      {
        editorItem->setData( NodeDisplayRole, mLayer->fields().field( fieldIndex ).alias() );
      }

      break;
    }

    case Qgis::AttributeEditorType::Action:
    {
      const QgsAttributeEditorAction *actionEditor = static_cast<const QgsAttributeEditorAction *>( editorElement );
      const QgsAction action { actionEditor->action( mLayer ) };
      if ( action.isValid() )
      {
        QgsAttributesFormTreeData::DnDTreeNodeData itemData = QgsAttributesFormTreeData::DnDTreeNodeData();
        setCommonProperties( itemData );

        editorItem->setData( NodeIdRole, action.id().toString() );
        editorItem->setData( NodeNameRole, action.shortTitle().isEmpty() ? action.name() : action.shortTitle() );
        editorItem->setData( NodeTypeRole, QgsAttributesFormTreeData::Action );
        editorItem->setData( NodeDataRole, itemData );
      }
      else
      {
        QgsDebugError( QStringLiteral( "Invalid form action" ) );
      }
      break;
    }

    case Qgis::AttributeEditorType::Relation:
    {
      QgsAttributesFormTreeData::DnDTreeNodeData itemData = QgsAttributesFormTreeData::DnDTreeNodeData();
      setCommonProperties( itemData );

      const QgsAttributeEditorRelation *relationEditor = static_cast<const QgsAttributeEditorRelation *>( editorElement );
      QgsAttributesFormTreeData::RelationEditorConfiguration relationEditorConfig;
      relationEditorConfig.mRelationWidgetType = relationEditor->relationWidgetTypeId();
      relationEditorConfig.mRelationWidgetConfig = relationEditor->relationEditorConfiguration();
      relationEditorConfig.nmRelationId = relationEditor->nmRelationId();
      relationEditorConfig.forceSuppressFormPopup = relationEditor->forceSuppressFormPopup();
      relationEditorConfig.label = relationEditor->label();
      itemData.setRelationEditorConfiguration( relationEditorConfig );

      QgsRelation relation = relationEditor->relation();
      if ( !relation.isValid() )
      {
        relation = mProject->relationManager()->relation( editorElement->name() );
      }

      editorItem->setData( NodeIdRole, relation.id() );
      editorItem->setData( NodeNameRole, relation.name() );
      editorItem->setData( NodeTypeRole, QgsAttributesFormTreeData::Relation );
      editorItem->setData( NodeDataRole, itemData );

      break;
    }

    case Qgis::AttributeEditorType::Container:
    {
      QgsAttributesFormTreeData::DnDTreeNodeData itemData = QgsAttributesFormTreeData::DnDTreeNodeData();
      setCommonProperties( itemData );

      editorItem->setData( NodeNameRole, editorElement->name() );
      editorItem->setData( NodeTypeRole, QgsAttributesFormTreeData::Container );

      const QgsAttributeEditorContainer *container = static_cast<const QgsAttributeEditorContainer *>( editorElement );
      if ( !container )
        break;

      itemData.setColumnCount( container->columnCount() );
      itemData.setContainerType( container->type() );
      itemData.setBackgroundColor( container->backgroundColor() );
      itemData.setVisibilityExpression( container->visibilityExpression() );
      itemData.setCollapsedExpression( container->collapsedExpression() );
      itemData.setCollapsed( container->collapsed() );

      editorItem->setData( NodeDataRole, itemData );

      const QList<QgsAttributeEditorElement *> children = container->children();
      for ( QgsAttributeEditorElement *childElement : children )
      {
        loadAttributeEditorElementItem( childElement, editorItem.get() );
      }
      break;
    }

    case Qgis::AttributeEditorType::QmlElement:
    {
      const QgsAttributeEditorQmlElement *qmlElementEditor = static_cast<const QgsAttributeEditorQmlElement *>( editorElement );
      QgsAttributesFormTreeData::DnDTreeNodeData itemData = QgsAttributesFormTreeData::DnDTreeNodeData();
      setCommonProperties( itemData );

      QgsAttributesFormTreeData::QmlElementEditorConfiguration qmlEdConfig;
      qmlEdConfig.qmlCode = qmlElementEditor->qmlCode();
      itemData.setQmlElementEditorConfiguration( qmlEdConfig );

      editorItem->setData( NodeNameRole, editorElement->name() );
      editorItem->setData( NodeTypeRole, QgsAttributesFormTreeData::QmlWidget );
      editorItem->setData( NodeDataRole, itemData );
      break;
    }

    case Qgis::AttributeEditorType::HtmlElement:
    {
      const QgsAttributeEditorHtmlElement *htmlElementEditor = static_cast<const QgsAttributeEditorHtmlElement *>( editorElement );
      QgsAttributesFormTreeData::DnDTreeNodeData itemData = QgsAttributesFormTreeData::DnDTreeNodeData();
      setCommonProperties( itemData );

      QgsAttributesFormTreeData::HtmlElementEditorConfiguration htmlEdConfig;
      htmlEdConfig.htmlCode = htmlElementEditor->htmlCode();
      itemData.setHtmlElementEditorConfiguration( htmlEdConfig );

      editorItem->setData( NodeNameRole, editorElement->name() );
      editorItem->setData( NodeTypeRole, QgsAttributesFormTreeData::HtmlWidget );
      editorItem->setData( NodeDataRole, itemData );
      break;
    }

    case Qgis::AttributeEditorType::TextElement:
    {
      const QgsAttributeEditorTextElement *textElementEditor = static_cast<const QgsAttributeEditorTextElement *>( editorElement );
      QgsAttributesFormTreeData::DnDTreeNodeData itemData = QgsAttributesFormTreeData::DnDTreeNodeData();
      setCommonProperties( itemData );

      QgsAttributesFormTreeData::TextElementEditorConfiguration textEdConfig;
      textEdConfig.text = textElementEditor->text();
      itemData.setTextElementEditorConfiguration( textEdConfig );

      editorItem->setData( NodeNameRole, editorElement->name() );
      editorItem->setData( NodeTypeRole, QgsAttributesFormTreeData::TextWidget );
      editorItem->setData( NodeDataRole, itemData );
      break;
    }

    case Qgis::AttributeEditorType::SpacerElement:
    {
      const QgsAttributeEditorSpacerElement *spacerElementEditor = static_cast<const QgsAttributeEditorSpacerElement *>( editorElement );
      QgsAttributesFormTreeData::DnDTreeNodeData itemData = QgsAttributesFormTreeData::DnDTreeNodeData();
      setCommonProperties( itemData );
      itemData.setShowLabel( false );

      QgsAttributesFormTreeData::SpacerElementEditorConfiguration spacerEdConfig;
      spacerEdConfig.drawLine = spacerElementEditor->drawLine();
      itemData.setSpacerElementEditorConfiguration( spacerEdConfig );

      editorItem->setData( NodeNameRole, editorElement->name() );
      editorItem->setData( NodeTypeRole, QgsAttributesFormTreeData::SpacerWidget );
      editorItem->setData( NodeDataRole, itemData );
      break;
    }

    case Qgis::AttributeEditorType::Invalid:
    {
      QgsDebugError( QStringLiteral( "Not loading invalid attribute editor type..." ) );
      break;
    }
  }

  if ( position >= 0 && position < parent->childCount() )
  {
    parent->insertChildNode( position, std::move( editorItem ) );
  }
  else
  {
    parent->addChildNode( std::move( editorItem ) );
  }
}

QVariant QgsAttributesFormLayoutModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const QgsAttributesFormTreeNode *node = nodeForIndex( index );
  if ( !node )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      if ( !showAliases() && node->type() == QgsAttributesFormTreeData::Field )
      {
        return node->name();
      }
      else
      {
        return node->displayName().isEmpty() ? node->name() : node->displayName();
      }
    }

    case Qt::ToolTipRole:
    {
      if ( node->type() == QgsAttributesFormTreeData::Field )
      {
        return node->name();
      }
      return QVariant();
    }

    case Qt::DecorationRole:
      return node->icon();

    case Qt::BackgroundRole:
    {
      if ( node->type() == QgsAttributesFormTreeData::Container )
        return QBrush( Qt::lightGray );

      return QVariant();
    }

    case Qt::ForegroundRole:
      if ( showAliases() && node->type() == QgsAttributesFormTreeData::Field )
      {
        if ( node->displayName().isEmpty() )
        {
          return QBrush( QColor( Qt::lightGray ) );
        }
      }
      return QVariant();

    case Qt::FontRole:
      if ( showAliases() && node->type() == QgsAttributesFormTreeData::Field )
      {
        if ( node->displayName().isEmpty() )
        {
          QFont font = QFont();
          font.setItalic( true );
          return font;
        }
      }
      return QVariant();

    case NodeDataRole:
    case NodeFieldConfigRole:
    case NodeNameRole:
    case NodeIdRole:
    case NodeTypeRole:
    case NodeDisplayRole:
      return node->data( role );

    default:
      return QVariant();
  }
}

bool QgsAttributesFormLayoutModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  if ( role == NodeFieldConfigRole ) // This model doesn't store data for that role
    return false;

  QgsAttributesFormTreeNode *node = nodeForIndex( index );
  bool result = node->setData( role, value );

  if ( result )
    emit dataChanged( index, index, { role } );

  return result;
}

bool QgsAttributesFormLayoutModel::removeRows( int row, int count, const QModelIndex &parent )
{
  if ( row < 0 )
    return false;

  QgsAttributesFormTreeNode *node = nodeForIndex( parent );

  if ( row > node->childCount() - count )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );
  for ( int r = 0; r < count; ++r )
    node->deleteChildAtIndex( row );
  endRemoveRows();
  return true;
}

bool QgsAttributesFormLayoutModel::removeRow( int row, const QModelIndex &parent )
{
  beginRemoveRows( parent, row, row );
  QgsAttributesFormTreeNode *node = nodeForIndex( parent );
  node->deleteChildAtIndex( row );
  endRemoveRows();
  return true;
}

Qt::DropActions QgsAttributesFormLayoutModel::supportedDragActions() const
{
  return Qt::MoveAction;
}

Qt::DropActions QgsAttributesFormLayoutModel::supportedDropActions() const
{
  return Qt::DropAction::CopyAction | Qt::DropAction::MoveAction;
}

QStringList QgsAttributesFormLayoutModel::mimeTypes() const
{
  return QStringList() << QStringLiteral( "application/x-qgsattributesformlayoutelement" ) << QStringLiteral( "application/x-qgsattributesformavailablewidgetsrelement" );
}

QModelIndexList QgsAttributesFormLayoutModel::curateIndexesForMimeData( const QModelIndexList &indexes ) const
{
  QModelIndexList containerList;
  for ( const auto index : indexes )
  {
    const auto indexType = static_cast< QgsAttributesFormTreeData::AttributesFormTreeNodeType >( index.data( QgsAttributesFormModel::NodeTypeRole ).toInt() );
    if ( indexType == QgsAttributesFormTreeData::Container )
    {
      containerList << index;
    }
  }

  if ( containerList.size() == 0 )
    return indexes;

  QModelIndexList curatedIndexes;

  // Iterate searching if current index is child of any container in containerList (recursively)
  for ( const auto index : indexes )
  {
    QModelIndex parent = index.parent();
    bool redundantChild = false;

    while ( parent.isValid() )
    {
      if ( containerList.contains( parent ) )
      {
        redundantChild = true;
        break;
      }

      parent = parent.parent();
    }

    if ( !redundantChild )
      curatedIndexes << index;
  }

  return curatedIndexes;
}

QMimeData *QgsAttributesFormLayoutModel::mimeData( const QModelIndexList &indexes ) const
{
  if ( indexes.count() == 0 )
    return nullptr;

  // Discard redundant indexes
  QModelIndexList curatedIndexes;
  if ( indexes.count() > 1 )
  {
    curatedIndexes = curateIndexesForMimeData( indexes );
  }
  else
  {
    curatedIndexes = indexes;
  }

  const QStringList types = mimeTypes();
  if ( types.isEmpty() )
    return nullptr;

  QMimeData *data = new QMimeData();
  const QString format = types.at( 0 );
  QByteArray encoded;
  QDataStream stream( &encoded, QIODevice::WriteOnly );

  // Sort indexes since their order reflects selection order
  std::sort( curatedIndexes.begin(), curatedIndexes.end() );

  for ( const QModelIndex &index : std::as_const( curatedIndexes ) )
  {
    if ( index.isValid() )
    {
      QDomDocument doc;

      QDomElement rootElem = doc.createElement( QStringLiteral( "form_layout_mime" ) );
      QgsAttributeEditorElement *editor = createAttributeEditorWidget( index, nullptr );
      QDomElement editorElem = editor->toDomElement( doc );
      rootElem.appendChild( editorElem );

      doc.appendChild( rootElem );
      stream << doc.toString( -1 );
    }
  }

  data->setData( format, encoded );
  return data;
}

bool QgsAttributesFormLayoutModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  Q_UNUSED( column )
  bool bDropSuccessful = false;
  int rows = 0;

  if ( row == -1 )            // Dropped at invalid index
    row = rowCount( parent ); // Let's append the node

  if ( action == Qt::IgnoreAction )
  {
    bDropSuccessful = true;
  }
  else if ( data->hasFormat( QStringLiteral( "application/x-qgsattributesformavailablewidgetsrelement" ) ) )
  {
    Q_ASSERT( action == Qt::CopyAction ); // External drop
    QByteArray itemData = data->data( QStringLiteral( "application/x-qgsattributesformavailablewidgetsrelement" ) );
    QDataStream stream( &itemData, QIODevice::ReadOnly );

    while ( !stream.atEnd() )
    {
      QString nodeId;
      int nodeTypeInt;
      QString nodeName;
      stream >> nodeId >> nodeTypeInt >> nodeName;

      const auto nodeType = static_cast< QgsAttributesFormTreeData::AttributesFormTreeNodeType >( nodeTypeInt );
      insertChild( parent, row + rows, nodeId, nodeType, nodeName );

      bDropSuccessful = true;

      QModelIndex addedIndex = index( row + rows, 0, parent );
      emit externalNodeDropped( addedIndex );

      rows++;
    }
  }
  else if ( data->hasFormat( QStringLiteral( "application/x-qgsattributesformlayoutelement" ) ) )
  {
    Q_ASSERT( action == Qt::MoveAction ); // Internal move
    QByteArray itemData = data->data( QStringLiteral( "application/x-qgsattributesformlayoutelement" ) );
    QDataStream stream( &itemData, QIODevice::ReadOnly );

    while ( !stream.atEnd() )
    {
      QString text;
      stream >> text;

      QDomDocument doc;
      if ( !doc.setContent( text ) )
        continue;
      const QDomElement rootElem = doc.documentElement();
      if ( rootElem.tagName() != QLatin1String( "form_layout_mime" ) || !rootElem.hasChildNodes() )
        continue;
      const QDomElement childElem = rootElem.firstChild().toElement();

      // Build editor element from XML and add/insert it to parent
      QgsAttributeEditorElement *editor = QgsAttributeEditorElement::create( childElem, mLayer->id(), mLayer->fields(), QgsReadWriteContext() );
      beginInsertRows( parent, row + rows, row + rows );
      loadAttributeEditorElementItem( editor, nodeForIndex( parent ), row + rows );
      endInsertRows();

      bDropSuccessful = true;

      QModelIndex addedIndex = index( row + rows, 0, parent );
      emit internalNodeDropped( addedIndex );

      rows++;
    }
  }

  return bDropSuccessful;
}

void QgsAttributesFormLayoutModel::updateAliasForFieldNodesRecursive( QgsAttributesFormTreeNode *parent, const QString &fieldName, const QString &fieldAlias )
{
  for ( int i = 0; i < parent->childCount(); i++ )
  {
    QgsAttributesFormTreeNode *child = parent->child( i );
    if ( child->name() == fieldName && child->type() == QgsAttributesFormTreeData::Field )
    {
      child->setData( NodeDisplayRole, fieldAlias );
      const QModelIndex index = createIndex( child->row(), 0, child );
      emit dataChanged( index, index );
    }

    if ( child->childCount() > 0 )
    {
      updateAliasForFieldNodesRecursive( child, fieldName, fieldAlias );
    }
  }
}

void QgsAttributesFormLayoutModel::updateAliasForFieldNodes( const QString &fieldName, const QString &fieldAlias )
{
  updateAliasForFieldNodesRecursive( mRootNode.get(), fieldName, fieldAlias );
}

QList< QgsAddAttributeFormContainerDialog::ContainerPair > QgsAttributesFormLayoutModel::recursiveListOfContainers( QgsAttributesFormTreeNode *parent ) const
{
  QList< QgsAddAttributeFormContainerDialog::ContainerPair > containerList;
  for ( int i = 0; i < parent->childCount(); i++ )
  {
    QgsAttributesFormTreeNode *child = parent->child( i );
    if ( child->type() == QgsAttributesFormTreeData::Container )
    {
      containerList << QgsAddAttributeFormContainerDialog::ContainerPair( child->name(), createIndex( child->row(), 0, child ) );
    }

    if ( child->childCount() > 0 )
    {
      containerList.append( recursiveListOfContainers( child ) );
    }
  }

  return containerList;
}

QgsAttributeEditorElement *QgsAttributesFormLayoutModel::createAttributeEditorWidget( const QModelIndex &index, QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorElement *widgetDef = nullptr;

  const QgsAttributesFormTreeData::DnDTreeNodeData itemData = index.data( QgsAttributesFormModel::NodeDataRole ).value<QgsAttributesFormTreeData::DnDTreeNodeData>();
  const int indexType = static_cast< QgsAttributesFormTreeData::AttributesFormTreeNodeType >( index.data( QgsAttributesFormModel::NodeTypeRole ).toInt() );
  const QString indexName = index.data( QgsAttributesFormModel::NodeNameRole ).toString();
  const QString indexId = index.data( QgsAttributesFormModel::NodeIdRole ).toString();

  switch ( indexType )
  {
    case QgsAttributesFormTreeData::Field:
    {
      const int fieldIndex = mLayer->fields().lookupField( indexName );
      widgetDef = new QgsAttributeEditorField( indexName, fieldIndex, parent );
      break;
    }

    case QgsAttributesFormTreeData::Action:
    {
      const QgsAction action { mLayer->actions()->action( indexId ) };
      widgetDef = new QgsAttributeEditorAction( action, parent );
      break;
    }

    case QgsAttributesFormTreeData::Relation:
    {
      const QgsRelation relation = mProject->relationManager()->relation( indexId );

      QgsAttributeEditorRelation *relDef = new QgsAttributeEditorRelation( relation, parent );
      const QgsAttributesFormTreeData::RelationEditorConfiguration relationEditorConfig = itemData.relationEditorConfiguration();
      relDef->setRelationWidgetTypeId( relationEditorConfig.mRelationWidgetType );
      relDef->setRelationEditorConfiguration( relationEditorConfig.mRelationWidgetConfig );
      relDef->setNmRelationId( relationEditorConfig.nmRelationId );
      relDef->setForceSuppressFormPopup( relationEditorConfig.forceSuppressFormPopup );
      relDef->setLabel( relationEditorConfig.label );
      widgetDef = relDef;
      break;
    }

    case QgsAttributesFormTreeData::Container:
    {
      QgsAttributeEditorContainer *container = new QgsAttributeEditorContainer( indexName, parent, itemData.backgroundColor() );
      container->setColumnCount( itemData.columnCount() );
      // only top-level containers can be tabs
      Qgis::AttributeEditorContainerType type = itemData.containerType();
      bool isTopLevel = !index.parent().isValid();
      if ( type == Qgis::AttributeEditorContainerType::Tab && !isTopLevel )
      {
        // a tab container found which isn't at the top level -- reset it to a group box instead
        type = Qgis::AttributeEditorContainerType::GroupBox;
      }
      container->setType( type );
      container->setCollapsed( itemData.collapsed() );
      container->setCollapsedExpression( itemData.collapsedExpression() );
      container->setVisibilityExpression( itemData.visibilityExpression() );
      container->setBackgroundColor( itemData.backgroundColor() );

      QModelIndex childIndex;
      for ( int t = 0; t < rowCount( index ); t++ )
      {
        childIndex = this->index( t, 0, index );
        QgsAttributeEditorElement *element { createAttributeEditorWidget( childIndex, container ) };
        if ( element )
          container->addChildElement( element );
      }
      widgetDef = container;
      break;
    }

    case QgsAttributesFormTreeData::QmlWidget:
    {
      QgsAttributeEditorQmlElement *element = new QgsAttributeEditorQmlElement( indexName, parent );
      element->setQmlCode( itemData.qmlElementEditorConfiguration().qmlCode );
      widgetDef = element;
      break;
    }

    case QgsAttributesFormTreeData::HtmlWidget:
    {
      QgsAttributeEditorHtmlElement *element = new QgsAttributeEditorHtmlElement( indexName, parent );
      element->setHtmlCode( itemData.htmlElementEditorConfiguration().htmlCode );
      widgetDef = element;
      break;
    }

    case QgsAttributesFormTreeData::TextWidget:
    {
      QgsAttributeEditorTextElement *element = new QgsAttributeEditorTextElement( indexName, parent );
      element->setText( itemData.textElementEditorConfiguration().text );
      widgetDef = element;
      break;
    }

    case QgsAttributesFormTreeData::SpacerWidget:
    {
      QgsAttributeEditorSpacerElement *element = new QgsAttributeEditorSpacerElement( indexName, parent );
      element->setDrawLine( itemData.spacerElementEditorConfiguration().drawLine );
      widgetDef = element;
      break;
    }

    case QgsAttributesFormTreeData::WidgetType:
    default:
      break;
  }

  if ( widgetDef )
  {
    widgetDef->setShowLabel( itemData.showLabel() );
    widgetDef->setLabelStyle( itemData.labelStyle() );
    widgetDef->setHorizontalStretch( itemData.horizontalStretch() );
    widgetDef->setVerticalStretch( itemData.verticalStretch() );
  }

  return widgetDef;
}

QList< QgsAddAttributeFormContainerDialog::ContainerPair > QgsAttributesFormLayoutModel::listOfContainers() const
{
  return recursiveListOfContainers( mRootNode.get() );
}

void QgsAttributesFormLayoutModel::addContainer( QModelIndex &parent, const QString &title, int columnCount, Qgis::AttributeEditorContainerType type )
{
  beginInsertRows( parent, rowCount( parent ), rowCount( parent ) );

  QgsAttributesFormTreeNode *parentNode = nodeForIndex( parent );

  std::unique_ptr< QgsAttributesFormTreeNode > containerNode = std::make_unique< QgsAttributesFormTreeNode >( QgsAttributesFormTreeData::Container, title, QString(), parentNode );

  QgsAttributesFormTreeData::DnDTreeNodeData nodeData;
  nodeData.setColumnCount( columnCount );
  nodeData.setContainerType( parent.isValid() ? type : Qgis::AttributeEditorContainerType::Tab );

  containerNode->setData( QgsAttributesFormModel::NodeDataRole, nodeData );
  parentNode->addChildNode( std::move( containerNode ) );

  endInsertRows();
}

void QgsAttributesFormLayoutModel::insertChild( const QModelIndex &parent, int row, const QString &nodeId, QgsAttributesFormTreeData::AttributesFormTreeNodeType nodeType, const QString &nodeName )
{
  if ( row < 0 )
    return;

  beginInsertRows( parent, row, row );
  std::unique_ptr< QgsAttributesFormTreeNode > node = std::make_unique< QgsAttributesFormTreeNode >();

  node->setData( QgsAttributesFormModel::NodeIdRole, nodeId );
  node->setData( QgsAttributesFormModel::NodeTypeRole, nodeType );
  node->setData( QgsAttributesFormModel::NodeNameRole, nodeName );

  nodeForIndex( parent )->insertChildNode( row, std::move( node ) );
  endInsertRows();
}

bool QgsAttributesFormLayoutModel::showAliases() const
{
  return mShowAliases;
}

void QgsAttributesFormLayoutModel::setShowAliases( bool show )
{
  mShowAliases = show;
  emit dataChanged( QModelIndex(), QModelIndex(), QVector<int>() << Qt::DisplayRole << Qt::ForegroundRole << Qt::FontRole );
}
