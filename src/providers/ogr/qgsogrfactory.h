/***************************************************************************
     qgsogrfactory.h
     --------------------------------------
    Date                 : Sun Sep 16 12:18:58 AKDT 2007
    Copyright            : (C) 2004 Christoph Spoerri 
    Email                : <spoerri@sourceforge.net>
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRFACTORY_H
#define QGSOGRFACTORY_H

#include <qstring.h>
#include <qstringlist.h>

#include <ogrsf_frmts.h>

#include "qgsshapefileprovider.h"
#include "../../src/qgsdataproviderfactory.h"
#include "../../src/qgsdataprovider.h"



/**
@author Christoph Spoerri
*/
class QgsOGRFactory : public QgsDataProviderFactory
{
public:
    QgsOGRFactory();
    virtual ~QgsOGRFactory();
    
    QString getFactoryType() { return "OGR Dataprovider Factory"; }
    bool testCapability(int);
    void setURI(QString uri);
    QStringList getLayers();
    bool create(QString newLocation, QString newName, QString type);
    QgsDataProvider* open(QString name);

    bool copy(QString oldName, QString newName);
    bool copy(QString oldName, QString newLocation, QString newName) {};
    bool move(QString newLocation) {};
    bool rename(QString newName) {};

private:
    bool valid;
    QString dataSourceURI;
    OGRDataSource * ogrDS;
    OGRSFDriver * ogrDriver;

};

#endif
