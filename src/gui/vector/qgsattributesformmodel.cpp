
#include "qgsactionmanager.h"
#include "qgsattributesformmodel.h"
#include "qgsgui.h"
#include "qgseditorwidgetregistry.h"
#include "qgsattributeeditoraction.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorrelation.h"


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
// AttributeFormTreeItem implementation
//

AttributeFormTreeItem::AttributeFormTreeItem( QgsAttributeFormTreeData::AttributeFormTreeItemType itemType, const QString &name, const QString &displayName, AttributeFormTreeItem *parent )
  : mName( name )
  , mDisplayName( !displayName.isEmpty() ? displayName : name )
  , mItemType( itemType )
  , mParent( parent )
{}

AttributeFormTreeItem::AttributeFormTreeItem( QgsAttributeFormTreeData::AttributeFormTreeItemType itemType, const QgsAttributeFormTreeData::DnDTreeItemData &data, const QString &name, const QString &displayName, AttributeFormTreeItem *parent )
  : mName( name )
  , mDisplayName( !displayName.isEmpty() ? displayName : name )
  , mItemType( itemType )
  , mItemData( data )
  , mParent( parent )
{}

AttributeFormTreeItem *AttributeFormTreeItem::child( int number )
{
  if ( !mChildren.empty() && number >= 0 && number < childCount() )
    return mChildren.at( number ).get();

  return nullptr;
}

AttributeFormTreeItem *AttributeFormTreeItem::firstChild( const QString &name ) const
{
  if ( !mChildren.empty() && name.trimmed().isEmpty() )
    return nullptr;

  // Search for first matching item by name
  const auto it = std::find_if( mChildren.cbegin(), mChildren.cend(), [name]( const std::unique_ptr< AttributeFormTreeItem > &treeItem ) {
    return treeItem->name() == name;
  } );

  if ( it != mChildren.cend() )
    return it->get();

  return nullptr;
}

int AttributeFormTreeItem::childCount() const
{
  return int( mChildren.size() );
}

int AttributeFormTreeItem::row() const
{
  if ( !mParent )
    return 0;

  const auto it = std::find_if( mParent->mChildren.cbegin(), mParent->mChildren.cend(), [this]( const std::unique_ptr< AttributeFormTreeItem > &treeItem ) {
    return treeItem.get() == this;
  } );

  if ( it != mParent->mChildren.cend() )
    return std::distance( mParent->mChildren.cbegin(), it );
  Q_ASSERT( false ); // should not happen
  return -1;
}

QVariant AttributeFormTreeItem::data( int role ) const
{
  switch ( role )
  {
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::TreeItemTypeRole:
      return mItemType;
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::DnDTreeRole:
      return QVariant::fromValue( mItemData );
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::FieldConfigRole:
      return QVariant::fromValue( mFieldConfigData );
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::TreeItemNameRole:
      return mName;
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::TreeItemIdRole:
      return mItemId;
    default:
      return QVariant();
  }
  return QVariant();
}

bool AttributeFormTreeItem::setData( int role, const QVariant &value )
{
  switch ( role )
  {
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::DnDTreeRole:
    {
      mItemData = value.value< QgsAttributeFormTreeData::DnDTreeItemData >();
      return true;
    }
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::FieldConfigRole:
    {
      mFieldConfigData = value.value< QgsAttributeFormTreeData::FieldConfig >();
      return true;
    }
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::TreeItemNameRole:
    {
      mName = value.toString();
      return true;
    }
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::TreeItemTypeRole:
    {
      mItemType = static_cast<QgsAttributeFormTreeData::AttributeFormTreeItemType>( value.toInt() );
      return true;
    }
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::TreeItemIdRole:
    {
      mItemId = value.toString();
      return true;
    }
    default:
      return false;
  }
  return false;
}

void AttributeFormTreeItem::addChildItem( std::unique_ptr< AttributeFormTreeItem > &&item )
{
  if ( !item )
    return;

  Q_ASSERT( !item->mParent );
  item->mParent = this;

  mChildren.push_back( std::move( item ) );
}

void AttributeFormTreeItem::deleteChildren()
{
  mChildren.clear();
}

