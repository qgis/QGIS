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
#include <qstring.h>
#include "qgspluginitem.h"

QgsPluginItem::QgsPluginItem(QString _name, QString _description, QString _fullPath) :
	name(_name), description(_description), fullPath(_fullPath)
{

}


QgsPluginItem::~QgsPluginItem()
{
}


