/***************************************************************************
    qgsrulebasedrendererv2widget.cpp - Settings widget for rule-based renderer
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrulebasedrendererv2widget.h"

#include "qgsrulebasedrendererv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgssymbolv2.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsexpression.h"
#include "qgssymbolv2selectordialog.h"
#include "qgslogger.h"
#include "qstring.h"

#include <QMenu>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QMessageBox>

QgsRendererV2Widget* QgsRuleBasedRendererV2Widget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsRuleBasedRendererV2Widget( layer, style, renderer );
}

QgsRuleBasedRendererV2Widget::QgsRuleBasedRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
{

  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")
  if ( !renderer || renderer->type() != "RuleRenderer" )
  {
    // we're not going to use it - so let's delete the renderer
    delete renderer;

    // some default options
    QgsSymbolV2* symbol = QgsSymbolV2::defaultSymbol( mLayer->geometryType() );

    mRenderer = new QgsRuleBasedRendererV2( symbol );
  }
  else
  {
    mRenderer = static_cast<QgsRuleBasedRendererV2*>( renderer );
  }

  setupUi( this );

  mModel = new QgsRuleBasedRendererV2Model( mRenderer );
  viewRules->setModel( mModel );

  mRefineMenu = new QMenu( btnRefineRule );
  mRefineMenu->addAction( tr( "Add scales" ), this, SLOT( refineRuleScales() ) );
  mRefineMenu->addAction( tr( "Add categories" ), this, SLOT( refineRuleCategories() ) );
  mRefineMenu->addAction( tr( "Add ranges" ), this, SLOT( refineRuleRanges() ) );
  btnRefineRule->setMenu( mRefineMenu );

  btnAddRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.png" ) ) );
  btnEditRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.png" ) ) );
  btnRemoveRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.png" ) ) );
  btnMoveUp->setIcon( QIcon( QgsApplication::iconPath( "symbologyUp.png" ) ) );
  btnMoveDown->setIcon( QIcon( QgsApplication::iconPath( "symbologyDown.png" ) ) );

  connect( viewRules, SIGNAL( doubleClicked( const QModelIndex & ) ), this, SLOT( editRule( const QModelIndex & ) ) );

  // support for context menu (now handled generically)
  connect( viewRules, SIGNAL( customContextMenuRequested( const QPoint& ) ),  this, SLOT( contextMenuViewCategories( const QPoint& ) ) );

  connect( btnAddRule, SIGNAL( clicked() ), this, SLOT( addRule() ) );
  connect( btnEditRule, SIGNAL( clicked() ), this, SLOT( editRule() ) );
  connect( btnRemoveRule, SIGNAL( clicked() ), this, SLOT( removeRule() ) );
  connect( btnMoveUp, SIGNAL( clicked() ), this, SLOT( moveUp() ) );
  connect( btnMoveDown, SIGNAL( clicked() ), this, SLOT( moveDown() ) );

  connect( btnRenderingOrder, SIGNAL( clicked() ), this, SLOT( setRenderingOrder() ) );
}

QgsRuleBasedRendererV2Widget::~QgsRuleBasedRendererV2Widget()
{
  delete mRenderer;
}

QgsFeatureRendererV2* QgsRuleBasedRendererV2Widget::renderer()
{
  return mRenderer;
}

void QgsRuleBasedRendererV2Widget::addRule()
{
  QgsSymbolV2* s = QgsSymbolV2::defaultSymbol( mLayer->geometryType() );
  QgsRuleBasedRendererV2::Rule* newrule = new QgsRuleBasedRendererV2::Rule( s );

  QgsRendererRulePropsDialog dlg( newrule, mLayer, mStyle );
  if ( dlg.exec() )
  {
    dlg.updateRuleFromGui();

    QgsRuleBasedRendererV2::Rule* current = currentRule();
    if ( current )
    {
      // add after this rule
      QModelIndex currentIndex = viewRules->selectionModel()->currentIndex();
      mModel->insertRule( currentIndex.parent(), currentIndex.row() + 1, newrule );
    }
    else
    {
      // append to root rule
      int rows = mModel->rowCount();
      mModel->insertRule( QModelIndex(), rows, newrule );
    }
  }
  else
  {
    delete newrule;
  }
}

QgsRuleBasedRendererV2::Rule* QgsRuleBasedRendererV2Widget::currentRule()
{
  QItemSelectionModel* sel = viewRules->selectionModel();
  QModelIndex idx = sel->currentIndex();
  if ( !idx.isValid() )
    return NULL;
  return static_cast<QgsRuleBasedRendererV2::Rule*>( idx.internalPointer() );
}

void QgsRuleBasedRendererV2Widget::editRule()
{
  editRule( viewRules->selectionModel()->currentIndex() );
}

void QgsRuleBasedRendererV2Widget::editRule( const QModelIndex& index )
{
  if ( !index.isValid() )
    return;
  QgsRuleBasedRendererV2::Rule* rule = static_cast<QgsRuleBasedRendererV2::Rule*>( index.internalPointer() );

  QgsRendererRulePropsDialog dlg( rule, mLayer, mStyle );
  if ( dlg.exec() )
  {
    // update rule
    dlg.updateRuleFromGui();

    // model should know about the change and emit dataChanged signal for the view
    mModel->updateRule( index );
  }
}

void QgsRuleBasedRendererV2Widget::removeRule()
{
  QModelIndex index = viewRules->selectionModel()->currentIndex();
  if ( !index.isValid() )
    return;

  mModel->removeRule( index );
}


void QgsRuleBasedRendererV2Widget::moveUp()
{
  // TODO: solve directly by drag'n'drop
}


void QgsRuleBasedRendererV2Widget::moveDown()
{
  // TODO: solve directly by drag'n'drop
}




#include "qgscategorizedsymbolrendererv2.h"
#include "qgscategorizedsymbolrendererv2widget.h"
#include "qgsgraduatedsymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2widget.h"
#include "qgssearchquerybuilder.h"
#include <QDialogButtonBox>
#include <QInputDialog>

void QgsRuleBasedRendererV2Widget::refineRule( int type )
{
  QModelIndex index = viewRules->selectionModel()->currentIndex();
  if ( !index.isValid() )
    return;

  QgsRuleBasedRendererV2::Rule* initialRule = static_cast<QgsRuleBasedRendererV2::Rule*>( index.internalPointer() );

  if ( type == 0 ) // categories
    refineRuleCategoriesGui( initialRule );
  else if ( type == 1 ) // ranges
    refineRuleRangesGui( initialRule );
  else // scales
    refineRuleScalesGui( initialRule );

  // TODO: set initial rule's symbol to NULL (?)

  // TODO: let model know things have changed
  mModel->updateRule( index );
}

void QgsRuleBasedRendererV2Widget::refineRuleCategories()
{
  refineRule( 0 );
}

void QgsRuleBasedRendererV2Widget::refineRuleRanges()
{
  refineRule( 1 );
}

void QgsRuleBasedRendererV2Widget::refineRuleScales()
{
  refineRule( 2 );
}

void QgsRuleBasedRendererV2Widget::refineRuleCategoriesGui( QgsRuleBasedRendererV2::Rule* initialRule )
{
  QDialog dlg;
  dlg.setWindowTitle( tr( "Refine a rule to categories" ) );
  QVBoxLayout* l = new QVBoxLayout();
  QgsCategorizedSymbolRendererV2Widget* w = new QgsCategorizedSymbolRendererV2Widget( mLayer, mStyle, NULL );
  l->addWidget( w );
  QDialogButtonBox* bb = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  l->addWidget( bb );
  connect( bb, SIGNAL( accepted() ), &dlg, SLOT( accept() ) );
  connect( bb, SIGNAL( rejected() ), &dlg, SLOT( reject() ) );
  dlg.setLayout( l );

  if ( !dlg.exec() )
    return;

  // create new rules
  QgsCategorizedSymbolRendererV2* r = static_cast<QgsCategorizedSymbolRendererV2*>( w->renderer() );
  QgsRuleBasedRendererV2::refineRuleCategories( initialRule, r );
}


void QgsRuleBasedRendererV2Widget::refineRuleRangesGui( QgsRuleBasedRendererV2::Rule* initialRule )
{
  QDialog dlg;
  dlg.setWindowTitle( tr( "Refine a rule to ranges" ) );
  QVBoxLayout* l = new QVBoxLayout();
  QgsGraduatedSymbolRendererV2Widget* w = new QgsGraduatedSymbolRendererV2Widget( mLayer, mStyle, NULL );
  l->addWidget( w );
  QDialogButtonBox* bb = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  l->addWidget( bb );
  connect( bb, SIGNAL( accepted() ), &dlg, SLOT( accept() ) );
  connect( bb, SIGNAL( rejected() ), &dlg, SLOT( reject() ) );
  dlg.setLayout( l );

  if ( !dlg.exec() )
    return;

  // create new rules
  QgsGraduatedSymbolRendererV2* r = static_cast<QgsGraduatedSymbolRendererV2*>( w->renderer() );
  QgsRuleBasedRendererV2::refineRuleRanges( initialRule, r );
}

void QgsRuleBasedRendererV2Widget::refineRuleScalesGui( QgsRuleBasedRendererV2::Rule* initialRule )
{
  QString txt = QInputDialog::getText( this,
                                       tr( "Scale refinement" ),
                                       tr( "Please enter scale denominators at which will split the rule, separate them by commas (e.g. 1000,5000):" ) );
  if ( txt.isEmpty() )
    return;

  QList<int> scales;
  bool ok;
  foreach( QString item, txt.split( ',' ) )
  {
    int scale = item.toInt( &ok );
    if ( ok )
      scales.append( scale );
    else
      QMessageBox::information( this, tr( "Error" ), QString( tr( "\"%1\" is not valid scale denominator, ignoring it." ) ).arg( item ) );
  }

  QgsRuleBasedRendererV2::refineRuleScales( initialRule, scales );
}

QList<QgsSymbolV2*> QgsRuleBasedRendererV2Widget::selectedSymbols()
{
  QList<QgsSymbolV2*> symbolList;

  if ( !mRenderer )
  {
    return symbolList;
  }

  QItemSelection sel = viewRules->selectionModel()->selection();
  foreach( QItemSelectionRange range, sel )
  {
    QModelIndex parent = range.parent();
    QgsRuleBasedRendererV2::Rule* parentRule = !parent.isValid() ? mRenderer->rootRule() :
        static_cast<QgsRuleBasedRendererV2::Rule*>( parent.internalPointer() );
    QgsRuleBasedRendererV2::RuleList& children = parentRule->children();
    for ( int row = range.top(); row <= range.bottom(); row++ )
    {
      symbolList.append( children[row]->symbol() );
    }
  }

  return symbolList;
}

void QgsRuleBasedRendererV2Widget::refreshSymbolView()
{
  // TODO: model/view
  /*
  if ( treeRules )
  {
    treeRules->populateRules();
  }
  */
}

