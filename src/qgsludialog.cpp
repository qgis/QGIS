/***************************************************************************
                         qgsludialog.cpp  -  description
                             -------------------
    begin                : September 2004
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
/* $Id$ */

#include "qgsludialog.h"


QgsLUDialog::QgsLUDialog(QWidget *parent, const char *name, bool modal)
: QDialog(parent, name, modal)
{
    setupUi(this);
    connect(mOkButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(mCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

QgsLUDialog::~QgsLUDialog()
{

}

QString QgsLUDialog::lowerValue() const
{
    return mLowerEdit->text();
}

QString QgsLUDialog::upperValue() const
{
    return mUpperEdit->text();
}

void QgsLUDialog::setLowerValue(QString val)
{
    mLowerEdit->setText(val);
}

void QgsLUDialog::setUpperValue(QString val)
{
    mUpperEdit->setText(val);
}
