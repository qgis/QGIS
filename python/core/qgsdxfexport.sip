/***************************************************************************
                         qgsdxfexport.sip
                         ----------------
    begin                : September 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

class QgsDxfExport
{
%TypeHeaderCode
#include <qgsdxfexport.h>
%End
    public:
        QgsDxfExport();
        ~QgsDxfExport();

        void addLayers( QList< QgsMapLayer* >& layers );
        int writeToFile( QIODevice* d );
};