#include "qgssymbollevelsv2dialog.h"

void QgsRuleBasedRendererV2Widget::setRenderingOrder()
{
  QgsSymbolV2List symbols = mRenderer->symbols();

  QgsSymbolLevelsV2Dialog dlg( symbols, true, this );
  dlg.exec();
}


///////////

QgsRendererRulePropsDialog::QgsRendererRulePropsDialog( QgsRuleBasedRendererV2::Rule* rule, QgsVectorLayer* layer, QgsStyleV2* style )
    : mRule( rule ), mLayer( layer )
{
  setupUi( this );

  editFilter->setText( mRule->filterExpression() );
  editLabel->setText( mRule->label() );
  editDescription->setText( mRule->description() );

  if ( mRule->dependsOnScale() )
  {
    groupScale->setChecked( true );
    spinMinScale->setValue( rule->scaleMinDenom() );
    spinMaxScale->setValue( rule->scaleMaxDenom() );
  }

  QgsSymbolV2SelectorDialog* symbolSel = new QgsSymbolV2SelectorDialog( mRule->symbol(), style, mLayer, this, true );
  QVBoxLayout* l = new QVBoxLayout;
  l->addWidget( symbolSel );
  groupSymbol->setLayout( l );

  connect( btnExpressionBuilder, SIGNAL( clicked() ), this, SLOT( buildExpression() ) );
  connect( btnTestFilter, SIGNAL( clicked() ), this, SLOT( testFilter() ) );
}

