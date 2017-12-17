/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdartmeasurement.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsDartMeasurement
{

%TypeHeaderCode
#include "qgsdartmeasurement.h"
%End
  public:
    enum Type
    {
      Text,
      ImagePng,
      Integer
    };

    QgsDartMeasurement();
%Docstring
Constructor for QgsDartMeasurement
%End

    QgsDartMeasurement( const QString &name, Type type, const QString &value );

    const QString toString() const;

    void send() const;

    static const QString typeToString( QgsDartMeasurement::Type type );
%Docstring
Convert a QgsDartMeasurement.Type enum to a string that is understood
by the system.

.. versionadded:: 2.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdartmeasurement.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
