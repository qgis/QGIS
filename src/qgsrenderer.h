/***************************************************************************
                         qgsrenderer.h  -  description
                             -------------------
    begin                : Sat Jan 4 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman @ mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRENDERER_H
#define QGSRENDERER_H
#include <map>
class QString;

class QgsRenderer {
	int type;
	QString field;
	map<QString, QgsRenderItem>items;
	public:
	QgsRenderer(int _type, QString _field);
	void addItem(QString key, QgsRenderItem ri);
	

};

#endif // QGSRENDERER_H
