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
#include <QSettings>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QMessageBox>

//#include "modeltest.h"

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
  //new ModelTest( mModel, this ); // for model validity checking
  viewRules->setModel( mModel );

  mRefineMenu = new QMenu( "Refine current rule", btnRefineRule );
  mRefineMenu->addAction( tr( "Add scales to rule" ), this, SLOT( refineRuleScales() ) );
  mRefineMenu->addAction( tr( "Add categories to rule" ), this, SLOT( refineRuleCategories() ) );
  mRefineMenu->addAction( tr( "Add ranges to rule" ), this, SLOT( refineRuleRanges() ) );
  btnRefineRule->setMenu( mRefineMenu );
  contextMenu->addMenu( mRefineMenu );

  btnAddRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.png" ) ) );
  btnEditRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.png" ) ) );
  btnRemoveRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.png" ) ) );

  connect( viewRules, SIGNAL( doubleClicked( const QModelIndex & ) ), this, SLOT( editRule( const QModelIndex & ) ) );

  // support for context menu (now handled generically)
  connect( viewRules, SIGNAL( customContextMenuRequested( const QPoint& ) ),  this, SLOT( contextMenuViewCategories( const QPoint& ) ) );

  connect( viewRules->selectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex ) ), this, SLOT( currentRuleChanged( QModelIndex, QModelIndex ) ) );

  connect( btnAddRule, SIGNAL( clicked() ), this, SLOT( addRule() ) );
  connect( btnEditRule, SIGNAL( clicked() ), this, SLOT( editRule() ) );
  connect( btnRemoveRule, SIGNAL( clicked() ), this, SLOT( removeRule() ) );

  connect( btnRenderingOrder, SIGNAL( clicked() ), this, SLOT( setRenderingOrder() ) );

  currentRuleChanged();

  // store/restore header section widths
  connect( viewRules->header(), SIGNAL( sectionResized( int, int, int ) ), this, SLOT( saveSectionWidth( int, int, int ) ) );

  restoreSectionWidths();

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
  return mModel->ruleForIndex( idx );
}

void QgsRuleBasedRendererV2Widget::editRule()
{
  editRule( viewRules->selectionModel()->currentIndex() );
}

void QgsRuleBasedRendererV2Widget::editRule( const QModelIndex& index )
{
  if ( !index.isValid() )
    return;
  QgsRuleBasedRendererV2::Rule* rule = mModel->ruleForIndex( index );

  QgsRendererRulePropsDialog dlg( rule, mLayer, mStyle );
  if ( dlg.exec() )
  {
    // model should know about the change and emit dataChanged signal for the view
    mModel->updateRule( index.parent(), index.row() );
  }
}

void QgsRuleBasedRendererV2Widget::removeRule()
{
  QItemSelection sel = viewRules->selectionModel()->selection();
  QgsDebugMsg( QString( "REMOVE RULES!!! ranges: %1" ).arg( sel.count() ) );
  foreach ( QItemSelectionRange range, sel )
  {
    QgsDebugMsg( QString( "RANGE: r %1 - %2" ).arg( range.top() ).arg( range.bottom() ) );
    if ( range.isValid() )
      mModel->removeRows( range.top(), range.bottom() - range.top() + 1, range.parent() );
  }
  // make sure that the selection is gone
  viewRules->selectionModel()->clear();
}

void QgsRuleBasedRendererV2Widget::currentRuleChanged( const QModelIndex& current, const QModelIndex& previous )
{
  Q_UNUSED( previous );
  btnRefineRule->setEnabled( current.isValid() );
}


#include "qgscategorizedsymbolrendererv2.h"
#include "qgscategorizedsymbolrendererv2widget.h"
#include "qgsgraduatedsymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2widget.h"
#include "qgsexpressionbuilderdialog.h"
#include <QDialogButtonBox>
#include <QInputDialog>

void QgsRuleBasedRendererV2Widget::refineRule( int type )
{
  QModelIndexList indexlist = viewRules->selectionModel()->selectedRows();

  if ( indexlist.isEmpty() )
    return;


  if ( type == 0 ) // categories
    refineRuleCategoriesGui( indexlist );
  else if ( type == 1 ) // ranges
    refineRuleRangesGui( indexlist );
  else // scales
    refineRuleScalesGui( indexlist );

  // TODO: set initial rule's symbol to NULL (?)

  // show the newly added rules
  foreach ( QModelIndex index, indexlist )
    viewRules->expand( index );
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

void QgsRuleBasedRendererV2Widget::refineRuleCategoriesGui( const QModelIndexList& indexList )
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
  foreach ( QModelIndex index, indexList )
  {
    QgsRuleBasedRendererV2::Rule* initialRule = mModel->ruleForIndex( index );
    mModel->willAddRules( index, r->categories().count() );
    QgsRuleBasedRendererV2::refineRuleCategories( initialRule, r );
  }
  mModel->finishedAddingRules();
}