void QgsRendererRulePropsDialog::buildExpression()
{
  QgsSearchQueryBuilder dlg( mLayer, this );
  dlg.setSearchString( editFilter->text() );

  if ( dlg.exec() )
    editFilter->setText( dlg.searchString() );
}

void QgsRendererRulePropsDialog::testFilter()
{
  QgsExpression filter( editFilter->text() );
  if ( filter.hasParserError() )
  {
    QMessageBox::critical( this, tr( "Error" ),  tr( "Filter expression parsing error:\n" ) + filter.parserErrorString() );
    return;
  }

  const QgsFieldMap& fields = mLayer->pendingFields();

  if ( !filter.prepare( fields ) )
  {
    QMessageBox::critical( this, tr( "Evaluation error" ), filter.evalErrorString() );
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  mLayer->select( fields.keys(), QgsRectangle(), false );

  int count = 0;
  QgsFeature f;
  while ( mLayer->nextFeature( f ) )
  {
    QVariant value = filter.evaluate( &f );
    if ( value.toInt() != 0 )
      count++;
    if ( filter.hasEvalError() )
      break;
  }

  QApplication::restoreOverrideCursor();

  QMessageBox::information( this, tr( "Filter" ), tr( "Filter returned %n feature(s)", "number of filtered features", count ) );
}

void QgsRendererRulePropsDialog::updateRuleFromGui()
{
  mRule->setFilterExpression( editFilter->text() );
  mRule->setLabel( editLabel->text() );
  mRule->setDescription( editDescription->text() );
  mRule->setScaleMinDenom( groupScale->isChecked() ? spinMinScale->value() : 0 );
  mRule->setScaleMaxDenom( groupScale->isChecked() ? spinMaxScale->value() : 0 );
}

////////

/*
  setDragEnabled(true);
  viewport()->setAcceptDrops(true);
  setDropIndicatorShown(true);
  setDragDropMode(QAbstractItemView::InternalMove);
*/

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

/////

QgsRuleBasedRendererV2Model::QgsRuleBasedRendererV2Model( QgsRuleBasedRendererV2* r )
    : mR( r )
{
}

Qt::ItemFlags QgsRuleBasedRendererV2Model::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return 0;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant QgsRuleBasedRendererV2Model::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsRuleBasedRendererV2::Rule* rule = static_cast<QgsRuleBasedRendererV2::Rule*>( index.internalPointer() );

  if ( role == Qt::DisplayRole )
  {
    switch ( index.column() )
    {
      case 0: return rule->label();
      case 1: return rule->filterExpression().isEmpty() ? tr( "(no filter)" ) : rule->filterExpression();
      case 2: return rule->dependsOnScale() ? _formatScale( rule->scaleMinDenom() ) : QVariant();
      case 3: return rule->dependsOnScale() ? _formatScale( rule->scaleMaxDenom() ) : QVariant();
      default: return QVariant();
    }
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 && rule->symbol() )
  {
    return QgsSymbolLayerV2Utils::symbolPreviewIcon( rule->symbol(), QSize( 16, 16 ) );
  }
  else if ( role == Qt::TextAlignmentRole )
  {
    return ( index.column() == 2 || index.column() == 3 ) ? Qt::AlignRight : Qt::AlignLeft;
  }
  else if ( role == Qt::EditRole )
  {
    switch ( index.column() )
    {
      case 0: return rule->label();
      case 1: return rule->filterExpression();
      case 2: return rule->scaleMinDenom();
      case 3: return rule->scaleMaxDenom();
      default: return QVariant();
    }
  }
  else
    return QVariant();
}

