/***************************************************************************
                               qgspluginitem.cpp  
                             -------------------
    begin                : 
    copyright            : (C) 2004 by Gary E.Sherman
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
#include "qgspluginitem.h"

QgsPluginItem::QgsPluginItem(QString _name, QString _description, QString _fullPath, QString _type):
m_name(_name), m_description(_description), m_fullPath(_fullPath), m_type(_type)
{

}

QString QgsPluginItem::name()
{
  return m_name;
}

QString QgsPluginItem::description()
{
  return m_description;
}

QString QgsPluginItem::fullPath()
{
  return m_fullPath;
}

QString QgsPluginItem::type()
{
  return m_type;
}

QgsPluginItem::~QgsPluginItem()
{
}
