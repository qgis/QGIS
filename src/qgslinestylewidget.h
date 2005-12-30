/***************************************************************************
                                qgslinestyle.h 
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
#ifndef QGSLINESTYLE_H
#define QGSLINESTYLE_H

#include "ui_qgslinestylewidgetbase.h"
#include <QDialog>

class QgsLineStyleWidget: public QDialog, private Ui::QgsLineStyleWidgetBase
{
  Q_OBJECT;
  public:

  //! Constructor
  QgsLineStyleWidget(QWidget *parent = 0, const char * name = 0,  Qt::WFlags f = 0 );

  ~QgsLineStyleWidget();


public slots:
  
private:
    
};

#endif
