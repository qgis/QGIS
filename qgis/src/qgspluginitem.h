/***************************************************************************
                          qgspluginitem.cpp -  description
                             -------------------
    begin                : May 29 2003
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
