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
    this->registerItem("Operators","^"," ^ ");
    this->registerItem("Operators","="," = ");
    this->registerItem("Operators","||"," || ","<b>|| (String Concatenation)</b> "\
                                                "<br> Joins two values together into a string " \
                                                "<br> <i>Usage:</i><br>'Dia' || Diameter");

    this->registerItem("Geometry","Area"," $area ","<b>$area</b> <br> Returns the area the object." \
                                                                "<br> Only applies to polygons.");
    this->registerItem("Geometry","Length"," $length ","<b>$length</b> <br> Returns the length the object. <br> Only applies to polylines.");
    this->registerItem("Geometry","Perimeter"," $perimeter ");
    this->registerItem("Geometry","X"," $x ");
    this->registerItem("Geometry","Y"," $y ");
    this->registerItem("Geometry","XAt"," xat( ");
    this->registerItem("Geometry","YAt"," yat(  ");
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


   // Handle the special case for fields
   // This is a bit hacky.  Should be done better. Handle it with a enum on QgsExpressionItem.
   QStandardItem* parent = mModel->itemFromIndex(index.parent());
   if ( parent == 0 )
       return;

   if (parent->text() == "Fields")
   {
       int fieldIndex = mLayer->fieldNameIndex(item->text());
       fillFieldValues(fieldIndex,10);
   }
   else
   {
       // Show the help for the current item.
       mValueListWidget->clear();
       txtHelpText->setText(item->getHelpText());
   }
}

void QgsExpressionBuilderWidget::on_expressionTree_doubleClicked(const QModelIndex &index)
{
   QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>(mModel->itemFromIndex(index));
   if (item == 0)
       return;

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

void QgsExpressionBuilderWidget::registerItem(QString group, QString label, QString expressionText)
{
    this->registerItem(group,label,expressionText,"");
}

void QgsExpressionBuilderWidget::registerItem(QString group, QString label, QString expressionText, QString helpText)
{
    QgsExpressionItem* item = new QgsExpressionItem(label,expressionText, helpText);
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

