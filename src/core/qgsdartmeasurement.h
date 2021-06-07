/***************************************************************************
    qgsdartmeasurement.h
     --------------------------------------
    Date                 : 8.11.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDARTMEASUREMENT_H
#define QGSDARTMEASUREMENT_H

#include <QString>

#include "qgis_core.h"

/**
 * \ingroup core
 * \class QgsDartMeasurement
 */
class CORE_EXPORT QgsDartMeasurement
{
  public:
    enum Type
    {
      Text,
      ImagePng,
      Integer
    };

    //! Constructor for QgsDartMeasurement
    QgsDartMeasurement() = default;

    QgsDartMeasurement( const QString &name, Type type, const QString &value );

    const QString toString() const;

    void send() const;

    /**
     * Convert a QgsDartMeasurement::Type enum to a string that is understood
     * by the system.
     *
     * \since QGIS 2.something
     */
    static const QString typeToString( QgsDartMeasurement::Type type );

  private:
    QString mName;
    Type mType = Text;
    QString mValue;
};

#endif // QGSDARTMEASUREMENT_H