//
// QgsAttributesAvailableWidgetsModel
//

QgsAttributesAvailableWidgetsModel::QgsAttributesAvailableWidgetsModel( QgsVectorLayer *layer, QgsProject *project, QObject *parent )
  : QAbstractItemModel( parent )
  , mLayer( layer )
  , mProject( project )
  , mRootItem( std::make_unique< AttributeFormTreeItem >() )
{
}

QgsAttributesAvailableWidgetsModel::~QgsAttributesAvailableWidgetsModel() = default;

QVariant QgsAttributesAvailableWidgetsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  Q_UNUSED( section )
  return orientation == Qt::Horizontal && role == Qt::DisplayRole ? tr( "Available Widgets" ) : QVariant {};
}

AttributeFormTreeItem *QgsAttributesAvailableWidgetsModel::getItem( const QModelIndex &index ) const
{
  if ( index.isValid() )
  {
    if ( auto *item = static_cast<AttributeFormTreeItem *>( index.internalPointer() ) )
      return item;
  }
  return mRootItem.get();
}

void QgsAttributesAvailableWidgetsModel::populate()
{
  beginResetModel();
  mRootItem->deleteChildren();

  // load Fields

  auto itemFields = std::make_unique< AttributeFormTreeItem >( QgsAttributeFormTreeData::AttributeFormTreeItemType::WidgetType, QStringLiteral( "Fields" ) );

  const QgsFields fields = mLayer->fields();
  for ( int i = 0; i < fields.size(); ++i )
  {
    const QgsField field = fields.at( i );
    auto itemData = QgsAttributeFormTreeData::DnDTreeItemData();
    itemData.setShowLabel( true );

    QgsAttributeFormTreeData::FieldConfig cfg( mLayer, i );

    auto item = std::make_unique< AttributeFormTreeItem >();
    item->setData( FieldConfigRole, cfg );
    item->setData( TreeItemNameRole, field.name() );
    item->setData( TreeItemTypeRole, QgsAttributeFormTreeData::Field );
    item->setData( DnDTreeRole, itemData );
    item->setIcon( fields.iconForField( i, true ) );

    itemFields->addChildItem( std::move( item ) );
  }

  mRootItem->addChildItem( std::move( itemFields ) );

  //load Relations

  auto itemRelations = std::make_unique< AttributeFormTreeItem >( QgsAttributeFormTreeData::AttributeFormTreeItemType::WidgetType, QStringLiteral( "Relations" ), tr( "Relations" ) );

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

    auto itemRelation = std::make_unique< AttributeFormTreeItem >();
    itemRelation->setData( TreeItemTypeRole, QgsAttributeFormTreeData::Relation );
    itemRelation->setData( TreeItemNameRole, name );
    itemRelation->setData( TreeItemIdRole, relation.id() );
    itemRelation->setData( DnDTreeRole, itemData );
    itemRelations->addChildItem( std::move( itemRelation ) );
  }

  mRootItem->addChildItem( std::move( itemRelations ) );

  // Load form actions

  auto itemActions = std::make_unique< AttributeFormTreeItem >( QgsAttributeFormTreeData::AttributeFormTreeItemType::WidgetType, QStringLiteral( "Actions" ), tr( "Actions" ) );
  const QList<QgsAction> actions { mLayer->actions()->actions() };

  for ( const auto &action : std::as_const( actions ) )
  {
    if ( action.isValid() && action.runable() && ( action.actionScopes().contains( QStringLiteral( "Feature" ) ) || action.actionScopes().contains( QStringLiteral( "Layer" ) ) ) )
    {
      const QString actionTitle { action.shortTitle().isEmpty() ? action.name() : action.shortTitle() };

      QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
      itemData.setShowLabel( true );

      auto itemAction = std::make_unique< AttributeFormTreeItem >();
      itemAction->setData( TreeItemIdRole, action.id().toString() );
      itemAction->setData( TreeItemTypeRole, QgsAttributeFormTreeData::Action );
      itemAction->setData( TreeItemNameRole, actionTitle );
      itemAction->setData( DnDTreeRole, itemData );

      itemActions->addChildItem( std::move( itemAction ) );
    }
  }

  mRootItem->addChildItem( std::move( itemActions ) );

  // QML/HTML widgets

  auto itemOtherWidgets = std::make_unique< AttributeFormTreeItem >( QgsAttributeFormTreeData::AttributeFormTreeItemType::WidgetType, QStringLiteral( "Other" ), tr( "Other Widgets" ) );

  auto itemData = QgsAttributeFormTreeData::DnDTreeItemData();
  itemData.setShowLabel( true );
  auto itemQml = std::make_unique< AttributeFormTreeItem >( QgsAttributeFormTreeData::AttributeFormTreeItemType::QmlWidget, itemData, QStringLiteral( "QML Widget" ), tr( "QML Widget" ) );
  itemOtherWidgets->addChildItem( std::move( itemQml ) );

  auto itemHtmlData = QgsAttributeFormTreeData::DnDTreeItemData();
  itemHtmlData.setShowLabel( true );
  auto itemHtml = std::make_unique< AttributeFormTreeItem >( QgsAttributeFormTreeData::AttributeFormTreeItemType::HtmlWidget, itemHtmlData, QStringLiteral( "HTML Widget" ), tr( "HTML Widget" ) );
  itemOtherWidgets->addChildItem( std::move( itemHtml ) );

  // auto itemDataText { DnDTreeItemData( DnDTreeItemData::TextWidget, QStringLiteral( "Text Widget" ), tr( "Text Widget" ) ) };
  // itemDataText.setShowLabel( true );
  // mAvailableWidgetsTree->addItem( catitem, itemDataText );

  // auto itemDataSpacer { DnDTreeItemData( DnDTreeItemData::SpacerWidget, QStringLiteral( "Spacer Widget" ), tr( "Spacer Widget" ) ) };
  // itemDataSpacer.setShowLabel( false );
  // mAvailableWidgetsTree->addItem( catitem, itemDataSpacer );

  mRootItem->addChildItem( std::move( itemOtherWidgets ) );

  endResetModel();
}

int QgsAttributesAvailableWidgetsModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() && parent.column() > 0 )
    return 0;

  const AttributeFormTreeItem *parentItem = getItem( parent );

  return parentItem ? parentItem->childCount() : 0;
}

int QgsAttributesAvailableWidgetsModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 1;
}

QModelIndex QgsAttributesAvailableWidgetsModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  AttributeFormTreeItem *parentItem = getItem( parent );
  if ( !parentItem )
    return QModelIndex();

  if ( AttributeFormTreeItem *childItem = parentItem->child( row ) )
    return createIndex( row, column, childItem );

  return QModelIndex();
}

QModelIndex QgsAttributesAvailableWidgetsModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  AttributeFormTreeItem *childItem = getItem( index );
  AttributeFormTreeItem *parentItem = childItem ? childItem->parent() : nullptr;

  return ( parentItem != mRootItem.get() && parentItem != nullptr )
           ? createIndex( parentItem->row(), 0, parentItem )
           : QModelIndex {};
}

QVariant QgsAttributesAvailableWidgetsModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  // if ( role != Qt::DisplayRole && role != Qt::EditRole )
  //   return QVariant();

  const AttributeFormTreeItem *item = getItem( index );
  if ( !item )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      return item->displayName().isEmpty() ? ( item->name().isEmpty() ? item->id() : item->name() ) : item->displayName();
    }

    case Qt::ToolTipRole:
    {
      if ( item->type() == QgsAttributeFormTreeData::AttributeFormTreeItemType::Field )
      {
        const auto cfg = item->data( FieldPropertiesRoles::FieldConfigRole ).value<QgsAttributeFormTreeData::FieldConfig>();
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

    case DnDTreeRole:
    case FieldConfigRole:
    case TreeItemNameRole:
    case TreeItemTypeRole:
    case TreeItemIdRole:
      return item->data( role );

    default:
      return QVariant();
  }
}

bool QgsAttributesAvailableWidgetsModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( role != QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::DnDTreeRole
       && role != QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::FieldConfigRole
       && role != QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::TreeItemIdRole
       && role != QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::TreeItemNameRole )
    return false;

  AttributeFormTreeItem *item = getItem( index );
  bool result = item->setData( role, value );

  if ( result )
    emit dataChanged( index, index, { role } );

  return result;
}

