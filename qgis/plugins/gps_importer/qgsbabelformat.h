/***************************************************************************
  qgsbabelformat.h - import/export formats for GPSBabel
Functions:

-------------------
begin                : Oct 20, 2004
copyright            : (C) 2004 by Lars Luthman
email                : larsl@users.sourceforge.net

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

#ifndef QGSBABELFORMAT_H
#define QGSBABELFORMAT_H

#include <map>

#include <qstring.h>
#include <qstringlist.h>


class QgsBabelFormat {
public:
  QgsBabelFormat();
  virtual ~QgsBabelFormat() { }
  
  const QString& getName() const;
  virtual QStringList getImportCommand(const QString& babel,
				       const QString& featuretype,
				       const QString& input,
				       const QString& output) const;
  virtual QStringList getExportCommand(const QString& babel,
				       const QString& featuretype,
				       const QString& input,
				       const QString& output) const;
  
  bool supportsImport() const;
  bool supportsExport() const;
  bool supportsWaypoints() const;
  bool supportsRoutes() const;
  bool supportsTracks() const;
  
protected:
  
  bool mSupportsImport, mSupportsExport, mSupportsWaypoints;
  bool mSupportsRoutes, mSupportsTracks;
};


class QgsSimpleBabelFormat : public QgsBabelFormat {
public:
  QgsSimpleBabelFormat(const QString& format, bool hasWaypoints, 
		       bool hasRoutes, bool hasTracks);
  QStringList getImportCommand(const QString& babel, 
			       const QString& featuretype,
			       const QString& input,
			       const QString& output) const;
protected:
  QString mFormat;
};


class QgsBabelCommand : public QgsBabelFormat {
public:
  QgsBabelCommand(const QString& importCmd, const QString& exportCmd);
  QStringList getImportCommand(const QString& babel,
			       const QString& featuretype,
			       const QString& input,
			       const QString& output) const;
  QStringList getExportCommand(const QString& babel,
			       const QString& featuretype,
			       const QString& input,
			       const QString& output) const;
protected:
  QStringList mImportCmd;
  QStringList mExportCmd;
};


typedef std::map<QString, QgsBabelFormat*> BabelMap;


#endif
