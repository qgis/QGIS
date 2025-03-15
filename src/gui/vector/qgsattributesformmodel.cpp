#include "qgsattributesformmodel.h"
#include "qgsgui.h"
#include "qgseditorwidgetregistry.h"


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
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::FieldNameRole:
    default:
      return mFieldNameData;
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
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::FieldNameRole:
    {
      mFieldNameData = value.toString();
      return true;
    }
    case QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::TreeItemTypeRole:
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
  rebuild();
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

void QgsAttributesAvailableWidgetsModel::rebuild()
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

    auto item = std::make_unique< AttributeFormTreeItem >( QgsAttributeFormTreeData::AttributeFormTreeItemType::Field, itemData, field.name() );
    item->setData( FieldConfigRole, cfg );
    item->setData( FieldNameRole, field.name() );
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

    auto itemRelation = std::make_unique< AttributeFormTreeItem >( QgsAttributeFormTreeData::AttributeFormTreeItemType::Relation, itemData, relation.id(), name );
    itemRelation->setData( FieldNameRole, relation.id() );
    itemRelations->addChildItem( std::move( itemRelation ) );
  }

  mRootItem->addChildItem( std::move( itemRelations ) );

  // Form actions
  // catItemData = DnDTreeItemData( DnDTreeItemData::WidgetType, QStringLiteral( "Actions" ), tr( "Actions" ) );
  // catitem = mAvailableWidgetsTree->addItem( mAvailableWidgetsTree->invisibleRootItem(), catItemData );

  // const QList<QgsAction> actions { mLayer->actions()->actions() };

  // for ( const auto &action : std::as_const( actions ) )
  // {
  //   if ( action.isValid() && action.runable() && ( action.actionScopes().contains( QStringLiteral( "Feature" ) ) || action.actionScopes().contains( QStringLiteral( "Layer" ) ) ) )
  //   {
  //     const QString actionTitle { action.shortTitle().isEmpty() ? action.name() : action.shortTitle() };
  //     DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Action, action.id().toString(), actionTitle );
  //     itemData.setShowLabel( true );
  //     mAvailableWidgetsTree->addItem( catitem, itemData );
  //   }
  // }

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
      return item->displayName();
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
    case FieldNameRole:
    case TreeItemTypeRole:
      return item->data( role );

    default:
      return QVariant();
  }
}

bool QgsAttributesAvailableWidgetsModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( role != QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::DnDTreeRole
       && role != QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::FieldConfigRole
       && role != QgsAttributesAvailableWidgetsModel::FieldPropertiesRoles::FieldNameRole )
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
