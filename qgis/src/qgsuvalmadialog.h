/***************************************************************************
                         qgsuvalmadialog.h  -  unique value marker dialog
                             -------------------
    begin                : September 2004
    copyright            : (C) 2004 by Lars Luthman
    email                : larsl@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSUVALMADIALOG_H
#define QGSUVALMADIALOG_H
#ifdef WIN32
#include "qgsuvalmadialogbase.h"
#else
#include "qgsuvalmadialogbase.uic.h"
#endif
#include "qgssimadialog.h"
#include <map>

class QgsVectorLayer;
class QgsRenderItem;

class QgsUValMaDialog: public QgsUValMaDialogBase
{
    Q_OBJECT
 public:
    QgsUValMaDialog(QgsVectorLayer* vl);
    ~QgsUValMaDialog();

 public slots:
     void apply();

 protected:
    /**Pointer to the associated vector layer*/
    QgsVectorLayer* mVectorLayer;
    /**Set to store the already entered values*/
    std::map<QString,QgsRenderItem*> mValues;
    QgsSiMaDialog madialog;
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
