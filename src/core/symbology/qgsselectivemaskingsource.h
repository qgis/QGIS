/***************************************************************************
    qgsselectivemaskingsource.h
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

#ifndef QGSSELECTIVEMASKINGSOURCE_H
#define QGSSELECTIVEMASKINGSOURCE_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"

#include <QString>

/**
 * \ingroup core
 * \class QgsSelectiveMaskSource
 *
 * \brief Encapsulates a single source for selective masking (e.g. a symbol layer or label rule).
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsSelectiveMaskSource
{
  public:

    /**
     * Constructor for an invalid QgsSelectiveMaskSource.
     */
    QgsSelectiveMaskSource() = default;

    /**
     * Constructor for QgsSelectiveMaskSource.
     *
     * \param layerId source layer ID
     * \param sourceType masking source type
     * \param sourceId optional ID for the symbol layer or label rule
     */
    QgsSelectiveMaskSource( const QString &layerId, Qgis::SelectiveMaskSourceType sourceType, const QString &sourceId = QString() );

    /**
     * Returns TRUE if the source is valid (i.e. has a non-empty layer ID).
     */
    bool isValid() const { return !mLayerId.isEmpty(); }

    /**
     * Returns the source layer id.
     *
     * \see setLayerId()
     */
    QString layerId() const { return mLayerId; }

    /**
     * Sets the source layer \a id.
     *
     * \see layerId()
     */
    void setLayerId( const QString &id ) { mLayerId = id; }

    /**
     * Returns the type of masking source.
     *
     * \see setSourceType()
     */
    Qgis::SelectiveMaskSourceType sourceType() const { return mSourceType; }

    /**
     * Sets the source \a type.
     *
     * \see sourceType()
     */
    void setSourceType( Qgis::SelectiveMaskSourceType type ) { mSourceType = type; }

    /**
     * Returns the symbol layer or label rule ID.
     *
     * \see setSourceId()
     */
    QString sourceId() const { return mSourceId; }

    /**
     * Sets the symbol layer or label rule \a id.
     *
     * \see sourceId()
     */
    void setSourceId( const QString &id ) { mSourceId = id; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    if ( !sipCpp->isValid() )
    {
      sipRes = PyUnicode_FromString( "<QgsSelectiveMaskSource: invalid>" );
    }
    else if ( sipCpp->sourceId().isEmpty() )
    {
      const QString str = u"<QgsSelectiveMaskSource: %1 (%2)>"_s.arg(
                            sipCpp->layerId(),
                            qgsEnumValueToKey( sipCpp->sourceType() ) );
      sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    }
    else
    {
      const QString str = u"<QgsSelectiveMaskSource: %1 - %2 (%3)>"_s.arg(
                            sipCpp->layerId(),
                            sipCpp->sourceId(),
                            qgsEnumValueToKey( sipCpp->sourceType() ) );
      sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    }
    % End
#endif

    bool operator==( const QgsSelectiveMaskSource &other ) const = default;
    bool operator!=( const QgsSelectiveMaskSource &other ) const = default;

  private:
    QString mLayerId;
    Qgis::SelectiveMaskSourceType mSourceType = Qgis::SelectiveMaskSourceType::SymbolLayer;
    QString mSourceId;
};

#endif //QGSSELECTIVEMASKINGSOURCE_H
