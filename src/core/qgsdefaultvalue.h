/***************************************************************************
  qgsdefaultvalue.h

 ---------------------
 begin                : 19.9.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDEFAULTVALUE_H
#define QGSDEFAULTVALUE_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QString>
#include <QObject>


/**
 * \ingroup core
 *
 * \brief The QgsDefaultValue class provides a container for managing client
 *        side default values for fields.
 *
 * A QgsDefaultValue consists of an expression string that will be evaluated
 * on the client when a default field value needs to be generated.
 *
 * Usual values for such an expression are
 *
 * - `now()` for a timestamp for a record
 * - `@some_variable` to insert a project or application level variable like
 *   the username of the one digitizing a feature
 * - `$length` to insert a derived attribute of a geometry
 *
 * QgsDefaultValue also has a `applyOnUpdate` flag which will indicate that a
 * default value should also be applied when a feature is updated. If this is
 * not set, the default value will only be used when a feature is created.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsDefaultValue
{
    Q_GADGET SIP_SKIP

    Q_PROPERTY( QString expression READ expression WRITE setExpression )
    Q_PROPERTY( bool applyOnUpdate READ applyOnUpdate WRITE setApplyOnUpdate )

  public:

    /**
     * Create a new default value with the given \a expression and \a applyOnUpdate flag.
     * \see QgsVectorLayer::setDefaultValueDefinition
     */
    explicit QgsDefaultValue( const QString &expression = QString(), bool applyOnUpdate = false );
    bool operator==( const QgsDefaultValue &other ) const;

    /**
     * The expression will be evaluated whenever a default value needs
     * to be calculated for a field.
     */
    QString expression() const;

    /**
     * The expression will be evaluated whenever a default value needs
     * to be calculated for a field.
     */
    void setExpression( const QString &expression );

    /**
     * The applyOnUpdate flag determines if this expression should also be
     * applied when a feature is updated or only when it's created.
     */
    bool applyOnUpdate() const;

    /**
     * The applyOnUpdate flag determines if this expression should also be
     * applied when a feature is updated or only when it's created.
     */
    void setApplyOnUpdate( bool applyOnUpdate );

    /**
     * Returns if this default value should be applied.
     * \returns FALSE if the expression is a null string.
     */
    bool isValid() const;

    /**
     * Checks if a default value is set. Alias for isValid().
     * \returns FALSE if the expression is a null string.
     */
    operator bool() const SIP_PYTHON_SPECIAL_BOOL( isValid );

  private:
    QString mExpression;
    bool mApplyOnUpdate = false;
};

Q_DECLARE_METATYPE( QgsDefaultValue )

#endif // QGSDEFAULTVALUE_H
