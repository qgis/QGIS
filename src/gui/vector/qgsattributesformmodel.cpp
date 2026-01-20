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

#include "qgsattributesformmodel.h"

#include "qgsactionmanager.h"
#include "qgsapplication.h"
#include "qgsattributeeditoraction.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributeeditorhtmlelement.h"
#include "qgsattributeeditorqmlelement.h"
#include "qgsattributeeditorrelation.h"
#include "qgsattributeeditorspacerelement.h"
#include "qgsattributeeditortextelement.h"
#include "qgseditorwidgetregistry.h"
#include "qgsgui.h"

#include <QMimeData>

#include "moc_qgsattributesformmodel.cpp"

QgsAttributesFormData::FieldConfig::FieldConfig( QgsVectorLayer *layer, int idx )
{
  if ( !layer || idx < 0 || idx >= layer->fields().count() )
    return;

  mAlias = layer->fields().at( idx ).alias();
  mDataDefinedProperties = layer->editFormConfig().dataDefinedFieldProperties( layer->fields().at( idx ).name() );
  mComment = layer->fields().at( idx ).comment();
  mEditable = !layer->editFormConfig().readOnly( idx );
  mLabelOnTop = layer->editFormConfig().labelOnTop( idx );
  mReuseLastValuePolicy = layer->editFormConfig().reuseLastValuePolicy( idx );
  mFieldConstraints = layer->fields().at( idx ).constraints();
  const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( layer, layer->fields().field( idx ).name() );
  mEditorWidgetType = setup.type();
  mEditorWidgetConfig = setup.config();
  mSplitPolicy = layer->fields().at( idx ).splitPolicy();
  mDuplicatePolicy = layer->fields().at( idx ).duplicatePolicy();
  mMergePolicy = layer->fields().at( idx ).mergePolicy();
  mDefaultValueExpression = layer->fields().at( idx ).defaultValueDefinition().expression();
  mApplyDefaultValueOnUpdate = layer->fields().at( idx ).defaultValueDefinition().applyOnUpdate();
}

QgsAttributesFormData::FieldConfig::operator QVariant()
{
  return QVariant::fromValue<QgsAttributesFormData::FieldConfig>( *this );
}

QgsAttributesFormData::RelationEditorConfiguration::operator QVariant()
{
  return QVariant::fromValue<QgsAttributesFormData::RelationEditorConfiguration>( *this );
}

Qgis::AttributeEditorContainerType QgsAttributesFormData::AttributeFormItemData::containerType() const
{
  return mContainerType;
}

void QgsAttributesFormData::AttributeFormItemData::setContainerType( Qgis::AttributeEditorContainerType type )
{
  mContainerType = type;
}

const QgsAttributeEditorElement::LabelStyle QgsAttributesFormData::AttributeFormItemData::labelStyle() const
{
  return mLabelStyle;
}

void QgsAttributesFormData::AttributeFormItemData::setLabelStyle( const QgsAttributeEditorElement::LabelStyle &labelStyle )
{
  mLabelStyle = labelStyle;
}

bool QgsAttributesFormData::AttributeFormItemData::showLabel() const
{
  return mShowLabel;
}

void QgsAttributesFormData::AttributeFormItemData::setShowLabel( bool showLabel )
{
  mShowLabel = showLabel;
}

QgsOptionalExpression QgsAttributesFormData::AttributeFormItemData::visibilityExpression() const
{
  return mVisibilityExpression;
}

void QgsAttributesFormData::AttributeFormItemData::setVisibilityExpression( const QgsOptionalExpression &visibilityExpression )
{
  mVisibilityExpression = visibilityExpression;
}

QgsOptionalExpression QgsAttributesFormData::AttributeFormItemData::collapsedExpression() const
{
  return mCollapsedExpression;
}

void QgsAttributesFormData::AttributeFormItemData::setCollapsedExpression( const QgsOptionalExpression &collapsedExpression )
{
  mCollapsedExpression = collapsedExpression;
}

QgsAttributesFormData::RelationEditorConfiguration QgsAttributesFormData::AttributeFormItemData::relationEditorConfiguration() const
{
  return mRelationEditorConfiguration;
}

void QgsAttributesFormData::AttributeFormItemData::setRelationEditorConfiguration( const QgsAttributesFormData::RelationEditorConfiguration &relationEditorConfiguration )
{
  mRelationEditorConfiguration = relationEditorConfiguration;
}

QgsAttributesFormData::QmlElementEditorConfiguration QgsAttributesFormData::AttributeFormItemData::qmlElementEditorConfiguration() const
{
  return mQmlElementEditorConfiguration;
}

void QgsAttributesFormData::AttributeFormItemData::setQmlElementEditorConfiguration( const QmlElementEditorConfiguration &qmlElementEditorConfiguration )
{
  mQmlElementEditorConfiguration = qmlElementEditorConfiguration;
}


QgsAttributesFormData::HtmlElementEditorConfiguration QgsAttributesFormData::AttributeFormItemData::htmlElementEditorConfiguration() const
{
  return mHtmlElementEditorConfiguration;
}

void QgsAttributesFormData::AttributeFormItemData::setHtmlElementEditorConfiguration( const HtmlElementEditorConfiguration &htmlElementEditorConfiguration )
{
  mHtmlElementEditorConfiguration = htmlElementEditorConfiguration;
}

QgsAttributesFormData::SpacerElementEditorConfiguration QgsAttributesFormData::AttributeFormItemData::spacerElementEditorConfiguration() const
{
  return mSpacerElementEditorConfiguration;
}

void QgsAttributesFormData::AttributeFormItemData::setSpacerElementEditorConfiguration( SpacerElementEditorConfiguration spacerElementEditorConfiguration )
{
  mSpacerElementEditorConfiguration = spacerElementEditorConfiguration;
}

QColor QgsAttributesFormData::AttributeFormItemData::backgroundColor() const
{
  return mBackgroundColor;
}

void QgsAttributesFormData::AttributeFormItemData::setBackgroundColor( const QColor &backgroundColor )
{
  mBackgroundColor = backgroundColor;
}

QgsAttributesFormData::TextElementEditorConfiguration QgsAttributesFormData::AttributeFormItemData::textElementEditorConfiguration() const
{
  return mTextElementEditorConfiguration;
}

void QgsAttributesFormData::AttributeFormItemData::setTextElementEditorConfiguration( const QgsAttributesFormData::TextElementEditorConfiguration &textElementEditorConfiguration )
{
  mTextElementEditorConfiguration = textElementEditorConfiguration;
}


QgsAttributesFormItem::QgsAttributesFormItem( QgsAttributesFormData::AttributesFormItemType itemType, const QString &name, const QString &displayName, QgsAttributesFormItem *parent )
  : mName( name )
  , mDisplayName( displayName )
  , mType( itemType )
  , mParent( parent )
{}

QgsAttributesFormItem::QgsAttributesFormItem( QgsAttributesFormData::AttributesFormItemType itemType, const QgsAttributesFormData::AttributeFormItemData &data, const QString &name, const QString &displayName, QgsAttributesFormItem *parent )
  : mName( name )
  , mDisplayName( displayName )
  , mType( itemType )
  , mData( data )
  , mParent( parent )
{}

