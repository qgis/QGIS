/***************************************************************************
                         qgsdelattrdialog.h  -  description
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Marco Hugentobler
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

#ifndef QGSDELATTRDIALOG_H
#define QGSDELATTRDIALOG_H

#include "ui_qgsdelattrdialogbase.h"
#include <QDialog>
#include <list>

class QgsVectorLayer;

class APP_EXPORT QgsDelAttrDialog: public QDialog, private Ui::QgsDelAttrDialogBase
{
    Q_OBJECT
  public:
    QgsDelAttrDialog( const QgsVectorLayer* vl );
    ~QgsDelAttrDialog();
    /** Returns the selected attribute indices*/
    QList<int> selectedAttributes();
};

#endif