QVariant QgsRuleBasedRendererV2Model::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < 5 )
  {
    QStringList lst; lst << tr( "Label" ) << tr( "Rule" ) << tr( "Min. scale" ) << tr( "Max.scale" );
    return lst[section];
  }

  return QVariant();
}

int QgsRuleBasedRendererV2Model::rowCount( const QModelIndex &parent ) const
{
  QgsRuleBasedRendererV2::Rule* parentRule;
  if ( parent.column() > 0 )
    return 0;

  if ( !parent.isValid() )
    parentRule = mR->rootRule();
  else
    parentRule = static_cast<QgsRuleBasedRendererV2::Rule*>( parent.internalPointer() );

  return parentRule->children().count();
}

int QgsRuleBasedRendererV2Model::columnCount( const QModelIndex & ) const
{
  return 4;
}

QModelIndex QgsRuleBasedRendererV2Model::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  QgsRuleBasedRendererV2::Rule* parentRule;

  if ( !parent.isValid() )
    parentRule = mR->rootRule();
  else
    parentRule = static_cast<QgsRuleBasedRendererV2::Rule*>( parent.internalPointer() );

  QgsRuleBasedRendererV2::Rule* childRule = parentRule->children()[row];
  if ( childRule )
    return createIndex( row, column, childRule );
  else
    return QModelIndex();
}

