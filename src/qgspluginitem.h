//
//
// C++ Interface: $MODULE$
//
// Description: 
//
//
// Author: Gary Sherman <sherman at mrcc.com>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QGSPLUGINITEM_H
#define QGSPLUGINITEM_H
class QString;
/**
Class to contain information about a loadable plugin, including its name, description and the full path to the shared library

@author Gary Sherman
*/
class QgsPluginItem{
public:
    QgsPluginItem(QString name=0, QString description=0, QString fullPath=0);

    ~QgsPluginItem();
private:
	QString name;
	QString description;
	QString fullPath;

};

#endif
