
/***************************************************************************
                                 qgslocationcapturewidget.h
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

#include "qgslocationcapturewidget.h"
#include <qgspoint.h>

QgsLocationCaptureWidget::QgsLocationCaptureWidget( QWidget *parent, const char * name, WFlags f)
           :QgsLocationCaptureWidgetBase( parent, name, f)
{
}

QgsLocationCaptureWidget::~QgsLocationCaptureWidget()
{
}

void QgsLocationCaptureWidget::qgsMapCanvas_xyClickCoordinates( QgsPoint & theQgsPoint )
{
      lblCapturePos->setText(tr("Captured Pos: ")+theQgsPoint.stringRep(2));
}


void QgsLocationCaptureWidget::qgsMapCanvas_xyCoordinates( QgsPoint & theQgsPoint)
{
    lblCapturePos->setText(tr("Current Pos: ")+theQgsPoint.stringRep(2));
}

