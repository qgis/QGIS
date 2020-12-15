#ifndef QGSSVGPARAMETER_H
#define QGSSVGPARAMETER_H
/***************************************************************************
 qgssvgparameter.h
 ---------------------
 begin                : December 2020
 copyright            : (C) 2020 by Denis Rouzaud
 email                : denis@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgis_core.h"

#include <QList>
#include <QString>

/**
\ingroup core
A simple class to hold information for SVG dynamic parameters
\since QGIS 3.18
*/
class CORE_EXPORT QgsSvgParameter
{
  public:
    //! Constructor
    QgsSvgParameter() = default;

    //! Constructor
    QgsSvgParameter( const QString &name, const QString &value )
      : mValid( !name.isEmpty() ), mName( name ), mValue( value ) {}

    //! Returns the name of the parameter
    QString name() const {return mName;}
    //! Sets the name of the parameter
    void setName( const QString &name ) {mName = name;}
    //! Returns the value of the parameter
    QString value() const {return mValue;}
    //! Sets the value of the parameter
    void setValue( const QString &value ) {mValue = value;}

  private:
    bool mValid = false;
    QString mName;
    QString mValue;
};

typedef QList<QgsSvgParameter> QgsSvgParameters;

#endif // QGSSVGPARAMETER_H
