/***************************************************************************
                         qgsgeomtypedialog.cpp  -  description
                             -------------------
    begin                : October 2004
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

#include "qgsgeomtypedialog.h"
#include <qradiobutton.h>

QgsGeomTypeDialog::QgsGeomTypeDialog(): QgsGeomTypeDialogBase()
{
    QObject::connect((QObject*)mOkButton, SIGNAL(clicked()), this, SLOT(accept()));//why is this cast necessary????
    QObject::connect((QObject*)mCancelButton, SIGNAL(clicked()), this, SLOT(reject()));//why is this cast necessary????
    mPointRadioButton->setChecked(true);
}

QgsGeomTypeDialog::~QgsGeomTypeDialog()
{

}

QGis::WKBTYPE QgsGeomTypeDialog::selectedType()
{
    if(mPointRadioButton->isChecked())
    {
	return QGis::WKBPoint;
    }
    else if(mLineRadioButton->isChecked())
    {
	return QGis::WKBLineString;
    }
    else if(mPolygonRadioButton->isChecked())
    {
	return QGis::WKBPolygon;
    }
}
