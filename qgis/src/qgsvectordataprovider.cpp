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


QgsVectorDataProvider::QgsVectorDataProvider(): mEditable(false), mModified(false)
{

}

bool QgsVectorDataProvider::startEditing()
{
    //providers supporting editing need to overwrite this method
    return false;
}

void QgsVectorDataProvider::stopEditing()
{
    mEditable=false;
}

bool QgsVectorDataProvider::commitChanges()
{
    if(mEditable)
    {
	bool returnvalue=true;
	for(std::list<QgsFeature*>::iterator it=mAddedFeatures.begin();it!=mAddedFeatures.end();++it)
	{
	    if(!commitFeature(*it))
	    {
		returnvalue=false;
	    }
	    delete *it;
	}
	mAddedFeatures.clear();
	mModified=false;
	return returnvalue;
    }
    else
    {
	return false;
    }
}

bool QgsVectorDataProvider::rollBack()
{
    if(mEditable)
    {
	for(std::list<QgsFeature*>::iterator it=mAddedFeatures.begin();it!=mAddedFeatures.end();++it)
	{
	    delete *it;
	}
	mAddedFeatures.clear();
	mModified=false;
	return true;
    }
    else
    {
	return false;
    }
}

bool QgsVectorDataProvider::addFeature(QgsFeature* f)
{
    if(mEditable)
    {
	mAddedFeatures.push_back(f);
	mAddedFeaturesIt=mAddedFeatures.begin();
	mModified=true;
	return true;
    }
    else
    {
	return false;
    }
}

bool QgsVectorDataProvider::deleteFeature(int id)
{
    //not yet implemented
    return false;
}


QString QgsVectorDataProvider::getDefaultValue(const QString& attr, 
					       QgsFeature* f) {
  return "";
}


bool QgsVectorDataProvider::commitFeature(QgsFeature* f)
{
    //needs to be done by subclasses
    return false;
}
