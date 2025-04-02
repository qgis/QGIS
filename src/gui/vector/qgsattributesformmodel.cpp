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
#include "qgsgui.h"
#include "qgseditorwidgetregistry.h"
#include "qgsattributeeditoraction.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorrelation.h"
#include "qgsattributeeditorqmlelement.h"
#include "qgsattributeeditorhtmlelement.h"
#include "qgsattributeeditortextelement.h"
#include "qgsattributeeditorspacerelement.h"

#include "QMimeData"

/*
 * FieldConfig implementation
 */
QgsAttributeFormTreeData::FieldConfig::FieldConfig( QgsVectorLayer *layer, int idx )
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

QgsAttributeFormTreeData::FieldConfig::operator QVariant()
{
  return QVariant::fromValue<QgsAttributeFormTreeData::FieldConfig>( *this );
}

/*
 * RelationEditorConfiguration implementation
 */
QgsAttributeFormTreeData::RelationEditorConfiguration::operator QVariant()
{
  return QVariant::fromValue<QgsAttributeFormTreeData::RelationEditorConfiguration>( *this );
}

Qgis::AttributeEditorContainerType QgsAttributeFormTreeData::DnDTreeItemData::containerType() const
{
  return mContainerType;
}

void QgsAttributeFormTreeData::DnDTreeItemData::setContainerType( Qgis::AttributeEditorContainerType type )
{
  mContainerType = type;
}

const QgsAttributeEditorElement::LabelStyle QgsAttributeFormTreeData::DnDTreeItemData::labelStyle() const
{
  return mLabelStyle;
}

void QgsAttributeFormTreeData::DnDTreeItemData::setLabelStyle( const QgsAttributeEditorElement::LabelStyle &labelStyle )
{
  mLabelStyle = labelStyle;
}

bool QgsAttributeFormTreeData::DnDTreeItemData::showLabel() const
{
  return mShowLabel;
}

void QgsAttributeFormTreeData::DnDTreeItemData::setShowLabel( bool showLabel )
{
  mShowLabel = showLabel;
}

QgsOptionalExpression QgsAttributeFormTreeData::DnDTreeItemData::visibilityExpression() const
{
  return mVisibilityExpression;
}

void QgsAttributeFormTreeData::DnDTreeItemData::setVisibilityExpression( const QgsOptionalExpression &visibilityExpression )
{
  mVisibilityExpression = visibilityExpression;
}

QgsOptionalExpression QgsAttributeFormTreeData::DnDTreeItemData::collapsedExpression() const
{
  return mCollapsedExpression;
}

void QgsAttributeFormTreeData::DnDTreeItemData::setCollapsedExpression( const QgsOptionalExpression &collapsedExpression )
{
  mCollapsedExpression = collapsedExpression;
}

QgsAttributeFormTreeData::RelationEditorConfiguration QgsAttributeFormTreeData::DnDTreeItemData::relationEditorConfiguration() const
{
  return mRelationEditorConfiguration;
}

void QgsAttributeFormTreeData::DnDTreeItemData::setRelationEditorConfiguration( const QgsAttributeFormTreeData::RelationEditorConfiguration &relationEditorConfiguration )
{
  mRelationEditorConfiguration = relationEditorConfiguration;
}

QgsAttributeFormTreeData::QmlElementEditorConfiguration QgsAttributeFormTreeData::DnDTreeItemData::qmlElementEditorConfiguration() const
{
  return mQmlElementEditorConfiguration;
}

void QgsAttributeFormTreeData::DnDTreeItemData::setQmlElementEditorConfiguration( const QmlElementEditorConfiguration &qmlElementEditorConfiguration )
{
  mQmlElementEditorConfiguration = qmlElementEditorConfiguration;
}


QgsAttributeFormTreeData::HtmlElementEditorConfiguration QgsAttributeFormTreeData::DnDTreeItemData::htmlElementEditorConfiguration() const
{
  return mHtmlElementEditorConfiguration;
}

void QgsAttributeFormTreeData::DnDTreeItemData::setHtmlElementEditorConfiguration( const HtmlElementEditorConfiguration &htmlElementEditorConfiguration )
{
  mHtmlElementEditorConfiguration = htmlElementEditorConfiguration;
}

QgsAttributeFormTreeData::SpacerElementEditorConfiguration QgsAttributeFormTreeData::DnDTreeItemData::spacerElementEditorConfiguration() const
{
  return mSpacerElementEditorConfiguration;
}

void QgsAttributeFormTreeData::DnDTreeItemData::setSpacerElementEditorConfiguration( SpacerElementEditorConfiguration spacerElementEditorConfiguration )
{
  mSpacerElementEditorConfiguration = spacerElementEditorConfiguration;
}

