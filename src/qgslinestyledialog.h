/***************************************************************************
                          qgslinestyledialog.h 
 Dialog for selecting line style for vector layers
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
#ifndef QGSLINESTYLEDIALOG_H
#define QGSLINESTYLEDIALOG_H

class qnamespace;
#include "qgslinestyledialogbase.h"

/**Dialog class to query line styles*/
class QgsLineStyleDialog: public QgsLineStyleDialogBase
{
  Q_OBJECT
 public:
    QgsLineStyleDialog(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
    ~QgsLineStyleDialog();
    Qt::PenStyle style();
 protected:
    Qt::PenStyle m_style;
 protected slots:
     /**Queries the selected style if the ok button is pressed and stores it in m_style*/
     void queryStyle();
};

inline QgsLineStyleDialog::~QgsLineStyleDialog()
{

}

#endif
