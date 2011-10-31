/***************************************************************************
    qgisexpressionbuilderwidget.cpp - A genric expression string builder widget.
     --------------------------------------
    Date                 :  29-May-2011
    Copyright            : (C) 2011 by Nathan Woodrow
    Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionbuilderwidget.h"
#include "qgslogger.h"
#include "qgsexpression.h"
#include "qgsmessageviewer.h"

#include <QMenu>

QgsExpressionBuilderWidget::QgsExpressionBuilderWidget( QWidget *parent )
    : QWidget( parent )
{
  setupUi( this );

  mValueListWidget->hide();
  mValueListLabel->hide();

  mModel = new QStandardItemModel( );
  mProxyModel = new QgsExpressionItemSearchProxy();
  mProxyModel->setSourceModel( mModel );
  expressionTree->setModel( mProxyModel );

  expressionTree->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( expressionTree, SIGNAL( customContextMenuRequested( const QPoint & ) ), this, SLOT( showContextMenu( const QPoint & ) ) );
  connect( btnPlusPushButton, SIGNAL( pressed() ), this, SLOT( operatorButtonClicked() ) );
  connect( btnMinusPushButton, SIGNAL( pressed() ), this, SLOT( operatorButtonClicked() ) );
  connect( btnDividePushButton, SIGNAL( pressed() ), this, SLOT( operatorButtonClicked() ) );
  connect( btnMultiplyPushButton, SIGNAL( pressed() ), this, SLOT( operatorButtonClicked() ) );
  connect( btnExpButton, SIGNAL( pressed() ), this, SLOT( operatorButtonClicked() ) );
  connect( btnConcatButton, SIGNAL( pressed() ), this, SLOT( operatorButtonClicked() ) );
  connect( btnOpenBracketPushButton, SIGNAL( pressed() ), this, SLOT( operatorButtonClicked() ) );
  connect( btnCloseBracketPushButton, SIGNAL( pressed() ), this, SLOT( operatorButtonClicked() ) );


  // TODO Can we move this stuff to QgsExpression, like the functions?
  registerItem( "Operators", "+", " + " );
  registerItem( "Operators", "-", " -" );
  registerItem( "Operators", "*", " * " );
  registerItem( "Operators", "/", " / " );
  registerItem( "Operators", "%", " % " );
  registerItem( "Operators", "^", " ^ " );
  registerItem( "Operators", "=", " = " );
  registerItem( "Operators", ">", " > " );
  registerItem( "Operators", "<", " < " );
  registerItem( "Operators", "<>", " <> " );
  registerItem( "Operators", "<=", " <= " );
  registerItem( "Operators", ">=", " >= " );
  registerItem( "Operators", "||", " || ", "<b>|| (String Concatenation)</b> "
                "<br> Joins two values together into a string "
                "<br> <i>Usage:</i><br>'Dia' || Diameter" );
  registerItem( "Operators", "LIKE", " LIKE " );
  registerItem( "Operators", "ILIKE", " ILIKE " );
  registerItem( "Operators", "IS", " IS NOT " );
  registerItem( "Operators", "OR", " OR " );
  registerItem( "Operators", "AND", " AND " );
  registerItem( "Operators", "NOT", " NOT " );


  // Load the fuctions from the QgsExpression class
  int count = QgsExpression::functionCount();
  for ( int i = 0; i < count; i++ )
  {
    QgsExpression::FunctionDef func = QgsExpression::BuiltinFunctions[i];
    QString name = func.mName;
    if ( func.mParams >= 1 )
      name += "(";
    registerItem( func.mGroup, func.mName, " " + name + " ", func.mHelpText );
  };
}


QgsExpressionBuilderWidget::~QgsExpressionBuilderWidget()
{

}

void QgsExpressionBuilderWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}

void QgsExpressionBuilderWidget::on_expressionTree_clicked( const QModelIndex &index )
{
  // Get the item
  QModelIndex idx = mProxyModel->mapToSource( index );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  if ( item == 0 )
    return;

  // Loading field values are handled with a
  // right click so we just show the help.
  if ( item->getItemType() == QgsExpressionItem::Field )
  {
    txtHelpText->setText( tr( "Double click to add field name to expression string. <br> "
                              "Or right click to select loading value options then "
                              "double click an item in the value list to add it to the expression string." ) );
    txtHelpText->setToolTip( txtHelpText->text() );
  }
  else
  {
    // Show the help for the current item.
    mValueListWidget->hide();
    mValueListLabel->hide();
    mValueListWidget->clear();
    txtHelpText->setText( item->getHelpText() );
    txtHelpText->setToolTip( txtHelpText->text() );
  }
}

void QgsExpressionBuilderWidget::on_expressionTree_doubleClicked( const QModelIndex &index )
{
  QModelIndex idx = mProxyModel->mapToSource( index );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  if ( item == 0 )
    return;

  // Don't handle the double click it we are on a header node.
  if ( item->getItemType() == QgsExpressionItem::Header )
    return;

  // Insert the expression text.
  txtExpressionString->insertPlainText( item->getExpressionText() );
}

void QgsExpressionBuilderWidget::loadFieldNames()
{
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer )
    return;

  const QgsFieldMap fieldMap = mLayer->pendingFields();
  QgsFieldMap::const_iterator fieldIt = fieldMap.constBegin();
  for ( ; fieldIt != fieldMap.constEnd(); ++fieldIt )
  {
    QString fieldName = fieldIt.value().name();
    registerItem( "Fields", fieldName, " " + fieldName + " ", "", QgsExpressionItem::Field );
  }
}

void QgsExpressionBuilderWidget::fillFieldValues( int fieldIndex, int countLimit )
{
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer )
    return;

  // TODO We should thread this so that we don't hold the user up if the layer is massive.
  mValueListWidget->clear();
  mValueListWidget->setUpdatesEnabled( false );
  mValueListWidget->blockSignals( true );

  QList<QVariant> values;
  mLayer->uniqueValues( fieldIndex, values, countLimit );
  foreach( QVariant value, values )
  {
    mValueListWidget->addItem( value.toString() );
  }

  mValueListWidget->setUpdatesEnabled( true );
  mValueListWidget->blockSignals( false );
}

void QgsExpressionBuilderWidget::registerItem( QString group,
    QString label,
    QString expressionText,
    QString helpText,
    QgsExpressionItem::ItemType type )
{
  QgsExpressionItem* item = new QgsExpressionItem( label, expressionText, helpText, type );
  // Look up the group and insert the new function.
  if ( mExpressionGroups.contains( group ) )
  {
    QgsExpressionItem* groupNode = mExpressionGroups.value( group );
    groupNode->appendRow( item );
  }
  else
  {
    // If the group doesn't exsit yet we make it first.
    QgsExpressionItem* newgroupNode = new QgsExpressionItem( group, "", QgsExpressionItem::Header );
    newgroupNode->appendRow( item );
    mModel->appendRow( newgroupNode );
    mExpressionGroups.insert( group , newgroupNode );
  }
}

QString QgsExpressionBuilderWidget::getExpressionString()
{
  return txtExpressionString->toPlainText();
}

void QgsExpressionBuilderWidget::setExpressionString( const QString expressionString )
{
  txtExpressionString->setPlainText( expressionString );
}

void QgsExpressionBuilderWidget::on_txtExpressionString_textChanged()
{
  QString text = txtExpressionString->toPlainText();

  // If the string is empty the expression will still "fail" although
  // we don't show the user an error as it will be confusing.
  if ( text.isEmpty() )
  {
    lblPreview->setText( "" );
    lblPreview->setStyleSheet( "" );
    txtExpressionString->setToolTip( "" );
    lblPreview->setToolTip( "" );
    // Return false for isVaild because a null expression is still invaild.
    emit expressionParsed( false );
    return;
  }

  QgsExpression exp( text );

  // TODO We could do this without a layer.
  // Maybe just calling exp.evaluate()?
  if ( mLayer )
  {
    if ( !mFeature.isValid() )
    {
        mLayer->select( mLayer->pendingAllAttributesList() );
        mLayer->nextFeature( mFeature );
    }

    if ( mFeature.isValid() )
    {
        QVariant value = exp.evaluate( &mFeature, mLayer->pendingFields() );
        if ( !exp.hasEvalError() )
          lblPreview->setText( value.toString() );
    }
    else
    {
        // The feautre is invaild because we don't have one but that doesn't mean user can't
        // build a expression string.  They just get no preview.
        lblPreview->setText("");
    }
  }

  if ( exp.hasParserError() || exp.hasEvalError() )
  {
    QString tooltip = "<b>Parser Error:</b> <br>" + exp.parserErrorString();
    if ( exp.hasEvalError() )
      tooltip += "<br><br> <b>Eval Error:</b> <br>" + exp.evalErrorString();

    lblPreview->setText( "Expression is invaild <a href=""more"">(more info)</a>" );
    lblPreview->setStyleSheet( "color: rgba(255, 6, 10,  255);" );
    txtExpressionString->setToolTip( tooltip );
    lblPreview->setToolTip( tooltip );
    emit expressionParsed( false );
    return;
  }
  else
  {
    lblPreview->setStyleSheet( "" );
    txtExpressionString->setToolTip( "" );
    lblPreview->setToolTip( "" );
    emit expressionParsed( true );
  }
}

void QgsExpressionBuilderWidget::on_txtSearchEdit_textChanged()
{
  mProxyModel->setFilterWildcard( txtSearchEdit->text() );
  if ( txtSearchEdit->text().isEmpty() )
    expressionTree->collapseAll();
  else
    expressionTree->expandAll();
}

void QgsExpressionBuilderWidget::on_lblPreview_linkActivated( QString link )
{
  Q_UNUSED( link );
  QgsMessageViewer * mv = new QgsMessageViewer( this );
  mv->setWindowTitle( "More info on expression error" );
  mv->setMessageAsHtml( txtExpressionString->toolTip() );
  mv->exec();
}

void QgsExpressionBuilderWidget::on_mValueListWidget_itemDoubleClicked( QListWidgetItem *item )
{
  txtExpressionString->insertPlainText( " " + item->text() + " " );
}

void QgsExpressionBuilderWidget::operatorButtonClicked()
{
  QPushButton* button = dynamic_cast<QPushButton*>( sender() );
  txtExpressionString->insertPlainText( " " + button->text() + " " );
}

void QgsExpressionBuilderWidget::showContextMenu( const QPoint & pt )
{
  QModelIndex idx = expressionTree->indexAt( pt );
  idx = mProxyModel->mapToSource( idx );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  if ( !item )
    return;

  if ( item->getItemType() == QgsExpressionItem::Field )
  {
    QMenu* menu = new QMenu( this );
    menu->addAction( tr( "Load top 10 unique values" ), this, SLOT( loadSampleValues() ) );
    menu->addAction( tr( "Load all unique values" ), this, SLOT( loadAllValues() ) );
    menu->popup( expressionTree->mapToGlobal( pt ) );
  }
}

void QgsExpressionBuilderWidget::loadSampleValues()
{
  QModelIndex idx = mProxyModel->mapToSource( expressionTree->currentIndex() );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer )
    return;

  mValueListWidget->show();
  mValueListLabel->show();
  int fieldIndex = mLayer->fieldNameIndex( item->text() );
  fillFieldValues( fieldIndex, 10 );
}

void QgsExpressionBuilderWidget::loadAllValues()
{
  QModelIndex idx = mProxyModel->mapToSource( expressionTree->currentIndex() );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer )
    return;

  mValueListWidget->show();
  mValueListLabel->show();
  int fieldIndex = mLayer->fieldNameIndex( item->text() );
  fillFieldValues( fieldIndex, -1 );
}