QColor QgsAttributeFormTreeData::DnDTreeItemData::backgroundColor() const
{
  return mBackgroundColor;
}

void QgsAttributeFormTreeData::DnDTreeItemData::setBackgroundColor( const QColor &backgroundColor )
{
  mBackgroundColor = backgroundColor;
}

QgsAttributeFormTreeData::TextElementEditorConfiguration QgsAttributeFormTreeData::DnDTreeItemData::textElementEditorConfiguration() const
{
  return mTextElementEditorConfiguration;
}

void QgsAttributeFormTreeData::DnDTreeItemData::setTextElementEditorConfiguration( const QgsAttributeFormTreeData::TextElementEditorConfiguration &textElementEditorConfiguration )
{
  mTextElementEditorConfiguration = textElementEditorConfiguration;
}

//
// AttributesFormTreeNode implementation
//
AttributesFormTreeNode::AttributesFormTreeNode( QgsAttributeFormTreeData::AttributeFormTreeItemType itemType, const QString &name, const QString &displayName, AttributesFormTreeNode *parent )
  : mName( name )
  , mDisplayName( !displayName.isEmpty() ? displayName : name )
  , mNodeType( itemType )
  , mParent( parent )
{}

AttributesFormTreeNode::AttributesFormTreeNode( QgsAttributeFormTreeData::AttributeFormTreeItemType itemType, const QgsAttributeFormTreeData::DnDTreeItemData &data, const QString &name, const QString &displayName, AttributesFormTreeNode *parent )
  : mName( name )
  , mDisplayName( !displayName.isEmpty() ? displayName : name )
  , mNodeType( itemType )
  , mNodeData( data )
  , mParent( parent )
{}

AttributesFormTreeNode *AttributesFormTreeNode::child( int number )
{
  if ( !mChildren.empty() && number >= 0 && number < childCount() )
    return mChildren.at( number ).get();

  return nullptr;
}

AttributesFormTreeNode *AttributesFormTreeNode::firstTopChild( const QgsAttributeFormTreeData::AttributeFormTreeItemType nodeType, const QString &nodeId ) const
{
  if ( !mChildren.empty() && nodeId.trimmed().isEmpty() )
    return nullptr;

  // Search for first matching item by name
  const auto it = std::find_if( mChildren.cbegin(), mChildren.cend(), [nodeType, nodeId]( const std::unique_ptr< AttributesFormTreeNode > &treeNode ) {
    return treeNode->type() == nodeType && treeNode->name() == nodeId;
  } );

  if ( it != mChildren.cend() )
    return it->get();

  return nullptr;
}

AttributesFormTreeNode *AttributesFormTreeNode::firstChildRecursive( const QgsAttributeFormTreeData::AttributeFormTreeItemType &nodeType, const QString &nodeId ) const
{
  if ( !mChildren.empty() && nodeId.trimmed().isEmpty() )
    return nullptr;

  // for ( const auto &child : std::as_const( mChildren ) )
  // {
  //   if ( child->childCount() == 0 )
  //   {
  //     if ( child->type() == nodeType && child->id() == nodeId )
  //       return child.get();
  //   }
  //   else
  //   {
  //     AttributesFormTreeNode *node = child->firstChildRecursive( nodeType, nodeId );
  //     if ( node )
  //       return node;
  //   }
  // }

  for ( const auto &child : std::as_const( mChildren ) )
  {
    if ( child->type() == nodeType && child->id() == nodeId )
      return child.get();

    if ( child->childCount() > 0 )
    {
      AttributesFormTreeNode *node = child->firstChildRecursive( nodeType, nodeId );
      if ( node )
        return node;
    }
  }

  return nullptr;
}

int AttributesFormTreeNode::childCount() const
{
  return int( mChildren.size() );
}

int AttributesFormTreeNode::row() const
{
  if ( !mParent )
    return 0;

  const auto it = std::find_if( mParent->mChildren.cbegin(), mParent->mChildren.cend(), [this]( const std::unique_ptr< AttributesFormTreeNode > &treeItem ) {
    return treeItem.get() == this;
  } );

  if ( it != mParent->mChildren.cend() )
    return ( int ) std::distance( mParent->mChildren.cbegin(), it );
  return -1;
}

void AttributesFormTreeNode::addChildItem( std::unique_ptr< AttributesFormTreeNode > &&item )
{
  if ( !item )
    return;

  if ( !item->mParent )
    item->mParent = this;

  mChildren.push_back( std::move( item ) );
}

void AttributesFormTreeNode::insertChildNode( int position, std::unique_ptr< AttributesFormTreeNode > &&node )
{
  if ( position < 0 || position > ( int ) mChildren.size() || !node )
    return;

  if ( !node->mParent )
    node->mParent = this;

  mChildren.insert( mChildren.begin() + position, std::move( node ) );
}

