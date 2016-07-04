/***************************************************************************
    qgscaleutils.h
    ----------------------
    begin                : July 2012
    copyright            : (C) 2012 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QString>
#include <QStringList>

#ifndef QGSSCALEUTILS_H
#define QGSSCALEUTILS_H

/** \ingroup core
 */
class CORE_EXPORT QgsScaleUtils
{
  public:
    /** Save scales to the given file
     * @param fileName the name of the output file
     * @param scales the list of scales to save
     * @param errorMessage it will contain the error message if something
     * went wrong
     * @return true on success and false if failed
     */
    static bool saveScaleList( const QString &fileName, const QStringList &scales, QString &errorMessage );

    /** Load scales from the given file
     * @param fileName the name of the file to process
     * @param scales it will contain loaded scales
     * @param errorMessage it will contain the error message if something
     * went wrong
     * @return true on success and false if failed
     */
    static bool loadScaleList( const QString &fileName, QStringList &scales, QString &errorMessage );
};

#endif