void QgsRuleBasedRendererV2Widget::refineRuleRangesGui( const QModelIndexList& indexList )
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
  foreach ( QModelIndex index, indexList )
  {
    QgsRuleBasedRendererV2::Rule* initialRule = mModel->ruleForIndex( index );
    mModel->willAddRules( index, r->ranges().count() );
    QgsRuleBasedRendererV2::refineRuleRanges( initialRule, r );
  }
  mModel->finishedAddingRules();
}

void QgsRuleBasedRendererV2Widget::refineRuleScalesGui( const QModelIndexList& indexList )
{
  foreach ( QModelIndex index, indexList )
  {
    QgsRuleBasedRendererV2::Rule* initialRule = mModel->ruleForIndex( index );

    // If any of the rules don't have a symbol let the user know and exit.
    if ( initialRule->symbol() == NULL )
    {
      QMessageBox::warning( this, tr( "Scale refinement" ), tr( "Parent rule %1 must have a symbol for this operation." ).arg( initialRule->label() ) );
      return;
    }
  }

  QString txt = QInputDialog::getText( this,
                                       tr( "Scale refinement" ),
                                       tr( "Please enter scale denominators at which will split the rule, separate them by commas (e.g. 1000,5000):" ) );
  if ( txt.isEmpty() )
    return;

  QList<int> scales;
  bool ok;
  foreach ( QString item, txt.split( ',' ) )
  {
    int scale = item.toInt( &ok );
    if ( ok )
      scales.append( scale );
    else
      QMessageBox::information( this, tr( "Error" ), QString( tr( "\"%1\" is not valid scale denominator, ignoring it." ) ).arg( item ) );
  }

  foreach ( QModelIndex index, indexList )
  {
    QgsRuleBasedRendererV2::Rule* initialRule = mModel->ruleForIndex( index );
    mModel->willAddRules( index, scales.count() + 1 );
    QgsRuleBasedRendererV2::refineRuleScales( initialRule, scales );
  }
  mModel->finishedAddingRules();
}