void AttributesFormTreeNode::deleteChildAtIndex( int index )
{
  if ( index >= 0 && index < ( int ) mChildren.size() )
    mChildren.erase( mChildren.begin() + index );
}

void AttributesFormTreeNode::deleteChildren()
{
  mChildren.clear();
}

QVariant AttributesFormTreeNode::data( int role ) const
{
  switch ( role )
  {
    case QgsAttributesFormModel::NodeTypeRole:
      return mNodeType;
    case QgsAttributesFormModel::NodeDataRole:
      return QVariant::fromValue( mNodeData );
    case QgsAttributesFormModel::NodeFieldConfigRole:
      return QVariant::fromValue( mFieldConfigData );
    case QgsAttributesFormModel::NodeNameRole:
      return mName;
    case QgsAttributesFormModel::NodeIdRole:
      return mNodeId;
    default:
      return QVariant();
  }
  return QVariant();
}

bool AttributesFormTreeNode::setData( int role, const QVariant &value )
{
  switch ( role )
  {
    case QgsAttributesFormModel::NodeDataRole:
    {
      mNodeData = value.value< QgsAttributeFormTreeData::DnDTreeItemData >();
      return true;
    }
    case QgsAttributesFormModel::NodeFieldConfigRole:
    {
      mFieldConfigData = value.value< QgsAttributeFormTreeData::FieldConfig >();
      return true;
    }
    case QgsAttributesFormModel::NodeNameRole:
    {
      mName = value.toString();
      return true;
    }
    case QgsAttributesFormModel::NodeTypeRole:
    {
      mNodeType = static_cast<QgsAttributeFormTreeData::AttributeFormTreeItemType>( value.toInt() );
      return true;
    }
    case QgsAttributesFormModel::NodeIdRole:
    {
      mNodeId = value.toString();
      return true;
    }
    default:
      return false;
  }
  return false;
}

//
// QgsAttributesFormModel implementation
//


QgsAttributesFormModel::QgsAttributesFormModel( QgsVectorLayer *layer, QObject *parent )
  : QAbstractItemModel( parent )
  , mRootItem( std::make_unique< AttributesFormTreeNode >() )
  , mLayer( layer )
{
}

QgsAttributesFormModel::~QgsAttributesFormModel() = default;

AttributesFormTreeNode *QgsAttributesFormModel::getItem( const QModelIndex &index ) const
{
  if ( index.isValid() )
  {
    if ( auto *item = static_cast<AttributesFormTreeNode *>( index.internalPointer() ) )
      return item;
  }
  return mRootItem.get();
}

int QgsAttributesFormModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() && parent.column() > 0 )
    return 0;

  const AttributesFormTreeNode *parentItem = getItem( parent );

  return parentItem ? parentItem->childCount() : 0;
}

int QgsAttributesFormModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 1;
}

QStringList QgsAttributesFormModel::mimeTypes() const
{
  return QStringList() << QStringLiteral( "application/x-qgsattributetabledesignerelement" );
}

QMimeData *QgsAttributesFormModel::mimeData( const QModelIndexList &indexes ) const
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
      const auto nodeData = index.data( QgsAttributesFormModel::NodeDataRole );

      stream << nodeId << nodeType << nodeName << nodeData;
    }
  }

  data->setData( format, encoded );
  return data;
}

QModelIndex QgsAttributesFormModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  AttributesFormTreeNode *parentItem = getItem( parent );
  if ( !parentItem )
    return QModelIndex();

  if ( AttributesFormTreeNode *childItem = parentItem->child( row ) )
    return createIndex( row, column, childItem );

  return QModelIndex();
}

QModelIndex QgsAttributesFormModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  AttributesFormTreeNode *childItem = getItem( index );
  AttributesFormTreeNode *parentItem = childItem ? childItem->parent() : nullptr;

  return ( parentItem != mRootItem.get() && parentItem != nullptr )
           ? createIndex( parentItem->row(), 0, parentItem )
           : QModelIndex();
}

QModelIndex QgsAttributesFormModel::getFirstTopMatchingModelIndex( const QgsAttributeFormTreeData::AttributeFormTreeItemType &nodeType, const QString &nodeId ) const
{
  AttributesFormTreeNode *node = mRootItem->firstTopChild( nodeType, nodeId );
  return node ? createIndex( node->row(), 0, node ) : QModelIndex();
}

QModelIndex QgsAttributesFormModel::getFirstRecursiveMatchingModelIndex( const QgsAttributeFormTreeData::AttributeFormTreeItemType &nodeType, const QString &nodeId ) const
{
  AttributesFormTreeNode *node = mRootItem->firstChildRecursive( nodeType, nodeId );
  return node ? createIndex( node->row(), 0, node ) : QModelIndex();
}