QModelIndex QgsRuleBasedRendererV2Model::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  QgsRuleBasedRendererV2::Rule* childRule = static_cast<QgsRuleBasedRendererV2::Rule*>( index.internalPointer() );
  QgsRuleBasedRendererV2::Rule* parentRule = childRule->parent();

  if ( parentRule == mR->rootRule() )
    return QModelIndex();

  int row = parentRule->children().indexOf( childRule );

  return createIndex( row, 0, parentRule );
}

bool QgsRuleBasedRendererV2Model::setData( const QModelIndex & index, const QVariant & value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
    return false;

  QgsRuleBasedRendererV2::Rule* rule = static_cast<QgsRuleBasedRendererV2::Rule*>( index.internalPointer() );

  switch ( index.column() )
  {
    case 0: // label
      rule->setLabel( value.toString() );
      break;
    case 1: // filter
      rule->setFilterExpression( value.toString() );
      break;
    case 2: // scale min
      rule->setScaleMinDenom( value.toInt() );
      break;
    case 3: // scale max
      rule->setScaleMaxDenom( value.toInt() );
      break;
    default:
      return false;
  }

  emit dataChanged( index, index );
  return true;
}


void QgsRuleBasedRendererV2Model::insertRule( const QModelIndex& parent, int before, QgsRuleBasedRendererV2::Rule* newrule )
{
  beginInsertRows( parent, before, before );

  QgsRuleBasedRendererV2::Rule* parentRule = parent.isValid() ?
      static_cast<QgsRuleBasedRendererV2::Rule*>( parent.internalPointer() ) : mR->rootRule();
  parentRule->insertChild( before, newrule );

  endInsertRows();
}

void QgsRuleBasedRendererV2Model::updateRule( const QModelIndex& index )
{
  emit dataChanged( index, index );
}

void QgsRuleBasedRendererV2Model::removeRule( const QModelIndex& index )
{
  beginRemoveRows( index.parent(), index.row(), index.row() );

  QgsRuleBasedRendererV2::Rule* rule = static_cast<QgsRuleBasedRendererV2::Rule*>( index.internalPointer() );
  rule->parent()->removeChild( rule );

  endRemoveRows();
}
