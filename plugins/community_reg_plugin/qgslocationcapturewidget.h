/***************************************************************************
                                locationcapturewidget.h 
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
#ifndef QGSLOCATIONCAPTUREWIDGET_H
#define QGSLOCATIONCAPTUREWIDGET_H

#ifdef WIN32
#include "qgslocationcapturewidgetbase.h"
#else
#include "qgslocationcapturewidgetbase.uic.h"
#endif

class QgsLocationCaptureWidget:public QgsLocationCaptureWidgetBase
{
  Q_OBJECT;
  public:

  //! Constructor
  QgsLocationCaptureWidget(QWidget *parent = 0, const char * name = 0,  WFlags f = 0 );

  ~QgsLocationCaptureWidget();


public slots:


private:
    
};

#endif
