/***************************************************************************
                              qgsidwinterpolatordialog.h
                              --------------------------
  begin                : March 25, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSIDWINTERPOLATORDIALOG_H
#define QGSIDWINTERPOLATORDIALOG_H

#include "ui_qgsidwinterpolatordialogbase.h"
#include "qgsinterpolatordialog.h"

/**A class that takes the input parameter for inverse distance weighting*/
class QgsIDWInterpolatorDialog: public QgsInterpolatorDialog, private Ui::QgsIDWInterpolatorDialogBase
{
    Q_OBJECT
  public:
    QgsIDWInterpolatorDialog( QWidget* parent, QgisInterface* iface );

    ~QgsIDWInterpolatorDialog();

    /**Creates an IDW interpolator with the specified distance coefficient
     @return 0 in case of error*/
    QgsInterpolator* createInterpolator() const override;
};

#endif
