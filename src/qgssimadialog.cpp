/***************************************************************************
                          qgssimadialog.cpp
 Single marker renderer dialog
                             -------------------
    begin                : March 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
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

#include "qgssimadialog.h"
#include "qgsvectorlayer.h"
#include <qfiledialog.h>
#include <qpushbutton.h>


QgsSiMaDialog::QgsSiMaDialog(QgsVectorLayer* vectorlayer): QgsSiMaDialogBase(), mVectorLayer(vectorlayer)
{
    QObject::connect(mImageButton,SIGNAL(clicked()),this,SLOT(selectMarker()));
}

QgsSiMaDialog::QgsSiMaDialog(): QgsSiMaDialogBase(), mVectorLayer(0)
{
    QObject::connect(mImageButton,SIGNAL(clicked()),this,SLOT(selectMarker()));
}

QgsSiMaDialog::~QgsSiMaDialog()
{

}

void QgsSiMaDialog::apply()
{
    
}

void QgsSiMaDialog::selectMarker()
{
    mImageButton->setName(QFileDialog::getOpenFileName());
}
