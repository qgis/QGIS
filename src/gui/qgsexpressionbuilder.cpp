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

QgsExpressionBuilderWidget::QgsExpressionBuilderWidget(QgsVectorLayer *layer)
    : QWidget(),
    mLayer( layer )
{
    if (!layer) return;

    setupUi(this);

    mValueListWidget->hide();
    mValueListLabel->hide();

    mModel = new QStandardItemModel( );
    expressionTree->setModel( mModel );

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


void QgsExpressionBuilderWidget::on_mAllPushButton_clicked()
{
    // We don't use this yet.
    // TODO
}

void QgsExpressionBuilderWidget::on_expressionTree_clicked(const QModelIndex &index)
{
   // Get the item
   QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>(mModel->itemFromIndex(index));
   if ( item == 0 )
       return;

   // If field item then we load a sample set of values from the field.
   if (item->getItemType() == QgsExpressionItem::Field)
   {
       mValueListWidget->show();
       mValueListLabel->show();
       int fieldIndex = mLayer->fieldNameIndex(item->text());
       fillFieldValues(fieldIndex,10);
       txtHelpText->setText("Double click to add field name to expression string. <br> " \
                            "Or double click an item in the value list to add it to the expression string.");
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
   QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>(mModel->itemFromIndex(index));
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
  if ( !mLayer )
  {
    return;
  }
 
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
    }
    else
    {
        this->txtExpressionString->setStyleSheet("");
    }
}



