/***************************************************************************
                         qgsattributedialog.h  -  description
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
/* $Id$ */
#ifndef QGSATTRIBUTEDIALOG_H
#define QGSATTRIBUTEDIALOG_H

#include "ui_qgsattributedialogbase.h"

#include "qgsfeatureattribute.h"
#include <vector>

class QDialog;

class QgsAttributeDialog: public QDialog, private Ui::QgsAttributeDialogBase
{
    Q_OBJECT
	public:
    QgsAttributeDialog(std::vector<QgsFeatureAttribute>* attributes);
    ~QgsAttributeDialog();
    /**Returns the field value of a row*/
    QString value(int row);
};

#endif
