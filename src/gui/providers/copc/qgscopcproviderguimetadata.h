/***************************************************************************
                         qgscopcproviderguimetadata.h
                         --------------------
    begin                : March 2022
    copyright            : (C) 2022 by Belgacem Nedjima
    email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOPCPROVIDERGUIMETADATA_H
#define QGSCOPCPROVIDERGUIMETADATA_H

///@cond PRIVATE
#define SIP_NO_FILE

#include <QList>
#include <QMainWindow>

#include "qgsproviderguimetadata.h"

class QgsCopcProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsCopcProviderGuiMetadata();
};

///@endcond

#endif // QGSCopcPROVIDERGUIMETADATA_H
