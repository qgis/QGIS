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
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgsexpression.h"
#include "qgsrendercontext.h"

#include <QVBoxLayout>
#include <QTreeWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QClipboard>

//
// QgsVariableEditorWidget
//

QgsVariableEditorWidget::QgsVariableEditorWidget( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *verticalLayout = new QVBoxLayout( this );
  verticalLayout->setSpacing( 3 );
  verticalLayout->setContentsMargins( 3, 3, 3, 3 );
  mTreeWidget = new QgsVariableEditorTree( this );
  mTreeWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  verticalLayout->addWidget( mTreeWidget );
  QHBoxLayout *horizontalLayout = new QHBoxLayout();
  horizontalLayout->setSpacing( 6 );
  QSpacerItem *horizontalSpacer = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  horizontalLayout->addItem( horizontalSpacer );
  mAddButton = new QPushButton();
  mAddButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/symbologyAdd.svg" ) ) );
  mAddButton->setEnabled( false );
  mAddButton->setToolTip( tr( "Add variable" ) );
  horizontalLayout->addWidget( mAddButton );
  mRemoveButton = new QPushButton();
  mRemoveButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/symbologyRemove.svg" ) ) );
  mRemoveButton->setEnabled( false );
  mRemoveButton->setToolTip( tr( "Remove variable" ) );
  horizontalLayout->addWidget( mRemoveButton );
  verticalLayout->addLayout( horizontalLayout );
  connect( mRemoveButton, &QAbstractButton::clicked, this, &QgsVariableEditorWidget::mRemoveButton_clicked );
  connect( mAddButton, &QAbstractButton::clicked, this, &QgsVariableEditorWidget::mAddButton_clicked );
  connect( mTreeWidget, &QTreeWidget::itemSelectionChanged, this, &QgsVariableEditorWidget::selectionChanged );
  connect( mTreeWidget, &QgsVariableEditorTree::scopeChanged, this, &QgsVariableEditorWidget::scopeChanged );

  //setContext clones context
  QgsExpressionContext *context = new QgsExpressionContext();
  setContext( context );
  delete context;
}

QgsVariableEditorWidget::~QgsVariableEditorWidget()
{
  QgsSettings settings;
  settings.setValue( saveKey() + "column0width", mTreeWidget->header()->sectionSize( 0 ) );
}

void QgsVariableEditorWidget::showEvent( QShowEvent *event )
{
  // initialize widget on first show event only
  if ( mShown )
  {
    event->accept();
    return;
  }

  //restore split size
  const QgsSettings settings;
  QVariant val;
  val = settings.value( saveKey() + "column0width" );
  bool ok;
  const int sectionSize = val.toInt( &ok );
  if ( ok )
  {
    mTreeWidget->header()->resizeSection( 0, sectionSize );
  }
  mShown = true;

  QWidget::showEvent( event );
}

void QgsVariableEditorWidget::setContext( QgsExpressionContext *context )
{
  mContext.reset( new QgsExpressionContext( *context ) );
  reloadContext();
}

void QgsVariableEditorWidget::reloadContext()
{
  mTreeWidget->resetTree();
  mTreeWidget->setContext( mContext.get() );
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

QgsExpressionContextScope *QgsVariableEditorWidget::editableScope() const
{
  if ( !mContext || mEditableScopeIndex < 0 || mEditableScopeIndex >= mContext->scopeCount() )
  {
    return nullptr;
  }
  return mContext->scope( mEditableScopeIndex );
}

QVariantMap QgsVariableEditorWidget::variablesInActiveScope() const
{
  QVariantMap variables;
  if ( !mContext || mEditableScopeIndex < 0 || mEditableScopeIndex >= mContext->scopeCount() )
  {
    return variables;
  }

  QgsExpressionContextScope *scope = mContext->scope( mEditableScopeIndex );
  const auto constVariableNames = scope->variableNames();
  for ( const QString &variable : constVariableNames )
  {
    if ( scope->isReadOnly( variable ) )
      continue;

    variables.insert( variable, scope->variable( variable ) );
  }

  return variables;
}

QString QgsVariableEditorWidget::saveKey() const
{
  // save key for load/save state
  // currently QgsVariableEditorTree/window()/object
  const QString setGroup = mSettingGroup.isEmpty() ? objectName() : mSettingGroup;
  QString saveKey = "/QgsVariableEditorTree/" + setGroup + '/';
  return saveKey;
}

void QgsVariableEditorWidget::mAddButton_clicked()
{
  if ( mEditableScopeIndex < 0 || mEditableScopeIndex >= mContext->scopeCount() )
    return;

  QgsExpressionContextScope *scope = mContext->scope( mEditableScopeIndex );
  scope->setVariable( QStringLiteral( "new_variable" ), QVariant() );
  mTreeWidget->refreshTree();
  QTreeWidgetItem *item = mTreeWidget->itemFromVariable( scope, QStringLiteral( "new_variable" ) );
  const QModelIndex index = mTreeWidget->itemToIndex( item );
  mTreeWidget->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect );
  mTreeWidget->editItem( item, 0 );

  emit scopeChanged();
}

