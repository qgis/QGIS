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
     * Struct for QgsReadWriteContext error or warning messages
     * \since QGIS 3.2
     */
    struct ReadWriteMessage
    {
        //! Construct a container for QgsReadWriteContext error or warning messages
        ReadWriteMessage( const QString &message, Qgis::MessageLevel level = Qgis::Warning, const QStringList &categories = QStringList() )
          : mMessage( message )
          , mLevel( level )
          , mCategories( categories )
        {}

        //! Return the message string
        QString message() const {return mMessage;}

        //! Return the message level
        Qgis::MessageLevel level() const {return mLevel;}

        //! Return the stack of categories of the message
        QStringList categories() const {return mCategories;}

      private:
        QString mMessage;
        Qgis::MessageLevel mLevel;
        QStringList mCategories;
    };

    /**
     * Constructor for QgsReadWriteContext.
     */
    QgsReadWriteContext() = default;

    ~QgsReadWriteContext();

    //! Returns path resolver for conversion between relative and absolute paths
    const QgsPathResolver &pathResolver() const;

    //! Sets up path resolver for conversion between relative and absolute paths
    void setPathResolver( const QgsPathResolver &resolver );

    /**
     * Append a message to the context
     * \since QGIS 3.2
     */
    void pushMessage( const QString &message, Qgis::MessageLevel level );

    /**
     * Push a category to the stack
     * \since QGIS 3.2
     */
    void enterCategory( const QString &category, const QString &details = QString() );

    /**
     * Pop the last category
     * \since QGIS 3.2
     */
    void leaveCategory();

    /**
     * Return the stored messages and remove them
     * \since QGIS 3.2
     */
    QList<QgsReadWriteContext::ReadWriteMessage> takeMessages();

  private:
    QgsPathResolver mPathResolver;
    QList<ReadWriteMessage> mMessages;
    QStringList mCategories = QStringList();
};

#endif // QGSREADWRITECONTEXT_H
