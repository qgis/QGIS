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

#ifdef WIN32
#include "qgsdelattrdialogbase.h"
#else
#include "qgsdelattrdialogbase.uic.h"
#endif

#include <list>

class QHeader;

class QgsDelAttrDialog: public QgsDelAttrDialogBase
{
    Q_OBJECT
 public:
    QgsDelAttrDialog(QHeader* header);
    const std::list<QString>* selectedAttributes();
 protected:
    std::list<QString> mSelectedItems;
};

#endif
