/***************************************************************************
                          QgsAttributeTableDisplay.h  -  description
                             -------------------
    begin                : Sat Nov 23 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
        Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id */

#ifndef QGSATTRIBUTETABLEDISPLAY_H
#define QGSATTRIBUTETABLEDISPLAY_H
#ifdef WIN32
#include "qgsattributetablebase.h"
#else
#include "qgsattributetablebase.uic.h"
#endif
class QgsAttributeTable;
/**
  *@author Gary E.Sherman
  */

class QgsAttributeTableDisplay:public QgsAttributeTableBase
{
  public:
	QgsAttributeTableDisplay();
	~QgsAttributeTableDisplay();
	QgsAttributeTable *table();
	void setTitle(QString title);
 signals:
	/**Is emitted before the widget deletes itself*/
	void deleted();
};

#endif
