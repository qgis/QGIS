/***************************************************************************
                         qgsgeomtypedialog.h  -  description
                             -------------------
    begin                : October 2004
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

#ifndef QGSGEOMTYPEDIALOG_H
#define QGSGEOMTYPEDIALOG_H

#ifdef WIN32
#include "qgsgeomtypedialogbase.h"
#else
#include "qgsgeomtypedialogbase.uic.h"
#endif //WIN32

#include "qgis.h"

class QgsGeomTypeDialog: public QgsGeomTypeDialogBase
{
  Q_OBJECT
  public:
  QgsGeomTypeDialog();
  ~QgsGeomTypeDialog();
  /**Returns the selected geometry type*/
  QGis::WKBTYPE selectedType();
  
};

#endif //QGSGEOMTYPEDIALOG_H
