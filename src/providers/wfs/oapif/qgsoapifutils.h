/***************************************************************************
    qgsoapifutils.h
    ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOAPIFUTILS_H
#define QGSOAPIFUTILS_H

#include <nlohmann/json.hpp>
using namespace nlohmann;

#include <QString>
#include <QStringList>

/**
 * Utility class
*/
class QgsOAPIFJson
{
  public:
    //! A OAPIF Link
    struct Link
    {
      QString href;
      QString rel;
      QString type;
      QString title;
      qint64 length = -1;
    };

    //! Parses the "link" property of jParent
    static std::vector<Link> parseLinks( const json &jParent );

    //! Find among links the one that matches rel, by using an optional list of preferable types.
    static QString findLink( const std::vector<Link> &links, const QString &rel, const QStringList &preferableTypes = QStringList() );
};

#endif // QGSOAPIFUTILS_H
