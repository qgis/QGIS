//
//
// C++ Implementation: $MODULE$
//
// Description: 
//
//
// Author: Gary Sherman <sherman at mrcc.com>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
/* $Id$ */
#include <qstring.h>
#include "qgspluginitem.h"

QgsPluginItem::QgsPluginItem(QString _name, QString _description, QString _fullPath) :
	m_name(_name), m_description(_description), m_fullPath(_fullPath)
{

}

QString QgsPluginItem::name(){
	return m_name;
}

QString QgsPluginItem::description(){
	return m_description;
}
QString QgsPluginItem::fullPath(){
	return m_fullPath;
}
QgsPluginItem::~QgsPluginItem()
{
}


