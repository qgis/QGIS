/***************************************************************************
                         qgsgeorefdescriptiondialog.h  -  description
                         ----------------------------
    begin                : Oct 2007
    copyright            : (C) 2007 by Marco Hugentobler
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
/* $Id$ */

#ifndef QGSGEOREFDESCRIPTIONDIALOG_H
#define QGSGEOREFDESCRIPTIONDIALOG_H

#include "ui_qgsgeorefdescriptiondialogbase.h"
#include <QDialog>

/**Dialog that shows logo and description of the georef plugin*/
class QgsGeorefDescriptionDialog: public QDialog, private Ui::QgsGeorefDescriptionDialogBase
{
    Q_OBJECT

  public:
    QgsGeorefDescriptionDialog( QWidget* parent );
};

#endif
