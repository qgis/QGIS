/***************************************************************************
    qgsexpressiontreeview.cpp
     --------------------------------------
    Date                 : march 2020 - quarantine day 9
    Copyright            : (C) 2020 by Denis Rouzaud
    Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMenu>
#include <QMessageBox>
#include <QVersionNumber>

#include "qgsexpressiontreeview.h"
#include "qgis.h"
#include "qgsfieldformatterregistry.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"
#include "qgssettings.h"
#include "qgsrelationmanager.h"
#include "qgsapplication.h"
#include "qgsiconutils.h"


//! Returns a HTML formatted string for use as a \a relation item help.
QString formatRelationHelp( const QgsRelation &relation )
{
  QString text = QStringLiteral( "<h3>%1</h3>\n<div class=\"description\"><p>%2</p></div>" )
                 .arg( QCoreApplication::translate( "relation_help", "relation %1" ).arg( relation.name() ),
                       QObject::tr( "Inserts the relation ID for the relation named '%1'." ).arg( relation.name() ) );

  text += QStringLiteral( "<h4>%1</h4><div class=\"description\"><pre>%2</pre></div>" )
          .arg( QObject::tr( "Current value" ), relation.id() );

  return text;
}


//! Returns a HTML formatted string for use as a \a layer item help.
QString formatLayerHelp( const QgsMapLayer *layer )
{
  QString text = QStringLiteral( "<h3>%1</h3>\n<div class=\"description\"><p>%2</p></div>" )
                 .arg( QCoreApplication::translate( "layer_help", "map layer %1" ).arg( layer->name() ),
                       QObject::tr( "Inserts the layer ID for the layer named '%1'." ).arg( layer->name() ) );

  text += QStringLiteral( "<h4>%1</h4><div class=\"description\"><pre>%2</pre></div>" )
          .arg( QObject::tr( "Current value" ), layer->id() );

  return text;
}

//! Returns a HTML formatted string for use as a \a recent \a expression item help.
QString formatRecentExpressionHelp( const QString &label, const QString &expression )
{
  QString text = QStringLiteral( "<h3>%1</h3>\n<div class=\"description\"><p>%2</p></div>" )
                 .arg( QCoreApplication::translate( "recent_expression_help", "expression %1" ).arg( label ),
                       QCoreApplication::translate( "recent_expression_help", "Recently used expression." ) );

  text += QStringLiteral( "<h4>%1</h4><div class=\"description\"><pre>%2</pre></div>" )
          .arg( QObject::tr( "Expression" ), expression );

  return text;
}

//! Returns a HTML formatted string for use as a \a user \a expression item help.
QString formatUserExpressionHelp( const QString &label, const QString &expression, const QString &description )
{
  QString text = QStringLiteral( "<h3>%1</h3>\n<div class=\"description\"><p>%2</p></div>" )
                 .arg( QCoreApplication::translate( "user_expression_help", "expression %1" ).arg( label ), description );

  text += QStringLiteral( "<h4>%1</h4><div class=\"description\"><pre>%2</pre></div>" )
          .arg( QObject::tr( "Expression" ), expression );

  return text;
}

//! Returns a HTML formatted string for use as a \a variable item help.
QString formatVariableHelp( const QString &variable, const QString &description, bool showValue, const QVariant &value )
{
  QString text = QStringLiteral( "<h3>%1</h3>\n<div class=\"description\"><p>%2</p></div>" )
                 .arg( QCoreApplication::translate( "variable_help", "variable %1" ).arg( variable ), description );

  if ( showValue )
  {
    QString valueString = !value.isValid()
                          ? QCoreApplication::translate( "variable_help", "not set" )
                          : QStringLiteral( "<pre>%1</pre>" ).arg( QgsExpression::formatPreviewString( value ) );

    text += QStringLiteral( "<h4>%1</h4><div class=\"description\"><p>%2</p></div>" )
            .arg( QObject::tr( "Current value" ), valueString );
  }

  return text;
}


// ****************************
// ****************************
// QgsExpressionTreeView
// ****************************


QgsExpressionTreeView::QgsExpressionTreeView( QWidget *parent )
  : QTreeView( parent )
  , mProject( QgsProject::instance() )
{
  connect( this, &QTreeView::doubleClicked, this, &QgsExpressionTreeView::onDoubleClicked );

  mModel = std::make_unique<QStandardItemModel>();
  mProxyModel = std::make_unique<QgsExpressionItemSearchProxy>();
  mProxyModel->setDynamicSortFilter( true );
  mProxyModel->setSourceModel( mModel.get() );
  setModel( mProxyModel.get() );
  setSortingEnabled( true );
  sortByColumn( 0, Qt::AscendingOrder );

  setSelectionMode( QAbstractItemView::SelectionMode::SingleSelection );

  setContextMenuPolicy( Qt::CustomContextMenu );
  connect( this, &QWidget::customContextMenuRequested, this, &QgsExpressionTreeView::showContextMenu );
  connect( selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsExpressionTreeView::currentItemChanged );

  updateFunctionTree();
  loadUserExpressions();

  // select the first item in the function list
  // in order to avoid a blank help widget
  QModelIndex firstItem = mProxyModel->index( 0, 0, QModelIndex() );
  setCurrentIndex( firstItem );
}

void QgsExpressionTreeView::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;

  //TODO - remove existing layer scope from context

  if ( mLayer )
    mExpressionContext << QgsExpressionContextUtils::layerScope( mLayer );

  loadFieldNames();
}

void QgsExpressionTreeView::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;
  updateFunctionTree();
  loadFieldNames();
  loadRecent( mRecentKey );
  loadUserExpressions( );
}

void QgsExpressionTreeView::setMenuProvider( QgsExpressionTreeView::MenuProvider *provider )
{
  mMenuProvider = provider;
}

void QgsExpressionTreeView::refresh()
{
  updateFunctionTree();
  loadFieldNames();
  loadRecent( mRecentKey );
  loadUserExpressions( );
}

QgsExpressionItem *QgsExpressionTreeView::currentItem() const
{
  QModelIndex idx = mProxyModel->mapToSource( currentIndex() );
  QgsExpressionItem *item = static_cast<QgsExpressionItem *>( mModel->itemFromIndex( idx ) );
  return item;
}

QStandardItemModel *QgsExpressionTreeView::model()
{
  return mModel.get();
}

QgsProject *QgsExpressionTreeView::project()
{
  return mProject;
}

void QgsExpressionTreeView::setProject( QgsProject *project )
{
  mProject = project;
  updateFunctionTree();
}


void QgsExpressionTreeView::setSearchText( const QString &text )
{
  mProxyModel->setFilterString( text );
  if ( text.isEmpty() )
  {
    collapseAll();
  }
  else
  {
    expandAll();
    QModelIndex index = mProxyModel->index( 0, 0 );
    if ( mProxyModel->hasChildren( index ) )
    {
      QModelIndex child = mProxyModel->index( 0, 0, index );
      selectionModel()->setCurrentIndex( child, QItemSelectionModel::ClearAndSelect );
    }
  }
}

void QgsExpressionTreeView::onDoubleClicked( const QModelIndex &index )
{
  QModelIndex idx = mProxyModel->mapToSource( index );
  QgsExpressionItem *item = static_cast<QgsExpressionItem *>( mModel->itemFromIndex( idx ) );
  if ( !item )
    return;

  // Don't handle the double-click if we are on a header node.
  if ( item->getItemType() == QgsExpressionItem::Header )
    return;

  emit expressionItemDoubleClicked( item->getExpressionText() );
}

void QgsExpressionTreeView::showContextMenu( QPoint pt )
{
  QModelIndex idx = indexAt( pt );
  idx = mProxyModel->mapToSource( idx );
  QgsExpressionItem *item = static_cast<QgsExpressionItem *>( mModel->itemFromIndex( idx ) );
  if ( !item )
    return;

  if ( !mMenuProvider )
    return;

  QMenu *menu = mMenuProvider->createContextMenu( item );

  if ( menu )
    menu->popup( mapToGlobal( pt ) );
}

void QgsExpressionTreeView::currentItemChanged( const QModelIndex &index, const QModelIndex & )
{
  // Get the item
  QModelIndex idx = mProxyModel->mapToSource( index );
  QgsExpressionItem *item = static_cast<QgsExpressionItem *>( mModel->itemFromIndex( idx ) );
  if ( !item )
    return;

  emit currentExpressionItemChanged( item );
}

void QgsExpressionTreeView::updateFunctionTree()
{
  mModel->clear();
  mExpressionGroups.clear();

  // TODO Can we move this stuff to QgsExpression, like the functions?
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "+" ), QStringLiteral( " + " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "-" ), QStringLiteral( " - " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "*" ), QStringLiteral( " * " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "/" ), QStringLiteral( " / " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "%" ), QStringLiteral( " % " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "^" ), QStringLiteral( " ^ " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "=" ), QStringLiteral( " = " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "~" ), QStringLiteral( " ~ " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( ">" ), QStringLiteral( " > " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "<" ), QStringLiteral( " < " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "<>" ), QStringLiteral( " <> " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "<=" ), QStringLiteral( " <= " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( ">=" ), QStringLiteral( " >= " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "[]" ), QStringLiteral( "[ ]" ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "||" ), QStringLiteral( " || " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "BETWEEN" ), QStringLiteral( " BETWEEN " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "NOT BETWEEN" ), QStringLiteral( " NOT BETWEEN " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "IN" ), QStringLiteral( " IN " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "LIKE" ), QStringLiteral( " LIKE " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "ILIKE" ), QStringLiteral( " ILIKE " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "IS" ), QStringLiteral( " IS " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "IS NOT" ), QStringLiteral( " IS NOT " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "OR" ), QStringLiteral( " OR " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "AND" ), QStringLiteral( " AND " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "NOT" ), QStringLiteral( " NOT " ) );

  QString casestring = QStringLiteral( "CASE WHEN condition THEN result END" );
  registerItem( QStringLiteral( "Conditionals" ), QStringLiteral( "CASE" ), casestring );

  // use -1 as sort order here -- NULL should always show before the field list
  registerItem( QStringLiteral( "Fields and Values" ), QStringLiteral( "NULL" ), QStringLiteral( "NULL" ), QString(), QgsExpressionItem::ExpressionNode, false, -1 );

  // Load the functions from the QgsExpression class
  int count = QgsExpression::functionCount();
  for ( int i = 0; i < count; i++ )
  {
    QgsExpressionFunction *func = QgsExpression::Functions()[i];
    QString name = func->name();
    if ( name.startsWith( '_' ) ) // do not display private functions
      continue;
    if ( func->isDeprecated() ) // don't show deprecated functions
      continue;
    if ( func->isContextual() )
    {
      //don't show contextual functions by default - it's up the the QgsExpressionContext
      //object to provide them if supported
      continue;
    }
    if ( func->params() != 0 )
      name += '(';
    else if ( !name.startsWith( '$' ) )
      name += QLatin1String( "()" );
    // this is where the functions are being registered, including functions under "Custom"
    registerItemForAllGroups( func->groups(), func->name(), ' ' + name + ' ', func->helpText(), QgsExpressionItem::ExpressionNode, mExpressionContext.isHighlightedFunction( func->name() ), 1, QgsExpression::tags( func->name() ) );
  }

  // load relation names
  loadRelations();

  // load layer IDs
  loadLayers();

  loadExpressionContext();
}

QgsExpressionItem *QgsExpressionTreeView::registerItem( const QString &group,
    const QString &label,
    const QString &expressionText,
    const QString &helpText,
    QgsExpressionItem::ItemType type, bool highlightedItem, int sortOrder, const QIcon &icon, const QStringList &tags, const QString &name )
{
  QgsExpressionItem *item = new QgsExpressionItem( label, expressionText, helpText, type );
  item->setData( label, Qt::UserRole );
  item->setData( sortOrder, QgsExpressionItem::CUSTOM_SORT_ROLE );
  item->setData( tags, QgsExpressionItem::SEARCH_TAGS_ROLE );
  item->setData( name, QgsExpressionItem::ITEM_NAME_ROLE );
  item->setIcon( icon );

  // Look up the group and insert the new function.
  if ( mExpressionGroups.contains( group ) )
  {
    QgsExpressionItem *groupNode = mExpressionGroups.value( group );
    groupNode->appendRow( item );
  }
  else
  {
    // If the group doesn't exist yet we make it first.
    QgsExpressionItem *newgroupNode = new QgsExpressionItem( QgsExpression::group( group ), QString(), QgsExpressionItem::Header );
    newgroupNode->setData( group, Qt::UserRole );
    //Recent group should always be last group
    newgroupNode->setData( group.startsWith( QLatin1String( "Recent (" ) ) ? 2 : 1, QgsExpressionItem::CUSTOM_SORT_ROLE );
    newgroupNode->appendRow( item );
    newgroupNode->setBackground( QBrush( QColor( 150, 150, 150, 150 ) ) );
    mModel->appendRow( newgroupNode );
    mExpressionGroups.insert( group, newgroupNode );
  }

  if ( highlightedItem )
  {
    //insert a copy as a top level item
    QgsExpressionItem *topLevelItem = new QgsExpressionItem( label, expressionText, helpText, type );
    topLevelItem->setData( label, Qt::UserRole );
    item->setData( 0, QgsExpressionItem::CUSTOM_SORT_ROLE );
    QFont font = topLevelItem->font();
    font.setBold( true );
    topLevelItem->setFont( font );
    mModel->appendRow( topLevelItem );
  }
  return item;
}

void QgsExpressionTreeView::registerItemForAllGroups( const QStringList &groups, const QString &label, const QString &expressionText, const QString &helpText, QgsExpressionItem::ItemType type, bool highlightedItem, int sortOrder, const QStringList &tags )
{
  const auto constGroups = groups;
  for ( const QString &group : constGroups )
  {
    registerItem( group, label, expressionText, helpText, type, highlightedItem, sortOrder, QIcon(), tags );
  }
}

void QgsExpressionTreeView::loadExpressionContext()
{
  QStringList variableNames = mExpressionContext.filteredVariableNames();
  const auto constVariableNames = variableNames;
  for ( const QString &variable : constVariableNames )
  {
    registerItem( QStringLiteral( "Variables" ), variable, " @" + variable + ' ',
                  formatVariableHelp( variable, mExpressionContext.description( variable ), true, mExpressionContext.variable( variable ) ),
                  QgsExpressionItem::ExpressionNode,
                  mExpressionContext.isHighlightedVariable( variable ) );
  }

  // Load the functions from the expression context
  QStringList contextFunctions = mExpressionContext.functionNames();
  const auto constContextFunctions = contextFunctions;
  for ( const QString &functionName : constContextFunctions )
  {
    QgsExpressionFunction *func = mExpressionContext.function( functionName );
    QString name = func->name();
    if ( name.startsWith( '_' ) ) // do not display private functions
      continue;
    if ( func->params() != 0 )
      name += '(';
    registerItemForAllGroups( func->groups(), func->name(), ' ' + name + ' ', func->helpText(), QgsExpressionItem::ExpressionNode, mExpressionContext.isHighlightedFunction( func->name() ), 1, QgsExpression::tags( func->name() ) );
  }
}

void QgsExpressionTreeView::loadLayers()
{
  if ( !mProject )
    return;

  QMap<QString, QgsMapLayer *> layers = mProject->mapLayers();
  QMap<QString, QgsMapLayer *>::const_iterator layerIt = layers.constBegin();
  for ( ; layerIt != layers.constEnd(); ++layerIt )
  {
    QIcon icon = QgsIconUtils::iconForLayer( layerIt.value() );
    QgsExpressionItem *parentItem = registerItem( QStringLiteral( "Map Layers" ), layerIt.value()->name(), QStringLiteral( "'%1'" ).arg( layerIt.key() ), formatLayerHelp( layerIt.value() ), QgsExpressionItem::ExpressionNode, false, 99, icon );
    loadLayerFields( qobject_cast<QgsVectorLayer *>( layerIt.value() ), parentItem );
  }

}

void QgsExpressionTreeView::loadLayerFields( QgsVectorLayer *layer, QgsExpressionItem *parentItem )
{
  if ( !layer )
    return;

  const QgsFields fields = layer->fields();
  for ( int fieldIdx = 0; fieldIdx < fields.count(); ++fieldIdx )
  {
    const QgsField field = fields.at( fieldIdx );
    QIcon icon = fields.iconForField( fieldIdx );
    const QString label { field.displayNameWithAlias() };
    QgsExpressionItem *item = new QgsExpressionItem( label, " '" + field.name() + "' ", QString(), QgsExpressionItem::Field );
    item->setData( label, Qt::UserRole );
    item->setData( 99, QgsExpressionItem::CUSTOM_SORT_ROLE );
    item->setData( QStringList(), QgsExpressionItem::SEARCH_TAGS_ROLE );
    item->setData( field.name(), QgsExpressionItem::ITEM_NAME_ROLE );
    item->setData( layer->id(), QgsExpressionItem::LAYER_ID_ROLE );
    item->setIcon( icon );
    parentItem->appendRow( item );
  }
}

void QgsExpressionTreeView::loadFieldNames( const QgsFields &fields )
{
  for ( int i = 0; i < fields.count(); ++i )
  {
    const QgsField field = fields.at( i );
    QIcon icon = fields.iconForField( i );
    registerItem( QStringLiteral( "Fields and Values" ), field.displayNameWithAlias(),
                  " \"" + field.name() + "\" ", QString(), QgsExpressionItem::Field, false, i, icon, QStringList(), field.name() );
  }
}

void QgsExpressionTreeView::loadFieldNames()
{
  // Cleanup
  if ( mExpressionGroups.contains( QStringLiteral( "Fields and Values" ) ) )
  {
    QgsExpressionItem *node = mExpressionGroups.value( QStringLiteral( "Fields and Values" ) );
    node->removeRows( 0, node->rowCount() );
    // Re-add NULL
    // use -1 as sort order here -- NULL should always show before the field list
    registerItem( QStringLiteral( "Fields and Values" ), QStringLiteral( "NULL" ), QStringLiteral( "NULL" ), QString(), QgsExpressionItem::ExpressionNode, false, -1 );
  }

  // this can happen if fields are manually set
  if ( !mLayer )
    return;

  const QgsFields &fields = mLayer->fields();

  loadFieldNames( fields );
}

void QgsExpressionTreeView::loadRelations()
{
  if ( !mProject )
    return;

  QMap<QString, QgsRelation> relations = mProject->relationManager()->relations();
  QMap<QString, QgsRelation>::const_iterator relIt = relations.constBegin();
  for ( ; relIt != relations.constEnd(); ++relIt )
  {
    registerItemForAllGroups( QStringList() << tr( "Relations" ), relIt->name(), QStringLiteral( "'%1'" ).arg( relIt->id() ), formatRelationHelp( relIt.value() ) );
  }
}

void QgsExpressionTreeView::loadRecent( const QString &collection )
{
  mRecentKey = collection;
  QString name = tr( "Recent (%1)" ).arg( collection );
  if ( mExpressionGroups.contains( name ) )
  {
    QgsExpressionItem *node = mExpressionGroups.value( name );
    node->removeRows( 0, node->rowCount() );
  }

  QgsSettings settings;
  const QString location = QStringLiteral( "/expressions/recent/%1" ).arg( collection );
  const QStringList expressions = settings.value( location ).toStringList();
  int i = 0;
  for ( const QString &expression : expressions )
  {
    QString help = formatRecentExpressionHelp( expression, expression );
    QString label = expression;
    label.replace( '\n', ' ' );
    registerItem( name, label, expression, help, QgsExpressionItem::ExpressionNode, false, i );
    i++;
  }
}

void QgsExpressionTreeView::saveToRecent( const QString &expressionText, const QString &collection )
{
  QgsSettings settings;
  QString location = QStringLiteral( "/expressions/recent/%1" ).arg( collection );
  QStringList expressions = settings.value( location ).toStringList();
  expressions.removeAll( expressionText );

  expressions.prepend( expressionText );

  while ( expressions.count() > 20 )
  {
    expressions.pop_back();
  }

  settings.setValue( location, expressions );
  loadRecent( collection );
}

void QgsExpressionTreeView::saveToUserExpressions( const QString &label, const QString &expression, const QString &helpText )
{
  QgsSettings settings;
  const QString location = QStringLiteral( "user" );
  settings.beginGroup( location, QgsSettings::Section::Expressions );
  settings.beginGroup( label );
  settings.setValue( QStringLiteral( "expression" ), expression );
  settings.setValue( QStringLiteral( "helpText" ), helpText );
  loadUserExpressions( );
  // Scroll
  const QModelIndexList idxs { mModel->match( mModel->index( 0, 0 ), Qt::DisplayRole, label, 1, Qt::MatchFlag::MatchRecursive ) };
  if ( ! idxs.isEmpty() )
  {
    scrollTo( idxs.first() );
  }
}

void QgsExpressionTreeView::removeFromUserExpressions( const QString &label )
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "user/%1" ).arg( label ), QgsSettings::Section::Expressions );
  loadUserExpressions( );
}

// this is potentially very slow if there are thousands of user expressions, every time entire cleanup and load
void QgsExpressionTreeView::loadUserExpressions( )
{
  // Cleanup
  if ( mExpressionGroups.contains( QStringLiteral( "UserGroup" ) ) )
  {
    QgsExpressionItem *node = mExpressionGroups.value( QStringLiteral( "UserGroup" ) );
    node->removeRows( 0, node->rowCount() );
  }

  QgsSettings settings;
  const QString location = QStringLiteral( "user" );
  settings.beginGroup( location, QgsSettings::Section::Expressions );
  QString helpText;
  QString expression;
  int i = 0;
  mUserExpressionLabels = settings.childGroups();
  for ( const auto &label : std::as_const( mUserExpressionLabels ) )
  {
    settings.beginGroup( label );
    expression = settings.value( QStringLiteral( "expression" ) ).toString();
    helpText = formatUserExpressionHelp( label, expression, settings.value( QStringLiteral( "helpText" ) ).toString() );
    registerItem( QStringLiteral( "UserGroup" ), label, expression, helpText, QgsExpressionItem::ExpressionNode, false, i++ );
    settings.endGroup();
  }
}

QStringList QgsExpressionTreeView::userExpressionLabels() const
{
  return mUserExpressionLabels;
}

QJsonDocument QgsExpressionTreeView::exportUserExpressions()
{
  const QString group = QStringLiteral( "user" );
  QgsSettings settings;
  QJsonArray exportList;
  QJsonObject exportObject
  {
    {"qgis_version", Qgis::version()},
    {"exported_at", QDateTime::currentDateTime().toString( Qt::ISODate )},
    {"author", QgsApplication::userFullName()},
    {"expressions", exportList}
  };

  settings.beginGroup( group, QgsSettings::Section::Expressions );

  mUserExpressionLabels = settings.childGroups();

  for ( const QString &label : std::as_const( mUserExpressionLabels ) )
  {
    settings.beginGroup( label );

    const QString expression = settings.value( QStringLiteral( "expression" ) ).toString();
    const QString helpText = settings.value( QStringLiteral( "helpText" ) ).toString();
    const QJsonObject expressionObject
    {
      {"name", label},
      {"type", "expression"},
      {"expression", expression},
      {"group", group},
      {"description", helpText}
    };
    exportList.push_back( expressionObject );

    settings.endGroup();
  }

  exportObject[QStringLiteral( "expressions" )] = exportList;
  QJsonDocument exportJson = QJsonDocument( exportObject );

  return exportJson;
}

void QgsExpressionTreeView::loadExpressionsFromJson( const QJsonDocument &expressionsDocument )
{
  // if the root of the json document is not an object, it means it's a wrong file
  if ( ! expressionsDocument.isObject() )
    return;

  QJsonObject expressionsObject = expressionsDocument.object();

  // validate json for manadatory fields
  if ( ! expressionsObject[QStringLiteral( "qgis_version" )].isString()
       || ! expressionsObject[QStringLiteral( "exported_at" )].isString()
       || ! expressionsObject[QStringLiteral( "author" )].isString()
       || ! expressionsObject[QStringLiteral( "expressions" )].isArray() )
    return;

  // validate versions
  QVersionNumber qgisJsonVersion = QVersionNumber::fromString( expressionsObject[QStringLiteral( "qgis_version" )].toString() );
  QVersionNumber qgisVersion = QVersionNumber::fromString( Qgis::version() );

  // if the expressions are from newer version of QGIS, we ask the user to confirm
  // they want to proceed
  if ( qgisJsonVersion > qgisVersion )
  {
    QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No;
    switch ( QMessageBox::question( this,
                                    tr( "QGIS Version Mismatch" ),
                                    tr( "The imported expressions are from newer version of QGIS (%1) "
                                        "and some of the expression might not work the current version (%2). "
                                        "Are you sure you want to continue?" ).arg( qgisJsonVersion.toString(), qgisVersion.toString() ), buttons ) )
    {
      case QMessageBox::No:
        return;

      case QMessageBox::Yes:
        break;

      default:
        break;
    }
  }

  // we store the number of
  QStringList skippedExpressionLabels;
  bool isApplyToAll = false;
  bool isOkToOverwrite = false;

  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "user" ), QgsSettings::Section::Expressions );
  mUserExpressionLabels = settings.childGroups();

  const QJsonArray expressions = expressionsObject[QStringLiteral( "expressions" )].toArray();
  for ( const QJsonValue && expressionValue : expressions )
  {
    // validate the type of the array element, can be anything
    if ( ! expressionValue.isObject() )
    {
      // try to stringify and put and indicator what happened
      skippedExpressionLabels.append( expressionValue.toString() );
      continue;
    }

    QJsonObject expressionObj = expressionValue.toObject();

    // make sure the required keys are the correct types
    if ( ! expressionObj[QStringLiteral( "name" )].isString()
         || ! expressionObj[QStringLiteral( "type" )].isString()
         || ! expressionObj[QStringLiteral( "expression" )].isString()
         || ! expressionObj[QStringLiteral( "group" )].isString()
         || ! expressionObj[QStringLiteral( "description" )].isString() )
    {
      // try to stringify and put an indicator what happened. Try to stringify the name, if fails, go with the expression.
      if ( ! expressionObj[QStringLiteral( "name" )].toString().isEmpty() )
        skippedExpressionLabels.append( expressionObj[QStringLiteral( "name" )].toString() );
      else
        skippedExpressionLabels.append( expressionObj[QStringLiteral( "expression" )].toString() );

      continue;
    }

    // we want to import only items of type expression for now
    if ( expressionObj[QStringLiteral( "type" )].toString() != QLatin1String( "expression" ) )
    {
      skippedExpressionLabels.append( expressionObj[QStringLiteral( "name" )].toString() );
      continue;
    }

    // we want to import only items of type expression for now
    if ( expressionObj[QStringLiteral( "group" )].toString() != QLatin1String( "user" ) )
    {
      skippedExpressionLabels.append( expressionObj[QStringLiteral( "name" )].toString() );
      continue;
    }

    const QString label = expressionObj[QStringLiteral( "name" )].toString();
    const QString expression = expressionObj[QStringLiteral( "expression" )].toString();
    const QString helpText = expressionObj[QStringLiteral( "description" )].toString();

    // make sure they have valid name
    if ( label.contains( QLatin1String( "\\" ) ) || label.contains( '/' ) )
    {
      skippedExpressionLabels.append( expressionObj[QStringLiteral( "name" )].toString() );
      continue;
    }

    settings.beginGroup( label );
    const QString oldExpression = settings.value( QStringLiteral( "expression" ) ).toString();
    settings.endGroup();

    // TODO would be nice to skip the cases when labels and expressions match
    if ( mUserExpressionLabels.contains( label ) && expression != oldExpression )
    {
      if ( ! isApplyToAll )
        showMessageBoxConfirmExpressionOverwrite( isApplyToAll, isOkToOverwrite, label, oldExpression, expression );

      if ( isOkToOverwrite )
        saveToUserExpressions( label, expression, helpText );
      else
      {
        skippedExpressionLabels.append( label );
        continue;
      }
    }
    else
    {
      saveToUserExpressions( label, expression, helpText );
    }
  }

  loadUserExpressions( );

  if ( ! skippedExpressionLabels.isEmpty() )
  {
    QStringList skippedExpressionLabelsQuoted;
    skippedExpressionLabelsQuoted.reserve( skippedExpressionLabels.size() );
    for ( const QString &skippedExpressionLabel : skippedExpressionLabels )
      skippedExpressionLabelsQuoted.append( QStringLiteral( "'%1'" ).arg( skippedExpressionLabel ) );

    QMessageBox::information( this,
                              tr( "Skipped Expression Imports" ),
                              QStringLiteral( "%1\n%2" ).arg( tr( "The following expressions have been skipped:" ),
                                  skippedExpressionLabelsQuoted.join( QLatin1String( ", " ) ) ) );
  }
}

const QList<QgsExpressionItem *> QgsExpressionTreeView::findExpressions( const QString &label )
{
  QList<QgsExpressionItem *> result;
  const QList<QStandardItem *> found { mModel->findItems( label, Qt::MatchFlag::MatchRecursive ) };
  result.reserve( found.size() );
  std::transform( found.begin(), found.end(), std::back_inserter( result ),
                  []( QStandardItem * item ) -> QgsExpressionItem* { return static_cast<QgsExpressionItem *>( item ); } );
  return result;
}

void QgsExpressionTreeView::showMessageBoxConfirmExpressionOverwrite(
  bool &isApplyToAll,
  bool &isOkToOverwrite,
  const QString &label,
  const QString &oldExpression,
  const QString &newExpression )
{
  QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll;
  switch ( QMessageBox::question( this,
                                  tr( "Expression Overwrite" ),
                                  tr( "The expression with label '%1' was already defined."
                                      "The old expression \"%2\" will be overwritten by \"%3\"."
                                      "Are you sure you want to overwrite the expression?" ).arg( label, oldExpression, newExpression ), buttons ) )
  {
    case QMessageBox::NoToAll:
      isApplyToAll = true;
      isOkToOverwrite = false;
      break;

    case QMessageBox::No:
      isApplyToAll = false;
      isOkToOverwrite = false;
      break;

    case QMessageBox::YesToAll:
      isApplyToAll = true;
      isOkToOverwrite = true;
      break;

    case QMessageBox::Yes:
      isApplyToAll = false;
      isOkToOverwrite = true;
      break;

    default:
      break;
  }
}


// ****************************
// ****************************
// QgsExpressionItemSearchProxy
// ****************************


QgsExpressionItemSearchProxy::QgsExpressionItemSearchProxy()
{
  setFilterCaseSensitivity( Qt::CaseInsensitive );
}

bool QgsExpressionItemSearchProxy::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
  const QgsExpressionItem::ItemType itemType = QgsExpressionItem::ItemType( sourceModel()->data( index, QgsExpressionItem::ITEM_TYPE_ROLE ).toInt() );

  if ( itemType == QgsExpressionItem::Header )
  {
    // show header if any child item matches
    int count = sourceModel()->rowCount( index );
    bool matchchild = false;
    for ( int i = 0; i < count; ++i )
    {
      if ( filterAcceptsRow( i, index ) )
      {
        matchchild = true;
        break;
      }
    }
    return matchchild;
  }

  // check match of item label or tags
  const QString name = sourceModel()->data( index, Qt::DisplayRole ).toString();
  if ( name.contains( mFilterString, Qt::CaseInsensitive ) )
  {
    return true;
  }

  const QStringList tags = sourceModel()->data( index, QgsExpressionItem::SEARCH_TAGS_ROLE ).toStringList();
  return std::any_of( tags.begin(), tags.end(), [this]( const QString & tag )
  {
    return tag.contains( mFilterString, Qt::CaseInsensitive );
  } );
}

void QgsExpressionItemSearchProxy::setFilterString( const QString &string )
{
  mFilterString = string;
  invalidate();
}

bool QgsExpressionItemSearchProxy::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  int leftSort = sourceModel()->data( left, QgsExpressionItem::CUSTOM_SORT_ROLE ).toInt();
  int rightSort = sourceModel()->data( right,  QgsExpressionItem::CUSTOM_SORT_ROLE ).toInt();
  if ( leftSort != rightSort )
    return leftSort < rightSort;

  QString leftString = sourceModel()->data( left, Qt::DisplayRole ).toString();
  QString rightString = sourceModel()->data( right, Qt::DisplayRole ).toString();

  //ignore $ prefixes when sorting
  if ( leftString.startsWith( '$' ) )
    leftString = leftString.mid( 1 );
  if ( rightString.startsWith( '$' ) )
    rightString = rightString.mid( 1 );

  return QString::localeAwareCompare( leftString, rightString ) < 0;
}
