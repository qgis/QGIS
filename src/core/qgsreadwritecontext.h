/***************************************************************************
                         qgsreadwritecontext.h
                         ----------------------
    begin                : May 2017
    copyright            : (C) 2017 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSREADWRITECONTEXT_H
#define QGSREADWRITECONTEXT_H

#include "qgspathresolver.h"
#include "qgis.h"

/**
 * \class QgsReadWriteContext
 * \ingroup core
 * The class is used as a container of context for various read/write operations on other objects.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsReadWriteContext
{
  public:

    /**
     * Constructor for QgsReadWriteContext.
     */
    QgsReadWriteContext() = default;

    //! Returns path resolver for conversion between relative and absolute paths
    const QgsPathResolver &pathResolver() const { return mPathResolver; }

    //! Sets up path resolver for conversion between relative and absolute paths
    void setPathResolver( const QgsPathResolver &resolver ) { mPathResolver = resolver; }

    //! append a message to the context
    void pushMessage( Qgis::MessageLevel level, const QString &message ) {mMessages.append( qMakePair( level, message ) );}

    //! return the stored messages and remove them
    QList<QPair<Qgis::MessageLevel, QString>> takeMessages() {return mMessages; mMessages.clear();}

  private:
    QgsPathResolver mPathResolver;
    QList<QPair<Qgis::MessageLevel, QString>> mMessages;
};

#endif // QGSREADWRITECONTEXT_H