QgsAttributesFormItem *QgsAttributesFormItem::child( int row )
{
  if ( !mChildren.empty() && row >= 0 && row < childCount() )
    return mChildren.at( row ).get();

  return nullptr;
}

QgsAttributesFormItem *QgsAttributesFormItem::firstTopChild( const QgsAttributesFormData::AttributesFormItemType itemType, const QString &itemId ) const
{
  if ( !mChildren.empty() && itemId.trimmed().isEmpty() )
    return nullptr;

  // Search for first matching item by name
  const auto it = std::find_if( mChildren.cbegin(), mChildren.cend(), [itemType, itemId]( const std::unique_ptr< QgsAttributesFormItem > &item ) {
    return item->type() == itemType && item->id() == itemId;
  } );

  if ( it != mChildren.cend() )
    return it->get();

  return nullptr;
}

QgsAttributesFormItem *QgsAttributesFormItem::firstChildRecursive( const QgsAttributesFormData::AttributesFormItemType &itemType, const QString &itemId ) const
{
  if ( !mChildren.empty() && itemId.trimmed().isEmpty() )
    return nullptr;

  for ( const auto &child : std::as_const( mChildren ) )
  {
    if ( child->type() == itemType && child->id() == itemId )
      return child.get();

    if ( child->childCount() > 0 )
    {
      QgsAttributesFormItem *item = child->firstChildRecursive( itemType, itemId );
      if ( item )
        return item;
    }
  }

  return nullptr;
}

int QgsAttributesFormItem::childCount() const
{
  return static_cast< int >( mChildren.size() );
}

int QgsAttributesFormItem::row() const
{
  if ( !mParent )
    return 0;

  const auto it = std::find_if( mParent->mChildren.cbegin(), mParent->mChildren.cend(), [this]( const std::unique_ptr< QgsAttributesFormItem > &item ) {
    return item.get() == this;
  } );

  if ( it != mParent->mChildren.cend() )
  {
    return static_cast< int >( std::distance( mParent->mChildren.cbegin(), it ) );
  }

  return -1;
}

void QgsAttributesFormItem::addChild( std::unique_ptr< QgsAttributesFormItem > &&item )
{
  if ( !item )
    return;

  if ( !item->mParent )
    item->mParent = this;

  // forward the signal towards the root
  connect( item.get(), &QgsAttributesFormItem::addedChildren, this, &QgsAttributesFormItem::addedChildren );

  mChildren.push_back( std::move( item ) );

  emit addedChildren( this, mChildren.size() - 1, mChildren.size() - 1 );
}

void QgsAttributesFormItem::insertChild( int position, std::unique_ptr< QgsAttributesFormItem > &&item )
{
  if ( position < 0 || position > static_cast< int >( mChildren.size() ) || !item )
    return;

  if ( !item->mParent )
    item->mParent = this;

  // forward the signal towards the root
  connect( item.get(), &QgsAttributesFormItem::addedChildren, this, &QgsAttributesFormItem::addedChildren );

  mChildren.insert( mChildren.begin() + position, std::move( item ) );

  emit addedChildren( this, position, position );
}

void QgsAttributesFormItem::deleteChildAtIndex( int index )
{
  if ( index >= 0 && index < static_cast< int >( mChildren.size() ) )
    mChildren.erase( mChildren.begin() + index );
}

void QgsAttributesFormItem::deleteChildren()
{
  mChildren.clear();
}

bool QgsAttributesFormItem::isGroup( QgsAttributesFormItem *item )
{
  return item->type() == QgsAttributesFormData::WidgetType || item->type() == QgsAttributesFormData::Container;
}

QVariant QgsAttributesFormItem::data( int role ) const
{
  switch ( role )
  {
    case QgsAttributesFormModel::ItemTypeRole:
      return mType;
    case QgsAttributesFormModel::ItemDataRole:
      return QVariant::fromValue( mData );
    case QgsAttributesFormModel::ItemNameRole:
      return mName;
    case QgsAttributesFormModel::ItemIdRole:
      return mId;
    case QgsAttributesFormModel::ItemDisplayRole:
      return mDisplayName;
    case QgsAttributesFormModel::ItemFieldConfigRole:
      return QVariant::fromValue( mFieldConfigData );
    default:
      return QVariant();
  }
}

bool QgsAttributesFormItem::setData( int role, const QVariant &value )
{
  switch ( role )
  {
    case QgsAttributesFormModel::ItemDataRole:
    {
      mData = value.value< QgsAttributesFormData::AttributeFormItemData >();
      return true;
    }
    case QgsAttributesFormModel::ItemNameRole:
    {
      mName = value.toString();
      return true;
    }
    case QgsAttributesFormModel::ItemDisplayRole:
    {
      mDisplayName = value.toString();
      return true;
    }
    case QgsAttributesFormModel::ItemTypeRole:
    {
      mType = static_cast<QgsAttributesFormData::AttributesFormItemType>( value.toInt() );
      return true;
    }
    case QgsAttributesFormModel::ItemIdRole:
    {
      mId = value.toString();
      return true;
    }
    case QgsAttributesFormModel::ItemFieldConfigRole:
    {
      mFieldConfigData = value.value< QgsAttributesFormData::FieldConfig >();
      return true;
    }
    default:
      return false;
  }
}


QgsAttributesFormModel::QgsAttributesFormModel( QgsVectorLayer *layer, QgsProject *project, QObject *parent )
  : QAbstractItemModel( parent )
  , mRootItem( std::make_unique< QgsAttributesFormItem >() )
  , mLayer( layer )
  , mProject( project )
{
}

QgsAttributesFormModel::~QgsAttributesFormModel() = default;

QgsAttributesFormItem *QgsAttributesFormModel::itemForIndex( const QModelIndex &index ) const
{
  if ( index.isValid() )
  {
    if ( auto *item = static_cast<QgsAttributesFormItem *>( index.internalPointer() ) )
      return item;
  }
  return mRootItem.get();
}

QgsAttributesFormItem *QgsAttributesFormModel::rootItem() const
{
  return mRootItem.get();
}


int QgsAttributesFormModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() && parent.column() > 0 )
    return 0;

  const QgsAttributesFormItem *parentItem = itemForIndex( parent );

  return parentItem ? parentItem->childCount() : 0;
}

int QgsAttributesFormModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

bool QgsAttributesFormModel::indexLessThan( const QModelIndex &a, const QModelIndex &b ) const
{
  const QVector<int> pathA = rootToLeafPath( itemForIndex( a ) );
  const QVector<int> pathB = rootToLeafPath( itemForIndex( b ) );

  for ( int i = 0; i < std::min( pathA.size(), pathB.size() ); i++ )
  {
    if ( pathA.at( i ) != pathB.at( i ) )
    {
      return pathA.at( i ) < pathB.at( i );
    }
  }

  return pathA.size() < pathB.size();
}

QVector<int> QgsAttributesFormModel::rootToLeafPath( QgsAttributesFormItem *item ) const
{
  QVector<int> path;
  if ( item != mRootItem.get() )
  {
    path << rootToLeafPath( item->parent() ) << item->row();
  }
  return path;
}

