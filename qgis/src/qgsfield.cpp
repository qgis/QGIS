/***************************************************************************
    qgsfield.cpp - Describes a field in a layer or table
     --------------------------------------
    Date                 : 01-Jan-2004
    Copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
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
#include "qgsfield.h"

QgsField::QgsField(QString nam, QString typ, int len, int prec) 
: name(nam), type(typ), length(len), precision(prec){
  // lower case the field name since some stores use upper case 
  // (eg. shapefiles)
  name = name.lower();
}
QgsField::~QgsField(){
}
QString QgsField::getName(){
  return name;
}
QString QgsField::getType(){
  return type;
}
int QgsField::getLength(){
  return length;
}
int QgsField::getPrecision(){
  return precision;
}

void QgsField::setName(QString nam){
  name = nam;
}
void QgsField::setType(QString typ){
  type = typ;
}
void QgsField::setLength(int len){
  length = len;
}
void QgsField::setPrecision(int prec){
  precision = prec;
}
