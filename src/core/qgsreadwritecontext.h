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
#include "qgsprojecttranslator.h"
#include "qgscoordinatetransformcontext.h"

class QgsReadWriteContextCategoryPopper;

/**
 * \class QgsReadWriteContext
 * \ingroup core
 * \brief A container for the context for various read/write operations on objects.
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
        ReadWriteMessage( const QString &message = QString(), Qgis::MessageLevel level = Qgis::MessageLevel::Warning, const QStringList &categories = QStringList() )
          : mMessage( message )
          , mLevel( level )
          , mCategories( categories )
        {}

        //! Returns the message string
        QString message() const {return mMessage;}

        //! Returns the message level
        Qgis::MessageLevel level() const {return mLevel;}

        //! Returns the stack of categories of the message
        QStringList categories() const {return mCategories;}

        // TODO c++20 - replace with = default

        bool operator==( const QgsReadWriteContext::ReadWriteMessage &other ) const
        {
          return mMessage == other.mMessage && mLevel == other.mLevel && mCategories == other.mCategories;
        }

        bool operator!=( const QgsReadWriteContext::ReadWriteMessage &other ) const
        {
          return !( *this == other );
        }

#ifdef SIP_RUN
        SIP_PYOBJECT __repr__();
        % MethodCode
        QString str = QStringLiteral( "<QgsReadWriteContext.ReadWriteMessage: %1>" ).arg( sipCpp->message() );
        sipRes = PyUnicode_FromString( str.toUtf8().constData() );
        % End
#endif

      private:
        QString mMessage;
        Qgis::MessageLevel mLevel;
        QStringList mCategories;
    };

    /**
     * Constructor for QgsReadWriteContext.
     */
    QgsReadWriteContext();

    ~QgsReadWriteContext();

    //! Returns path resolver for conversion between relative and absolute paths
    const QgsPathResolver &pathResolver() const;

    //! Sets up path resolver for conversion between relative and absolute paths
    void setPathResolver( const QgsPathResolver &resolver );

    /**
     * Append a message to the context
     * \since QGIS 3.2
     */
    void pushMessage( const QString &message, Qgis::MessageLevel level = Qgis::MessageLevel::Warning ) const;

    /**
     * Push a category to the stack
     * \note The return value should always be used so category can be automatically left.
     * \note It is not aimed at being used in Python. Instead use the context manager.
     * \code{.py}
     *   context = QgsReadWriteContext()
     *   with QgsReadWriteContext.enterCategory(context, category, details):
     *     # do something
     * \endcode
     * \since QGIS 3.2
     */
    MAYBE_UNUSED NODISCARD QgsReadWriteContextCategoryPopper enterCategory( const QString &category, const QString &details = QString() ) const SIP_PYNAME( _enterCategory );

    /**
     * Returns the stored messages and remove them
     * \since QGIS 3.2
     */
    QList<QgsReadWriteContext::ReadWriteMessage> takeMessages();

    /**
     * Returns the project translator
     * \since QGIS 3.4
     */
    const QgsProjectTranslator *projectTranslator( ) const { return mProjectTranslator; }

    /**
     * Sets the project translator.
     * It's usually the QgsProject where the function with the context is made and won't be changed anymore.
     *
     * \since QGIS 3.4
     */
    void setProjectTranslator( QgsProjectTranslator *projectTranslator );

    /**
     * Returns data provider coordinate transform context
     *
     * \see setTransformContext()
     *
     * \since QGIS 3.8
     */
    QgsCoordinateTransformContext transformContext() const;

    /**
     * Sets data coordinate transform context to \a transformContext
     *
     * \see transformContext()
     *
     * \since QGIS 3.8
     */
    void setTransformContext( const QgsCoordinateTransformContext &transformContext );

  private:

    //! Pop the last category
    void leaveCategory() const;

    QgsPathResolver mPathResolver;
    mutable QList<ReadWriteMessage> mMessages;
    mutable QStringList mCategories = QStringList();
    QgsProjectTranslator *mProjectTranslator = nullptr;
    friend class QgsReadWriteContextCategoryPopper;
    QgsCoordinateTransformContext mCoordinateTransformContext = QgsCoordinateTransformContext();
};


/**
 * \class QgsReadWriteContextCategoryPopper
 * \ingroup core
 * \brief Allows entering a context category and takes care of
 * leaving this category on deletion of the class.
 *
 * This would happen when it gets out of scope.
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsReadWriteContextCategoryPopper
{
  public:
    //! Creates a popper
    QgsReadWriteContextCategoryPopper( const QgsReadWriteContext &context ) : mContext( context ) {}
    ~QgsReadWriteContextCategoryPopper() {mContext.leaveCategory();}
  private:
#ifdef SIP_RUN
    QgsReadWriteContextCategoryPopper &operator=( const QgsReadWriteContextCategoryPopper & );
#endif

    const QgsReadWriteContext &mContext;
};

#endif // QGSREADWRITECONTEXT_H
