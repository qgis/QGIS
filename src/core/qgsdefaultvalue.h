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

#include <QString>
#include <QObject>

class CORE_EXPORT QgsDefaultValue
{
    Q_GADGET

    Q_PROPERTY( QString expression READ expression WRITE setExpression )
    Q_PROPERTY( bool applyOnUpdate READ applyOnUpdate WRITE setApplyOnUpdate )

  public:
    QgsDefaultValue( const QString &expression = QString(), bool applyOnUpdate = false );
    bool operator==( const QgsDefaultValue &other ) const;

    QString expression() const;
    void setExpression( const QString &expression );

    bool applyOnUpdate() const;
    void setApplyOnUpdate( bool applyOnUpdate );

  private:
    QString mExpression;
    bool mApplyOnUpdate = false;
};

#endif // QGSDEFAULTVALUE_H
