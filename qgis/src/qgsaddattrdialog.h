/***************************************************************************
                         qgsaddattrdialog.h  -  description
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

#ifndef QGSADDATTRDIALOG_H
#define QGSADDATTRDIALOG_H

#ifdef WIN32
#include "qgsaddattrdialogbase.h"
#else
#include "qgsaddattrdialogbase.uic.h"
#endif

class QgsVectorDataProvider;

class QgsAddAttrDialog: public QgsAddAttrDialogBase
{
    Q_OBJECT
 public:
    QgsAddAttrDialog(QgsVectorDataProvider* provider);
    QgsAddAttrDialog(const std::list<QString>& typelist);
    QString name() const;
    QString type() const;
 protected:
    QgsVectorDataProvider* mDataProvider;
};

#endif
