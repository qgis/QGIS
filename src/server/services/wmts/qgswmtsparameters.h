/***************************************************************************
                              qgswmtsparameters.h
                              -------------------
  begin                : Aug 10, 2018
  copyright            : (C) 2018 by René-Luc Dhont
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

#ifndef QGSWMTSPARAMETERS_H
#define QGSWMTSPARAMETERS_H

#include <QMap>
#include <QObject>
#include <QMetaEnum>

#include "qgsprojectversion.h"
#include "qgsserverparameters.h"

namespace QgsWmts
{

  /**
   * \ingroup server
   * \class QgsWmts::QgsWmsParameterForWmts
   * \brief WMS parameter used by WMTS service.
   * \since QGIS 3.4
   */
  class QgsWmsParameterForWmts : public QgsServerParameterDefinition
  {
      Q_GADGET

    public:
      //! Available parameters for translating WMTS requests to WMS requests
      enum Name
      {
        UNKNOWN,
        LAYERS,
        STYLES,
        CRS,
        BBOX,
        WIDTH,
        HEIGHT,
        FORMAT,
        TRANSPARENT,
        DPI,
        TILED,
        QUERY_LAYERS,
        I,
        J,
        INFO_FORMAT
      };
      Q_ENUM( Name )

      /**
       * Converts a parameter's name into its string representation.
       */
      static QString name( const QgsWmsParameterForWmts::Name );

      /**
       * Converts a string into a parameter's name (UNKNOWN in case of an
       * invalid string).
       */
      static QgsWmsParameterForWmts::Name name( const QString &name );
  };

  /**
   * \ingroup server
   * \class QgsWmts::QgsWmtsParameter
   * \brief WMTS parameter received from the client.
   * \since QGIS 3.4
   */
  class QgsWmtsParameter : public QgsServerParameterDefinition
  {
      Q_GADGET

    public:
      //! Available parameters for WMTS requests
      enum Name
      {
        UNKNOWN,
        LAYER,
        FORMAT,
        TILEMATRIXSET,
        TILEMATRIX,
        TILEROW,
        TILECOL,
        INFOFORMAT,
        I,
        J
      };
      Q_ENUM( Name )

      /**
       * Constructor for QgsWmtsParameter.
      * \param name Name of the WMS parameter
      * \param type Type of the parameter
      * \param defaultValue Default value of the parameter
       */
      QgsWmtsParameter( const QgsWmtsParameter::Name name = QgsWmtsParameter::UNKNOWN,
                        const QVariant::Type type = QVariant::String,
                        const QVariant defaultValue = QVariant( "" ) );

      /**
       * Default destructor for QgsWmtsParameter.
       */
      virtual ~QgsWmtsParameter() = default;

      /**
       * Converts the parameter into an integer.
       * \returns An integer
       * \throws QgsBadRequestException Invalid parameter exception
       */
      int toInt() const;

      /**
       * Raises an error in case of an invalid conversion.
       * \throws QgsBadRequestException Invalid parameter exception
       */
      void raiseError() const;

      /**
       * Converts a parameter's name into its string representation.
       */
      static QString name( const QgsWmtsParameter::Name );

      /**
       * Converts a string into a parameter's name (UNKNOWN in case of an
       * invalid string).
       */
      static QgsWmtsParameter::Name name( const QString &name );

      QgsWmtsParameter::Name mName;
  };


  /**
   * \ingroup server
   * \class QgsWmts::QgsWmtsParameters
   * \brief Provides an interface to retrieve and manipulate WMTS parameters received from the client.
   * \since QGIS 3.4
   */
  class QgsWmtsParameters : public QgsServerParameters
  {
      Q_GADGET

    public:

      //! Output format for the response
      enum Format
      {
        NONE,
        JPG,
        PNG,
        TEXT,
        XML,
        HTML,
        GML
      };

      /**
       * Constructor for WMTS parameters with specific values.
       * \param parameters Map of parameters where keys are parameters' names.
       */
      QgsWmtsParameters( const QgsServerParameters &parameters );

      /**
       * Constructor for WMTS parameters with default values only.
       */
      QgsWmtsParameters();

      /**
       * Default destructor for QgsWmtsParameters.
       */
      virtual ~QgsWmtsParameters() = default;

      /**
       * Dumps parameters.
       */
      void dump() const;

      /**
       * Returns VERSION parameter if defined or its default value.
       * \returns version
       */
      QgsProjectVersion versionAsNumber() const;

      /**
       * Returns LAYER parameter as a string.
       * \returns layer parameter as string
       */
      QString layer() const;

      /**
       * Returns FORMAT parameter as a string.
       * \returns Format parameter as string
       */
      QString formatAsString() const;

      /**
       * Returns format. If the FORMAT parameter is not used, then the
       * default value is NONE.
       * \returns format
       */
      Format format() const;

      /**
       * Returns TILEMATRIXSET parameter as a string.
       * \returns tileMatrixSet parameter as string
       */
      QString tileMatrixSet() const;

      /**
       * Returns TILEMATRIX parameter as a string.
       * \returns tileMatrix parameter as string
       */
      QString tileMatrix() const;

      /**
       * Returns TILEMATRIX parameter as an int or its default value if not
       * defined. An exception is raised if TILEMATRIX is defined and cannot be
       * converted.
       * \returns tileMatrix parameter
       * \throws QgsBadRequestException
       */
      int tileMatrixAsInt() const;

      /**
       * Returns TILEROW parameter as a string.
       * \returns tileRow parameter as string
       */
      QString tileRow() const;

      /**
       * Returns TILEROW parameter as an int or its default value if not
       * defined. An exception is raised if TILEROW is defined and cannot be
       * converted.
       * \returns tileRow parameter
       * \throws QgsBadRequestException
       */
      int tileRowAsInt() const;

      /**
       * Returns TILECOL parameter as a string.
       * \returns tileCol parameter as string
       */
      QString tileCol() const;

      /**
       * Returns TILECOL parameter as an int or its default value if not
       * defined. An exception is raised if TILECOL is defined and cannot be
       * converted.
       * \returns tileCol parameter
       * \throws QgsBadRequestException
       */
      int tileColAsInt() const;

      /**
       * Returns INFO_FORMAT parameter as a string.
       * \returns INFO_FORMAT parameter as string
       */
      QString infoFormatAsString() const;

      /**
       * Returns infoFormat. If the INFO_FORMAT parameter is not used, then the
       * default value is text/plain.
       * \returns infoFormat
       */
      Format infoFormat() const;

      /**
       * Returns the infoFormat version for GML. If the INFO_FORMAT is not GML,
       * then the default value is -1.
       * \returns infoFormat version
       */
      int infoFormatVersion() const;

      /**
       * Returns I parameter or an empty string if not defined.
       * \returns i parameter
       */
      QString i() const;

      /**
       * Returns I parameter as an int or its default value if not
       * defined. An exception is raised if I is defined and cannot be
       * converted.
       * \returns i parameter
       * \throws QgsBadRequestException
       */
      int iAsInt() const;

      /**
       * Returns J parameter or an empty string if not defined.
       * \returns j parameter
       */
      QString j() const;

      /**
       * Returns J parameter as an int or its default value if not
       * defined. An exception is raised if J is defined and cannot be
       * converted.
       * \returns j parameter
       * \throws QgsBadRequestException
       */
      int jAsInt() const;

    private:
      bool loadParameter( const QString &key, const QString &value ) override;
      void save( const QgsWmtsParameter &parameter );

      void log( const QString &msg ) const;

      QList<QgsProjectVersion> mVersions;
      QMap<QgsWmtsParameter::Name, QgsWmtsParameter> mWmtsParameters;
  };
}

#endif
