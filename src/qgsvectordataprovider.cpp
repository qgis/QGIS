/***************************************************************************
    qgsvectordataprovider.cpp - DataProvider Interface for vector layers
     --------------------------------------
    Date                 : 26-Oct-2004
    Copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsvectordataprovider.h"
#include "qgsfeature.h"


QgsVectorDataProvider::QgsVectorDataProvider(): mEncoding(QTextCodec::codecForLocale())
{

}


bool QgsVectorDataProvider::addFeatures(std::list<QgsFeature*> flist)
{
  return false;
}

bool QgsVectorDataProvider::deleteFeatures(std::list<int> const & id)
{
  return false;
}

bool QgsVectorDataProvider::addAttributes(std::map<QString,QString> const & name)
{
  return false;
}

bool QgsVectorDataProvider::deleteAttributes(std::set<QString> const & name)
{
  return false;
}

bool QgsVectorDataProvider::changeAttributeValues(std::map<int,std::map<QString,QString> > const & attr_map)
{
  return false;
}

QString QgsVectorDataProvider::getDefaultValue(const QString& attr, 
    QgsFeature* f) {
  return "";
}

bool QgsVectorDataProvider::changeGeometryValues(std::map<int, QgsGeometry> & geometry_map)
{
  return false;
}

bool QgsVectorDataProvider::createSpatialIndex()
{
    return false;
}

void QgsVectorDataProvider::setEncoding(const QString& e)
{
    QTextCodec* ncodec=QTextCodec::codecForName(e);
    if(ncodec)
    {
	mEncoding=ncodec;
    }
    else
    {
#ifdef QGISDEBUG
	qWarning("error finding QTextCodec in QgsVectorDataProvider::setEncoding");
#endif
    }
}

QString QgsVectorDataProvider::encoding() const
{
    if(mEncoding)
    {
	return mEncoding->name();
    }
    else
    {
	return "";
    }
}

QString QgsVectorDataProvider::capabilitiesString() const
{
  QStringList abilitiesList;

  int abilities = capabilities();

  if (abilities & QgsVectorDataProvider::AddFeatures)
  {
    abilitiesList += "Add Features";
#ifdef QGISDEBUG
        std::cerr << "QgsVectorDataProvider::capabilitiesString "
          << "Add Features" << std::endl;
#endif

  }

  if (abilities & QgsVectorDataProvider::DeleteFeatures)
  {
    abilitiesList += "Delete Features";
#ifdef QGISDEBUG
        std::cerr << "QgsVectorDataProvider::capabilitiesString "
          << "Delete Features" << std::endl;
#endif
  }

  if (abilities & QgsVectorDataProvider::ChangeAttributeValues)
  {
    abilitiesList += "Change Attribute Values";
#ifdef QGISDEBUG
        std::cerr << "QgsVectorDataProvider::capabilitiesString "
          << "Change Attribute Values" << std::endl;
#endif
  }

  if (abilities & QgsVectorDataProvider::AddAttributes)
  {
    abilitiesList += "Add Attributes";
#ifdef QGISDEBUG
        std::cerr << "QgsVectorDataProvider::capabilitiesString "
          << "Add Attributes" << std::endl;
#endif
  }

  if (abilities & QgsVectorDataProvider::DeleteAttributes)
  {
    abilitiesList += "Delete Attributes";
#ifdef QGISDEBUG
        std::cerr << "QgsVectorDataProvider::capabilitiesString "
          << "Delete Attributes" << std::endl;
#endif
  }

  if (abilities & QgsVectorDataProvider::SaveAsShapefile)
  {
    abilitiesList += "Save As Shapefile";
#ifdef QGISDEBUG
        std::cerr << "QgsVectorDataProvider::capabilitiesString "
          << "Save As Shapefile" << std::endl;
#endif
  }

  if (abilities & QgsVectorDataProvider::CreateSpatialIndex)
  {
    // TODO: Tighten up this test.  See QgsOgrProvider for details.
    abilitiesList += "Create Spatial Index";
#ifdef QGISDEBUG
        std::cerr << "QgsVectorDataProvider::capabilitiesString "
          << "Create Spatial Index" << std::endl;
#endif
  }

  if (abilities & QgsVectorDataProvider::SelectAtId)
  {
    // Not really meaningful to the user.
    // abilitiesList = "Select at ID";
#ifdef QGISDEBUG
        std::cerr << "QgsVectorDataProvider::capabilitiesString "
          << "Select at ID" << std::endl;
#endif
  }

  if (abilities & QgsVectorDataProvider::ChangeGeometries)
  {
    abilitiesList += "Change Geometries";
#ifdef QGISDEBUG
        std::cerr << "QgsVectorDataProvider::capabilitiesString "
          << "Change Geometries" << std::endl;
#endif
  }

  return abilitiesList.join(", ");

}