//
// QgsAttributesAvailableWidgetsModel
//

QgsAttributesAvailableWidgetsModel::QgsAttributesAvailableWidgetsModel( QgsVectorLayer *layer, QgsProject *project, QObject *parent )
  : QgsAttributesFormModel( layer, parent )
  , mProject( project )
{
}

//QgsAttributesAvailableWidgetsModel::~QgsAttributesAvailableWidgetsModel() = default;

Qt::ItemFlags QgsAttributesAvailableWidgetsModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::NoItemFlags;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;

  AttributesFormTreeNode *node = getItem( index );
  if ( node->type() == QgsAttributeFormTreeData::WidgetType )
    flags |= Qt::ItemIsDropEnabled;

  return flags;
}

QVariant QgsAttributesAvailableWidgetsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  Q_UNUSED( section )
  return orientation == Qt::Horizontal && role == Qt::DisplayRole ? tr( "Available Widgets" ) : QVariant {};
}

void QgsAttributesAvailableWidgetsModel::populate()
{
  beginResetModel();
  mRootItem->deleteChildren();

  // load Fields

  auto itemFields = std::make_unique< AttributesFormTreeNode >( QgsAttributeFormTreeData::WidgetType, QStringLiteral( "Fields" ) );

  const QgsFields fields = mLayer->fields();
  for ( int i = 0; i < fields.size(); ++i )
  {
    const QgsField field = fields.at( i );
    auto itemData = QgsAttributeFormTreeData::DnDTreeItemData();
    itemData.setShowLabel( true );

    QgsAttributeFormTreeData::FieldConfig cfg( mLayer, i );

    auto item = std::make_unique< AttributesFormTreeNode >();
    item->setData( NodeFieldConfigRole, cfg );
    item->setData( NodeNameRole, field.name() );
    item->setData( NodeIdRole, field.name() ); // Field names act as ids
    item->setData( NodeTypeRole, QgsAttributeFormTreeData::Field );
    item->setData( NodeDataRole, itemData );
    item->setIcon( fields.iconForField( i, true ) );

    itemFields->addChildItem( std::move( item ) );
  }

  mRootItem->addChildItem( std::move( itemFields ) );

  //load Relations

  auto itemRelations = std::make_unique< AttributesFormTreeNode >( QgsAttributeFormTreeData::WidgetType, QStringLiteral( "Relations" ), tr( "Relations" ) );

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
    QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
    itemData.setShowLabel( true );

    auto itemRelation = std::make_unique< AttributesFormTreeNode >();
    itemRelation->setData( NodeTypeRole, QgsAttributeFormTreeData::Relation );
    itemRelation->setData( NodeNameRole, name );
    itemRelation->setData( NodeIdRole, relation.id() );
    itemRelation->setData( NodeDataRole, itemData );
    itemRelations->addChildItem( std::move( itemRelation ) );
  }

  mRootItem->addChildItem( std::move( itemRelations ) );

  // Load form actions

  auto itemActions = std::make_unique< AttributesFormTreeNode >( QgsAttributeFormTreeData::WidgetType, QStringLiteral( "Actions" ), tr( "Actions" ) );
  const QList<QgsAction> actions { mLayer->actions()->actions() };

  for ( const auto &action : std::as_const( actions ) )
  {
    if ( action.isValid() && action.runable() && ( action.actionScopes().contains( QStringLiteral( "Feature" ) ) || action.actionScopes().contains( QStringLiteral( "Layer" ) ) ) )
    {
      const QString actionTitle { action.shortTitle().isEmpty() ? action.name() : action.shortTitle() };

      QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
      itemData.setShowLabel( true );

      auto itemAction = std::make_unique< AttributesFormTreeNode >();
      itemAction->setData( NodeIdRole, action.id().toString() );
      itemAction->setData( NodeTypeRole, QgsAttributeFormTreeData::Action );
      itemAction->setData( NodeNameRole, actionTitle );
      itemAction->setData( NodeDataRole, itemData );

      itemActions->addChildItem( std::move( itemAction ) );
    }
  }

  mRootItem->addChildItem( std::move( itemActions ) );

  // QML/HTML widgets

  auto itemOtherWidgets = std::make_unique< AttributesFormTreeNode >( QgsAttributeFormTreeData::WidgetType, QStringLiteral( "Other" ), tr( "Other Widgets" ) );

  QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
  itemData.setShowLabel( true );
  auto itemQml = std::make_unique< AttributesFormTreeNode >( QgsAttributeFormTreeData::QmlWidget, itemData, QStringLiteral( "QML Widget" ), tr( "QML Widget" ) );
  itemOtherWidgets->addChildItem( std::move( itemQml ) );

  QgsAttributeFormTreeData::DnDTreeItemData itemHtmlData = QgsAttributeFormTreeData::DnDTreeItemData();
  itemHtmlData.setShowLabel( true );
  auto itemHtml = std::make_unique< AttributesFormTreeNode >( QgsAttributeFormTreeData::HtmlWidget, itemHtmlData, QStringLiteral( "HTML Widget" ), tr( "HTML Widget" ) );
  itemOtherWidgets->addChildItem( std::move( itemHtml ) );

  QgsAttributeFormTreeData::DnDTreeItemData itemTextData = QgsAttributeFormTreeData::DnDTreeItemData();
  itemTextData.setShowLabel( true );
  auto itemText = std::make_unique< AttributesFormTreeNode >( QgsAttributeFormTreeData::TextWidget, itemTextData, QStringLiteral( "Text Widget" ), tr( "Text Widget" ) );
  itemOtherWidgets->addChildItem( std::move( itemText ) );

  QgsAttributeFormTreeData::DnDTreeItemData itemSpacerData = QgsAttributeFormTreeData::DnDTreeItemData();
  itemTextData.setShowLabel( false );
  auto itemSpacer = std::make_unique< AttributesFormTreeNode >( QgsAttributeFormTreeData::SpacerWidget, QStringLiteral( "Spacer Widget" ), tr( "Spacer Widget" ) );
  itemOtherWidgets->addChildItem( std::move( itemSpacer ) );

  mRootItem->addChildItem( std::move( itemOtherWidgets ) );

  endResetModel();
}

QVariant QgsAttributesAvailableWidgetsModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const AttributesFormTreeNode *item = getItem( index );
  if ( !item )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      return item->displayName().isEmpty() ? item->name() : item->displayName();
    }

    case Qt::ToolTipRole:
    {
      if ( item->type() == QgsAttributeFormTreeData::AttributeFormTreeItemType::Field )
      {
        const auto cfg = item->data( NodeFieldConfigRole ).value<QgsAttributeFormTreeData::FieldConfig>();
        if ( !cfg.mAlias.isEmpty() )
          return tr( "%1 (%2)" ).arg( item->name(), cfg.mAlias );
        else
          return item->name();
      }
      return QVariant();
    }

    case Qt::DecorationRole:
      return item->icon();

    case Qt::BackgroundRole:
    {
      if ( item->type() == QgsAttributeFormTreeData::AttributeFormTreeItemType::WidgetType )
        return QBrush( Qt::lightGray );

      return QVariant();
    }

    case NodeDataRole:
    case NodeFieldConfigRole:
    case NodeNameRole:
    case NodeTypeRole:
    case NodeIdRole:
      return item->data( role );

    default:
      return QVariant();
  }
}

bool QgsAttributesAvailableWidgetsModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  AttributesFormTreeNode *item = getItem( index );
  bool result = item->setData( role, value );

  if ( result )
    emit dataChanged( index, index, { role } );

  return result;
}

QModelIndex QgsAttributesAvailableWidgetsModel::getFieldContainer() const
{
  if ( mRootItem->childCount() > 0 )
  {
    const int row = 0;
    AttributesFormTreeNode *node = mRootItem->child( row );
    if ( node && node->name() == QLatin1String( "Fields" ) )
      return createIndex( row, 0, node );
  }
  return QModelIndex();
}

QModelIndex QgsAttributesAvailableWidgetsModel::getRelationContainer() const
{
  if ( mRootItem->childCount() > 1 )
  {
    const int row = 1;
    AttributesFormTreeNode *node = mRootItem->child( row );
    if ( node && node->name() == QLatin1String( "Relations" ) )
      return createIndex( row, 0, node );
  }
  return QModelIndex();
}

QModelIndex QgsAttributesAvailableWidgetsModel::getFieldModelIndex( const QString &fieldName ) const
{
  if ( mRootItem->childCount() == 0 )
    return QModelIndex();

  AttributesFormTreeNode *fieldsItems = mRootItem->child( 0 );
  if ( !fieldsItems || fieldsItems->name() != QLatin1String( "Fields" ) )
    return QModelIndex();

  AttributesFormTreeNode *item = fieldsItems->firstTopChild( QgsAttributeFormTreeData::Field, fieldName );
  return item ? createIndex( item->row(), 0, item ) : QModelIndex();
}


//
// QgsAttributesFormLayoutModel
//

