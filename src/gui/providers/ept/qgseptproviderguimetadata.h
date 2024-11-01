/***************************************************************************
                         qgseptproviderguimetadata.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEPTPROVIDERGUIMETADATA_H
#define QGSEPTPROVIDERGUIMETADATA_H

///@cond PRIVATE
#define SIP_NO_FILE

#include <QList>
#include <QMainWindow>

#include "qgsproviderguimetadata.h"

class QgsEptProviderGuiMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsEptProviderGuiMetadata();
};

///@endcond

#endif // QGSEPTPROVIDERGUIMETADATA_H
