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
/* $Id$ */
#ifndef QGSUVALDIALOG_H
#define QGSUVALDIALOG_H
#ifdef WIN32
#include "qgsuvaldialogbase.h"
#else
#include "qgsuvaldialogbase.uic.h"
#endif
#include "qgssisydialog.h"
#include <map>

class QgsVectorLayer;
class QgsRenderItem;

class QgsUValDialog: public QgsUValDialogBase
{
    Q_OBJECT
 public:
    QgsUValDialog(QgsVectorLayer* vl);
    ~QgsUValDialog();

 public slots:
     void apply();

 protected:
    /**Pointer to the associated vector layer*/
    QgsVectorLayer* mVectorLayer;
    /**Set to store the already entered values*/
    std::map<QString,QgsRenderItem*> mValues;
    QgsSiSyDialog sydialog;
    /**Value for which symbology settings are displayed*/
    QString currentValue;

 protected slots:
    /**Set new attribut for classification*/
    void changeClassificationAttribute(int nr);
    /**Changes the display of the single symbol dialog*/
    void changeCurrentValue();
    /**Writes changes in the single symbol dialog to the corresponding QgsSymbol*/
    void applySymbologyChanges();
};

#endif
