/***************************************************************************
  qgsrasterattributetable.h - QgsRasterAttributeTable

 ---------------------
 begin                : 3.12.2021
 copyright            : (C) 2021 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERATTRIBUTETABLE_H
#define QGSRASTERATTRIBUTETABLE_H

#include "qgis_core.h"
#include <QObject>

class CORE_EXPORT QgsRasterAttributeTable: public QObject
{
    Q_OBJECT
  public:
    QgsRasterAttributeTable();
};

#endif // QGSRASTERATTRIBUTETABLE_H
