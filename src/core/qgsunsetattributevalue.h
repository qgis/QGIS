/***************************************************************************
                         qgsunsetattributevalue.h
                         ------------------------
    begin                : July 2022
    copyright            : (C) 2022 by Nyall Dawson
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

#ifndef QGSUNSETATTRIBUTEVALUE_H
#define QGSUNSETATTRIBUTEVALUE_H

#include "qgis_core.h"
#include <QString>
#include <QVariant>

/**
 * \ingroup core
 * \brief Represents a default, "not-specified" value for a feature attribute.
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsUnsetAttributeValue
{
  public:

    /**
     * Constructor for a QgsUnsetAttributeValue.
     */
    QgsUnsetAttributeValue() = default;

    /**
     * Constructor for a QgsUnsetAttributeValue, with the specified original provider's default value clause.
     */
    explicit QgsUnsetAttributeValue( const QString &defaultValueClause );

    /**
     * Returns the original data provider's default value clause.
     */
    QString defaultValueClause() const { return mDefaultValueClause; }

    /**
     * QgsUnsetAttributeValues are always considered equal to each other, regardless of what the original provider's default value clause was.
     */
    inline bool operator==( const QgsUnsetAttributeValue & ) const { return true; }

    /**
     * QgsUnsetAttributeValues are always considered equal to each other, regardless of what the original provider's default value clause was.
     */
    inline bool operator!=( const QgsUnsetAttributeValue & ) const { return false; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str;
    if ( !sipCpp->defaultValueClause().isEmpty() )
      str = QStringLiteral( "<QgsUnsetAttributeValue: %1>" ).arg( sipCpp->defaultValueClause() );
    else
      str = QStringLiteral( "<QgsUnsetAttributeValue>" );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    //! Allows direct construction of QVariants from unset values.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

  private:

    QString mDefaultValueClause;

};

Q_DECLARE_METATYPE( QgsUnsetAttributeValue )


inline bool CORE_EXPORT operator==( const QgsUnsetAttributeValue &value, const QString &other )
{
  return other == value.defaultValueClause();
}

inline bool operator!=( const QgsUnsetAttributeValue &value, const QString &other )
{
  return other != value.defaultValueClause();
}

#ifndef SIP_RUN
inline bool operator==( const QString &other, const QgsUnsetAttributeValue &value )
{
  return other == value.defaultValueClause();
}

inline bool operator!=( const QString &other, const QgsUnsetAttributeValue &value )
{
  return other != value.defaultValueClause();
}
#endif

#endif // QGSUNSETATTRIBUTEVALUE_H


