/***************************************************************************
                          qgscontcoldialog.h 
 Continuous color renderer dialog
                             -------------------
    begin                : 2004-02-12
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
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
#ifndef QGSCONTCOLDIALOG_H
#define QGSCONTCOLDIALOG_H

#include "qgscontcoldialogbase.uic.h"
#include <map>

class QgsVectorLayer;

class QgsContColDialog: public QgsContColDialogBase
{
    Q_OBJECT
 public: 
    QgsContColDialog(QgsVectorLayer* layer);
    ~QgsContColDialog();
 public slots:
    void apply();	
 protected slots:
    void selectMinimumColor();
    void selectMaximumColor();
 protected:
    QgsVectorLayer* mVectorLayer;
    /**Stores the names and numbers of the fields with numeric values*/
     std::map<QString,int> mFieldMap;
 private:
    QgsContColDialog();
};

#endif
