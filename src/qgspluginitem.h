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
/* $Id$ */
#ifndef QGSPLUGINITEM_H
#define QGSPLUGINITEM_H
class QString;
/**
Class to contain information about a loadable plugin, including its name, description and the full path to the shared library

@author Gary Sherman
*/
class QgsPluginItem{
public:
    QgsPluginItem(QString name=0, QString description=0, QString fullPath=0, QString type=0);
    QString name();
    QString description();
    QString fullPath();
    QString type();
    ~QgsPluginItem();
private:
	QString m_name;
	QString m_description;
	QString m_fullPath;
  //! Plugin type (either ui or maplayer)
  QString m_type;

};

#endif
