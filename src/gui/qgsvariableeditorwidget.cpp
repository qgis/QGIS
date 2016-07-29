/***************************************************************************
     qgsvariableeditorwidget.cpp
     ---------------------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvariableeditorwidget.h"
#include "qgsexpressioncontext.h"
#include "qgsfeature.h"
#include "qgsapplication.h"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QSettings>


//
// QgsVariableEditorWidget
//

QgsVariableEditorWidget::QgsVariableEditorWidget( QWidget *parent )
    : QWidget( parent )
    , mContext( nullptr )
    , mEditableScopeIndex( -1 )
    , mShown( false )
{
  QVBoxLayout* verticalLayout = new QVBoxLayout( this );
  verticalLayout->setSpacing( 3 );
  verticalLayout->setContentsMargins( 3, 3, 3, 3 );
  mTreeWidget = new QgsVariableEditorTree( this );
  mTreeWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  verticalLayout->addWidget( mTreeWidget );
  QHBoxLayout* horizontalLayout = new QHBoxLayout();
  horizontalLayout->setSpacing( 6 );
  QSpacerItem* horizontalSpacer = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  horizontalLayout->addItem( horizontalSpacer );
  mAddButton = new QPushButton();
  mAddButton->setIcon( QgsApplication::getThemeIcon( "/symbologyAdd.svg" ) );
  mAddButton->setEnabled( false );
  horizontalLayout->addWidget( mAddButton );
  mRemoveButton = new QPushButton();
  mRemoveButton->setIcon( QgsApplication::getThemeIcon( "/symbologyRemove.svg" ) );
  mRemoveButton->setEnabled( false );
  horizontalLayout->addWidget( mRemoveButton );
  verticalLayout->addLayout( horizontalLayout );
  connect( mRemoveButton, SIGNAL( clicked() ), this, SLOT( on_mRemoveButton_clicked() ) );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( on_mAddButton_clicked() ) );
  connect( mTreeWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( selectionChanged() ) );
  connect( mTreeWidget, SIGNAL( scopeChanged() ), this, SIGNAL( scopeChanged() ) );

  //setContext clones context
  QgsExpressionContext* context = new QgsExpressionContext();
  setContext( context );
  delete context;
}

QgsVariableEditorWidget::~QgsVariableEditorWidget()
{
  QSettings settings;
  settings.setValue( saveKey() + "column0width", mTreeWidget->header()->sectionSize( 0 ) );
}

void QgsVariableEditorWidget::showEvent( QShowEvent * event )
{
  // initialise widget on first show event only
  if ( mShown )
  {
    event->accept();
    return;
  }

  //restore split size
  QSettings settings;
  QVariant val;
  val = settings.value( saveKey() + "column0width" );
  bool ok;
  int sectionSize = val.toInt( &ok );
  if ( ok )
  {
    mTreeWidget->header()->resizeSection( 0, sectionSize );
  }
  mShown = true;

  QWidget::showEvent( event );
}

void QgsVariableEditorWidget::setContext( QgsExpressionContext* context )
{
  mContext.reset( new QgsExpressionContext( *context ) );
  reloadContext();
}

void QgsVariableEditorWidget::reloadContext()
{
  mTreeWidget->resetTree();
  mTreeWidget->setContext( mContext.data() );
  mTreeWidget->refreshTree();
}

void QgsVariableEditorWidget::setEditableScopeIndex( int scopeIndex )
{
  mEditableScopeIndex = scopeIndex;
  if ( mEditableScopeIndex >= 0 )
  {
    mAddButton->setEnabled( true );
  }
  mTreeWidget->setEditableScopeIndex( scopeIndex );
  mTreeWidget->refreshTree();
}

QgsExpressionContextScope* QgsVariableEditorWidget::editableScope() const
{
  if ( !mContext || mEditableScopeIndex < 0 || mEditableScopeIndex >= mContext->scopeCount() )
  {
    return nullptr;
  }
  return mContext->scope( mEditableScopeIndex );
}

QgsStringMap QgsVariableEditorWidget::variablesInActiveScope() const
{
  QgsStringMap variables;
  if ( !mContext || mEditableScopeIndex < 0 || mEditableScopeIndex >= mContext->scopeCount() )
  {
    return variables;
  }

  QgsExpressionContextScope* scope = mContext->scope( mEditableScopeIndex );
  Q_FOREACH ( const QString& variable, scope->variableNames() )
  {
    if ( scope->isReadOnly( variable ) )
      continue;

    variables.insert( variable, scope->variable( variable ).toString() );
  }

  return variables;
}

QString QgsVariableEditorWidget::saveKey() const
{
  // save key for load/save state
  // currently QgsVariableEditorTree/window()/object
  QString setGroup = mSettingGroup.isEmpty() ? objectName() : mSettingGroup;
  QString saveKey = "/QgsVariableEditorTree/" + setGroup + '/';
  return saveKey;
}

void QgsVariableEditorWidget::on_mAddButton_clicked()
{
  if ( mEditableScopeIndex < 0 || mEditableScopeIndex >= mContext->scopeCount() )
    return;

  QgsExpressionContextScope* scope = mContext->scope( mEditableScopeIndex );
  scope->setVariable( "new_variable", QVariant() );
  mTreeWidget->refreshTree();
  QTreeWidgetItem* item = mTreeWidget->itemFromVariable( scope, "new_variable" );
  QModelIndex index = mTreeWidget->itemToIndex( item );
  mTreeWidget->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect );
  mTreeWidget->editItem( item, 0 );

  emit scopeChanged();
}

void QgsVariableEditorWidget::on_mRemoveButton_clicked()
{
  if ( mEditableScopeIndex < 0 || mEditableScopeIndex >= mContext->scopeCount() )
    return;

  QgsExpressionContextScope* editableScope = mContext->scope( mEditableScopeIndex );
  QList<QTreeWidgetItem*> selectedItems = mTreeWidget->selectedItems();

  Q_FOREACH ( QTreeWidgetItem* item, selectedItems )
  {
    if ( !( item->flags() & Qt::ItemIsEditable ) )
      continue;

    QString name = item->text( 0 );
    QgsExpressionContextScope* itemScope = mTreeWidget->scopeFromItem( item );
    if ( itemScope != editableScope )
      continue;

    if ( itemScope->isReadOnly( name ) )
      continue;

    itemScope->removeVariable( name );
    mTreeWidget->removeItem( item );
  }
  mTreeWidget->refreshTree();
}

void QgsVariableEditorWidget::selectionChanged()
{
  if ( mEditableScopeIndex < 0 || mEditableScopeIndex >= mContext->scopeCount() )
  {
    mRemoveButton->setEnabled( false );
    return;
  }

  QgsExpressionContextScope* editableScope = mContext->scope( mEditableScopeIndex );
  QList<QTreeWidgetItem*> selectedItems = mTreeWidget->selectedItems();

  bool removeEnabled = true;
  Q_FOREACH ( QTreeWidgetItem* item, selectedItems )
  {
    if ( !( item->flags() & Qt::ItemIsEditable ) )
    {
      removeEnabled = false;
      break;
    }

    QString name = item->text( 0 );
    QgsExpressionContextScope* itemScope = mTreeWidget->scopeFromItem( item );
    if ( itemScope != editableScope )
    {
      removeEnabled = false;
      break;
    }

    if ( editableScope->isReadOnly( name ) )
    {
      removeEnabled = false;
      break;
    }
  }
  mRemoveButton->setEnabled( removeEnabled );
}


/// @cond PRIVATE
//
// VariableEditorTree
//

QgsVariableEditorTree::QgsVariableEditorTree( QWidget *parent )
    : QTreeWidget( parent )
    , mEditorDelegate( nullptr )
    , mEditableScopeIndex( -1 )
    , mContext( nullptr )
{
  // init icons
  if ( mExpandIcon.isNull() )
  {
    QPixmap pix( 14, 14 );
    pix.fill( Qt::transparent );
    mExpandIcon.addPixmap( QgsApplication::getThemeIcon( "/mIconExpandSmall.svg" ).pixmap( 14, 14 ), QIcon::Normal, QIcon::Off );
    mExpandIcon.addPixmap( QgsApplication::getThemeIcon( "/mIconExpandSmall.svg" ).pixmap( 14, 14 ), QIcon::Selected, QIcon::Off );
    mExpandIcon.addPixmap( QgsApplication::getThemeIcon( "/mIconCollapseSmall.svg" ).pixmap( 14, 14 ), QIcon::Normal, QIcon::On );
    mExpandIcon.addPixmap( QgsApplication::getThemeIcon( "/mIconCollapseSmall.svg" ).pixmap( 14, 14 ), QIcon::Selected, QIcon::On );
  }

  setIconSize( QSize( 18, 18 ) );
  setColumnCount( 2 );
  setHeaderLabels( QStringList() << tr( "Variable" ) << tr( "Value" ) );
  setAlternatingRowColors( true );
  setEditTriggers( QAbstractItemView::AllEditTriggers );
  setRootIsDecorated( false );
  header()->setMovable( false );
  header()->setResizeMode( QHeaderView::Interactive );

  mEditorDelegate = new VariableEditorDelegate( this, this );
  setItemDelegate( mEditorDelegate );
}

QgsExpressionContextScope* QgsVariableEditorTree::scopeFromItem( QTreeWidgetItem *item ) const
{
  if ( !item )
    return nullptr;

  bool ok;
  int contextIndex = item->data( 0, ContextIndex ).toInt( &ok );
  if ( !ok )
    return nullptr;

  if ( !mContext )
  {
    return nullptr;
  }
  else if ( mContext->scopeCount() > contextIndex )
  {
    return mContext->scope( contextIndex );
  }
  else
  {
    return nullptr;
  }
}

QTreeWidgetItem* QgsVariableEditorTree::itemFromVariable( QgsExpressionContextScope *scope, const QString &name ) const
{
  int contextIndex = mContext ? mContext->indexOfScope( scope ) : 0;
  if ( contextIndex < 0 )
    return nullptr;
  return mVariableToItem.value( qMakePair( contextIndex, name ) );
}

QgsExpressionContextScope* QgsVariableEditorTree::editableScope()
{
  if ( !mContext || mEditableScopeIndex < 0 || mEditableScopeIndex >= mContext->scopeCount() )
  {
    return nullptr;
  }

  return mContext->scope( mEditableScopeIndex );
}

void QgsVariableEditorTree::refreshTree()
{
  if ( !mContext || mEditableScopeIndex < 0 )
  {
    clear();
    return;
  }

  //add all scopes from the context
  int scopeIndex = 0;
  Q_FOREACH ( QgsExpressionContextScope* scope, mContext->scopes() )
  {
    refreshScopeItems( scope, scopeIndex );
    scopeIndex++;
  }
}

void QgsVariableEditorTree::refreshScopeVariables( QgsExpressionContextScope* scope, int scopeIndex )
{
  QColor baseColor = rowColor( scopeIndex );
  bool isCurrent = scopeIndex == mEditableScopeIndex;
  QTreeWidgetItem* scopeItem = mScopeToItem.value( scopeIndex );

  Q_FOREACH ( const QString& name, scope->filteredVariableNames() )
  {
    QTreeWidgetItem* item;
    if ( mVariableToItem.contains( qMakePair( scopeIndex, name ) ) )
    {
      item = mVariableToItem.value( qMakePair( scopeIndex, name ) );
    }
    else
    {
      item = new QTreeWidgetItem( scopeItem );
      mVariableToItem.insert( qMakePair( scopeIndex, name ), item );
    }

    bool readOnly = scope->isReadOnly( name );
    bool isActive = true;
    QgsExpressionContextScope* activeScope = nullptr;
    if ( mContext )
    {
      activeScope = mContext->activeScopeForVariable( name );
      isActive = activeScope == scope;
    }

    item->setFlags( item->flags() | Qt::ItemIsEnabled );
    item->setText( 0, name );
    QString value = scope->variable( name ).toString();
    item->setText( 1, value );
    QFont font = item->font( 0 );
    if ( readOnly || !isCurrent )
    {
      font.setItalic( true );
      item->setFlags( item->flags() ^ Qt::ItemIsEditable );
    }
    else
    {
      font.setItalic( false );
      item->setFlags( item->flags() | Qt::ItemIsEditable );
    }
    if ( !isActive )
    {
      //overridden
      font.setStrikeOut( true );
      QString toolTip = tr( "Overridden by value from %1" ).arg( activeScope->name() );
      item->setToolTip( 0, toolTip );
      item->setToolTip( 1, toolTip );
    }
    else
    {
      font.setStrikeOut( false );
      item->setToolTip( 0, name );
      item->setToolTip( 1, value );
    }
    item->setFont( 0, font );
    item->setFont( 1, font );
    item->setData( 0, RowBaseColor, baseColor );
    item->setData( 0, ContextIndex, scopeIndex );
    item->setFirstColumnSpanned( false );
  }
}

void QgsVariableEditorTree::refreshScopeItems( QgsExpressionContextScope* scope, int scopeIndex )
{
  QSettings settings;

  //add top level item
  bool isCurrent = scopeIndex == mEditableScopeIndex;

  QTreeWidgetItem* scopeItem;
  if ( mScopeToItem.contains( scopeIndex ) )
  {
    //retrieve existing item
    scopeItem = mScopeToItem.value( scopeIndex );
  }
  else
  {
    //create new top-level item
    scopeItem = new QTreeWidgetItem();
    mScopeToItem.insert( scopeIndex, scopeItem );
    scopeItem->setFlags( scopeItem->flags() | Qt::ItemIsEnabled );
    scopeItem->setText( 0, scope->name() );
    scopeItem->setFlags( scopeItem->flags() ^ Qt::ItemIsEditable );
    scopeItem->setFirstColumnSpanned( true );
    QFont scopeFont = scopeItem->font( 0 );
    scopeFont .setBold( true );
    scopeItem->setFont( 0, scopeFont );
    scopeItem->setFirstColumnSpanned( true );

    addTopLevelItem( scopeItem );

    //expand by default if current context or context was previously expanded
    if ( isCurrent || settings.value( "QgsVariableEditor/" + scopeItem->text( 0 ) + "/expanded" ).toBool() )
      scopeItem->setExpanded( true );

    scopeItem->setIcon( 0, mExpandIcon );
  }

  refreshScopeVariables( scope, scopeIndex );
}

void QgsVariableEditorTree::removeItem( QTreeWidgetItem *item )
{
  if ( !item )
    return;

  mVariableToItem.remove( mVariableToItem.key( item ) );
  item->parent()->takeChild( item->parent()->indexOfChild( item ) );

  emit scopeChanged();
}

void QgsVariableEditorTree::renameItem( QTreeWidgetItem *item, const QString& name )
{
  if ( !item )
    return;

  int contextIndex = mVariableToItem.key( item ).first;
  mVariableToItem.remove( mVariableToItem.key( item ) );
  mVariableToItem.insert( qMakePair( contextIndex, name ), item );
  item->setText( 0, name );

  emit scopeChanged();
}

void QgsVariableEditorTree::resetTree()
{
  mVariableToItem.clear();
  mScopeToItem.clear();
  clear();
}

void QgsVariableEditorTree::emitChanged()
{
  emit scopeChanged();
}

void QgsVariableEditorTree::drawRow( QPainter* painter, const QStyleOptionViewItem& option,
                                     const QModelIndex& index ) const
{
  QStyleOptionViewItemV3 opt = option;
  QTreeWidgetItem* item = itemFromIndex( index );
  if ( index.parent().isValid() )
  {
    //not a top-level item, so shade row background by context
    const QColor baseColor = item->data( 0, RowBaseColor ).value<QColor>();
    painter->fillRect( option.rect, baseColor );
    opt.palette.setColor( QPalette::AlternateBase, baseColor.lighter( 110 ) );
  }
  QTreeWidget::drawRow( painter, opt, index );
  QColor color = static_cast<QRgb>( QApplication::style()->styleHint( QStyle::SH_Table_GridLineColor, &opt ) );
  painter->save();
  painter->setPen( QPen( color ) );
  painter->drawLine( opt.rect.x(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom() );
  painter->restore();
}

QColor QgsVariableEditorTree::rowColor( int index ) const
{
  //return some nice (inspired by Qt Designer) background row colors
  int colorIdx = index % 6;
  switch ( colorIdx )
  {
    case 0:
      return QColor( 255, 220, 167 );
    case 1:
      return QColor( 255, 255, 191 );
    case 2:
      return QColor( 191, 255, 191 );
    case 3:
      return QColor( 199, 255, 255 );
    case 4:
      return QColor( 234, 191, 255 );
    case 5:
    default:
      return QColor( 255, 191, 239 );
  }
}

void QgsVariableEditorTree::toggleContextExpanded( QTreeWidgetItem* item )
{
  if ( !item )
    return;

  item->setExpanded( !item->isExpanded() );

  //save expanded state
  QSettings settings;
  settings.setValue( "QgsVariableEditor/" + item->text( 0 ) + "/expanded", item->isExpanded() );
}

void QgsVariableEditorTree::editNext( const QModelIndex& index )
{
  if ( !index.isValid() )
    return;

  if ( index.column() == 0 )
  {
    //switch to next column
    QModelIndex nextIndex = index.sibling( index.row(), 1 );
    if ( nextIndex.isValid() )
    {
      setCurrentIndex( nextIndex );
      edit( nextIndex );
    }
  }
  else
  {
    QModelIndex nextIndex = model()->index( index.row() + 1, 0, index.parent() );
    if ( nextIndex.isValid() )
    {
      //start editing next row
      setCurrentIndex( nextIndex );
      edit( nextIndex );
    }
    else
    {
      edit( index );
    }
  }
}

QModelIndex QgsVariableEditorTree::moveCursor( QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers )
{
  if ( cursorAction == QAbstractItemView::MoveNext )
  {
    QModelIndex index = currentIndex();
    if ( index.isValid() )
    {
      if ( index.column() + 1 < model()->columnCount() )
        return index.sibling( index.row(), index.column() + 1 );
      else if ( index.row() + 1 < model()->rowCount( index.parent() ) )
        return index.sibling( index.row() + 1, 0 );
      else
        return QModelIndex();
    }
  }
  else if ( cursorAction == QAbstractItemView::MovePrevious )
  {
    QModelIndex index = currentIndex();
    if ( index.isValid() )
    {
      if ( index.column() >= 1 )
        return index.sibling( index.row(), index.column() - 1 );
      else if ( index.row() >= 1 )
        return index.sibling( index.row() - 1, model()->columnCount() - 1 );
      else
        return QModelIndex();
    }
  }

  return QTreeWidget::moveCursor( cursorAction, modifiers );
}

void QgsVariableEditorTree::keyPressEvent( QKeyEvent *event )
{
  switch ( event->key() )
  {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
    {
      QTreeWidgetItem *item = currentItem();
      if ( item && !item->parent() )
      {
        event->accept();
        toggleContextExpanded( item );
        return;
      }
      else if ( item && ( item->flags() & Qt::ItemIsEditable ) )
      {
        event->accept();
        editNext( currentIndex() );
        return;
      }
      break;
    }
    default:
      break;
  }
  QTreeWidget::keyPressEvent( event );
}

void QgsVariableEditorTree::mousePressEvent( QMouseEvent *event )
{
  QTreeWidget::mousePressEvent( event );
  QTreeWidgetItem* item = itemAt( event->pos() );
  if ( !item )
    return;

  if ( item->parent() )
  {
    //not a top-level item
    return;
  }

  if ( event->pos().x() + header()->offset() > 20 )
  {
    //not clicking on expand icon
    return;
  }

  if ( event->modifiers() & Qt::ShiftModifier )
  {
    //shift modifier expands all
    if ( !item->isExpanded() )
    {
      expandAll();
    }
    else
    {
      collapseAll();
    }
  }
  else
  {
    toggleContextExpanded( item );
  }
}

//
// VariableEditorDelegate
//

QWidget* VariableEditorDelegate::createEditor( QWidget *parent,
    const QStyleOptionViewItem&,
    const QModelIndex &index ) const
{
  if ( !mParentTree )
    return nullptr;

  //no editing for top level items
  if ( !index.parent().isValid() )
    return nullptr;

  QTreeWidgetItem *item = mParentTree->indexToItem( index );
  QgsExpressionContextScope* scope = mParentTree->scopeFromItem( item );
  if ( !item || !scope )
    return nullptr;

  QString variableName = mParentTree->variableNameFromIndex( index );

  //no editing inherited or read-only variables
  if ( scope != mParentTree->editableScope() || scope->isReadOnly( variableName ) )
    return nullptr;

  QLineEdit *lineEdit = new QLineEdit( parent );
  lineEdit->setText( index.column() == 0 ? variableName : mParentTree->editableScope()->variable( variableName ).toString() );
  lineEdit->setAutoFillBackground( true );
  return lineEdit;
}

void VariableEditorDelegate::updateEditorGeometry( QWidget *editor,
    const QStyleOptionViewItem &option,
    const QModelIndex & ) const
{
  editor->setGeometry( option.rect.adjusted( 0, 0, 0, -1 ) );
}

QSize VariableEditorDelegate::sizeHint( const QStyleOptionViewItem &option,
                                        const QModelIndex &index ) const
{
  return QItemDelegate::sizeHint( option, index ) + QSize( 3, 4 );
}

void VariableEditorDelegate::setModelData( QWidget* widget, QAbstractItemModel *model,
    const QModelIndex& index ) const
{
  Q_UNUSED( model );

  if ( !mParentTree )
    return;

  QTreeWidgetItem *item = mParentTree->indexToItem( index );
  QgsExpressionContextScope *scope = mParentTree->scopeFromItem( item );
  if ( !item || !scope )
    return;

  QLineEdit* lineEdit = qobject_cast< QLineEdit* >( widget );
  if ( !lineEdit )
    return;

  QString variableName = mParentTree->variableNameFromIndex( index );
  if ( index.column() == 0 )
  {
    //edited variable name
    QString newName = lineEdit->text();

    //test for validity
    if ( newName == variableName )
    {
      return;
    }
    if ( scope->hasVariable( newName ) )
    {
      //existing name
      QMessageBox::warning( mParentTree, tr( "Rename variable" ), tr( "A variable with the name \"%1\" already exists in this context." ).arg( newName ) );
      newName.append( "_1" );
    }

    QString value = scope->variable( variableName ).toString();
    mParentTree->renameItem( item, newName );
    scope->removeVariable( variableName );
    scope->setVariable( newName, value );
    mParentTree->emitChanged();
  }
  else if ( index.column() == 1 )
  {
    //edited variable value
    QString value = lineEdit->text();
    if ( scope->variable( variableName ).toString() == value )
    {
      return;
    }
    scope->setVariable( variableName, value );
    mParentTree->emitChanged();
  }
  mParentTree->refreshTree();
}

///@endcond
