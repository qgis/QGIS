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


QgsVectorDataProvider::QgsVectorDataProvider()
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

bool QgsVectorDataProvider::addAttributes(std::list<QString> const & name, std::list<QString> const & type)
{
    return false;
}

bool QgsVectorDataProvider::deleteAttributes(std::list<QString> const & name)
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

bool QgsVectorDataProvider::supportsFeatureAddition() const
{
    //needs to be overwritten by providers if they provide feature editing
    return false;
}

bool QgsVectorDataProvider::supportsFeatureDeletion() const
{
    //needs to be overwritten by providers supporting this
    return false;
}

bool QgsVectorDataProvider::supportsAttributeEditing() const
{
    //needs to be overwritten by providers supporting this
    return false;
}

bool QgsVectorDataProvider::supportsSaveAsShapefile() const
{
  // default implementation is no support 
  return false; 
}