QModelIndex QgsAttributesAvailableWidgetsModel::getFieldModelIndex( const QString &fieldName ) const
{
  AttributeFormTreeItem *fieldsItems = mRootItem->child( 0 );
  if ( !fieldsItems || fieldsItems->name() != QLatin1String( "Fields" ) )
    return QModelIndex();

  AttributeFormTreeItem *item = fieldsItems->firstChild( fieldName );
  return item ? createIndex( item->row(), 0, item ) : QModelIndex();
}

//
// QgsAttributesFormLayoutModel
//

// QgsAttributesFormLayoutModel::QgsAttributesFormLayoutModel( QObject *parent )
//   : QAbstractItemModel( parent )
// {
// }

// QVariant QgsAttributesFormLayoutModel::headerData( int section, Qt::Orientation orientation, int role ) const
// {
//   // FIXME: Implement me!
// }

// QModelIndex QgsAttributesFormLayoutModel::index( int row, int column, const QModelIndex &parent ) const
// {
//   // FIXME: Implement me!
// }

// QModelIndex QgsAttributesFormLayoutModel::parent( const QModelIndex &index ) const
// {
//   // FIXME: Implement me!
// }

// int QgsAttributesFormLayoutModel::rowCount( const QModelIndex &parent ) const
// {
//   if ( !parent.isValid() )
//     return 0;

//   // FIXME: Implement me!
// }

// int QgsAttributesFormLayoutModel::columnCount( const QModelIndex &parent ) const
// {
//   if ( !parent.isValid() )
//     return 0;

//   // FIXME: Implement me!
// }

// QVariant QgsAttributesFormLayoutModel::data( const QModelIndex &index, int role ) const
// {
//   if ( !index.isValid() )
//     return QVariant();

//   // FIXME: Implement me!
//   return QVariant();
// }

// bool QgsAttributesFormLayoutModel::insertRows( int row, int count, const QModelIndex &parent )
// {
//   beginInsertRows( parent, row, row + count - 1 );
//   // FIXME: Implement me!
//   endInsertRows();
//   return true;
// }

// bool QgsAttributesFormLayoutModel::insertColumns( int column, int count, const QModelIndex &parent )
// {
//   beginInsertColumns( parent, column, column + count - 1 );
//   // FIXME: Implement me!
//   endInsertColumns();
//   return true;
// }

// bool QgsAttributesFormLayoutModel::removeRows( int row, int count, const QModelIndex &parent )
// {
//   beginRemoveRows( parent, row, row + count - 1 );
//   // FIXME: Implement me!
//   endRemoveRows();
//   return true;
// }

// bool QgsAttributesFormLayoutModel::removeColumns( int column, int count, const QModelIndex &parent )
// {
//   beginRemoveColumns( parent, column, column + count - 1 );
//   // FIXME: Implement me!
//   endRemoveColumns();
//   return true;
// }

//
// AttributeFormLayoutTreeItem implementation
//

AttributeFormLayoutTreeItem::AttributeFormLayoutTreeItem( QgsAttributeFormTreeData::AttributeFormTreeItemType itemType, const QString &name, const QString &displayName, AttributeFormLayoutTreeItem *parent )
  : mName( name )
  , mDisplayName( !displayName.isEmpty() ? displayName : name )
  , mItemType( itemType )
  , mParent( parent )
{}

AttributeFormLayoutTreeItem::AttributeFormLayoutTreeItem( QgsAttributeFormTreeData::AttributeFormTreeItemType itemType, const QgsAttributeFormTreeData::DnDTreeItemData &data, const QString &name, const QString &displayName, AttributeFormLayoutTreeItem *parent )
  : mName( name )
  , mDisplayName( !displayName.isEmpty() ? displayName : name )
  , mItemType( itemType )
  , mItemData( data )
  , mParent( parent )
{}

AttributeFormLayoutTreeItem *AttributeFormLayoutTreeItem::child( int number )
{
  if ( !mChildren.empty() && number >= 0 && number < childCount() )
    return mChildren.at( number ).get();

  return nullptr;
}

