/***************************************************************************
    qgisexpressionbuilder.cpp - A genric expression string builder widget.
     --------------------------------------
    Date                 :  29-May-2011
    Copyright            : (C) 2006 by Nathan Woodrow
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
#include "ui_qgsexpressionbuilder.h"

#include "qgslogger.h"

QgsExpressionBuilderWidget::QgsExpressionBuilderWidget(QgsVectorLayer *layer)
    : QWidget(),
    mLayer( layer )
{
    setupUi(this);
    if (!layer) return;

    mModel = new QStandardItemModel( );
    expressionTree->setModel( mModel );

    this->registerItem("Operators","+"," + ");
    this->registerItem("Operators","-"," -");
    this->registerItem("Operators","*"," * ");
    this->registerItem("Operators","/"," / ");
}

QgsExpressionBuilderWidget::~QgsExpressionBuilderWidget()
{
    
}


void QgsExpressionBuilderWidget::on_mAllPushButton_clicked()
{

}

void QgsExpressionBuilderWidget::on_expressionTree_doubleClicked(const QModelIndex &index)
{
   QgsExpressionItem* item = static_cast<QgsExpressionItem*>(mModel->itemFromIndex(index));
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

    //insert into field list and field combo box
    //mFieldMap.insert( fieldName, fieldIt.key() );
    this->registerItem("Fields", fieldName, " " + fieldName + " ");
    //mFieldsListWidget->addItem( fieldName );
  }
}

void QgsExpressionBuilderWidget::fillFieldValues(int fieldIndex, int countLimit)
{
    // determine the field type
    QList<QVariant> values;
    mLayer->uniqueValues( fieldIndex, values, countLimit );

    foreach(QVariant value, values)
    {
        //mValueListWidget->addItem(value.toString());
    }
}

void QgsExpressionBuilderWidget::registerItem(QString group, QString label, QString expressionText)
{
    QgsExpressionItem* item = new QgsExpressionItem(label,expressionText);
    // Look up the group and insert the new function.
    if (mExpressionGroups.contains(group))
    {
        QgsExpressionItem* groupNode = mExpressionGroups.value(group);
        groupNode->appendRow(item);
    }
    else
    {
        QgsExpressionItem* newgroupNode = new QgsExpressionItem(group,"");
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