QgsAttributesFormLayoutModel::QgsAttributesFormLayoutModel( QgsVectorLayer *layer, QObject *parent )
  : QgsAttributesFormModel( layer, parent )
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
    return Qt::NoItemFlags;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;

  AttributesFormTreeNode *node = getItem( index );
  if ( node->type() == QgsAttributeFormTreeData::WidgetType || node->type() == QgsAttributeFormTreeData::Container )
    flags |= Qt::ItemIsDropEnabled;

  return flags;
}

void QgsAttributesFormLayoutModel::populate()
{
  beginResetModel();
  mRootItem->deleteChildren();

  const auto editorElements = mLayer->editFormConfig().tabs();
  for ( QgsAttributeEditorElement *editorElement : editorElements )
  {
    loadAttributeEditorElementItem( editorElement, mRootItem.get() );
  }

  endResetModel();
}

void QgsAttributesFormLayoutModel::loadAttributeEditorElementItem( QgsAttributeEditorElement *const editorElement, AttributesFormTreeNode *parent )
{
  auto setCommonProperties = [editorElement]( QgsAttributeFormTreeData::DnDTreeItemData &itemData ) {
    itemData.setShowLabel( editorElement->showLabel() );
    itemData.setLabelStyle( editorElement->labelStyle() );
    itemData.setHorizontalStretch( editorElement->horizontalStretch() );
    itemData.setVerticalStretch( editorElement->verticalStretch() );
  };

  auto editorItem = std::make_unique< AttributesFormTreeNode >();

  switch ( editorElement->type() )
  {
    case Qgis::AttributeEditorType::Field:
    {
      QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
      setCommonProperties( itemData );

      editorItem->setData( NodeNameRole, editorElement->name() );
      editorItem->setData( NodeIdRole, editorElement->name() ); // Field names act as ids
      editorItem->setData( NodeTypeRole, QgsAttributeFormTreeData::Field );
      editorItem->setData( NodeDataRole, itemData );

      break;
    }

    case Qgis::AttributeEditorType::Action:
    {
      const QgsAttributeEditorAction *actionEditor = static_cast<const QgsAttributeEditorAction *>( editorElement );
      const QgsAction action { actionEditor->action( mLayer ) };
      if ( action.isValid() )
      {
        QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
        setCommonProperties( itemData );

        editorItem->setData( NodeIdRole, action.id().toString() );
        editorItem->setData( NodeNameRole, action.shortTitle().isEmpty() ? action.name() : action.shortTitle() );
        editorItem->setData( NodeTypeRole, QgsAttributeFormTreeData::Action );
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
      QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
      setCommonProperties( itemData );

      const QgsAttributeEditorRelation *relationEditor = static_cast<const QgsAttributeEditorRelation *>( editorElement );
      QgsAttributeFormTreeData::RelationEditorConfiguration relationEditorConfig;
      relationEditorConfig.mRelationWidgetType = relationEditor->relationWidgetTypeId();
      relationEditorConfig.mRelationWidgetConfig = relationEditor->relationEditorConfiguration();
      relationEditorConfig.nmRelationId = relationEditor->nmRelationId();
      relationEditorConfig.forceSuppressFormPopup = relationEditor->forceSuppressFormPopup();
      relationEditorConfig.label = relationEditor->label();
      itemData.setRelationEditorConfiguration( relationEditorConfig );

      editorItem->setData( NodeIdRole, relationEditor->relation().id() );
      editorItem->setData( NodeNameRole, relationEditor->relation().name() );
      editorItem->setData( NodeTypeRole, QgsAttributeFormTreeData::Relation );
      editorItem->setData( NodeDataRole, itemData );

      break;
    }

    case Qgis::AttributeEditorType::Container:
    {
      QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
      setCommonProperties( itemData );

      editorItem->setData( NodeNameRole, editorElement->name() );
      editorItem->setData( NodeTypeRole, QgsAttributeFormTreeData::Container );

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
      QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
      setCommonProperties( itemData );

      QgsAttributeFormTreeData::QmlElementEditorConfiguration qmlEdConfig;
      qmlEdConfig.qmlCode = qmlElementEditor->qmlCode();
      itemData.setQmlElementEditorConfiguration( qmlEdConfig );

      editorItem->setData( NodeNameRole, editorElement->name() );
      editorItem->setData( NodeTypeRole, QgsAttributeFormTreeData::QmlWidget );
      editorItem->setData( NodeDataRole, itemData );
      break;
    }

    case Qgis::AttributeEditorType::HtmlElement:
    {
      const QgsAttributeEditorHtmlElement *htmlElementEditor = static_cast<const QgsAttributeEditorHtmlElement *>( editorElement );
      QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
      setCommonProperties( itemData );

      QgsAttributeFormTreeData::HtmlElementEditorConfiguration htmlEdConfig;
      htmlEdConfig.htmlCode = htmlElementEditor->htmlCode();
      itemData.setHtmlElementEditorConfiguration( htmlEdConfig );

      editorItem->setData( NodeNameRole, editorElement->name() );
      editorItem->setData( NodeTypeRole, QgsAttributeFormTreeData::HtmlWidget );
      editorItem->setData( NodeDataRole, itemData );
      break;
    }

    case Qgis::AttributeEditorType::TextElement:
    {
      const QgsAttributeEditorTextElement *textElementEditor = static_cast<const QgsAttributeEditorTextElement *>( editorElement );
      QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
      setCommonProperties( itemData );

      QgsAttributeFormTreeData::TextElementEditorConfiguration textEdConfig;
      textEdConfig.text = textElementEditor->text();
      itemData.setTextElementEditorConfiguration( textEdConfig );

      editorItem->setData( NodeNameRole, editorElement->name() );
      editorItem->setData( NodeTypeRole, QgsAttributeFormTreeData::TextWidget );
      editorItem->setData( NodeDataRole, itemData );
      break;
    }

    case Qgis::AttributeEditorType::SpacerElement:
    {
      const QgsAttributeEditorSpacerElement *spacerElementEditor = static_cast<const QgsAttributeEditorSpacerElement *>( editorElement );
      QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
      setCommonProperties( itemData );
      itemData.setShowLabel( false );

      QgsAttributeFormTreeData::SpacerElementEditorConfiguration spacerEdConfig;
      spacerEdConfig.drawLine = spacerElementEditor->drawLine();
      itemData.setSpacerElementEditorConfiguration( spacerEdConfig );

      editorItem->setData( NodeNameRole, editorElement->name() );
      editorItem->setData( NodeTypeRole, QgsAttributeFormTreeData::SpacerWidget );
      editorItem->setData( NodeDataRole, itemData );
      break;
    }

    case Qgis::AttributeEditorType::Invalid:
    {
      QgsDebugError( QStringLiteral( "Not loading invalid attribute editor type..." ) );
      break;
    }
  }

  parent->addChildItem( std::move( editorItem ) );
}

QVariant QgsAttributesFormLayoutModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const AttributesFormTreeNode *item = getItem( index );
  if ( !item )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      return item->displayName().isEmpty() ? item->name() : item->displayName();
    }

    case Qt::ToolTipRole:
    {
      if ( item->type() == QgsAttributeFormTreeData::Field )
      {
        // const auto cfg = item->data( FieldPropertiesRoles::FieldConfigRole ).value<QgsAttributeFormTreeData::FieldConfig>();
        // if ( !cfg.mAlias.isEmpty() )
        //   return tr( "%1 (%2)" ).arg( item->name(), cfg.mAlias );
        // else
        return item->name();
      }
      return QVariant();
    }

    case Qt::DecorationRole:
      return item->icon();

    case Qt::BackgroundRole:
    {
      if ( item->type() == QgsAttributeFormTreeData::Container )
        return QBrush( Qt::lightGray );

      return QVariant();
    }

    case NodeDataRole:
    case NodeFieldConfigRole:
    case NodeNameRole:
    case NodeIdRole:
    case NodeTypeRole:
      return item->data( role );

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

  AttributesFormTreeNode *item = getItem( index );
  bool result = item->setData( role, value );

  if ( result )
    emit dataChanged( index, index, { role } );

  return result;
}