QModelIndex QgsAttributesFormModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  QgsAttributesFormItem *parentItem = itemForIndex( parent );
  if ( !parentItem )
    return QModelIndex();

  if ( QgsAttributesFormItem *childItem = parentItem->child( row ) )
    return createIndex( row, column, childItem );

  return QModelIndex();
}

QModelIndex QgsAttributesFormModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  QgsAttributesFormItem *childItem = itemForIndex( index );
  QgsAttributesFormItem *parentItem = childItem ? childItem->parent() : nullptr;

  return ( parentItem != mRootItem.get() && parentItem != nullptr )
           ? createIndex( parentItem->row(), 0, parentItem )
           : QModelIndex();
}

QModelIndex QgsAttributesFormModel::firstTopMatchingModelIndex( const QgsAttributesFormData::AttributesFormItemType &itemType, const QString &itemId ) const
{
  QgsAttributesFormItem *item = mRootItem->firstTopChild( itemType, itemId );
  return item ? createIndex( item->row(), 0, item ) : QModelIndex();
}

QModelIndex QgsAttributesFormModel::firstRecursiveMatchingModelIndex( const QgsAttributesFormData::AttributesFormItemType &itemType, const QString &itemId ) const
{
  QgsAttributesFormItem *item = mRootItem->firstChildRecursive( itemType, itemId );
  return item ? createIndex( item->row(), 0, item ) : QModelIndex();
}

bool QgsAttributesFormModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  QgsAttributesFormItem *item = itemForIndex( index );
  bool result = item->setData( role, value );

  if ( result )
  {
    emit dataChanged( index, index, { role } );

    if ( role == QgsAttributesFormModel::ItemFieldConfigRole )
    {
      emit fieldConfigDataChanged( item );
    }
  }

  return result;
}

bool QgsAttributesFormModel::showAliases() const
{
  return mShowAliases;
}

void QgsAttributesFormModel::setShowAliases( bool show )
{
  mShowAliases = show;

  emitDataChangedRecursively( QModelIndex(), QVector<int>() << Qt::DisplayRole << Qt::ForegroundRole << Qt::FontRole );
}

void QgsAttributesFormModel::emitDataChangedRecursively( const QModelIndex &parent, const QVector<int> &roles )
{
  emit dataChanged( index( 0, 0, parent ), index( rowCount( parent ) - 1, 0, parent ), roles );
  for ( int i = 0; i < rowCount( parent ); i++ )
  {
    const QModelIndex childIndex = index( i, 0, parent );
    if ( hasChildren( childIndex ) )
    {
      emitDataChangedRecursively( childIndex, roles );
    }
  }
}


QgsAttributesAvailableWidgetsModel::QgsAttributesAvailableWidgetsModel( QgsVectorLayer *layer, QgsProject *project, QObject *parent )
  : QgsAttributesFormModel( layer, project, parent )
{
}

Qt::ItemFlags QgsAttributesAvailableWidgetsModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::NoItemFlags;

  Qt::ItemFlags flags = Qt::ItemIsEnabled;

  const auto indexType = static_cast< QgsAttributesFormData::AttributesFormItemType >( index.data( QgsAttributesFormModel::ItemTypeRole ).toInt() );
  if ( indexType != QgsAttributesFormData::WidgetType )
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
  mRootItem->deleteChildren();

  // Load fields

  auto itemFields = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::WidgetType, u"Fields"_s, tr( "Fields" ) );

  const QgsFields fields = mLayer->fields();
  for ( int i = 0; i < fields.size(); ++i )
  {
    const QgsField field = fields.at( i );
    auto itemData = QgsAttributesFormData::AttributeFormItemData();
    itemData.setShowLabel( true );

    QgsAttributesFormData::FieldConfig cfg( mLayer, i );

    auto item = std::make_unique< QgsAttributesFormItem >();
    item->setData( ItemFieldConfigRole, cfg );
    item->setData( ItemNameRole, field.name() );
    item->setData( ItemIdRole, field.name() ); // Field names act as ids
    item->setData( ItemDisplayRole, field.alias() );
    item->setData( ItemTypeRole, QgsAttributesFormData::Field );
    item->setData( ItemDataRole, itemData );
    item->setIcon( fields.iconForField( i, true ) );

    itemFields->addChild( std::move( item ) );
  }

  mRootItem->addChild( std::move( itemFields ) );

  // Load relations

  auto itemRelations = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::WidgetType, u"Relations"_s, tr( "Relations" ) );

  const QList<QgsRelation> relations = mProject->relationManager()->referencedRelations( mLayer );

  for ( const QgsRelation &relation : relations )
  {
    QString name;
    const QgsPolymorphicRelation polymorphicRelation = relation.polymorphicRelation();
    if ( polymorphicRelation.isValid() )
    {
      name = u"%1 (%2)"_s.arg( relation.name(), polymorphicRelation.name() );
    }
    else
    {
      name = relation.name();
    }
    QgsAttributesFormData::AttributeFormItemData itemData = QgsAttributesFormData::AttributeFormItemData();
    itemData.setShowLabel( true );

    auto itemRelation = std::make_unique< QgsAttributesFormItem >();
    itemRelation->setData( ItemTypeRole, QgsAttributesFormData::Relation );
    itemRelation->setData( ItemNameRole, name );
    itemRelation->setData( ItemIdRole, relation.id() );
    itemRelation->setData( ItemDataRole, itemData );
    itemRelation->setIcon( QgsApplication::getThemeIcon( u"/mEditorWidgetRelationEditor.svg"_s ) );
    itemRelations->addChild( std::move( itemRelation ) );
  }

  mRootItem->addChild( std::move( itemRelations ) );

  // Load form actions

  auto itemActions = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::WidgetType, u"Actions"_s, tr( "Actions" ) );
  mRootItem->addChild( std::move( itemActions ) );
  populateActionItems( mLayer->actions()->actions() );

  // Other widgets

  auto itemOtherWidgets = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::WidgetType, u"Other"_s, tr( "Other Widgets" ) );

  QgsAttributesFormData::AttributeFormItemData itemData = QgsAttributesFormData::AttributeFormItemData();
  itemData.setShowLabel( true );
  auto itemQml = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::QmlWidget, itemData, u"QML Widget"_s, tr( "QML Widget" ) );
  itemQml->setIcon( QgsApplication::getThemeIcon( u"/mEditorWidgetQml.svg"_s ) );
  itemOtherWidgets->addChild( std::move( itemQml ) );

  QgsAttributesFormData::AttributeFormItemData itemHtmlData = QgsAttributesFormData::AttributeFormItemData();
  itemHtmlData.setShowLabel( true );
  auto itemHtml = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::HtmlWidget, itemHtmlData, u"HTML Widget"_s, tr( "HTML Widget" ) );
  itemHtml->setIcon( QgsApplication::getThemeIcon( u"/mEditorWidgetHtml.svg"_s ) );
  itemOtherWidgets->addChild( std::move( itemHtml ) );

  QgsAttributesFormData::AttributeFormItemData itemTextData = QgsAttributesFormData::AttributeFormItemData();
  itemTextData.setShowLabel( true );
  auto itemText = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::TextWidget, itemTextData, u"Text Widget"_s, tr( "Text Widget" ) );
  itemText->setIcon( QgsApplication::getThemeIcon( u"/mEditorWidgetText.svg"_s ) );
  itemOtherWidgets->addChild( std::move( itemText ) );

  QgsAttributesFormData::AttributeFormItemData itemSpacerData = QgsAttributesFormData::AttributeFormItemData();
  itemTextData.setShowLabel( false );
  auto itemSpacer = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::SpacerWidget, u"Spacer Widget"_s, tr( "Spacer Widget" ) );
  itemSpacer->setIcon( QgsApplication::getThemeIcon( u"/mEditorWidgetSpacer.svg"_s ) );
  itemOtherWidgets->addChild( std::move( itemSpacer ) );

  mRootItem->addChild( std::move( itemOtherWidgets ) );

  endResetModel();
}

