/***************************************************************************
                         qgsuvaldialog.h  -  description
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

#ifndef QGSUVALDIALOG_H
#define QGSUVALDIALOG_H

#include "qgsuvaldialogbase.uic.h"
#include <set.h>

class QgsVectorLayer;
class QgsGraMaExtensionWidget;

class QgsUValDialog: public QgsUValDialogBase
{
 public:
    QgsUValDialog(QgsVectorLayer* vl);
    ~QgsUValDialog();

 public slots:
     void apply();

 protected:
    /**Pointer to the curret extension widget*/
    QgsGraMaExtensionWidget* ext;
    /**Pointer to the associated vector layer*/
    QgsVectorLayer* mVectorLayer;
    /**Set to store the already entered values*/
    std::set<QString> mValues;

 protected slots:
    void changeClassificationAttribute(int nr);
};

#endif