bool QgsAttributesFormLayoutModel::removeRows( int row, int count, const QModelIndex &parent )
{
  if ( row < 0 )
    return false;

  AttributesFormTreeNode *node = getItem( parent );

  if ( row >= node->childCount() )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );

  // for (int i=row+count-1; i==row; i-- )
  // {
  //   node->deleteChildAtIndex( i );
  // }
  while ( count-- )
    node->deleteChildAtIndex( row );
  endRemoveRows();
  return true;
}

bool QgsAttributesFormLayoutModel::removeRow( int row, const QModelIndex &parent )
{
  beginRemoveRows( parent, row, row );
  AttributesFormTreeNode *node = getItem( parent );
  node->deleteChildAtIndex( row );
  endRemoveRows();
  return true;
}

Qt::DropActions QgsAttributesFormLayoutModel::supportedDropActions() const
{
  return Qt::DropAction::CopyAction | Qt::DropAction::MoveAction;
}

bool QgsAttributesFormLayoutModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  Q_UNUSED( column )
  bool bDropSuccessful = false;
  int rows = 0;

  if ( row == -1 )            // Dropped at invalid index
    row = rowCount( parent ); // Let's append the item

  if ( action == Qt::IgnoreAction )
  {
    bDropSuccessful = true;
  }
  else if ( data->hasFormat( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) ) )
  {
    QByteArray itemData = data->data( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) );
    QDataStream stream( &itemData, QIODevice::ReadOnly );
    QgsAttributeFormTreeData::DnDTreeItemData itemElement;

    while ( !stream.atEnd() )
    {
      QString nodeId;
      int nodeTypeInt;
      QString nodeName;
      QgsAttributeFormTreeData::DnDTreeItemData nodeData;
      stream >> nodeId >> nodeTypeInt >> nodeName >> nodeData;

      const auto nodeType = static_cast< QgsAttributeFormTreeData::AttributeFormTreeItemType >( nodeTypeInt );
      insertNode( parent, row + rows, nodeId, nodeType, nodeName, nodeData );

      bDropSuccessful = true;

      if ( nodeType == QgsAttributeFormTreeData::QmlWidget
           || nodeType == QgsAttributeFormTreeData::HtmlWidget
           || nodeType == QgsAttributeFormTreeData::TextWidget
           || nodeType == QgsAttributeFormTreeData::SpacerWidget )
      {
        // Emit signal to open their dialogs
      }

      //QModelIndex addedIndex = index( row + rows, 0, parent );
      rows++;
      //emit nodeDropped( addedIndex );
    }
  }

  return bDropSuccessful;
}