QList<QgsSymbolV2*> QgsRuleBasedRendererV2Widget::selectedSymbols()
{
  QList<QgsSymbolV2*> symbolList;

  if ( !mRenderer )
  {
    return symbolList;
  }

  QItemSelection sel = viewRules->selectionModel()->selection();
  foreach ( QItemSelectionRange range, sel )
  {
    QModelIndex parent = range.parent();
    QgsRuleBasedRendererV2::Rule* parentRule = mModel->ruleForIndex( parent );
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
  QgsLegendSymbolList lst = mRenderer->legendSymbolItems();

  QgsSymbolLevelsV2Dialog dlg( lst, true, this );
  dlg.setForceOrderingEnabled( true );

  dlg.exec();
}

void QgsRuleBasedRendererV2Widget::saveSectionWidth( int section, int oldSize, int newSize )
{
  Q_UNUSED( oldSize );
  // skip last section, as it stretches
  if ( section == 3 )
    return;
  QSettings settings;
  QString path = "/Windows/RuleBasedTree/sectionWidth/" + QString::number( section );
  settings.setValue( path, newSize );
}

void QgsRuleBasedRendererV2Widget::restoreSectionWidths()
{
  QSettings settings;
  QString path = "/Windows/RuleBasedTree/sectionWidth/";
  QHeaderView* head = viewRules->header();
  head->resizeSection( 0, settings.value( path + QString::number( 0 ), 200 ).toInt() );
  head->resizeSection( 1, settings.value( path + QString::number( 1 ), 200 ).toInt() );
  head->resizeSection( 2, settings.value( path + QString::number( 2 ), 100 ).toInt() );
}


///////////

QgsRendererRulePropsDialog::QgsRendererRulePropsDialog( QgsRuleBasedRendererV2::Rule* rule, QgsVectorLayer* layer, QgsStyleV2* style )
    : mRule( rule ), mLayer( layer ), mSymbolSelector( NULL ), mSymbol( NULL )
{
  setupUi( this );

  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

  editFilter->setText( mRule->filterExpression() );
  editFilter->setToolTip( mRule->filterExpression() );
  editLabel->setText( mRule->label() );
  editDescription->setText( mRule->description() );
  editDescription->setToolTip( mRule->description() );

  if ( mRule->dependsOnScale() )
  {
    groupScale->setChecked( true );
    spinMinScale->setValue( rule->scaleMinDenom() );
    spinMaxScale->setValue( rule->scaleMaxDenom() );
  }

  if ( mRule->symbol() )
  {
    groupSymbol->setChecked( true );
    mSymbol = mRule->symbol()->clone(); // use a clone!
  }
  else
  {
    groupSymbol->setChecked( false );
    mSymbol = QgsSymbolV2::defaultSymbol( mLayer->geometryType() );
  }

  mSymbolSelector = new QgsSymbolV2SelectorDialog( mSymbol, style, mLayer, this, true );
  QVBoxLayout* l = new QVBoxLayout;
  l->addWidget( mSymbolSelector );
  groupSymbol->setLayout( l );

  connect( btnExpressionBuilder, SIGNAL( clicked() ), this, SLOT( buildExpression() ) );
  connect( btnTestFilter, SIGNAL( clicked() ), this, SLOT( testFilter() ) );
}

QgsRendererRulePropsDialog::~QgsRendererRulePropsDialog()
{
  delete mSymbol;
}

void QgsRendererRulePropsDialog::buildExpression()
{
  QgsExpressionBuilderDialog dlg( mLayer, editFilter->text(), this );

  if ( dlg.exec() )
    editFilter->setText( dlg.expressionText() );
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

void QgsRendererRulePropsDialog::accept()
{
  mRule->setFilterExpression( editFilter->text() );
  mRule->setLabel( editLabel->text() );
  mRule->setDescription( editDescription->text() );
  mRule->setScaleMinDenom( groupScale->isChecked() ? spinMinScale->value() : 0 );
  mRule->setScaleMaxDenom( groupScale->isChecked() ? spinMaxScale->value() : 0 );
  mRule->setSymbol( groupSymbol->isChecked() ? mSymbol->clone() : NULL );

  QDialog::accept();
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
    return Qt::ItemIsDropEnabled;

  // allow drop only at first column
  Qt::ItemFlag drop = ( index.column() == 0 ? Qt::ItemIsDropEnabled : Qt::NoItemFlags );

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable |
         Qt::ItemIsEditable |
         Qt::ItemIsDragEnabled | drop;
}

QVariant QgsRuleBasedRendererV2Model::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsRuleBasedRendererV2::Rule* rule = ruleForIndex( index );

  if ( role == Qt::DisplayRole || role == Qt::ToolTipRole )
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
  if ( parent.column() > 0 )
    return 0;

  QgsRuleBasedRendererV2::Rule* parentRule = ruleForIndex( parent );

  return parentRule->children().count();
}

int QgsRuleBasedRendererV2Model::columnCount( const QModelIndex & ) const
{
  return 4;
}

QModelIndex QgsRuleBasedRendererV2Model::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    QgsRuleBasedRendererV2::Rule* parentRule = ruleForIndex( parent );
    QgsRuleBasedRendererV2::Rule* childRule = parentRule->children()[row];
    return createIndex( row, column, childRule );
  }
  return QModelIndex();
}

QModelIndex QgsRuleBasedRendererV2Model::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  QgsRuleBasedRendererV2::Rule* childRule = ruleForIndex( index );
  QgsRuleBasedRendererV2::Rule* parentRule = childRule->parent();

  if ( parentRule == mR->rootRule() )
    return QModelIndex();

  // this is right: we need to know row number of our parent (in our grandparent)
  int row = parentRule->parent()->children().indexOf( parentRule );

  return createIndex( row, 0, parentRule );
}

