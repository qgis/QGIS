// $Id$
//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 by Mateusz Loskot <mateusz@loskot.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef QGIS_PLUGIN_OGRCONV_FORMATS_H_INCLUDED
#define QGIS_PLUGIN_OGRCONV_FORMATS_H_INCLUDED

// Qt4
#include <QMap>
#include <QString>

class Format
{
  public:

    enum Type
    {
      eUnknown = 0,
      eFile = 1,
      eDirectory = 2,
      eProtocol = 4
    };

    Format();
    Format( QString const& c, QString const& n );
    Format( QString const& c, QString const& n, unsigned char const& t );
    Format( QString const& c, QString const& n, QString const& p, unsigned char const& t );

    QString const& code() const;
    QString const& name() const;
    QString const& protocol() const;
    unsigned char const& type() const;

  private:

    QString mCode;
    QString mName;
    QString mProtocol;
    unsigned char mTypeFlags;
};

inline bool isFormatType( unsigned char const& frmt, Format::Type const& type )
{
  return (( frmt & type ) == type );
}

class FormatsRegistry
{
  public:

    FormatsRegistry();

    void add( Format const& frmt );
    Format const&  find( QString const& code );

  private:

    void init();

    QMap<QString, Format> mFrmts;
    Format mCache;
};

#endif // QGIS_PLUGIN_OGRCONV_FORMATS_H_INCLUDED
