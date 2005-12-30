/***************************************************************************
                                qgspointstyle.h 
                               ------------------
        begin                : March 2005
        copyright            : (C) 2005 by Tim Sutton
        email                : tim@linfiniti.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOINTSTYLE_H
#define QGSPOINTSTYLE_H

#include "ui_qgspointstylewidgetbase.h"
#include <QDialog>


class QgsPointStyleWidget:public QDialog, private Ui::QgsPointStyleWidgetBase
{
  Q_OBJECT;
  public:

  //! Constructor
  QgsPointStyleWidget(QWidget *parent = 0, const char * name = 0,  Qt::WFlags f = 0 );

  ~QgsPointStyleWidget();


public slots:
  
private:
    
};

#endif
