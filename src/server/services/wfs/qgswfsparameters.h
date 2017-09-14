/***************************************************************************
                              qgswfsparameters.h
                              ------------------
  begin                : Sept 14, 2017
  copyright            : (C) 2017 by René-Luc Dhont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSPARAMETERS_H
#define QGSWFSPARAMETERS_H

#include <QMap>
#include <QObject>
#include <QMetaEnum>

#include "qgswfsserviceexception.h"
#include "qgsserverrequest.h"
#include "qgsprojectversion.h"

/**
 * QgsWfsParameters provides an interface to retrieve and manipulate WFS
 *  parameters received from the client.
 * \since QGIS 3.0
 */
namespace QgsWfs
{

  class QgsWfsParameters
  {
      Q_GADGET

    public:
      enum ParameterName
      {
        OUTPUTFORMAT,
        SRSNAME
      };
      Q_ENUM( ParameterName )

      enum Format
      {
        NONE,
        GML2,
        GML3,
        GeoJSON,
        XSD
      };

      struct Parameter
      {
        ParameterName mName;
        QVariant::Type mType;
        QVariant mDefaultValue;
        QVariant mValue;
      };

      /**
       * Constructor.
       * \param map of parameters where keys are parameters' names.
       */
      QgsWfsParameters( const QgsServerRequest::Parameters &parameters );

      /**
       * Constructor.
        */
      QgsWfsParameters();

      /**
       * Loads new parameters.
       * \param map of parameters
       */
      void load( const QgsServerRequest::Parameters &parameters );

      /**
       * Dumps parameters.
       */
      void dump() const;

<<<<<<< HEAD
      /** Returns VERSION parameter as a string or an empty string if not
=======
      /**
       * Returns REQUEST parameter as a string or an empty string if not
       *  defined.
       * \returns request
       */
      QString request() const;

      /**
       * Returns VERSION parameter as a string or an empty string if not
>>>>>>> 747f00d... QgsWfsParameters
       *  defined.
       * \returns version
       */
      QString version() const;

      /**
       * Returns VERSION parameter if defined or its default value.
       * \returns version
       */
      QgsProjectVersion versionAsNumber() const;

      /** Returns OUTPUTFORMAT parameter as a string.
       * \returns OUTPUTFORMAT parameter as string
       */
      QString outputFormatAsString() const;

      /**
       * Returns format. If the OUTPUTFORMAT parameter is not used, then the
       *  default value is GML2 or GML3.
       * \returns format
       */
      Format outputFormat() const;

      /** Returns SRSNAME parameter as a string.
       * \returns SRSNAME parameter as string
       */
      QString srsName() const;


    private:
      QString name( ParameterName name ) const;
      void raiseError( ParameterName name ) const;
      void raiseError( const QString &msg ) const;
      QVariant value( ParameterName name ) const;
      QVariant defaultValue( ParameterName name ) const;
      void log( const QString &msg ) const;
      void save( const Parameter &parameter );

      QgsServerRequest::Parameters mRequestParameters;
      QMap<ParameterName, Parameter> mParameters;
      QList<QgsProjectVersion> mVersions;
  };
}

#endif
