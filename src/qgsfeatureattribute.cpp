/***************************************************************************
                          qgsfeatureattribute.cpp  -  description
                             -------------------
    begin                : Mon Sep 01 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <qstring.h>
#include "qgsfeatureattribute.h"

QgsFeatureAttribute::QgsFeatureAttribute(QString fld, QString val):field(fld), value(val)
{
}

QgsFeatureAttribute::~QgsFeatureAttribute()
{
}
QString QgsFeatureAttribute::fieldName() const
{
  return field;
}

QString QgsFeatureAttribute::fieldValue() const
{
  return value;
}
