/***************************************************************************
                         qgsprocessingalgexception.h
                         ------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGEXCEPTION_H
#define QGSPROCESSINGEXCEPTION_H

#include "qgis_core.h"
#include "qgsexception.h"

/**
 * \class QgsProcessingException
 * \ingroup core
 * Custom exception class for processing related exceptions.
 */
class CORE_EXPORT QgsProcessingException : public QgsException
{
  public:
    QgsProcessingException( const QString &what ) : QgsException( what ) {}

};
#endif // QGSPROCESSINGEXCEPTION_H
