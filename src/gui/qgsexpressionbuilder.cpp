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

QgsExpressionBuilderWidget::QgsExpressionBuilderWidget(QgsVectorLayer *layer)
    : QWidget(),
    mLayer( layer )
{
    setupUi(this);
    if (!layer) return;
}

QgsExpressionBuilderWidget::~QgsExpressionBuilderWidget()
{
    
}

void QgsExpressionBuilderWidget::on_mAllPushButton_clicked()
{

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
    mFieldsListWidget->addItem( fieldName );
  }
}

void QgsExpressionBuilderWidget::fillFieldValues(int fieldIndex, int countLimit)
{
    // determine the field type
    QList<QVariant> values;
    mLayer->uniqueValues( fieldIndex, values, countLimit );

    foreach(QVariant value, values)
    {
        mValueListWidget->addItem(value.toString());
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

