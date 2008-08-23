/***************************************************************************
  qgsbabelformat.h - import/export formats for GPSBabel
   -------------------
  begin                : Oct 20, 2004
  copyright            : (C) 2004 by Lars Luthman
  email                : larsl@users.sourceforge.net

 ***************************************************************************
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

#include <QStringList>

class QString;

class QgsBabelFormat
{
  public:
    QgsBabelFormat( const QString& name = "" );
    virtual ~QgsBabelFormat() { }

    const QString& name() const;
    virtual QStringList importCommand( const QString& babel,
                                       const QString& featuretype,
                                       const QString& input,
                                       const QString& output ) const;
    virtual QStringList exportCommand( const QString& babel,
                                       const QString& featuretype,
                                       const QString& input,
                                       const QString& output ) const;

    bool supportsImport() const;
    bool supportsExport() const;
    bool supportsWaypoints() const;
    bool supportsRoutes() const;
    bool supportsTracks() const;

  protected:

    QString mName;
    bool mSupportsImport, mSupportsExport;
    bool mSupportsWaypoints, mSupportsRoutes, mSupportsTracks;
};



class QgsSimpleBabelFormat : public QgsBabelFormat
{
  public:
    QgsSimpleBabelFormat( const QString& format, bool hasWaypoints,
                          bool hasRoutes, bool hasTracks );
    QStringList importCommand( const QString& babel,
                               const QString& featuretype,
                               const QString& input,
                               const QString& output ) const;
  protected:
    QString mFormat;
};



class QgsBabelCommand : public QgsBabelFormat
{
  public:
    QgsBabelCommand( const QString& importCmd, const QString& exportCmd );
    QStringList importCommand( const QString& babel,
                               const QString& featuretype,
                               const QString& input,
                               const QString& output ) const;
    QStringList exportCommand( const QString& babel,
                               const QString& featuretype,
                               const QString& input,
                               const QString& output ) const;
  protected:
    QStringList mImportCmd;
    QStringList mExportCmd;
};


typedef std::map<QString, QgsBabelFormat*> BabelMap;


#endif