AttributeFormLayoutTreeItem *AttributeFormLayoutTreeItem::firstChild( const QString &name ) const
{
  if ( !mChildren.empty() && name.trimmed().isEmpty() )
    return nullptr;

  // Search for first matching item by name
  const auto it = std::find_if( mChildren.cbegin(), mChildren.cend(), [name]( const std::unique_ptr< AttributeFormLayoutTreeItem > &treeItem ) {
    return treeItem->name() == name;
  } );

  if ( it != mChildren.cend() )
    return it->get();

  return nullptr;
}

int AttributeFormLayoutTreeItem::childCount() const
{
  return int( mChildren.size() );
}

int AttributeFormLayoutTreeItem::row() const
{
  if ( !mParent )
    return 0;

  const auto it = std::find_if( mParent->mChildren.cbegin(), mParent->mChildren.cend(), [this]( const std::unique_ptr< AttributeFormLayoutTreeItem > &treeItem ) {
    return treeItem.get() == this;
  } );

  if ( it != mParent->mChildren.cend() )
    return std::distance( mParent->mChildren.cbegin(), it );
  Q_ASSERT( false ); // should not happen
  return -1;
}

QVariant AttributeFormLayoutTreeItem::data( int role ) const
{
  switch ( role )
  {
    case QgsAttributesFormLayoutModel::FieldPropertiesRoles::TreeItemTypeRole:
      return mItemType;
    case QgsAttributesFormLayoutModel::FieldPropertiesRoles::DnDTreeRole:
      return QVariant::fromValue( mItemData );
    case QgsAttributesFormLayoutModel::FieldPropertiesRoles::TreeItemNameRole:
      return mName;
    case QgsAttributesFormLayoutModel::FieldPropertiesRoles::TreeItemIdRole:
      return mItemId;
    default:
      return mItemType;
  }
  return QVariant();
}

bool AttributeFormLayoutTreeItem::setData( int role, const QVariant &value )
{
  switch ( role )
  {
    case QgsAttributesFormLayoutModel::FieldPropertiesRoles::DnDTreeRole:
    {
      mItemData = value.value< QgsAttributeFormTreeData::DnDTreeItemData >();
      return true;
    }
    case QgsAttributesFormLayoutModel::FieldPropertiesRoles::TreeItemNameRole:
    {
      mName = value.toString();
      return true;
    }
    case QgsAttributesFormLayoutModel::FieldPropertiesRoles::TreeItemIdRole:
    {
      mItemId = value.toString();
      return true;
    }
    case QgsAttributesFormLayoutModel::FieldPropertiesRoles::TreeItemTypeRole:
    {
      mItemType = static_cast< QgsAttributeFormTreeData::AttributeFormTreeItemType >( value.toInt() );
      return true;
    }
    default:
      return false;
  }
  return false;
}

void AttributeFormLayoutTreeItem::addChildItem( std::unique_ptr< AttributeFormLayoutTreeItem > &&item )
{
  if ( !item )
    return;

  Q_ASSERT( !item->mParent );
  item->mParent = this;

  mChildren.push_back( std::move( item ) );
}

void AttributeFormLayoutTreeItem::deleteChildren()
{
  mChildren.clear();
}

//
// QgsAttributesFormLayoutModel
//

QgsAttributesFormLayoutModel::QgsAttributesFormLayoutModel( QgsVectorLayer *layer, QObject *parent )
  : QAbstractItemModel( parent )
  , mLayer( layer )
  , mRootItem( std::make_unique< AttributeFormLayoutTreeItem >() )
{
  populate();
}

QgsAttributesFormLayoutModel::~QgsAttributesFormLayoutModel() = default;

QVariant QgsAttributesFormLayoutModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  Q_UNUSED( section )
  return orientation == Qt::Horizontal && role == Qt::DisplayRole ? tr( "Form Layout" ) : QVariant {};
}

