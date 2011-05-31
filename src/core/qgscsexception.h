/***************************************************************************
                qgscsexception.h - Coordinate System Exception
                             -------------------
  begin                : 2004-12-29
  copyright            : (C) 2004 by Gary Sherman
  email                : sherman at mrcc dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCSEXCEPTION_H
#define QGSCSEXCEPTION_H

#include "qgsexception.h"
/** \ingroup core
 * Custom exception class for Coordinate Reference System related exceptions.
 */
class CORE_EXPORT QgsCsException : public QgsException
{
  public:
    QgsCsException( QString const &what ) : QgsException( what ) {};

};
#endif //QGCSEXCEPTION_H