bool QgsRuleBasedRendererV2Model::setData( const QModelIndex & index, const QVariant & value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
    return false;

  QgsRuleBasedRendererV2::Rule* rule = ruleForIndex( index );

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

Qt::DropActions QgsRuleBasedRendererV2Model::supportedDropActions() const
{
  return Qt::MoveAction; // | Qt::CopyAction
}

QStringList QgsRuleBasedRendererV2Model::mimeTypes() const
{
  QStringList types;
  types << "application/vnd.text.list";
  return types;
}

QMimeData *QgsRuleBasedRendererV2Model::mimeData( const QModelIndexList &indexes ) const
{
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream( &encodedData, QIODevice::WriteOnly );

  foreach ( const QModelIndex &index, indexes )
  {
    // each item consists of several columns - let's add it with just first one
    if ( !index.isValid() || index.column() != 0 )
      continue;

    QgsRuleBasedRendererV2::Rule* rule = ruleForIndex( index );
    QDomDocument doc;
    QgsSymbolV2Map symbols;

    QDomElement rootElem = doc.createElement( "rule_mime" );
    QDomElement rulesElem = rule->save( doc, symbols );
    rootElem.appendChild( rulesElem );
    QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( symbols, "symbols", doc );
    rootElem.appendChild( symbolsElem );
    doc.appendChild( rootElem );

    stream << doc.toString( -1 );
  }

  mimeData->setData( "application/vnd.text.list", encodedData );
  return mimeData;
}

bool QgsRuleBasedRendererV2Model::dropMimeData( const QMimeData *data,
    Qt::DropAction action, int row, int column, const QModelIndex &parent )
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
    QDomElement symbolsElem = rootElem.firstChildElement( "symbols" );
    if ( symbolsElem.isNull() )
      continue;
    QgsSymbolV2Map symbolMap = QgsSymbolLayerV2Utils::loadSymbols( symbolsElem );
    QDomElement ruleElem = rootElem.firstChildElement( "rule" );
    QgsRuleBasedRendererV2::Rule* rule = QgsRuleBasedRendererV2::Rule::create( ruleElem, symbolMap );

    insertRule( parent, row + rows, rule );

    ++rows;
  }
  return true;
}

QgsRuleBasedRendererV2::Rule* QgsRuleBasedRendererV2Model::ruleForIndex( const QModelIndex& index ) const
{
  if ( index.isValid() )
    return static_cast<QgsRuleBasedRendererV2::Rule*>( index.internalPointer() );
  return mR->rootRule();
}

bool QgsRuleBasedRendererV2Model::removeRows( int row, int count, const QModelIndex & parent )
{
  QgsRuleBasedRendererV2::Rule* parentRule = ruleForIndex( parent );

  if ( row < 0 || row >= parentRule->children().count() )
    return false;

  QgsDebugMsg( QString( "Called: row %1 count %2 parent ~~%3~~" ).arg( row ).arg( count ).arg( parentRule->dump() ) );

  beginRemoveRows( parent, row, row + count - 1 );

  for ( int i = 0; i < count; i++ )
  {
    if ( row < parentRule->children().count() )
    {
      //QgsRuleBasedRendererV2::Rule* r = parentRule->children()[row];
      parentRule->removeChildAt( row );
      //parentRule->takeChildAt( row );
    }
    else
    {
      QgsDebugMsg( "trying to remove invalid index - this should not happen!" );
    }
  }

  endRemoveRows();

  return true;
}


void QgsRuleBasedRendererV2Model::insertRule( const QModelIndex& parent, int before, QgsRuleBasedRendererV2::Rule* newrule )
{
  beginInsertRows( parent, before, before );

  QgsDebugMsg( QString( "insert before %1 rule: %2" ).arg( before ).arg( newrule->dump() ) );

  QgsRuleBasedRendererV2::Rule* parentRule = ruleForIndex( parent );
  parentRule->insertChild( before, newrule );

  endInsertRows();
}

void QgsRuleBasedRendererV2Model::updateRule( const QModelIndex& parent, int row )
{
  emit dataChanged( index( row, 0, parent ),
                    index( row, columnCount( parent ), parent ) );
}

void QgsRuleBasedRendererV2Model::removeRule( const QModelIndex& index )
{
  if ( !index.isValid() )
    return;

  beginRemoveRows( index.parent(), index.row(), index.row() );

  QgsRuleBasedRendererV2::Rule* rule = ruleForIndex( index );
  rule->parent()->removeChild( rule );

  endRemoveRows();
}

void QgsRuleBasedRendererV2Model::willAddRules( const QModelIndex& parent, int count )
{
  int row = rowCount( parent ); // only consider appending
  beginInsertRows( parent, row, row + count - 1 );
}

void QgsRuleBasedRendererV2Model::finishedAddingRules()
{
  emit endInsertRows();
}
