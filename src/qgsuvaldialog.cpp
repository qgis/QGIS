/***************************************************************************
                         qgsuvaldialog.cpp  -  description
                             -------------------
    begin                : July 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsuvaldialog.h"
#include "qgsdataprovider.h"
#include "qgsvectorlayer.h"
#include <qcombobox.h>

QgsUValDialog::QgsUValDialog(QgsVectorLayer* vl): QgsUValDialogBase(), ext(0), mVectorLayer(vl)
{
    //find out the fields of mVectorLayer
    QgsDataProvider *provider;
    if (provider = mVectorLayer->getDataProvider())
    {
	std::vector < QgsField > &fields = provider->fields();
	QString str;
	
	for (std::vector < QgsField >::iterator it = fields.begin(); it != fields.end(); ++it)
        {
	    str = (*it).name();
	    str = str.left(1).upper() + str.right(str.length() - 1);  //make the first letter uppercase
	    mClassificationComboBox->insertItem(str);
        }
    } 
    else
    {
	qWarning("Warning, data provider is null in QgsUValDialog::QgsUValDialog");
	return;
    }
}

QgsUValDialog::~QgsUValDialog()
{

}

void QgsUValDialog::apply()
{

}

void QgsUValDialog::changeClassificationAttribute(int nr)
{
    mValues.clear();
    //go through all the features and insert their value into the set
    
    //go through the set and insert entries to the extension widget
}