QList< QgsAddAttributeFormContainerDialog::ContainerPair > QgsAttributesFormLayoutModel::getRecursiveListOfContainers( AttributesFormTreeNode *parent ) const
{
  QList< QgsAddAttributeFormContainerDialog::ContainerPair > containerList;
  for ( int i = 0; i < parent->childCount(); i++ )
  {
    AttributesFormTreeNode *child = parent->child( i );
    if ( child->type() == QgsAttributeFormTreeData::Container )
    {
      containerList << QgsAddAttributeFormContainerDialog::ContainerPair( child->name(), createIndex( child->row(), 0, child ) );
    }

    if ( child->childCount() > 0 )
    {
      containerList.append( getRecursiveListOfContainers( child ) );
    }
  }

  return containerList;
}

QList< QgsAddAttributeFormContainerDialog::ContainerPair > QgsAttributesFormLayoutModel::getListOfContainers() const
{
  return getRecursiveListOfContainers( mRootItem.get() );
}

void QgsAttributesFormLayoutModel::addContainer( QModelIndex &parent, const QString &title, int columnCount, Qgis::AttributeEditorContainerType type )
{
  beginInsertRows( parent, rowCount( parent ), rowCount( parent ) );

  AttributesFormTreeNode *parentNode = getItem( parent );

  std::unique_ptr< AttributesFormTreeNode > containerNode = std::make_unique< AttributesFormTreeNode >( QgsAttributeFormTreeData::Container, title, QString(), parentNode );

  QgsAttributeFormTreeData::DnDTreeItemData nodeData;
  nodeData.setColumnCount( columnCount );
  nodeData.setContainerType( parent.isValid() ? type : Qgis::AttributeEditorContainerType::Tab );

  containerNode->setData( QgsAttributesFormModel::NodeDataRole, nodeData );
  parentNode->addChildItem( std::move( containerNode ) );

  endInsertRows();
}

void QgsAttributesFormLayoutModel::insertNode( const QModelIndex &parent, int row, QString &nodeId, QgsAttributeFormTreeData::AttributeFormTreeItemType nodeType, QString &nodeName, QgsAttributeFormTreeData::DnDTreeItemData nodeData )
{
  if ( row < 0 )
    return;

  beginInsertRows( parent, row, row );
  std::unique_ptr< AttributesFormTreeNode > node = std::make_unique< AttributesFormTreeNode >();

  node->setData( QgsAttributesFormModel::NodeIdRole, nodeId );
  node->setData( QgsAttributesFormModel::NodeTypeRole, nodeType );
  node->setData( QgsAttributesFormModel::NodeNameRole, nodeName );
  node->setData( QgsAttributesFormModel::NodeDataRole, nodeData );

  getItem( parent )->insertChildNode( row, std::move( node ) );
  endInsertRows();
}


/*
 * Serialization helpers for DesigerTreeItemData so we can stuff this easily into QMimeData
 */

QDataStream &operator<<( QDataStream &stream, const QgsAttributeFormTreeData::DnDTreeItemData &data )
{
  QVariant streamData = QVariant::fromValue<QgsAttributeFormTreeData::DnDTreeItemData>( data );
  stream << streamData;
  return stream;
}

QDataStream &operator>>( QDataStream &stream, QgsAttributeFormTreeData::DnDTreeItemData &data )
{
  QVariant streamData;
  stream >> streamData;
  data = streamData.value< QgsAttributeFormTreeData::DnDTreeItemData >();

  return stream;
}
