/***************************************************************************
                         qgseptdecoder.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEPTDECODER_H
#define QGSEPTDECODER_H


#include "qgis_core.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

#include <QVector>
#include <QString>

namespace QgsEptDecoder
{
  // These two should be really merged to one function ....
  QVector<qint32> decompressBinary( const QString &filename, int pointRecordSize );
  QVector<char> decompressBinaryClasses( const QString &filename, int pointRecordSize );

  QVector<qint32> decompressZStandard( const QString &filename, int pointRecordSize );
  QVector<qint32> decompressLaz( const QString &filename );
};

///@endcond
#endif // QGSEPTDECODER_H
