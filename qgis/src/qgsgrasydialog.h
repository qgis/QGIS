/***************************************************************************
                         qgsgrasydialog.h  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
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
/* $Id */

#ifndef QGSGRASYDIALOG_H
#define QGSGRASYDIALOG_H
#ifdef WIN32
#include "qgsgrasydialogbase.h"
#else
#include "qgsgrasydialogbase.uic.h"
#endif
#include "qgsrangerenderitem.h"
#include "qgssisydialog.h"
#include <map>

class QgsGraSyExtensionWidget;
class QgsVectorLayer;
class QScrollView;

class QgsGraSyDialog: public QgsGraSyDialogBase
{
    Q_OBJECT
 public:
    /**Enumeration describing the automatic settings of values*/
    enum mode{EMPTY, EQUAL_INTERVAL, QUANTILES};
    QgsGraSyDialog(QgsVectorLayer* layer);
    ~QgsGraSyDialog();
 public slots:
     void apply();
 protected slots:
     /**Changes only the number of classes*/
     void adjustNumberOfClasses();
     /**Sets a new classification field and a new classification mode*/
     void adjustClassification();
     /**Changes the display of the single symbol dialog*/
     void changeCurrentValue();
     /**Writes changes in the single symbol dialog to the corresponding QgsRangeRenderItem*/
     void applySymbologyChanges();
     /**Shows a dialog to modify lower and upper values*/
     void changeClass(QListBoxItem* item);
 protected:
     /**Pointer to the associated vector layer*/
     QgsVectorLayer* mVectorLayer;
     /**Stores the names and numbers of the fields with numeric values*/
     std::map<QString,int> mFieldMap;
     /**Stores the classes*/
     std::map<QString,QgsRangeRenderItem*> mEntries;
     /**Dialog which shows the settings of the activated class*/
     QgsSiSyDialog sydialog;
     int mClassificationField;
 private:
     /**Default constructor is privat to not use is*/
     QgsGraSyDialog();
};

#endif
