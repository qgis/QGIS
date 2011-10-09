/***************************************************************************
    qgisexpressionbuilder.cpp - A genric expression string builder widget.
     --------------------------------------
    Date                 :  29-May-2011
    Copyright            : (C) 2011 by Nathan Woodrow
    Email                : nathan.woodrow at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionbuilder.h"
#include "qgslogger.h"
#include "qgsexpression.h"

#include <QMenu>

QgsExpressionBuilderWidget::QgsExpressionBuilderWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    mValueListWidget->hide();
    mValueListLabel->hide();

    mModel = new QStandardItemModel( );
    mProxyModel = new QgsExpressionItemSearhProxy();
    mProxyModel->setSourceModel( mModel );
    expressionTree->setModel( mProxyModel );

    expressionTree->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( expressionTree, SIGNAL( customContextMenuRequested( const QPoint & ) ), this, SLOT( showContextMenu( const QPoint & ) ) );

    this->registerItem("Operators","+"," + ");
    this->registerItem("Operators","-"," -");
    this->registerItem("Operators","*"," * ");
    this->registerItem("Operators","/"," / ");
    this->registerItem("Operators","^"," ^ ");
    this->registerItem("Operators","="," = ");
    this->registerItem("Operators","||"," || ","<b>|| (String Concatenation)</b> "\
                                                "<br> Joins two values together into a string " \
                                                "<br> <i>Usage:</i><br>'Dia' || Diameter");

    // Load the fuctions from the QgsExpression class
    int count = QgsExpression::functionCount();
    for ( int i = 0; i < count; i++ )
    {
        QgsExpression::FunctionDef func = QgsExpression::BuiltinFunctions[i];
        QString name = func.mName;
        if ( func.mParams >= 1 )
            name += "(";
        this->registerItem(func.mGroup,func.mName, " " + name + " ", func.mHelpText);
    };
}

QgsExpressionBuilderWidget::~QgsExpressionBuilderWidget()
{
    
}

void QgsExpressionBuilderWidget::setLayer( QgsVectorLayer *layer )
{
    mLayer = layer;
}

void QgsExpressionBuilderWidget::on_expressionTree_clicked(const QModelIndex &index)
{
   // Get the item
   QModelIndex idx = mProxyModel->mapToSource(index);
   QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>(mModel->itemFromIndex( idx ));
   if ( item == 0 )
       return;

   // Loading field values are handled with a
   // right click so we just show the help.
   if (item->getItemType() == QgsExpressionItem::Field)
   {
       txtHelpText->setText( tr("Double click to add field name to expression string. <br> " \
                            "Or right click to select loading value options then " \
                            "double click an item in the value list to add it to the expression string."));
       txtHelpText->setToolTip(txtHelpText->text());
   }
   else
   {
       // Show the help for the current item.
       mValueListWidget->hide();
       mValueListLabel->hide();
       mValueListWidget->clear();
       txtHelpText->setText(item->getHelpText());
       txtHelpText->setToolTip(txtHelpText->text());
   }
}

void QgsExpressionBuilderWidget::on_expressionTree_doubleClicked(const QModelIndex &index)
{
   QModelIndex idx = mProxyModel->mapToSource(index);
   QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>(mModel->itemFromIndex( idx ));
   if (item == 0)
       return;

   // Don't handle the double click it we are on a header node.
   if (item->getItemType() == QgsExpressionItem::Header)
       return;

   // Insert the expression text.
   txtExpressionString->insertPlainText(item->getExpressionText());
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
    this->registerItem("Fields", fieldName, " " + fieldName + " ","", QgsExpressionItem::Field);
  }
}

void QgsExpressionBuilderWidget::fillFieldValues(int fieldIndex, int countLimit)
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
    foreach(QVariant value, values)
    {
        mValueListWidget->addItem(value.toString());
    }

    mValueListWidget->setUpdatesEnabled( true );
    mValueListWidget->blockSignals( false );
}

void QgsExpressionBuilderWidget::registerItem(QString group,
                                              QString label,
                                              QString expressionText,
                                              QString helpText,
                                              QgsExpressionItem::ItemType type)
{
    QgsExpressionItem* item = new QgsExpressionItem(label,expressionText, helpText, type);
    // Look up the group and insert the new function.
    if (mExpressionGroups.contains(group))
    {
        QgsExpressionItem* groupNode = mExpressionGroups.value(group);
        groupNode->appendRow(item);
    }
    else
    {
        // If the group doesn't exsit yet we make it first.
        QgsExpressionItem* newgroupNode = new QgsExpressionItem(group,"", QgsExpressionItem::Header);
        newgroupNode->appendRow(item);
        mModel->appendRow(newgroupNode);
        mExpressionGroups.insert(group , newgroupNode );
    }
}

QString QgsExpressionBuilderWidget::getExpressionString()
{
    return this->txtExpressionString->toPlainText();
}

void QgsExpressionBuilderWidget::setExpressionString(const QString expressionString)
{
    this->txtExpressionString->setPlainText(expressionString);
}

bool QgsExpressionBuilderWidget::hasExpressionError()
{
    QString text = this->txtExpressionString->toPlainText();
    QgsExpression exp( text );
    return exp.hasParserError();
}

void QgsExpressionBuilderWidget::on_txtExpressionString_textChanged()
{
    QString text = this->txtExpressionString->toPlainText();
    QgsExpression exp( text );
    if ( exp.hasParserError())
    {
        this->txtExpressionString->setStyleSheet("background-color: rgba(255, 6, 10, 75);");
        this->txtExpressionString->setToolTip(exp.parserErrorString());
        emit expressionParsed(false);
    }
    else
    {
        this->txtExpressionString->setStyleSheet("");
        this->txtExpressionString->setToolTip("");
        emit expressionParsed(true);
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

void QgsExpressionBuilderWidget::showContextMenu( const QPoint & pt)
{
    QModelIndex idx = expressionTree->indexAt( pt );
    idx = mProxyModel->mapToSource( idx );
    QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>(mModel->itemFromIndex( idx ));
    if ( !item )
      return;

    if (item->getItemType() == QgsExpressionItem::Field)
    {
        QMenu* menu = new QMenu( this );
        menu->addAction( tr( "Load top 10 unique values" ), this, SLOT( loadSampleValues()) );
        menu->addAction( tr( "Load all unique values" ), this, SLOT( loadAllValues() ) );
        menu->popup( expressionTree->mapToGlobal( pt ) );
    }
}

void QgsExpressionBuilderWidget::loadSampleValues()
{
    QModelIndex idx = mProxyModel->mapToSource(expressionTree->currentIndex());
    QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>(mModel->itemFromIndex( idx ));
    // TODO We should really return a error the user of the widget that
    // the there is no layer set.
    if ( !mLayer )
        return;

    mValueListWidget->show();
    mValueListLabel->show();
    int fieldIndex = mLayer->fieldNameIndex(item->text());
    fillFieldValues(fieldIndex,10);
}

void QgsExpressionBuilderWidget::loadAllValues()
{
    QModelIndex idx = mProxyModel->mapToSource(expressionTree->currentIndex());
    QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>(mModel->itemFromIndex( idx ));
    // TODO We should really return a error the user of the widget that
    // the there is no layer set.
    if ( !mLayer )
        return;

    mValueListWidget->show();
    mValueListLabel->show();
    int fieldIndex = mLayer->fieldNameIndex(item->text());
    fillFieldValues(fieldIndex,-1);
}

