/***************************************************************************
    qgsfield.h - Describes a field in a layer or table
     --------------------------------------
    Date                 : 01-Jan-2004
    Copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
class QString;

class QgsField{
public:
QgsField(QString nam=0, QString typ=0, int len=0, int prec=0);
~QgsField();
QString getName();
QString getType();
int getLength();
int getPrecision();

void setName(QString nam);
void setType(QString typ);
void setLength(int len);
void setPrecision(int prec);
private:
QString name;
QString type;
int length;
int precision;
};
