/***************************************************************************
  qgsrulebased3drendererwidget.cpp
  --------------------------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrulebased3drendererwidget.h"

#include "qgs3dutils.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgslogger.h"
#include "qgsrulebased3drenderer.h"
#include "qgsvectorlayer.h"
#include "qgssymbol3dwidget.h"
#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"

#include <QAction>
#include <QClipboard>
#include <QMessageBox>
#include <QMimeData>

QgsRuleBased3DRendererWidget::QgsRuleBased3DRendererWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  btnAddRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  btnEditRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.svg" ) ) );
  btnRemoveRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );

  mCopyAction = new QAction( tr( "Copy" ), this );
  mCopyAction->setShortcut( QKeySequence( QKeySequence::Copy ) );
  mPasteAction = new QAction( tr( "Paste" ), this );
  mPasteAction->setShortcut( QKeySequence( QKeySequence::Paste ) );
  mDeleteAction = new QAction( tr( "Remove Rule" ), this );
  mDeleteAction->setShortcut( QKeySequence( QKeySequence::Delete ) );

  viewRules->addAction( mCopyAction );
  viewRules->addAction( mPasteAction );
  viewRules->addAction( mDeleteAction );

  connect( viewRules, &QAbstractItemView::doubleClicked, this, static_cast<void ( QgsRuleBased3DRendererWidget::* )( const QModelIndex & )>( &QgsRuleBased3DRendererWidget::editRule ) );

  connect( btnAddRule, &QAbstractButton::clicked, this, &QgsRuleBased3DRendererWidget::addRule );
  connect( btnEditRule, &QAbstractButton::clicked, this, static_cast<void ( QgsRuleBased3DRendererWidget::* )()>( &QgsRuleBased3DRendererWidget::editRule ) );
  connect( btnRemoveRule, &QAbstractButton::clicked, this, &QgsRuleBased3DRendererWidget::removeRule );
  connect( mCopyAction, &QAction::triggered, this, &QgsRuleBased3DRendererWidget::copy );
  connect( mPasteAction, &QAction::triggered, this, &QgsRuleBased3DRendererWidget::paste );
  connect( mDeleteAction, &QAction::triggered, this, &QgsRuleBased3DRendererWidget::removeRule );

}

QgsRuleBased3DRendererWidget::~QgsRuleBased3DRendererWidget()
{
  delete mRootRule;
}

void QgsRuleBased3DRendererWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;

  QgsAbstract3DRenderer *r = layer->renderer3D();
  if ( r && r->type() == QLatin1String( "rulebased" ) )
  {
    QgsRuleBased3DRenderer *ruleRenderer = static_cast<QgsRuleBased3DRenderer *>( r );
    mRootRule = ruleRenderer->rootRule()->clone();
  }
  else
  {
    // TODO: handle the special case when switching from single symbol renderer
    mRootRule = new QgsRuleBased3DRenderer::Rule( nullptr );
  }

  mModel = new QgsRuleBased3DRendererModel( mRootRule );
  viewRules->setModel( mModel );

  connect( mModel, &QAbstractItemModel::dataChanged, this, &QgsRuleBased3DRendererWidget::widgetChanged );
  connect( mModel, &QAbstractItemModel::rowsInserted, this, &QgsRuleBased3DRendererWidget::widgetChanged );
  connect( mModel, &QAbstractItemModel::rowsRemoved, this, &QgsRuleBased3DRendererWidget::widgetChanged );
}

void QgsRuleBased3DRendererWidget::setDockMode( bool dockMode )
{
  if ( dockMode )
  {
    // when in dock mode, these shortcuts conflict with the main window shortcuts and cannot be used
    if ( mCopyAction )
      mCopyAction->setShortcut( QKeySequence() );
    if ( mPasteAction )
      mPasteAction->setShortcut( QKeySequence() );
    if ( mDeleteAction )
      mDeleteAction->setShortcut( QKeySequence() );
  }
  QgsPanelWidget::setDockMode( dockMode );
}


void QgsRuleBased3DRendererWidget::addRule()
{
  std::unique_ptr< QgsAbstract3DSymbol > newSymbol( QgsApplication::symbol3DRegistry()->defaultSymbolForGeometryType( mLayer->geometryType() ) );
  newSymbol->setDefaultPropertiesFromLayer( mLayer );
  QgsRuleBased3DRenderer::Rule *newrule = new QgsRuleBased3DRenderer::Rule( newSymbol.release() );

  QgsRuleBased3DRenderer::Rule *current = currentRule();
  if ( current )
  {
    // add after this rule
    const QModelIndex currentIndex = viewRules->selectionModel()->currentIndex();
    mModel->insertRule( currentIndex.parent(), currentIndex.row() + 1, newrule );
    const QModelIndex newindex = mModel->index( currentIndex.row() + 1, 0, currentIndex.parent() );
    viewRules->selectionModel()->setCurrentIndex( newindex, QItemSelectionModel::ClearAndSelect );
  }
  else
  {
    // append to root rule
    const int rows = mModel->rowCount();
    mModel->insertRule( QModelIndex(), rows, newrule );
    const QModelIndex newindex = mModel->index( rows, 0 );
    viewRules->selectionModel()->setCurrentIndex( newindex, QItemSelectionModel::ClearAndSelect );
  }
  editRule();
}

void QgsRuleBased3DRendererWidget::ruleWidgetPanelAccepted( QgsPanelWidget *panel )
{
  Qgs3DRendererRulePropsWidget *widget = qobject_cast<Qgs3DRendererRulePropsWidget *>( panel );
  widget->apply();

  const QModelIndex index = viewRules->selectionModel()->currentIndex();
  mModel->updateRule( index.parent(), index.row() );
}

void QgsRuleBased3DRendererWidget::liveUpdateRuleFromPanel()
{
  ruleWidgetPanelAccepted( qobject_cast<QgsPanelWidget *>( sender() ) );
}


void QgsRuleBased3DRendererWidget::editRule()
{
  editRule( viewRules->selectionModel()->currentIndex() );
}

void QgsRuleBased3DRendererWidget::editRule( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  QgsRuleBased3DRenderer::Rule *rule = mModel->ruleForIndex( index );

  Qgs3DRendererRulePropsWidget *widget = new Qgs3DRendererRulePropsWidget( rule, mLayer, this );
  widget->setPanelTitle( tr( "Edit Rule" ) );
  connect( widget, &QgsPanelWidget::panelAccepted, this, &QgsRuleBased3DRendererWidget::ruleWidgetPanelAccepted );
  connect( widget, &Qgs3DRendererRulePropsWidget::widgetChanged, this, &QgsRuleBased3DRendererWidget::liveUpdateRuleFromPanel );
  openPanel( widget );
}

void QgsRuleBased3DRendererWidget::removeRule()
{
  const QItemSelection sel = viewRules->selectionModel()->selection();
  const auto constSel = sel;
  for ( const QItemSelectionRange &range : constSel )
  {
    if ( range.isValid() )
      mModel->removeRows( range.top(), range.bottom() - range.top() + 1, range.parent() );
  }
  // make sure that the selection is gone
  viewRules->selectionModel()->clear();
}

void QgsRuleBased3DRendererWidget::copy()
{
  const QModelIndexList indexlist = viewRules->selectionModel()->selectedRows();

  if ( indexlist.isEmpty() )
    return;

  QMimeData *mime = mModel->mimeData( indexlist );
  QApplication::clipboard()->setMimeData( mime );
}

void QgsRuleBased3DRendererWidget::paste()
{
  const QMimeData *mime = QApplication::clipboard()->mimeData();
  QModelIndexList indexlist = viewRules->selectionModel()->selectedRows();
  QModelIndex index;
  if ( indexlist.isEmpty() )
    index = mModel->index( mModel->rowCount(), 0 );
  else
    index = indexlist.first();
  mModel->dropMimeData( mime, Qt::CopyAction, index.row(), index.column(), index.parent() );
}

QgsRuleBased3DRenderer::Rule *QgsRuleBased3DRendererWidget::currentRule()
{
  QItemSelectionModel *sel = viewRules->selectionModel();
  const QModelIndex idx = sel->currentIndex();
  if ( !idx.isValid() )
    return nullptr;
  return mModel->ruleForIndex( idx );
}


////

QgsRuleBased3DRendererModel::QgsRuleBased3DRendererModel( QgsRuleBased3DRenderer::Rule *rootRule, QObject *parent )
  : QAbstractItemModel( parent )
  , mRootRule( rootRule )
{
}

Qt::ItemFlags QgsRuleBased3DRendererModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemIsDropEnabled;

  // allow drop only at first column
  const Qt::ItemFlag drop = ( index.column() == 0 ? Qt::ItemIsDropEnabled : Qt::NoItemFlags );

  const Qt::ItemFlag checkable = ( index.column() == 0 ? Qt::ItemIsUserCheckable : Qt::NoItemFlags );

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable |
         Qt::ItemIsEditable | checkable |
         Qt::ItemIsDragEnabled | drop;
}

QVariant QgsRuleBased3DRendererModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsRuleBased3DRenderer::Rule *rule = ruleForIndex( index );

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
      default:
        return QVariant();
    }
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 && rule->symbol() )
  {
    // TODO return QgsSymbolLayerUtils::symbolPreviewIcon( rule->symbol(), QSize( 16, 16 ) );
    return QVariant();
  }
  else if ( role == Qt::TextAlignmentRole )
  {
    return static_cast<Qt::Alignment::Int>( Qt::AlignLeft );
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

QVariant QgsRuleBased3DRendererModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < 2 )
  {
    QStringList lst;
    lst << tr( "Label" ) << tr( "Rule" ); // << tr( "Count" ) << tr( "Duplicate Count" );
    return lst[section];
  }

  return QVariant();
}

int QgsRuleBased3DRendererModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.column() > 0 )
    return 0;

  QgsRuleBased3DRenderer::Rule *parentRule = ruleForIndex( parent );

  return parentRule->children().count();
}

int QgsRuleBased3DRendererModel::columnCount( const QModelIndex & ) const
{
  return 2;
}

QModelIndex QgsRuleBased3DRendererModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    QgsRuleBased3DRenderer::Rule *parentRule = ruleForIndex( parent );
    QgsRuleBased3DRenderer::Rule *childRule = parentRule->children()[row];
    return createIndex( row, column, childRule );
  }
  return QModelIndex();
}

QModelIndex QgsRuleBased3DRendererModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  QgsRuleBased3DRenderer::Rule *childRule = ruleForIndex( index );
  QgsRuleBased3DRenderer::Rule *parentRule = childRule->parent();

  if ( parentRule == mRootRule )
    return QModelIndex();

  // this is right: we need to know row number of our parent (in our grandparent)
  const int row = parentRule->parent()->children().indexOf( parentRule );

  return createIndex( row, 0, parentRule );
}

bool QgsRuleBased3DRendererModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  QgsRuleBased3DRenderer::Rule *rule = ruleForIndex( index );

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
    default:
      return false;
  }

  emit dataChanged( index, index );
  return true;
}

Qt::DropActions QgsRuleBased3DRendererModel::supportedDropActions() const
{
  return Qt::MoveAction; // | Qt::CopyAction
}

QStringList QgsRuleBased3DRendererModel::mimeTypes() const
{
  QStringList types;
  types << QStringLiteral( "application/vnd.text.list" );
  return types;
}

// manipulate DOM before dropping it so that rules are more useful
void _renderer2renderer3DRules( QDomElement &ruleElem )
{
  // labeling rules recognize only "description"
  if ( ruleElem.hasAttribute( QStringLiteral( "label" ) ) )
    ruleElem.setAttribute( QStringLiteral( "description" ), ruleElem.attribute( QStringLiteral( "label" ) ) );

  // run recursively
  QDomElement childRuleElem = ruleElem.firstChildElement( QStringLiteral( "rule" ) );
  while ( !childRuleElem.isNull() )
  {
    _renderer2renderer3DRules( childRuleElem );
    childRuleElem = childRuleElem.nextSiblingElement( QStringLiteral( "rule" ) );
  }
}

QMimeData *QgsRuleBased3DRendererModel::mimeData( const QModelIndexList &indexes ) const
{
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream( &encodedData, QIODevice::WriteOnly );

  const auto constIndexes = indexes;
  for ( const QModelIndex &index : constIndexes )
  {
    // each item consists of several columns - let's add it with just first one
    if ( !index.isValid() || index.column() != 0 )
      continue;

    // we use a clone of the existing rule because it has a new unique rule key
    // non-unique rule keys would confuse other components using them (e.g. legend)
    QgsRuleBased3DRenderer::Rule *rule = ruleForIndex( index )->clone();
    QDomDocument doc;

    QDomElement rootElem = doc.createElement( QStringLiteral( "rule_mime" ) );
    rootElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "labeling" ) ); // for determining whether rules are from renderer or labeling
    const QDomElement rulesElem = rule->save( doc, QgsReadWriteContext() );
    rootElem.appendChild( rulesElem );
    doc.appendChild( rootElem );

    delete rule;

    stream << doc.toString( -1 );
  }

  mimeData->setData( QStringLiteral( "application/vnd.text.list" ), encodedData );
  return mimeData;
}

bool QgsRuleBased3DRendererModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  Q_UNUSED( column )

  if ( action == Qt::IgnoreAction )
    return true;

  if ( !data->hasFormat( QStringLiteral( "application/vnd.text.list" ) ) )
    return false;

  if ( parent.column() > 0 )
    return false;

  QByteArray encodedData = data->data( QStringLiteral( "application/vnd.text.list" ) );
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
    const QDomElement rootElem = doc.documentElement();
    if ( rootElem.tagName() != QLatin1String( "rule_mime" ) )
      continue;
    QDomElement ruleElem = rootElem.firstChildElement( QStringLiteral( "rule" ) );
    if ( rootElem.attribute( QStringLiteral( "type" ) ) == QLatin1String( "renderer" ) )
      _renderer2renderer3DRules( ruleElem ); // do some modifications so that we load the rules more nicely
    QgsRuleBased3DRenderer::Rule *rule = QgsRuleBased3DRenderer::Rule::create( ruleElem, QgsReadWriteContext() );

    insertRule( parent, row + rows, rule );

    ++rows;
  }
  return true;
}

bool QgsRuleBased3DRendererModel::removeRows( int row, int count, const QModelIndex &parent )
{
  QgsRuleBased3DRenderer::Rule *parentRule = ruleForIndex( parent );

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
      QgsDebugMsg( QStringLiteral( "trying to remove invalid index - this should not happen!" ) );
    }
  }

  endRemoveRows();

  return true;
}

QgsRuleBased3DRenderer::Rule *QgsRuleBased3DRendererModel::ruleForIndex( const QModelIndex &index ) const
{
  if ( index.isValid() )
    return static_cast<QgsRuleBased3DRenderer::Rule *>( index.internalPointer() );
  return mRootRule;
}

void QgsRuleBased3DRendererModel::insertRule( const QModelIndex &parent, int before, QgsRuleBased3DRenderer::Rule *newrule )
{
  beginInsertRows( parent, before, before );

  QgsRuleBased3DRenderer::Rule *parentRule = ruleForIndex( parent );
  parentRule->insertChild( before, newrule );

  endInsertRows();
}

void QgsRuleBased3DRendererModel::updateRule( const QModelIndex &parent, int row )
{
  emit dataChanged( index( row, 0, parent ),
                    index( row, columnCount( parent ), parent ) );
}


/////////

Qgs3DRendererRulePropsWidget::Qgs3DRendererRulePropsWidget( QgsRuleBased3DRenderer::Rule *rule, QgsVectorLayer *layer, QWidget *parent )
  : QgsPanelWidget( parent )
  , mRule( rule )
  , mLayer( layer )
{
  setupUi( this );

  mElseRadio->setChecked( mRule->isElse() );
  mFilterRadio->setChecked( !mRule->isElse() );
  editFilter->setText( mRule->filterExpression() );
  editFilter->setToolTip( mRule->filterExpression() );
  editDescription->setText( mRule->description() );
  editDescription->setToolTip( mRule->description() );

  if ( mRule->symbol() )
  {
    groupSymbol->setChecked( true );
    mSymbol.reset( mRule->symbol()->clone() ); // use a clone!
  }
  else
  {
    groupSymbol->setChecked( false );
    mSymbol.reset( QgsApplication::symbol3DRegistry()->defaultSymbolForGeometryType( layer->geometryType() ) );
    mSymbol->setDefaultPropertiesFromLayer( layer );
  }

  mSymbolWidget = new QgsSymbol3DWidget( layer, this );
  mSymbolWidget->setSymbol( mSymbol.get(), layer );
  QVBoxLayout *l = new QVBoxLayout;
  l->addWidget( mSymbolWidget );
  groupSymbol->setLayout( l );

  connect( btnExpressionBuilder, &QAbstractButton::clicked, this, &Qgs3DRendererRulePropsWidget::buildExpression );
  connect( btnTestFilter, &QAbstractButton::clicked, this, &Qgs3DRendererRulePropsWidget::testFilter );
  connect( editFilter, &QLineEdit::textEdited, this, &Qgs3DRendererRulePropsWidget::widgetChanged );
  connect( editDescription, &QLineEdit::textChanged, this, &Qgs3DRendererRulePropsWidget::widgetChanged );
  connect( groupSymbol, &QGroupBox::toggled, this, &Qgs3DRendererRulePropsWidget::widgetChanged );
  connect( mSymbolWidget, &QgsSymbol3DWidget::widgetChanged, this, &Qgs3DRendererRulePropsWidget::widgetChanged );
  connect( mFilterRadio, &QRadioButton::toggled, this, [ = ]( bool toggled ) { filterFrame->setEnabled( toggled ) ; } );
  connect( mElseRadio, &QRadioButton::toggled, this, [ = ]( bool toggled ) { if ( toggled ) editFilter->setText( QStringLiteral( "ELSE" ) );} );
}

Qgs3DRendererRulePropsWidget::~Qgs3DRendererRulePropsWidget() = default;

void Qgs3DRendererRulePropsWidget::testFilter()
{
  if ( !mFilterRadio->isChecked() )
    return;

  QgsExpression filter( editFilter->text() );
  if ( filter.hasParserError() )
  {
    QMessageBox::critical( this, tr( "Test Filter" ),  tr( "Filter expression parsing error:\n" ) + filter.parserErrorString() );
    return;
  }

  QgsExpressionContext context( Qgs3DUtils::globalProjectLayerExpressionContext( mLayer ) );

  if ( !filter.prepare( &context ) )
  {
    QMessageBox::critical( this, tr( "Test Filter" ), filter.evalErrorString() );
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsFeatureIterator fit = mLayer->getFeatures();

  int count = 0;
  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    context.setFeature( f );

    const QVariant value = filter.evaluate( &context );
    if ( value.toInt() != 0 )
      count++;
    if ( filter.hasEvalError() )
      break;
  }

  QApplication::restoreOverrideCursor();

  QMessageBox::information( this, tr( "Test Filter" ), tr( "Filter returned %n feature(s)", "number of filtered features", count ) );
}


void Qgs3DRendererRulePropsWidget::buildExpression()
{
  const QgsExpressionContext context( Qgs3DUtils::globalProjectLayerExpressionContext( mLayer ) );

  QgsExpressionBuilderDialog dlg( mLayer, editFilter->text(), this, QStringLiteral( "generic" ), context );

  if ( dlg.exec() )
    editFilter->setText( dlg.expressionText() );
}

void Qgs3DRendererRulePropsWidget::apply()
{
  const QString filter = mElseRadio->isChecked() ? QStringLiteral( "ELSE" ) : editFilter->text();
  mRule->setFilterExpression( filter );
  mRule->setDescription( editDescription->text() );
  std::unique_ptr< QgsAbstract3DSymbol > newSymbol;
  if ( groupSymbol->isChecked() )
    newSymbol = mSymbolWidget->symbol();
  mRule->setSymbol( newSymbol.release() );
}
