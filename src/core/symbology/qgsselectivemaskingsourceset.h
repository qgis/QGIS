/***************************************************************************
    qgsselectivemaskingsourceset.h
    ---------------------------
    begin                : January 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSELECTIVEMASKINGSOURCESET_H
#define QGSSELECTIVEMASKINGSOURCESET_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsselectivemaskingsource.h"

class QDomElement;
class QDomDocument;
class QgsReadWriteContext;

/**
 * \ingroup core
 * \class QgsSelectiveMaskSourceSet
 *
 * \brief Represents a named set of selective masking sources (QgsSelectiveMaskSource).
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsSelectiveMaskingSourceSet
{
  public:

    /**
     * Constructor for an empty (invalid) QgsSelectiveMaskingSourceSet.
     */
    QgsSelectiveMaskingSourceSet();

    /**
     * Returns TRUE if the source set is valid, or FALSE if it is invalid (default constructed).
     */
    bool isValid() const { return mIsValid; }

    /**
     * Sets the unique identifier for the set.
     *
     * This id must be unique in the whole project. A default random one is
     * automatically generated when a new QgsSelectiveMaskingSourceSet is constructed.
     *
     * \see id()
     */
    void setId( const QString &id ) { mId = id; }

    /**
     * Returns a unique identifier for the set.
     *
     * This id must be unique in the whole project. It is automatically generated
     * when a new QgsSelectiveMaskingSourceSet is constructed.
     *
     * \see setId()
     */
    QString id() const { return mId; }

    /**
     * Returns the set's unique name.
     *
     * \see setName()
     */
    QString name() const { return mName; }

    /**
     * Sets the set's unique \a name.
     *
     * \see name()
     */
    void setName( const QString &name ) { mName = name; mIsValid = true; }

    /**
     * Returns the list of selective mask sources configured in this set.
     *
     * \see setSources()
     */
    QVector< QgsSelectiveMaskSource > sources() const;

    /**
     * Sets the list of selective mask \a sources for this set.
     *
     * \see sources()
     */
    void setSources( const QVector< QgsSelectiveMaskSource > &sources );

    /**
     * Appends a \source to the set.
     */
    void append( const QgsSelectiveMaskSource &source );

    /**
     * Writes the set's state to a DOM element.
     *
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Sets the set's state from a DOM \a element.
     *
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );

    /**
     * Returns the mask source at the specified index.
     */
    QgsSelectiveMaskSource &operator[]( int index ) SIP_FACTORY;
#ifdef SIP_RUN
    % MethodCode
    SIP_SSIZE_T idx = sipConvertFromSequenceIndex( a0, sipCpp->size() );
    if ( idx < 0 )
      sipIsErr = 1;
    else
      sipRes = new QgsSelectiveMaskSource( sipCpp->operator[]( idx ) );
    % End
#endif

    /**
     * Returns the number of sources in the set.
     */
    int size() const;

    /**
     * Returns TRUE if the set is empty.
     */
    bool isEmpty() const;

#ifdef SIP_RUN
    int __len__() const;
    % MethodCode
    sipRes = sipCpp->size();
    % End
#endif

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    if ( !sipCpp->isValid() )
    {
      sipRes = PyUnicode_FromString( "<QgsSelectiveMaskingSourceSet: invalid>" );
    }
    else if ( !sipCpp->name().isEmpty() )
    {
      const QString str = u"<QgsSelectiveMaskingSourceSet: %1 (%2)>"_s.arg(
                            sipCpp->id(),
                            sipCpp->name() );
      sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    }
    else
    {
      const QString str = u"<QgsSelectiveMaskingSourceSet: %1>"_s.arg(
                            sipCpp->id() );
      sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    }
    % End
#endif

  private:
    bool mIsValid = false;
    QString mId;
    QString mName;
    QVector< QgsSelectiveMaskSource > mSources;
};

Q_DECLARE_METATYPE( QgsSelectiveMaskingSourceSet )

#endif //QGSSELECTIVEMASKINGSOURCESET_H
