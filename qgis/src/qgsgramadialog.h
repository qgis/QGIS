/***************************************************************************
                          qgsgramadialog.h  -  description
                             -------------------
    begin                : April 2004
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
/* $Id */

#ifndef QGSGRAMADIALOG_H
#define QGSGRAMADIALOG_H

#include "qgsgramadialogbase.uic.h"
#include <map>

class QgsGraMaExtensionWidget;
class QgsVectorLayer;

class QgsGraMaDialog: public QgsGraMaDialogBase
{
    Q_OBJECT
 public: 
    QgsGraMaDialog(QgsVectorLayer* layer);
    ~QgsGraMaDialog();
 protected:
    /**Pointer to the curret extension widget*/
    QgsGraMaExtensionWidget* ext;
    /**Pointer to the associated vector layer*/
    QgsVectorLayer* mVectorLayer;
    /**Stores the names and numbers of the fields with numeric values*/
     std::map<QString,int> mFieldMap;
public slots:
     void apply();
protected slots:
     /**Creates a new extension widget*/
     void adjustNumberOfClasses();
private:
     /**Default constructor is privat to not use is*/
     QgsGraMaDialog();
};

#endif