AttributeFormLayoutTreeItem *QgsAttributesFormLayoutModel::getItem( const QModelIndex &index ) const
{
  if ( index.isValid() )
  {
    if ( auto *item = static_cast<AttributeFormLayoutTreeItem *>( index.internalPointer() ) )
      return item;
  }
  return mRootItem.get();
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

void QgsAttributesFormLayoutModel::loadAttributeEditorElementItem( QgsAttributeEditorElement *const editorElement, AttributeFormLayoutTreeItem *parent )
{
  auto setCommonProperties = [editorElement]( QgsAttributeFormTreeData::DnDTreeItemData &itemData ) {
    itemData.setShowLabel( editorElement->showLabel() );
    itemData.setLabelStyle( editorElement->labelStyle() );
    itemData.setHorizontalStretch( editorElement->horizontalStretch() );
    itemData.setVerticalStretch( editorElement->verticalStretch() );
  };

  auto editorItem = std::make_unique< AttributeFormLayoutTreeItem >();

  switch ( editorElement->type() )
  {
    case Qgis::AttributeEditorType::Field:
    {
      QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
      setCommonProperties( itemData );

      editorItem->setData( TreeItemNameRole, editorElement->name() );
      editorItem->setData( TreeItemTypeRole, QgsAttributeFormTreeData::Field );
      editorItem->setData( DnDTreeRole, itemData );

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

        editorItem->setData( TreeItemIdRole, action.id().toString() );
        editorItem->setData( TreeItemNameRole, action.shortTitle().isEmpty() ? action.name() : action.shortTitle() );
        editorItem->setData( TreeItemTypeRole, QgsAttributeFormTreeData::Action );
        editorItem->setData( DnDTreeRole, itemData );
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

      editorItem->setData( TreeItemIdRole, relationEditor->relation().id() );
      editorItem->setData( TreeItemNameRole, relationEditor->relation().name() );
      editorItem->setData( TreeItemTypeRole, QgsAttributeFormTreeData::Relation );
      editorItem->setData( DnDTreeRole, itemData );

      break;
    }

    case Qgis::AttributeEditorType::Container:
    {
      QgsAttributeFormTreeData::DnDTreeItemData itemData = QgsAttributeFormTreeData::DnDTreeItemData();
      setCommonProperties( itemData );

      editorItem->setData( TreeItemNameRole, editorElement->name() );
      editorItem->setData( TreeItemTypeRole, QgsAttributeFormTreeData::Container );

      const QgsAttributeEditorContainer *container = static_cast<const QgsAttributeEditorContainer *>( editorElement );
      if ( !container )
        break;

      itemData.setColumnCount( container->columnCount() );
      itemData.setContainerType( container->type() );
      itemData.setBackgroundColor( container->backgroundColor() );
      itemData.setVisibilityExpression( container->visibilityExpression() );
      itemData.setCollapsedExpression( container->collapsedExpression() );
      itemData.setCollapsed( container->collapsed() );

      editorItem->setData( DnDTreeRole, itemData );

      const QList<QgsAttributeEditorElement *> children = container->children();
      for ( QgsAttributeEditorElement *childElement : children )
      {
        loadAttributeEditorElementItem( childElement, editorItem.get() );
      }
      break;
    }

      // case Qgis::AttributeEditorType::QmlElement:
      // {
      //   const QgsAttributeEditorQmlElement *qmlElementEditor = static_cast<const QgsAttributeEditorQmlElement *>( editorElement );
      //   DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::QmlWidget, editorElement->name(), editorElement->name() );
      //   QmlElementEditorConfiguration qmlEdConfig;
      //   qmlEdConfig.qmlCode = qmlElementEditor->qmlCode();
      //   itemData.setQmlElementEditorConfiguration( qmlEdConfig );
      //   setCommonProperties( itemData );
      //   newWidget = tree->addItem( parent, itemData );
      //   break;
      // }

      // case Qgis::AttributeEditorType::HtmlElement:
      // {
      //   const QgsAttributeEditorHtmlElement *htmlElementEditor = static_cast<const QgsAttributeEditorHtmlElement *>( editorElement );
      //   DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::HtmlWidget, editorElement->name(), editorElement->name() );
      //   HtmlElementEditorConfiguration htmlEdConfig;
      //   htmlEdConfig.htmlCode = htmlElementEditor->htmlCode();
      //   itemData.setHtmlElementEditorConfiguration( htmlEdConfig );
      //   setCommonProperties( itemData );
      //   newWidget = tree->addItem( parent, itemData );
      //   break;
      // }

      // case Qgis::AttributeEditorType::TextElement:
      // {
      //   const QgsAttributeEditorTextElement *textElementEditor = static_cast<const QgsAttributeEditorTextElement *>( editorElement );
      //   DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::TextWidget, editorElement->name(), editorElement->name() );
      //   TextElementEditorConfiguration textEdConfig;
      //   textEdConfig.text = textElementEditor->text();
      //   itemData.setTextElementEditorConfiguration( textEdConfig );
      //   setCommonProperties( itemData );
      //   newWidget = tree->addItem( parent, itemData );
      //   break;
      // }

      // case Qgis::AttributeEditorType::SpacerElement:
      // {
      //   const QgsAttributeEditorSpacerElement *spacerElementEditor = static_cast<const QgsAttributeEditorSpacerElement *>( editorElement );
      //   DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::SpacerWidget, editorElement->name(), editorElement->name() );
      //   SpacerElementEditorConfiguration spacerEdConfig;
      //   spacerEdConfig.drawLine = spacerElementEditor->drawLine();
      //   itemData.setSpacerElementEditorConfiguration( spacerEdConfig );
      //   setCommonProperties( itemData );
      //   itemData.setShowLabel( false );
      //   newWidget = tree->addItem( parent, itemData );
      //   break;
      // }

      // case Qgis::AttributeEditorType::Invalid:
      // {
      //   QgsDebugError( QStringLiteral( "Not loading invalid attribute editor type..." ) );
      //   break;
      // }
  }

  parent->addChildItem( std::move( editorItem ) );

  // if ( newWidget )
  //   newWidget->setExpanded( true );

  // return newWidget;
}

int QgsAttributesFormLayoutModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() && parent.column() > 0 )
    return 0;

  const AttributeFormLayoutTreeItem *parentItem = getItem( parent );

  return parentItem ? parentItem->childCount() : 0;
}

int QgsAttributesFormLayoutModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 1;
}

QModelIndex QgsAttributesFormLayoutModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  AttributeFormLayoutTreeItem *parentItem = getItem( parent );
  if ( !parentItem )
    return QModelIndex();

  if ( AttributeFormLayoutTreeItem *childItem = parentItem->child( row ) )
    return createIndex( row, column, childItem );

  return QModelIndex();
}

QModelIndex QgsAttributesFormLayoutModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  AttributeFormLayoutTreeItem *childItem = getItem( index );
  AttributeFormLayoutTreeItem *parentItem = childItem ? childItem->parent() : nullptr;

  return ( parentItem != mRootItem.get() && parentItem != nullptr )
           ? createIndex( parentItem->row(), 0, parentItem )
           : QModelIndex {};
}

QVariant QgsAttributesFormLayoutModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  // if ( role != Qt::DisplayRole && role != Qt::EditRole )
  //   return QVariant();

  const AttributeFormLayoutTreeItem *item = getItem( index );
  if ( !item )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      return item->displayName().isEmpty() ? ( item->name().isEmpty() ? item->id() : item->name() ) : item->displayName();
    }

    case Qt::ToolTipRole:
    {
      if ( item->type() == QgsAttributeFormTreeData::AttributeFormTreeItemType::Field )
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
      if ( item->type() == QgsAttributeFormTreeData::AttributeFormTreeItemType::Container )
        return QBrush( Qt::lightGray );

      return QVariant();
    }

    case DnDTreeRole:
    //case FieldConfigRole:
    //case TreeItemNameRole:
    case TreeItemTypeRole:
      return item->data( role );

    default:
      return QVariant();
  }
}

bool QgsAttributesFormLayoutModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( role != QgsAttributesFormLayoutModel::FieldPropertiesRoles::DnDTreeRole
       && role != QgsAttributesFormLayoutModel::FieldPropertiesRoles::TreeItemTypeRole ) // TODO
    return false;

  AttributeFormLayoutTreeItem *item = getItem( index );
  bool result = item->setData( role, value );

  if ( result )
    emit dataChanged( index, index, { role } );

  return result;
}
