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

#ifndef QGSGRASYDIALOG_H
#define QGSGRASYDIALOG_H

#include "qgsgrasydialogbase.h"
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
     void apply() const;
 protected slots:
     /**Creates a new extension widget*/
     void adjustNumberOfClasses();
 protected:
     /**Pointer to the curret extension widget*/
     QgsGraSyExtensionWidget* ext;
     QScrollView* scv;
     QgsVectorLayer* m_vectorlayer;
     /**Stores the names and numbers of the fields with numeric values*/
     std::map<QString,int> m_fieldmap;
 private:
     /**Default constructor is privat to not use is*/
     QgsGraSyDialog();
};

#endif