void QgsAttributesAvailableWidgetsModel::populateLayerActions( const QList< QgsAction > actions )
{
  QModelIndex actionsIndex = actionContainer();
  QgsAttributesFormItem *itemActions = itemForIndex( actionsIndex );

  beginRemoveRows( actionsIndex, 0, itemActions->childCount() );
  itemActions->deleteChildren();
  endRemoveRows();

  int count = 0;
  for ( const auto &action : std::as_const( actions ) )
  {
    if ( action.isValid() && action.runable() && ( action.actionScopes().contains( u"Feature"_s ) || action.actionScopes().contains( u"Layer"_s ) ) )
    {
      count++;
    }
  }

  if ( count > 0 )
  {
    beginInsertRows( actionsIndex, 0, count - 1 );
    populateActionItems( actions );
    endInsertRows();
  }
}

void QgsAttributesAvailableWidgetsModel::populateActionItems( const QList<QgsAction> actions )
{
  QModelIndex actionsIndex = actionContainer();
  QgsAttributesFormItem *itemActions = itemForIndex( actionsIndex );

  for ( const auto &action : std::as_const( actions ) )
  {
    if ( action.isValid() && action.runable() && ( action.actionScopes().contains( u"Feature"_s ) || action.actionScopes().contains( u"Layer"_s ) ) )
    {
      const QString actionTitle { action.shortTitle().isEmpty() ? action.name() : action.shortTitle() };

      QgsAttributesFormData::AttributeFormItemData itemData = QgsAttributesFormData::AttributeFormItemData();
      itemData.setShowLabel( true );

      auto itemAction = std::make_unique< QgsAttributesFormItem >();
      itemAction->setData( ItemIdRole, action.id().toString() );
      itemAction->setData( ItemTypeRole, QgsAttributesFormData::Action );
      itemAction->setData( ItemNameRole, actionTitle );
      itemAction->setData( ItemDataRole, itemData );
      itemAction->setIcon( QgsApplication::getThemeIcon( u"/mEditorWidgetAction.svg"_s ) );

      itemActions->addChild( std::move( itemAction ) );
    }
  }
}

QVariant QgsAttributesAvailableWidgetsModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const QgsAttributesFormItem *item = itemForIndex( index );
  if ( !item )
    return QVariant();

  // Relations may be broken due to missing layers or references.
  // Make those stand out from valid ones.
  bool invalidRelation = false;
  if ( ( role == Qt::ToolTipRole || role == Qt::ForegroundRole ) && item->type() == QgsAttributesFormData::Relation )
  {
    invalidRelation = !QgsProject::instance()->relationManager()->relation( item->id() ).isValid();
  }

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      if ( !showAliases() && item->type() == QgsAttributesFormData::Field )
      {
        return item->name();
      }

      return item->displayName().isEmpty() ? item->name() : item->displayName();
    }

    case Qt::ToolTipRole:
    {
      if ( item->type() == QgsAttributesFormData::AttributesFormItemType::Field )
      {
        const auto cfg = item->data( ItemFieldConfigRole ).value<QgsAttributesFormData::FieldConfig>();
        if ( !cfg.mAlias.isEmpty() )
          return tr( "%1 (%2)" ).arg( item->name(), cfg.mAlias );
        else
          return item->name();
      }

      if ( item->type() == QgsAttributesFormData::Relation && invalidRelation )
      {
        // Relation name will be displayed, inform users why it's red via tooltip
        return tr( "Invalid relation" );
      }

      return QVariant();
    }

    case Qt::DecorationRole:
      return item->icon();

    case Qt::BackgroundRole:
    {
      if ( item->type() == QgsAttributesFormData::AttributesFormItemType::WidgetType )
        return QBrush( QColor( 140, 140, 140, 50 ) );

      return QVariant();
    }

    case Qt::ForegroundRole:
    {
      if ( item->type() == QgsAttributesFormData::Field )
      {
        if ( showAliases() && item->displayName().isEmpty() )
        {
          return QBrush( QColor( Qt::lightGray ) );
        }
      }

      if ( item->type() == QgsAttributesFormData::Relation && invalidRelation )
      {
        return QBrush( QColor( 255, 0, 0 ) );
      }

      return QVariant();
    }

    case Qt::FontRole:
    {
      if ( item->type() == QgsAttributesFormData::Field )
      {
        if ( showAliases() && item->displayName().isEmpty() )
        {
          QFont font = QFont();
          font.setItalic( true );
          return font;
        }
      }
      return QVariant();
    }

    case ItemDataRole:
    case ItemFieldConfigRole:
    case ItemNameRole:
    case ItemTypeRole:
    case ItemIdRole:
    case ItemDisplayRole:
      return item->data( role );

    default:
      return QVariant();
  }
}

Qt::DropActions QgsAttributesAvailableWidgetsModel::supportedDragActions() const
{
  return Qt::CopyAction;
}

QStringList QgsAttributesAvailableWidgetsModel::mimeTypes() const
{
  return QStringList() << u"application/x-qgsattributesformavailablewidgetsrelement"_s;
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

  std::sort( sortedIndexes.begin(), sortedIndexes.end(), [this]( const QModelIndex &a, const QModelIndex &b ) {
    return indexLessThan( a, b );
  } );

  for ( const QModelIndex &index : std::as_const( sortedIndexes ) )
  {
    if ( index.isValid() )
    {
      const QString itemId = index.data( QgsAttributesFormModel::ItemIdRole ).toString();
      const QString itemName = index.data( QgsAttributesFormModel::ItemNameRole ).toString();
      int itemType = index.data( QgsAttributesFormModel::ItemTypeRole ).toInt();

      stream << itemId << itemType << itemName;
    }
  }

  data->setData( format, encoded );
  return data;
}

QModelIndex QgsAttributesAvailableWidgetsModel::fieldContainer() const
{
  if ( mRootItem->childCount() > 0 )
  {
    const int row = 0;
    QgsAttributesFormItem *item = mRootItem->child( row );
    if ( item && item->name() == "Fields"_L1 && item->type() == QgsAttributesFormData::WidgetType )
      return createIndex( row, 0, item );
  }
  return QModelIndex();
}

