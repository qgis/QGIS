/***************************************************************************
    begin                : Jul 10 2003
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
#ifndef QGSMAPLAYERINTERFACE_H
#define QGSMAPLAYERINTERFACE_H
/** 
* Interface class for map layer plugins
*/
#include <qobject.h>

class QMainWindow;

class QgsMapLayerInterface: public QObject{
	Q_OBJECT
public:
virtual void setQgisMainWindow(QMainWindow *qgis) = 0;
// a test function to return an int
virtual int getInt()=0;
// setup the plugin's GUI
virtual void initGui()=0;
// unload the plugin
virtual void unload()=0;
// draw function
virtual void draw() = 0;

};
#endif // QGSMAPLAYERINTERFACE_H
