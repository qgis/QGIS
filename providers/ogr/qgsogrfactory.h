//
// C++ Interface: qgsogrfactory
//
// Description: 
//
//
// Author: Christoph Spoerri <spoerri@sourceforge.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