QModelIndex QgsAttributesAvailableWidgetsModel::relationContainer() const
{
  if ( mRootItem->childCount() > 1 )
  {
    const int row = 1;
    QgsAttributesFormItem *item = mRootItem->child( row );
    if ( item && item->name() == "Relations"_L1 && item->type() == QgsAttributesFormData::WidgetType )
      return createIndex( row, 0, item );
  }
  return QModelIndex();
}

QModelIndex QgsAttributesAvailableWidgetsModel::actionContainer() const
{
  if ( mRootItem->childCount() > 2 )
  {
    const int row = 2;
    QgsAttributesFormItem *item = mRootItem->child( row );
    if ( item && item->name() == "Actions"_L1 && item->type() == QgsAttributesFormData::WidgetType )
      return createIndex( row, 0, item );
  }
  return QModelIndex();
}

QModelIndex QgsAttributesAvailableWidgetsModel::fieldModelIndex( const QString &fieldName ) const
{
  if ( mRootItem->childCount() == 0 )
    return QModelIndex();

  QgsAttributesFormItem *fieldItems = mRootItem->child( 0 );
  if ( !fieldItems || fieldItems->name() != "Fields"_L1 || fieldItems->type() != QgsAttributesFormData::WidgetType )
    return QModelIndex();

  QgsAttributesFormItem *item = fieldItems->firstTopChild( QgsAttributesFormData::Field, fieldName );
  return item ? createIndex( item->row(), 0, item ) : QModelIndex();
}


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

  QgsAttributesFormItem *item = itemForIndex( index );
  if ( item->type() == QgsAttributesFormData::WidgetType || item->type() == QgsAttributesFormData::Container )
    flags |= Qt::ItemIsDropEnabled;

  return flags;
}

void QgsAttributesFormLayoutModel::populate()
{
  if ( !mLayer )
    return;

  beginResetModel();
  mRootItem->deleteChildren();

  const auto editorElements = mLayer->editFormConfig().tabs();
  for ( QgsAttributeEditorElement *editorElement : editorElements )
  {
    loadAttributeEditorElementItem( editorElement, mRootItem.get() );
  }

  endResetModel();
}

void QgsAttributesFormLayoutModel::loadAttributeEditorElementItem( QgsAttributeEditorElement *const editorElement, QgsAttributesFormItem *parent, const int position )
{
  auto setCommonProperties = [editorElement]( QgsAttributesFormData::AttributeFormItemData &itemData ) {
    itemData.setShowLabel( editorElement->showLabel() );
    itemData.setLabelStyle( editorElement->labelStyle() );
    itemData.setHorizontalStretch( editorElement->horizontalStretch() );
    itemData.setVerticalStretch( editorElement->verticalStretch() );
  };

  auto editorItem = std::make_unique< QgsAttributesFormItem >();

  switch ( editorElement->type() )
  {
    case Qgis::AttributeEditorType::Field:
    {
      QgsAttributesFormData::AttributeFormItemData itemData = QgsAttributesFormData::AttributeFormItemData();
      setCommonProperties( itemData );

      editorItem->setData( ItemNameRole, editorElement->name() );
      editorItem->setData( ItemIdRole, editorElement->name() ); // Field names act as ids
      editorItem->setData( ItemTypeRole, QgsAttributesFormData::Field );
      editorItem->setData( ItemDataRole, itemData );

      const int fieldIndex = mLayer->fields().indexOf( editorElement->name() );
      if ( fieldIndex != -1 )
      {
        editorItem->setData( ItemDisplayRole, mLayer->fields().field( fieldIndex ).alias() );

        QgsAttributesFormData::FieldConfig config( mLayer, fieldIndex );
        editorItem->setData( ItemFieldConfigRole, config );
        editorItem->setIcon( QgsGui::instance()->editorWidgetRegistry()->icon( config.mEditorWidgetType ) );
      }

      break;
    }

    case Qgis::AttributeEditorType::Action:
    {
      const QgsAttributeEditorAction *actionEditor = static_cast<const QgsAttributeEditorAction *>( editorElement );
      const QgsAction action { actionEditor->action( mLayer ) };
      if ( action.isValid() )
      {
        QgsAttributesFormData::AttributeFormItemData itemData = QgsAttributesFormData::AttributeFormItemData();
        setCommonProperties( itemData );

        editorItem->setData( ItemIdRole, action.id().toString() );
        editorItem->setData( ItemNameRole, action.shortTitle().isEmpty() ? action.name() : action.shortTitle() );
        editorItem->setData( ItemTypeRole, QgsAttributesFormData::Action );
        editorItem->setData( ItemDataRole, itemData );
        editorItem->setIcon( QgsApplication::getThemeIcon( u"/mEditorWidgetAction.svg"_s ) );
      }
      else
      {
        QgsDebugError( u"Invalid form action"_s );
      }
      break;
    }

    case Qgis::AttributeEditorType::Relation:
    {
      QgsAttributesFormData::AttributeFormItemData itemData = QgsAttributesFormData::AttributeFormItemData();
      setCommonProperties( itemData );

      const QgsAttributeEditorRelation *relationEditor = static_cast<const QgsAttributeEditorRelation *>( editorElement );
      QgsAttributesFormData::RelationEditorConfiguration relationEditorConfig;
      relationEditorConfig.mRelationWidgetType = relationEditor->relationWidgetTypeId();
      relationEditorConfig.mRelationWidgetConfig = relationEditor->relationEditorConfiguration();
      relationEditorConfig.nmRelationId = relationEditor->nmRelationId();
      relationEditorConfig.forceSuppressFormPopup = relationEditor->forceSuppressFormPopup();
      relationEditorConfig.label = relationEditor->label();
      itemData.setRelationEditorConfiguration( relationEditorConfig );

      QgsRelation relation = relationEditor->relation();
      if ( relation.id().isEmpty() )
      {
        // If relation is coming from an internal move, we lose the id.
        // Go to relation manager and bring relation properties.
        relation = mProject->relationManager()->relation( editorElement->name() );
      }

      editorItem->setData( ItemIdRole, relation.id() );
      editorItem->setData( ItemNameRole, relation.name() );
      editorItem->setData( ItemDisplayRole, relationEditorConfig.label );
      editorItem->setData( ItemTypeRole, QgsAttributesFormData::Relation );
      editorItem->setData( ItemDataRole, itemData );
      editorItem->setIcon( QgsApplication::getThemeIcon( u"/mEditorWidgetRelationEditor.svg"_s ) );

      break;
    }

    case Qgis::AttributeEditorType::Container:
    {
      QgsAttributesFormData::AttributeFormItemData itemData = QgsAttributesFormData::AttributeFormItemData();
      setCommonProperties( itemData );

      editorItem->setData( ItemNameRole, editorElement->name() );
      editorItem->setData( ItemIdRole, editorElement->name() ); // Containers don't have id, use name to make them searchable
      editorItem->setData( ItemTypeRole, QgsAttributesFormData::Container );

      const QgsAttributeEditorContainer *container = static_cast<const QgsAttributeEditorContainer *>( editorElement );
      if ( !container )
        break;

      itemData.setColumnCount( container->columnCount() );
      itemData.setContainerType( container->type() );
      itemData.setBackgroundColor( container->backgroundColor() );
      itemData.setVisibilityExpression( container->visibilityExpression() );
      itemData.setCollapsedExpression( container->collapsedExpression() );
      itemData.setCollapsed( container->collapsed() );

      editorItem->setData( ItemDataRole, itemData );

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
      QgsAttributesFormData::AttributeFormItemData itemData = QgsAttributesFormData::AttributeFormItemData();
      setCommonProperties( itemData );

      QgsAttributesFormData::QmlElementEditorConfiguration qmlEdConfig;
      qmlEdConfig.qmlCode = qmlElementEditor->qmlCode();
      itemData.setQmlElementEditorConfiguration( qmlEdConfig );

      editorItem->setData( ItemNameRole, editorElement->name() );
      editorItem->setData( ItemTypeRole, QgsAttributesFormData::QmlWidget );
      editorItem->setData( ItemDataRole, itemData );
      editorItem->setIcon( QgsApplication::getThemeIcon( u"/mEditorWidgetQml.svg"_s ) );
      break;
    }

    case Qgis::AttributeEditorType::HtmlElement:
    {
      const QgsAttributeEditorHtmlElement *htmlElementEditor = static_cast<const QgsAttributeEditorHtmlElement *>( editorElement );
      QgsAttributesFormData::AttributeFormItemData itemData = QgsAttributesFormData::AttributeFormItemData();
      setCommonProperties( itemData );

      QgsAttributesFormData::HtmlElementEditorConfiguration htmlEdConfig;
      htmlEdConfig.htmlCode = htmlElementEditor->htmlCode();
      itemData.setHtmlElementEditorConfiguration( htmlEdConfig );

      editorItem->setData( ItemNameRole, editorElement->name() );
      editorItem->setData( ItemTypeRole, QgsAttributesFormData::HtmlWidget );
      editorItem->setData( ItemDataRole, itemData );
      editorItem->setIcon( QgsApplication::getThemeIcon( u"/mEditorWidgetHtml.svg"_s ) );
      break;
    }

    case Qgis::AttributeEditorType::TextElement:
    {
      const QgsAttributeEditorTextElement *textElementEditor = static_cast<const QgsAttributeEditorTextElement *>( editorElement );
      QgsAttributesFormData::AttributeFormItemData itemData = QgsAttributesFormData::AttributeFormItemData();
      setCommonProperties( itemData );

      QgsAttributesFormData::TextElementEditorConfiguration textEdConfig;
      textEdConfig.text = textElementEditor->text();
      itemData.setTextElementEditorConfiguration( textEdConfig );

      editorItem->setData( ItemNameRole, editorElement->name() );
      editorItem->setData( ItemTypeRole, QgsAttributesFormData::TextWidget );
      editorItem->setData( ItemDataRole, itemData );
      editorItem->setIcon( QgsApplication::getThemeIcon( u"/mEditorWidgetText.svg"_s ) );
      break;
    }

    case Qgis::AttributeEditorType::SpacerElement:
    {
      const QgsAttributeEditorSpacerElement *spacerElementEditor = static_cast<const QgsAttributeEditorSpacerElement *>( editorElement );
      QgsAttributesFormData::AttributeFormItemData itemData = QgsAttributesFormData::AttributeFormItemData();
      setCommonProperties( itemData );
      itemData.setShowLabel( false );

      QgsAttributesFormData::SpacerElementEditorConfiguration spacerEdConfig;
      spacerEdConfig.drawLine = spacerElementEditor->drawLine();
      itemData.setSpacerElementEditorConfiguration( spacerEdConfig );

      editorItem->setData( ItemNameRole, editorElement->name() );
      editorItem->setData( ItemTypeRole, QgsAttributesFormData::SpacerWidget );
      editorItem->setData( ItemDataRole, itemData );
      editorItem->setIcon( QgsApplication::getThemeIcon( u"/mEditorWidgetSpacer.svg"_s ) );
      break;
    }

    case Qgis::AttributeEditorType::Invalid:
    {
      QgsDebugError( u"Not loading invalid attribute editor type..."_s );
      break;
    }
  }

  if ( position >= 0 && position < parent->childCount() )
  {
    parent->insertChild( position, std::move( editorItem ) );
  }
  else
  {
    parent->addChild( std::move( editorItem ) );
  }
}

QVariant QgsAttributesFormLayoutModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( role == ItemFieldConfigRole ) // This model doesn't store data for that role
    return false;

  QgsAttributesFormItem *item = itemForIndex( index );
  if ( !item )
    return QVariant();

  // Fields may be present in the form layout configuration
  // even if their corresponding layer fields were deleted.
  // Make those stand out from existent ones.
  const int fieldIndex = mLayer->fields().indexOf( item->name() );
  const bool invalidField = fieldIndex == -1;

  // Relations may be broken due to missing layers or references.
  // Make those stand out from valid ones.
  bool invalidRelation = false;
  if ( ( role == Qt::DisplayRole || role == Qt::ToolTipRole || role == Qt::ForegroundRole ) && item->type() == QgsAttributesFormData::Relation )
  {
    invalidRelation = !QgsProject::instance()->relationManager()->relation( item->id() ).isValid();
  }

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      if ( item->type() == QgsAttributesFormData::Relation && invalidRelation )
      {
        // Invalid relations can have an id, if that's the case, we have a name.
        // Only set a new name if id is missing.
        if ( item->id().isEmpty() )
        {
          return tr( "Invalid relation" );
        }
      }

      if ( !showAliases() && ( item->type() == QgsAttributesFormData::Field || item->type() == QgsAttributesFormData::Relation ) )
      {
        return item->name();
      }

      return item->displayName().isEmpty() ? item->name() : item->displayName();
    }

    case Qt::ToolTipRole:
    {
      if ( item->type() == QgsAttributesFormData::Field )
      {
        if ( invalidField )
        {
          return tr( "Invalid field" );
        }
        else
        {
          return item->name();
        }
      }

      if ( item->type() == QgsAttributesFormData::Relation && invalidRelation )
      {
        if ( !item->id().isEmpty() )
        {
          // The relation name is shown, let's inform users via tooltip why it's red
          return tr( "Invalid relation" );
        }
      }

      return QVariant();
    }

    case Qt::DecorationRole:
      return item->icon();

    case Qt::BackgroundRole:
    {
      if ( item->type() == QgsAttributesFormData::Container )
        return QBrush( QColor( 140, 140, 140, 50 ) );

      return QVariant();
    }

    case Qt::ForegroundRole:
    {
      if ( item->type() == QgsAttributesFormData::Field )
      {
        if ( invalidField )
        {
          return QBrush( QColor( 255, 0, 0 ) );
        }
        else if ( showAliases() && item->displayName().isEmpty() )
        {
          return QBrush( QColor( Qt::lightGray ) );
        }
      }

      if ( item->type() == QgsAttributesFormData::Relation )
      {
        if ( invalidRelation )
        {
          return QBrush( QColor( 255, 0, 0 ) );
        }
        else if ( showAliases() && item->displayName().isEmpty() )
        {
          return QBrush( QColor( Qt::lightGray ) );
        }
      }

      return QVariant();
    }

    case Qt::FontRole:
    {
      if ( item->type() == QgsAttributesFormData::Field )
      {
        if ( !invalidField && showAliases() && item->displayName().isEmpty() )
        {
          QFont font = QFont();
          font.setItalic( true );
          return font;
        }
      }

      if ( item->type() == QgsAttributesFormData::Relation )
      {
        if ( !invalidRelation && showAliases() && item->displayName().isEmpty() )
        {
          QFont font = QFont();
          font.setItalic( true );
          return font;
        }
      }

      return QVariant();
    }

    case ItemDataRole:
    case ItemNameRole:
    case ItemIdRole:
    case ItemTypeRole:
    case ItemDisplayRole:
      return item->data( role );

    default:
      return QVariant();
  }
}