void QgsVariableEditorWidget::mRemoveButton_clicked()
{
  if ( mEditableScopeIndex < 0 || mEditableScopeIndex >= mContext->scopeCount() )
    return;

  QgsExpressionContextScope *editableScope = mContext->scope( mEditableScopeIndex );
  const QList<QTreeWidgetItem *> selectedItems = mTreeWidget->selectedItems();

  const auto constSelectedItems = selectedItems;
  for ( QTreeWidgetItem *item : constSelectedItems )
  {
    if ( !( item->flags() & Qt::ItemIsEditable ) )
      continue;

    const QString name = item->text( 0 );
    QgsExpressionContextScope *itemScope = mTreeWidget->scopeFromItem( item );
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

  QgsExpressionContextScope *editableScope = mContext->scope( mEditableScopeIndex );
  const QList<QTreeWidgetItem *> selectedItems = mTreeWidget->selectedItems();

  bool removeEnabled = true;
  const auto constSelectedItems = selectedItems;
  for ( QTreeWidgetItem *item : constSelectedItems )
  {
    if ( !( item->flags() & Qt::ItemIsEditable ) )
    {
      removeEnabled = false;
      break;
    }

    const QString name = item->text( 0 );
    QgsExpressionContextScope *itemScope = mTreeWidget->scopeFromItem( item );
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
{
  // init icons
  if ( mExpandIcon.isNull() )
  {
    QPixmap pix( 14, 14 );
    pix.fill( Qt::transparent );
    mExpandIcon.addPixmap( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpandSmall.svg" ) ).pixmap( 14, 14 ), QIcon::Normal, QIcon::Off );
    mExpandIcon.addPixmap( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpandSmall.svg" ) ).pixmap( 14, 14 ), QIcon::Selected, QIcon::Off );
    mExpandIcon.addPixmap( QgsApplication::getThemeIcon( QStringLiteral( "/mIconCollapseSmall.svg" ) ).pixmap( 14, 14 ), QIcon::Normal, QIcon::On );
    mExpandIcon.addPixmap( QgsApplication::getThemeIcon( QStringLiteral( "/mIconCollapseSmall.svg" ) ).pixmap( 14, 14 ), QIcon::Selected, QIcon::On );
  }

  setIconSize( QSize( 18, 18 ) );
  setColumnCount( 2 );
  setHeaderLabels( QStringList() << tr( "Variable" ) << tr( "Value" ) );
  setEditTriggers( QAbstractItemView::AllEditTriggers );
  setRootIsDecorated( false );
  header()->setSectionsMovable( false );
  header()->setSectionResizeMode( QHeaderView::Interactive );

  mEditorDelegate = new VariableEditorDelegate( this, this );
  setItemDelegate( mEditorDelegate );
}

QgsExpressionContextScope *QgsVariableEditorTree::scopeFromItem( QTreeWidgetItem *item ) const
{
  if ( !item )
    return nullptr;

  bool ok;
  const int contextIndex = item->data( 0, ContextIndex ).toInt( &ok );
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

QTreeWidgetItem *QgsVariableEditorTree::itemFromVariable( QgsExpressionContextScope *scope, const QString &name ) const
{
  const int contextIndex = mContext ? mContext->indexOfScope( scope ) : 0;
  if ( contextIndex < 0 )
    return nullptr;
  return mVariableToItem.value( qMakePair( contextIndex, name ) );
}

QgsExpressionContextScope *QgsVariableEditorTree::editableScope()
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
  const auto constScopes = mContext->scopes();
  for ( QgsExpressionContextScope *scope : constScopes )
  {
    refreshScopeItems( scope, scopeIndex );
    scopeIndex++;
  }
}

void QgsVariableEditorTree::refreshScopeVariables( QgsExpressionContextScope *scope, int scopeIndex )
{
  const QColor baseColor = rowColor( scopeIndex );
  const bool isCurrent = scopeIndex == mEditableScopeIndex;
  QTreeWidgetItem *scopeItem = mScopeToItem.value( scopeIndex );

  const QStringList names = scope->filteredVariableNames();
  for ( const QString &name : names )
  {
    QTreeWidgetItem *item = mVariableToItem.value( qMakePair( scopeIndex, name ) );
    if ( !item )
    {
      item = new QTreeWidgetItem( scopeItem );
      mVariableToItem.insert( qMakePair( scopeIndex, name ), item );
    }

    const bool readOnly = scope->isReadOnly( name );
    bool isActive = true;
    QgsExpressionContextScope *activeScope = nullptr;
    if ( mContext )
    {
      activeScope = mContext->activeScopeForVariable( name );
      isActive = activeScope == scope;
    }

    item->setFlags( item->flags() | Qt::ItemIsEnabled );
    item->setText( 0, name );
    const QVariant value = scope->variable( name );
    const QString previewString = QgsExpression::formatPreviewString( value, false );
    item->setText( 1, previewString );
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
      const QString toolTip = tr( "Overridden by value from %1" ).arg( activeScope->name() );
      item->setToolTip( 0, toolTip );
      item->setToolTip( 1, toolTip );
    }
    else
    {
      font.setStrikeOut( false );
      item->setToolTip( 0, name );
      item->setToolTip( 1, previewString );
    }
    item->setFont( 0, font );
    item->setFont( 1, font );
    item->setData( 0, RowBaseColor, baseColor );
    item->setData( 0, ContextIndex, scopeIndex );
    item->setFirstColumnSpanned( false );
  }
}

void QgsVariableEditorTree::refreshScopeItems( QgsExpressionContextScope *scope, int scopeIndex )
{
  const QgsSettings settings;

  //add top level item
  const bool isCurrent = scopeIndex == mEditableScopeIndex;

  QTreeWidgetItem *scopeItem = nullptr;
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

void QgsVariableEditorTree::renameItem( QTreeWidgetItem *item, const QString &name )
{
  if ( !item )
    return;

  const int contextIndex = mVariableToItem.key( item ).first;
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

void QgsVariableEditorTree::drawRow( QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index ) const
{
  QStyleOptionViewItem opt = option;
  QTreeWidgetItem *item = itemFromIndex( index );
  if ( index.parent().isValid() )
  {
    //not a top-level item, so shade row background by context
    QColor baseColor = item->data( 0, RowBaseColor ).value<QColor>();
    if ( index.row() % 2 == 1 )
    {
      baseColor.setAlpha( 59 );
    }
    painter->fillRect( option.rect, baseColor );
  }
  QTreeWidget::drawRow( painter, opt, index );
  const QColor color = static_cast<QRgb>( QApplication::style()->styleHint( QStyle::SH_Table_GridLineColor, &opt ) );
  const QgsScopedQPainterState painterState( painter );
  painter->setPen( QPen( color ) );
  painter->drawLine( opt.rect.x(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom() );
}

QColor QgsVariableEditorTree::rowColor( int index ) const
{
  //return some nice (inspired by Qt Designer) background row colors
  const int colorIdx = index % 6;
  switch ( colorIdx )
  {
    case 0:
      return QColor( 255, 163, 0, 89 );
    case 1:
      return QColor( 255, 255, 77, 89 );
    case 2:
      return QColor( 0, 255, 77, 89 );
    case 3:
      return QColor( 0, 255, 255, 89 );
    case 4:
      return QColor( 196, 125, 255, 89 );
    case 5:
    default:
      return QColor( 255, 125, 225, 89 );
  }
}

void QgsVariableEditorTree::toggleContextExpanded( QTreeWidgetItem *item )
{
  if ( !item )
    return;

  item->setExpanded( !item->isExpanded() );

  //save expanded state
  QgsSettings settings;
  settings.setValue( "QgsVariableEditor/" + item->text( 0 ) + "/expanded", item->isExpanded() );
}

void QgsVariableEditorTree::editNext( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  if ( index.column() == 0 )
  {
    //switch to next column
    const QModelIndex nextIndex = index.sibling( index.row(), 1 );
    if ( nextIndex.isValid() )
    {
      setCurrentIndex( nextIndex );
      edit( nextIndex );
    }
  }
  else
  {
    const QModelIndex nextIndex = model()->index( index.row() + 1, 0, index.parent() );
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
    const QModelIndex index = currentIndex();
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
    const QModelIndex index = currentIndex();
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

  if ( event == QKeySequence::Copy )
  {
    const QList<QTreeWidgetItem *> selected = selectedItems();
    if ( selected.size() > 0 )
    {
      QString text = selected.at( 0 )->text( 0 );
      const QString varName = variableNameFromItem( selected.at( 0 ) );
      QgsExpressionContextScope *scope = scopeFromItem( selected.at( 0 ) );
      if ( !varName.isEmpty() && scope )
        text = scope->variable( varName ).toString();

      QClipboard *clipboard = QApplication::clipboard();
      clipboard->setText( text );
      event->accept();
      return;
    }
  }

  QTreeWidget::keyPressEvent( event );
}

void QgsVariableEditorTree::mousePressEvent( QMouseEvent *event )
{
  QTreeWidget::mousePressEvent( event );
  QTreeWidgetItem *item = itemAt( event->pos() );
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

QWidget *VariableEditorDelegate::createEditor( QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &index ) const
{
  if ( !mParentTree )
    return nullptr;

  //no editing for top level items
  if ( !index.parent().isValid() )
    return nullptr;

  QTreeWidgetItem *item = mParentTree->indexToItem( index );
  QgsExpressionContextScope *scope = mParentTree->scopeFromItem( item );
  if ( !item || !scope )
    return nullptr;

  const QString variableName = mParentTree->variableNameFromIndex( index );

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

void VariableEditorDelegate::setModelData( QWidget *widget, QAbstractItemModel *model,
    const QModelIndex &index ) const
{
  Q_UNUSED( model )

  if ( !mParentTree )
    return;

  QTreeWidgetItem *item = mParentTree->indexToItem( index );
  QgsExpressionContextScope *scope = mParentTree->scopeFromItem( item );
  if ( !item || !scope )
    return;

  QLineEdit *lineEdit = qobject_cast< QLineEdit * >( widget );
  if ( !lineEdit )
    return;

  const QString variableName = mParentTree->variableNameFromIndex( index );
  if ( index.column() == 0 )
  {
    //edited variable name
    QString newName = lineEdit->text();
    newName = newName.trimmed();
    newName = newName.replace( ' ', '_' );

    //test for validity
    if ( newName == variableName )
    {
      return;
    }
    if ( scope->hasVariable( newName ) )
    {
      //existing name
      QMessageBox::warning( mParentTree, tr( "Rename Variable" ), tr( "A variable with the name \"%1\" already exists in this context." ).arg( newName ) );
      newName.append( "_1" );
    }

    const QString value = scope->variable( variableName ).toString();
    mParentTree->renameItem( item, newName );
    scope->removeVariable( variableName );
    scope->setVariable( newName, value );
    mParentTree->emitChanged();
  }
  else if ( index.column() == 1 )
  {
    //edited variable value
    const QString value = lineEdit->text();
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
