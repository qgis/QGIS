/***************************************************************************
    qgsrulebasedlabelingwidget.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrulebasedlabelingwidget.h"

#include "qgsapplication.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgslabelinggui.h"
#include "qgsrulebasedlabeling.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"

#include <QClipboard>
#include <QMessageBox>

QgsRuleBasedLabelingWidget::QgsRuleBasedLabelingWidget( QgsVectorLayer* layer, QgsMapCanvas* canvas, QWidget* parent )
    : QgsPanelWidget( parent )
    , mLayer( layer )
    , mCanvas( canvas )
    , mRootRule( nullptr )
    , mModel( nullptr )
{
  setupUi( this );

  btnAddRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  btnEditRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.png" ) ) );
  btnRemoveRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );

  mCopyAction = new QAction( tr( "Copy" ), this );
  mCopyAction->setShortcut( QKeySequence( QKeySequence::Copy ) );
  mPasteAction = new QAction( tr( "Paste" ), this );
  mPasteAction->setShortcut( QKeySequence( QKeySequence::Paste ) );
  mDeleteAction = new QAction( tr( "Remove Rule" ), this );
  mDeleteAction->setShortcut( QKeySequence( QKeySequence::Delete ) );

  viewRules->addAction( mDeleteAction );
  viewRules->addAction( mCopyAction );
  viewRules->addAction( mPasteAction );

  connect( viewRules, SIGNAL( doubleClicked( const QModelIndex & ) ), this, SLOT( editRule( const QModelIndex & ) ) );

  connect( btnAddRule, SIGNAL( clicked() ), this, SLOT( addRule() ) );
  connect( btnEditRule, SIGNAL( clicked() ), this, SLOT( editRule() ) );
  connect( btnRemoveRule, SIGNAL( clicked() ), this, SLOT( removeRule() ) );
  connect( mCopyAction, SIGNAL( triggered( bool ) ), this, SLOT( copy() ) );
  connect( mPasteAction, SIGNAL( triggered( bool ) ), this, SLOT( paste() ) );
  connect( mDeleteAction, SIGNAL( triggered( bool ) ), this, SLOT( removeRule() ) );

  if ( mLayer->labeling() && mLayer->labeling()->type() == "rule-based" )
  {
    const QgsRuleBasedLabeling* rl = static_cast<const QgsRuleBasedLabeling*>( mLayer->labeling() );
    mRootRule = rl->rootRule()->clone();
  }
  else
  {
    mRootRule = new QgsRuleBasedLabeling::Rule( nullptr );
  }

  mModel = new QgsRuleBasedLabelingModel( mRootRule );
  viewRules->setModel( mModel );

  connect( mModel, SIGNAL( dataChanged( QModelIndex, QModelIndex ) ), this, SIGNAL( widgetChanged() ) );
  connect( mModel, SIGNAL( rowsInserted( QModelIndex, int, int ) ), this, SIGNAL( widgetChanged() ) );
  connect( mModel, SIGNAL( rowsRemoved( QModelIndex, int, int ) ), this, SIGNAL( widgetChanged() ) );
}

QgsRuleBasedLabelingWidget::~QgsRuleBasedLabelingWidget()
{
  delete mRootRule;
}

void QgsRuleBasedLabelingWidget::writeSettingsToLayer()
{
  // also clear old-style labeling config
  mLayer->removeCustomProperty( "labeling" );

  mLayer->setLabeling( new QgsRuleBasedLabeling( mRootRule->clone() ) );
}

void QgsRuleBasedLabelingWidget::addRule()
{

  QgsRuleBasedLabeling::Rule* newrule = new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings );

  QgsRuleBasedLabeling::Rule* current = currentRule();
  if ( current )
  {
    // add after this rule
    QModelIndex currentIndex = viewRules->selectionModel()->currentIndex();
    mModel->insertRule( currentIndex.parent(), currentIndex.row() + 1, newrule );
    QModelIndex newindex = mModel->index( currentIndex.row() + 1, 0, currentIndex.parent() );
    viewRules->selectionModel()->setCurrentIndex( newindex, QItemSelectionModel::ClearAndSelect );
  }
  else
  {
    // append to root rule
    int rows = mModel->rowCount();
    mModel->insertRule( QModelIndex(), rows, newrule );
    QModelIndex newindex = mModel->index( rows, 0 );
    viewRules->selectionModel()->setCurrentIndex( newindex, QItemSelectionModel::ClearAndSelect );
  }
  editRule();
}

void QgsRuleBasedLabelingWidget::ruleWidgetPanelAccepted( QgsPanelWidget* panel )
{
  QgsLabelingRulePropsWidget* widget = qobject_cast<QgsLabelingRulePropsWidget*>( panel );
  widget->apply();

  QModelIndex index = viewRules->selectionModel()->currentIndex();
  mModel->updateRule( index.parent(), index.row() );
}

void QgsRuleBasedLabelingWidget::liveUpdateRuleFromPanel()
{
  ruleWidgetPanelAccepted( qobject_cast<QgsPanelWidget*>( sender() ) );
}


void QgsRuleBasedLabelingWidget::editRule()
{
  editRule( viewRules->selectionModel()->currentIndex() );
}

void QgsRuleBasedLabelingWidget::editRule( const QModelIndex& index )
{
  if ( !index.isValid() )
    return;

  QgsRuleBasedLabeling::Rule* rule = mModel->ruleForIndex( index );

  QgsLabelingRulePropsWidget* widget = new QgsLabelingRulePropsWidget( rule, mLayer, this, mCanvas );
  widget->setDockMode( true );
  widget->setPanelTitle( tr( "Edit rule" ) );
  connect( widget, SIGNAL( panelAccepted( QgsPanelWidget* ) ), this, SLOT( ruleWidgetPanelAccepted( QgsPanelWidget* ) ) );
  connect( widget, SIGNAL( widgetChanged() ), this, SLOT( liveUpdateRuleFromPanel() ) );
  openPanel( widget );
}

void QgsRuleBasedLabelingWidget::removeRule()
{
  QItemSelection sel = viewRules->selectionModel()->selection();
  QgsDebugMsg( QString( "REMOVE RULES!!! ranges: %1" ).arg( sel.count() ) );
  Q_FOREACH ( const QItemSelectionRange& range, sel )
  {
    QgsDebugMsg( QString( "RANGE: r %1 - %2" ).arg( range.top() ).arg( range.bottom() ) );
    if ( range.isValid() )
      mModel->removeRows( range.top(), range.bottom() - range.top() + 1, range.parent() );
  }
  // make sure that the selection is gone
  viewRules->selectionModel()->clear();
}

void QgsRuleBasedLabelingWidget::copy()
{
  QModelIndexList indexlist = viewRules->selectionModel()->selectedRows();
  QgsDebugMsg( QString( "%1" ).arg( indexlist.count() ) );

  if ( indexlist.isEmpty() )
    return;

  QMimeData* mime = mModel->mimeData( indexlist );
  QApplication::clipboard()->setMimeData( mime );
}

void QgsRuleBasedLabelingWidget::paste()
{
  const QMimeData* mime = QApplication::clipboard()->mimeData();
  QModelIndexList indexlist = viewRules->selectionModel()->selectedRows();
  QModelIndex index;
  if ( indexlist.isEmpty() )
    index = mModel->index( mModel->rowCount(), 0 );
  else
    index = indexlist.first();
  mModel->dropMimeData( mime, Qt::CopyAction, index.row(), index.column(), index.parent() );
}

QgsRuleBasedLabeling::Rule* QgsRuleBasedLabelingWidget::currentRule()
{
  QItemSelectionModel* sel = viewRules->selectionModel();
  QModelIndex idx = sel->currentIndex();
  if ( !idx.isValid() )
    return nullptr;
  return mModel->ruleForIndex( idx );
}


////

static QString _formatScale( int denom )
{
  if ( denom != 0 )
  {
    QString txt = QString( "1:%L1" ).arg( denom );
    return txt;
  }
  else
    return QString();
}

////

QgsRuleBasedLabelingModel::QgsRuleBasedLabelingModel( QgsRuleBasedLabeling::Rule* rootRule, QObject* parent )
    : QAbstractItemModel( parent )
    , mRootRule( rootRule )
{
}

Qt::ItemFlags QgsRuleBasedLabelingModel::flags( const QModelIndex& index ) const
{
  if ( !index.isValid() )
    return Qt::ItemIsDropEnabled;

  // allow drop only at first column
  Qt::ItemFlag drop = ( index.column() == 0 ? Qt::ItemIsDropEnabled : Qt::NoItemFlags );

  Qt::ItemFlag checkable = ( index.column() == 0 ? Qt::ItemIsUserCheckable : Qt::NoItemFlags );

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable |
         Qt::ItemIsEditable | checkable |
         Qt::ItemIsDragEnabled | drop;
}

QVariant QgsRuleBasedLabelingModel::data( const QModelIndex& index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsRuleBasedLabeling::Rule* rule = ruleForIndex( index );

  if ( role == Qt::DisplayRole || role == Qt::ToolTipRole )
  {
    switch ( index.column() )
    {
      case 0:
        return rule->description();
      case 1:
        if ( rule->isElse() )
        {
          return "ELSE";
        }
        else
        {
          return rule->filterExpression().isEmpty() ? tr( "(no filter)" ) : rule->filterExpression();
        }
      case 2:
        return rule->dependsOnScale() ? _formatScale( rule->scaleMaxDenom() ) : QVariant();
      case 3:
        return rule->dependsOnScale() ? _formatScale( rule->scaleMinDenom() ) : QVariant();
      case 4:
        return rule->settings() ? rule->settings()->fieldName : QVariant();
      default:
        return QVariant();
    }
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 && rule->settings() )
  {
    // TODO return QgsSymbolLayerV2Utils::symbolPreviewIcon( rule->symbol(), QSize( 16, 16 ) );
    return QVariant();
  }
  else if ( role == Qt::TextAlignmentRole )
  {
    return ( index.column() == 2 || index.column() == 3 ) ? Qt::AlignRight : Qt::AlignLeft;
  }
  else if ( role == Qt::FontRole && index.column() == 1 )
  {
    if ( rule->isElse() )
    {
      QFont italicFont;
      italicFont.setItalic( true );
      return italicFont;
    }
    return QVariant();
  }
  else if ( role == Qt::EditRole )
  {
    switch ( index.column() )
    {
      case 0:
        return rule->description();
      case 1:
        return rule->filterExpression();
      case 2:
        return rule->scaleMaxDenom();
      case 3:
        return rule->scaleMinDenom();
      case 4:
        return rule->settings() ? rule->settings()->fieldName : QVariant();
      default:
        return QVariant();
    }
  }
  else if ( role == Qt::CheckStateRole )
  {
    if ( index.column() != 0 )
      return QVariant();
    return rule->active() ? Qt::Checked : Qt::Unchecked;
  }
  else
    return QVariant();
}

QVariant QgsRuleBasedLabelingModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < 5 )
  {
    QStringList lst;
    lst << tr( "Label" ) << tr( "Rule" ) << tr( "Min. scale" ) << tr( "Max. scale" ) << tr( "Text" ); // << tr( "Count" ) << tr( "Duplicate count" );
    return lst[section];
  }

  return QVariant();
}

int QgsRuleBasedLabelingModel::rowCount( const QModelIndex& parent ) const
{
  if ( parent.column() > 0 )
    return 0;

  QgsRuleBasedLabeling::Rule* parentRule = ruleForIndex( parent );

  return parentRule->children().count();
}

int QgsRuleBasedLabelingModel::columnCount( const QModelIndex& ) const
{
  return 5;
}

QModelIndex QgsRuleBasedLabelingModel::index( int row, int column, const QModelIndex& parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    QgsRuleBasedLabeling::Rule* parentRule = ruleForIndex( parent );
    QgsRuleBasedLabeling::Rule* childRule = parentRule->children()[row];
    return createIndex( row, column, childRule );
  }
  return QModelIndex();
}

QModelIndex QgsRuleBasedLabelingModel::parent( const QModelIndex& index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  QgsRuleBasedLabeling::Rule* childRule = ruleForIndex( index );
  QgsRuleBasedLabeling::Rule* parentRule = childRule->parent();

  if ( parentRule == mRootRule )
    return QModelIndex();

  // this is right: we need to know row number of our parent (in our grandparent)
  int row = parentRule->parent()->children().indexOf( parentRule );

  return createIndex( row, 0, parentRule );
}

bool QgsRuleBasedLabelingModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  if ( !index.isValid() )
    return false;

  QgsRuleBasedLabeling::Rule* rule = ruleForIndex( index );

  if ( role == Qt::CheckStateRole )
  {
    rule->setActive( value.toInt() == Qt::Checked );
    emit dataChanged( index, index );
    return true;
  }

  if ( role != Qt::EditRole )
    return false;

  switch ( index.column() )
  {
    case 0: // description
      rule->setDescription( value.toString() );
      break;
    case 1: // filter
      rule->setFilterExpression( value.toString() );
      break;
    case 2: // scale min
      rule->setScaleMaxDenom( value.toInt() );
      break;
    case 3: // scale max
      rule->setScaleMinDenom( value.toInt() );
      break;
    case 4: // label text
      if ( !rule->settings() )
        return false;
      rule->settings()->fieldName = value.toString();
      break;
    default:
      return false;
  }

  emit dataChanged( index, index );
  return true;
}

Qt::DropActions QgsRuleBasedLabelingModel::supportedDropActions() const
{
  return Qt::MoveAction; // | Qt::CopyAction
}

QStringList QgsRuleBasedLabelingModel::mimeTypes() const
{
  QStringList types;
  types << "application/vnd.text.list";
  return types;
}

// manipulate DOM before dropping it so that rules are more useful
void _renderer2labelingRules( QDomElement& ruleElem )
{
  // labeling rules recognize only "description"
  if ( ruleElem.hasAttribute( "label" ) )
    ruleElem.setAttribute( "description", ruleElem.attribute( "label" ) );

  // run recursively
  QDomElement childRuleElem = ruleElem.firstChildElement( "rule" );
  while ( !childRuleElem.isNull() )
  {
    _renderer2labelingRules( childRuleElem );
    childRuleElem = childRuleElem.nextSiblingElement( "rule" );
  }
}

QMimeData*QgsRuleBasedLabelingModel::mimeData( const QModelIndexList& indexes ) const
{
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream( &encodedData, QIODevice::WriteOnly );

  Q_FOREACH ( const QModelIndex &index, indexes )
  {
    // each item consists of several columns - let's add it with just first one
    if ( !index.isValid() || index.column() != 0 )
      continue;

    // we use a clone of the existing rule because it has a new unique rule key
    // non-unique rule keys would confuse other components using them (e.g. legend)
    QgsRuleBasedLabeling::Rule* rule = ruleForIndex( index )->clone();
    QDomDocument doc;

    QDomElement rootElem = doc.createElement( "rule_mime" );
    rootElem.setAttribute( "type", "labeling" ); // for determining whether rules are from renderer or labeling
    QDomElement rulesElem = rule->save( doc );
    rootElem.appendChild( rulesElem );
    doc.appendChild( rootElem );

    delete rule;

    stream << doc.toString( -1 );
  }

  mimeData->setData( "application/vnd.text.list", encodedData );
  return mimeData;
}

bool QgsRuleBasedLabelingModel::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
  Q_UNUSED( column );

  if ( action == Qt::IgnoreAction )
    return true;

  if ( !data->hasFormat( "application/vnd.text.list" ) )
    return false;

  if ( parent.column() > 0 )
    return false;

  QByteArray encodedData = data->data( "application/vnd.text.list" );
  QDataStream stream( &encodedData, QIODevice::ReadOnly );
  int rows = 0;

  if ( row == -1 )
  {
    // the item was dropped at a parent - we may decide where to put the items - let's append them
    row = rowCount( parent );
  }

  while ( !stream.atEnd() )
  {
    QString text;
    stream >> text;

    QDomDocument doc;
    if ( !doc.setContent( text ) )
      continue;
    QDomElement rootElem = doc.documentElement();
    if ( rootElem.tagName() != "rule_mime" )
      continue;
    QDomElement ruleElem = rootElem.firstChildElement( "rule" );
    if ( rootElem.attribute( "type" ) == "renderer" )
      _renderer2labelingRules( ruleElem ); // do some modifications so that we load the rules more nicely
    QgsRuleBasedLabeling::Rule* rule = QgsRuleBasedLabeling::Rule::create( ruleElem );

    insertRule( parent, row + rows, rule );

    ++rows;
  }
  return true;
}

bool QgsRuleBasedLabelingModel::removeRows( int row, int count, const QModelIndex& parent )
{
  QgsRuleBasedLabeling::Rule* parentRule = ruleForIndex( parent );

  if ( row < 0 || row >= parentRule->children().count() )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );

  for ( int i = 0; i < count; i++ )
  {
    if ( row < parentRule->children().count() )
    {
      parentRule->removeChildAt( row );
    }
    else
    {
      QgsDebugMsg( "trying to remove invalid index - this should not happen!" );
    }
  }

  endRemoveRows();

  return true;
}

QgsRuleBasedLabeling::Rule*QgsRuleBasedLabelingModel::ruleForIndex( const QModelIndex& index ) const
{
  if ( index.isValid() )
    return static_cast<QgsRuleBasedLabeling::Rule*>( index.internalPointer() );
  return mRootRule;
}

void QgsRuleBasedLabelingModel::insertRule( const QModelIndex& parent, int before, QgsRuleBasedLabeling::Rule* newrule )
{
  beginInsertRows( parent, before, before );

  QgsRuleBasedLabeling::Rule* parentRule = ruleForIndex( parent );
  parentRule->insertChild( before, newrule );

  endInsertRows();
}

void QgsRuleBasedLabelingModel::updateRule( const QModelIndex& parent, int row )
{
  emit dataChanged( index( row, 0, parent ),
                    index( row, columnCount( parent ), parent ) );
}

/////////

QgsLabelingRulePropsWidget::QgsLabelingRulePropsWidget( QgsRuleBasedLabeling::Rule* rule, QgsVectorLayer* layer, QWidget* parent, QgsMapCanvas* mapCanvas )
    : QgsPanelWidget( parent )
    , mRule( rule )
    , mLayer( layer )
    , mLabelingGui( nullptr )
    , mSettings( nullptr )
    , mMapCanvas( mapCanvas )
{
  setupUi( this );

  editFilter->setText( mRule->filterExpression() );
  editFilter->setToolTip( mRule->filterExpression() );
  editDescription->setText( mRule->description() );
  editDescription->setToolTip( mRule->description() );

  if ( mRule->dependsOnScale() )
  {
    groupScale->setChecked( true );
    // caution: rule uses scale denom, scale widget uses true scales
    if ( rule->scaleMinDenom() > 0 )
      mScaleRangeWidget->setMaximumScale( 1.0 / rule->scaleMinDenom() );
    if ( rule->scaleMaxDenom() > 0 )
      mScaleRangeWidget->setMinimumScale( 1.0 / rule->scaleMaxDenom() );
  }
  mScaleRangeWidget->setMapCanvas( mMapCanvas );

  if ( mRule->settings() )
  {
    groupSettings->setChecked( true );
    mSettings = new QgsPalLayerSettings( *mRule->settings() ); // use a clone!
  }
  else
  {
    groupSettings->setChecked( false );
    mSettings = new QgsPalLayerSettings;
  }

  mLabelingGui = new QgsLabelingGui( nullptr, mMapCanvas, mSettings, this );
  mLabelingGui->layout()->setContentsMargins( 0, 0, 0, 0 );
  QVBoxLayout* l = new QVBoxLayout;
  l->addWidget( mLabelingGui );
  groupSettings->setLayout( l );

  mLabelingGui->setLabelMode( QgsLabelingGui::Labels );
  mLabelingGui->setLayer( mLayer );

  connect( btnExpressionBuilder, SIGNAL( clicked() ), this, SLOT( buildExpression() ) );
  connect( btnTestFilter, SIGNAL( clicked() ), this, SLOT( testFilter() ) );
  connect( editFilter, SIGNAL( textEdited( QString ) ), this, SIGNAL( widgetChanged() ) );
  connect( editDescription, SIGNAL( textChanged( QString ) ), this, SIGNAL( widgetChanged() ) );
  connect( groupScale, SIGNAL( toggled( bool ) ), this, SIGNAL( widgetChanged() ) );
  connect( mScaleRangeWidget, SIGNAL( rangeChanged( double, double ) ), this, SIGNAL( widgetChanged() ) );
  connect( groupSettings, SIGNAL( toggled( bool ) ), this, SIGNAL( widgetChanged() ) );
  connect( mLabelingGui, SIGNAL( widgetChanged() ), this, SIGNAL( widgetChanged() ) );
}

QgsLabelingRulePropsWidget::~QgsLabelingRulePropsWidget()
{
  delete mSettings;
}

void QgsLabelingRulePropsWidget::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );
  mLabelingGui->setDockMode( dockMode );
}

void QgsLabelingRulePropsWidget::testFilter()
{
  QgsExpression filter( editFilter->text() );
  if ( filter.hasParserError() )
  {
    QMessageBox::critical( this, tr( "Error" ),  tr( "Filter expression parsing error:\n" ) + filter.parserErrorString() );
    return;
  }

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::atlasScope( nullptr );
  if ( mMapCanvas )
  {
    context << QgsExpressionContextUtils::mapSettingsScope( mMapCanvas->mapSettings() )
    << new QgsExpressionContextScope( mMapCanvas->expressionContextScope() );
  }
  else
  {
    context << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }
  context << QgsExpressionContextUtils::layerScope( mLayer );

  if ( !filter.prepare( &context ) )
  {
    QMessageBox::critical( this, tr( "Evaluation error" ), filter.evalErrorString() );
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsFeatureIterator fit = mLayer->getFeatures();

  int count = 0;
  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    context.setFeature( f );

    QVariant value = filter.evaluate( &context );
    if ( value.toInt() != 0 )
      count++;
    if ( filter.hasEvalError() )
      break;
  }

  QApplication::restoreOverrideCursor();

  QMessageBox::information( this, tr( "Filter" ), tr( "Filter returned %n feature(s)", "number of filtered features", count ) );
}

void QgsLabelingRulePropsWidget::buildExpression()
{
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::atlasScope( nullptr );
  if ( mMapCanvas )
  {
    context << QgsExpressionContextUtils::mapSettingsScope( mMapCanvas->mapSettings() )
    << new QgsExpressionContextScope( mMapCanvas->expressionContextScope() );
  }
  else
  {
    context << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }
  context << QgsExpressionContextUtils::layerScope( mLayer );

  QgsExpressionBuilderDialog dlg( mLayer, editFilter->text(), this, "generic", context );

  if ( dlg.exec() )
    editFilter->setText( dlg.expressionText() );
}

void QgsLabelingRulePropsWidget::apply()
{
  mRule->setFilterExpression( editFilter->text() );
  mRule->setDescription( editDescription->text() );
  // caution: rule uses scale denom, scale widget uses true scales
  mRule->setScaleMinDenom( groupScale->isChecked() ? mScaleRangeWidget->minimumScaleDenom() : 0 );
  mRule->setScaleMaxDenom( groupScale->isChecked() ? mScaleRangeWidget->maximumScaleDenom() : 0 );
  mRule->setSettings( groupSettings->isChecked() ? new QgsPalLayerSettings( mLabelingGui->layerSettings() ) : nullptr );
}
