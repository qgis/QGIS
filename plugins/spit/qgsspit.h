/***************************************************************************
                          qgsspit.h  -  description
                             -------------------
    begin                : Fri Dec 19 2003
    copyright            : (C) 2003 by Denis Antipov
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <vector>
#include <algorithm>
#include <qstringlist.h>
#include <qsettings.h>
#include "qgsspitbase.h"
#include "qgsshapefile.h"
extern "C"
{
  #include <libpq-fe.h>
}

class QgsSpit :public QgsSpitBase{
  public:
  QgsSpit(QWidget *parent=0, const char *name=0);
  ~QgsSpit();
	void populateConnectionList();
  void dbConnect();
  QStringList selectedTables();
  QString connInfo();
  void newConnection();
  void editConnection();
	void removeConnection();
  void addFile();
  void removeFile();
  void removeAllFiles();
  void useDefaultSrid();
  void useDefaultGeom();
  void helpInfo();
	void getSchema();
	void updateSchema();
  void import();

  private:
	PGconn* checkConnection();
	QStringList schema_list;
  int total_features;
  std::vector <QgsShapeFile *> fileList;
  int defSrid;
  QString defGeom;
  int defaultSridValue;
  QString defaultGeomValue;
	QString gl_key;
};