bool QgsAttributesFormLayoutModel::removeRows( int row, int count, const QModelIndex &parent )
{
  if ( row < 0 )
    return false;

  QgsAttributesFormItem *item = itemForIndex( parent );

  if ( row > item->childCount() - count )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );
  for ( int r = 0; r < count; ++r )
    item->deleteChildAtIndex( row );
  endRemoveRows();
  return true;
}

bool QgsAttributesFormLayoutModel::removeRow( int row, const QModelIndex &parent )
{
  beginRemoveRows( parent, row, row );
  QgsAttributesFormItem *item = itemForIndex( parent );
  item->deleteChildAtIndex( row );
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
  return QStringList() << u"application/x-qgsattributesformlayoutelement"_s << u"application/x-qgsattributesformavailablewidgetsrelement"_s;
}

QModelIndexList QgsAttributesFormLayoutModel::curateIndexesForMimeData( const QModelIndexList &indexes ) const
{
  QModelIndexList containerList;
  for ( const auto index : indexes )
  {
    const auto indexType = static_cast< QgsAttributesFormData::AttributesFormItemType >( index.data( QgsAttributesFormModel::ItemTypeRole ).toInt() );
    if ( indexType == QgsAttributesFormData::Container )
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
  std::sort( curatedIndexes.begin(), curatedIndexes.end(), [this]( const QModelIndex &a, const QModelIndex &b ) {
    return indexLessThan( a, b );
  } );

  for ( const QModelIndex &index : std::as_const( curatedIndexes ) )
  {
    if ( index.isValid() )
    {
      QDomDocument doc;

      QDomElement rootElem = doc.createElement( u"form_layout_mime"_s );
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
  bool isDropSuccessful = false;
  int rows = 0;

  if ( row == -1 )            // Dropped at invalid index
    row = rowCount( parent ); // Let's append the item

  if ( action == Qt::IgnoreAction )
  {
    isDropSuccessful = true;
  }
  else if ( data->hasFormat( u"application/x-qgsattributesformavailablewidgetsrelement"_s ) )
  {
    Q_ASSERT( action == Qt::CopyAction ); // External drop
    QByteArray itemData = data->data( u"application/x-qgsattributesformavailablewidgetsrelement"_s );
    QDataStream stream( &itemData, QIODevice::ReadOnly );

    while ( !stream.atEnd() )
    {
      QString itemId;
      int itemTypeInt;
      QString itemName;
      stream >> itemId >> itemTypeInt >> itemName;

      const auto itemType = static_cast< QgsAttributesFormData::AttributesFormItemType >( itemTypeInt );
      insertChild( parent, row + rows, itemId, itemType, itemName );

      isDropSuccessful = true;

      QModelIndex addedIndex = index( row + rows, 0, parent );
      emit externalItemDropped( addedIndex );

      rows++;
    }
  }
  else if ( data->hasFormat( u"application/x-qgsattributesformlayoutelement"_s ) )
  {
    Q_ASSERT( action == Qt::MoveAction ); // Internal move
    QByteArray itemData = data->data( u"application/x-qgsattributesformlayoutelement"_s );
    QDataStream stream( &itemData, QIODevice::ReadOnly );

    while ( !stream.atEnd() )
    {
      QString text;
      stream >> text;

      QDomDocument doc;
      if ( !doc.setContent( text ) )
        continue;
      const QDomElement rootElem = doc.documentElement();
      if ( rootElem.tagName() != "form_layout_mime"_L1 || !rootElem.hasChildNodes() )
        continue;
      const QDomElement childElem = rootElem.firstChild().toElement();

      // Build editor element from XML and add/insert it to parent
      QgsAttributeEditorElement *editor = QgsAttributeEditorElement::create( childElem, mLayer->id(), mLayer->fields(), QgsReadWriteContext() );
      beginInsertRows( parent, row + rows, row + rows );
      loadAttributeEditorElementItem( editor, itemForIndex( parent ), row + rows );
      endInsertRows();

      isDropSuccessful = true;

      QModelIndex addedIndex = index( row + rows, 0, parent );
      emit internalItemDropped( addedIndex );

      rows++;
    }
  }

  return isDropSuccessful;
}

void QgsAttributesFormLayoutModel::updateFieldConfigForFieldItemsRecursive( QgsAttributesFormItem *parent, const QString &fieldName, const QgsAttributesFormData::FieldConfig &config )
{
  for ( int i = 0; i < parent->childCount(); i++ )
  {
    QgsAttributesFormItem *child = parent->child( i );
    if ( child->name() == fieldName && child->type() == QgsAttributesFormData::Field )
    {
      child->setData( ItemFieldConfigRole, QVariant::fromValue( config ) );
      child->setIcon( QgsGui::instance()->editorWidgetRegistry()->icon( config.mEditorWidgetType ) );
      emit fieldConfigDataChanged( child ); // Item's field config has changed, let views know about it
    }

    if ( child->childCount() > 0 )
    {
      updateFieldConfigForFieldItemsRecursive( child, fieldName, config );
    }
  }
}

void QgsAttributesFormLayoutModel::updateFieldConfigForFieldItems( const QString &fieldName, const QgsAttributesFormData::FieldConfig &config )
{
  updateFieldConfigForFieldItemsRecursive( mRootItem.get(), fieldName, config );
}

void QgsAttributesFormLayoutModel::updateAliasForFieldItemsRecursive( QgsAttributesFormItem *parent, const QString &fieldName, const QString &fieldAlias )
{
  for ( int i = 0; i < parent->childCount(); i++ )
  {
    QgsAttributesFormItem *child = parent->child( i );
    if ( child->name() == fieldName && child->type() == QgsAttributesFormData::Field )
    {
      child->setData( ItemDisplayRole, fieldAlias );
      const QModelIndex index = createIndex( child->row(), 0, child );
      emit dataChanged( index, index ); // Item's alias has changed, let views know about it
    }

    if ( child->childCount() > 0 )
    {
      updateAliasForFieldItemsRecursive( child, fieldName, fieldAlias );
    }
  }
}

void QgsAttributesFormLayoutModel::updateAliasForFieldItems( const QString &fieldName, const QString &fieldAlias )
{
  updateAliasForFieldItemsRecursive( mRootItem.get(), fieldName, fieldAlias );
}

QList< QgsAddAttributeFormContainerDialog::ContainerPair > QgsAttributesFormLayoutModel::recursiveListOfContainers( QgsAttributesFormItem *parent ) const
{
  QList< QgsAddAttributeFormContainerDialog::ContainerPair > containerList;
  for ( int i = 0; i < parent->childCount(); i++ )
  {
    QgsAttributesFormItem *child = parent->child( i );
    if ( child->type() == QgsAttributesFormData::Container )
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

  const QgsAttributesFormData::AttributeFormItemData itemData = index.data( QgsAttributesFormModel::ItemDataRole ).value<QgsAttributesFormData::AttributeFormItemData>();
  const int indexType = static_cast< QgsAttributesFormData::AttributesFormItemType >( index.data( QgsAttributesFormModel::ItemTypeRole ).toInt() );
  const QString indexName = index.data( QgsAttributesFormModel::ItemNameRole ).toString();
  const QString indexId = index.data( QgsAttributesFormModel::ItemIdRole ).toString();

  switch ( indexType )
  {
    case QgsAttributesFormData::Field:
    {
      const int fieldIndex = mLayer->fields().lookupField( indexName );
      widgetDef = new QgsAttributeEditorField( indexName, fieldIndex, parent );
      break;
    }

    case QgsAttributesFormData::Action:
    {
      const QgsAction action { mLayer->actions()->action( indexId ) };
      widgetDef = new QgsAttributeEditorAction( action, parent );
      break;
    }

    case QgsAttributesFormData::Relation:
    {
      const QgsRelation relation = mProject->relationManager()->relation( indexId );

      QgsAttributeEditorRelation *relDef = new QgsAttributeEditorRelation( relation, parent );
      const QgsAttributesFormData::RelationEditorConfiguration relationEditorConfig = itemData.relationEditorConfiguration();
      relDef->setRelationWidgetTypeId( relationEditorConfig.mRelationWidgetType );
      relDef->setRelationEditorConfiguration( relationEditorConfig.mRelationWidgetConfig );
      relDef->setNmRelationId( relationEditorConfig.nmRelationId );
      relDef->setForceSuppressFormPopup( relationEditorConfig.forceSuppressFormPopup );
      relDef->setLabel( relationEditorConfig.label );
      widgetDef = relDef;
      break;
    }

    case QgsAttributesFormData::Container:
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

    case QgsAttributesFormData::QmlWidget:
    {
      QgsAttributeEditorQmlElement *element = new QgsAttributeEditorQmlElement( indexName, parent );
      element->setQmlCode( itemData.qmlElementEditorConfiguration().qmlCode );
      widgetDef = element;
      break;
    }

    case QgsAttributesFormData::HtmlWidget:
    {
      QgsAttributeEditorHtmlElement *element = new QgsAttributeEditorHtmlElement( indexName, parent );
      element->setHtmlCode( itemData.htmlElementEditorConfiguration().htmlCode );
      widgetDef = element;
      break;
    }

    case QgsAttributesFormData::TextWidget:
    {
      QgsAttributeEditorTextElement *element = new QgsAttributeEditorTextElement( indexName, parent );
      element->setText( itemData.textElementEditorConfiguration().text );
      widgetDef = element;
      break;
    }

    case QgsAttributesFormData::SpacerWidget:
    {
      QgsAttributeEditorSpacerElement *element = new QgsAttributeEditorSpacerElement( indexName, parent );
      element->setDrawLine( itemData.spacerElementEditorConfiguration().drawLine );
      widgetDef = element;
      break;
    }

    case QgsAttributesFormData::WidgetType:
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
  return recursiveListOfContainers( mRootItem.get() );
}

void QgsAttributesFormLayoutModel::addContainer( QModelIndex &parent, const QString &name, int columnCount, Qgis::AttributeEditorContainerType type )
{
  beginInsertRows( parent, rowCount( parent ), rowCount( parent ) );

  QgsAttributesFormItem *parentItem = itemForIndex( parent );

  auto containerItem = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::Container, name, QString(), parentItem );

  QgsAttributesFormData::AttributeFormItemData itemData;
  itemData.setColumnCount( columnCount );
  itemData.setContainerType( parent.isValid() ? type : Qgis::AttributeEditorContainerType::Tab );

  containerItem->setData( QgsAttributesFormModel::ItemDataRole, itemData );
  containerItem->setData( QgsAttributesFormModel::ItemIdRole, name ); // Make it searchable
  parentItem->addChild( std::move( containerItem ) );

  endInsertRows();
}

void QgsAttributesFormLayoutModel::insertChild( const QModelIndex &parent, int row, const QString &itemId, QgsAttributesFormData::AttributesFormItemType itemType, const QString &itemName )
{
  if ( row < 0 )
    return;

  beginInsertRows( parent, row, row );
  auto item = std::make_unique< QgsAttributesFormItem >();

  item->setData( QgsAttributesFormModel::ItemIdRole, itemId );
  item->setData( QgsAttributesFormModel::ItemTypeRole, itemType );
  item->setData( QgsAttributesFormModel::ItemNameRole, itemName );

  itemForIndex( parent )->insertChild( row, std::move( item ) );
  endInsertRows();
}


QgsAttributesFormProxyModel::QgsAttributesFormProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
}

void QgsAttributesFormProxyModel::setAttributesFormSourceModel( QgsAttributesFormModel *model )
{
  mModel = model;
  QSortFilterProxyModel::setSourceModel( mModel );
}

QgsAttributesFormModel *QgsAttributesFormProxyModel::sourceAttributesFormModel() const
{
  return mModel;
}

const QString QgsAttributesFormProxyModel::filterText() const
{
  return mFilterText;
}

void QgsAttributesFormProxyModel::setFilterText( const QString &filterText )
{
  // Since we want to allow refreshing the filter when, e.g.,
  // users switch to aliases, then we allow this method to be
  // executed even if previous and new filters are equal

  mFilterText = filterText.trimmed();
  invalidate();
}

bool QgsAttributesFormProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  if ( mFilterText.isEmpty() )
    return true;

  QModelIndex sourceIndex = sourceModel()->index( sourceRow, 0, sourceParent );
  if ( !sourceIndex.isValid() )
    return false;

  // If name or alias match, accept it before any other checks
  if ( sourceIndex.data( QgsAttributesFormModel::ItemNameRole ).toString().contains( mFilterText, Qt::CaseInsensitive ) || sourceIndex.data( QgsAttributesFormModel::ItemDisplayRole ).toString().contains( mFilterText, Qt::CaseInsensitive ) )
    return true;

  // Child is accepted if any of its parents is accepted
  QModelIndex parent = sourceIndex.parent();
  while ( parent.isValid() )
  {
    if ( parent.data( QgsAttributesFormModel::ItemNameRole ).toString().contains( mFilterText, Qt::CaseInsensitive ) || parent.data( QgsAttributesFormModel::ItemDisplayRole ).toString().contains( mFilterText, Qt::CaseInsensitive ) )
      return true;

    parent = parent.parent();
  }

  return false;
}
