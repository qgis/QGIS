//
// C++ Implementation: qgsogrfactory
//
// Description: 
//
//
// Author: Christoph Spoerri <spoerri@sourceforge.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <iostream>
#include <qmessagebox.h>

#include "qgsogrfactory.h"

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

QgsOGRFactory::QgsOGRFactory()
{
}


QgsOGRFactory::~QgsOGRFactory()
{
}

bool QgsOGRFactory::testCapability(int t)
{
  switch (t)
  {
  case QgsDataProviderFactory::Copy:
    return TRUE;
  case QgsDataProviderFactory::Rename:
    return FALSE;
  case QgsDataProviderFactory::Move:
    return TRUE;
  default:
    return FALSE;
  }
}

void QgsOGRFactory::setURI(QString uri)
{
  dataSourceURI = uri;
  OGRRegisterAll();

  // make connection to the data source
  std::cerr << "Data source uri is " << dataSourceURI << std::endl;
  // try to open for read
  ogrDS = OGRSFDriverRegistrar::Open((const char *) dataSourceURI, FALSE, &ogrDriver);
  if(ogrDS != NULL)
  {
#ifdef QGISDEBUG
    std::cerr << "Data source is valid" << std::endl;
#endif
    valid = true;
  } else
  {
    QMessageBox::warning(NULL, "Wrong Path/URI", "The provided path for the dataset is not valid.",QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    valid = false;
  }
}

bool QgsOGRFactory::create(QString newLocation, QString newName, QString type)
{
    
}

QgsDataProvider* QgsOGRFactory::open( QString name)
{
  QgsDataProvider *d = (QgsDataProvider *) new QgsShapeFileProvider(dataSourceURI+QString("/")+name);
  return d;
}

QStringList QgsOGRFactory::getLayers()
{
  OGRLayer *lyr;
  OGRFeatureDefn *lyrDef;
  QStringList lyrList = QStringList();
  
  if (valid) {
    for (int i = 0 ; i < ogrDS->GetLayerCount(); i++) 
    {
      lyr = ogrDS->GetLayer(i);
      lyrDef = lyr->GetLayerDefn();
      std::cerr << "Layer " << i << " is: " << lyrDef->GetName() << "\n";
      //TODO append extension to name, so that it can be opened properly
      lyrList.append(lyrDef->GetName());
    }
  }
  return lyrList;
}

bool QgsOGRFactory::copy(QString oldName, QString newName)
{
  OGRLayer *oldLyr;
  std::cout << "Copy ogr datasource" << std::endl;
  
  if (valid)
  {
    oldLyr = ogrDS->GetLayerByName((const char*) oldName);
    std::cout << "datasource is " << ogrDriver->GetName() << std::endl;
    if (QString::compare(ogrDriver->GetName(),"ESRI Shapefile")==0) {
      std::cout << "Copy shapefile" << std::endl;
      
    }
  }
}

/**
 * Class factory to return a pointer to a newly created 
 * QgsShapeFileProvider object
 */
QGISEXTERN QgsOGRFactory * classFactory_Fact()
{
  return new QgsOGRFactory();
}
