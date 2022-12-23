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
#include <QMetaEnum>

#include "qgsrectangle.h"
#include "qgsprojectversion.h"
#include "qgsserverparameters.h"

namespace QgsWfs
{

  /**
   * \ingroup server
   * \class QgsWfs::QgsWfsParameter
   * \brief WFS parameter received from the client.
   * \since QGIS 3.4
   */
  class QgsWfsParameter : public QgsServerParameterDefinition
  {
      Q_GADGET

    public:
      //! Available parameters for WFS requests
      enum Name
      {
        UNKNOWN,
        OUTPUTFORMAT,
        RESULTTYPE,
        PROPERTYNAME,
        MAXFEATURES,
        STARTINDEX,
        SRSNAME,
        TYPENAME,
        FEATUREID,
        FILTER,
        BBOX,
        SORTBY,
        EXP_FILTER,
        GEOMETRYNAME
      };
      Q_ENUM( Name )

      /**
       * Constructor for QgsWfsParameter.
      * \param name Name of the WMS parameter
      * \param type Type of the parameter
      * \param defaultValue Default value of the parameter
       */
      QgsWfsParameter( const QgsWfsParameter::Name name = QgsWfsParameter::UNKNOWN,
                       const QVariant::Type type = QVariant::String,
                       const QVariant defaultValue = QVariant( "" ) );

      /**
       * Default destructor for QgsWfsParameter.
       */
      virtual ~QgsWfsParameter() = default;

      /**
       * Converts the parameter into an integer.
       * \returns An integer
       * \throws QgsBadRequestException Invalid parameter exception
       */
      int toInt() const;

      /**
       * Converts the parameter into a list of string.
       * \param exp The expression to use for splitting, pass an empty string to avoid splitting
       * \returns A list of strings
       * \throws QgsBadRequestException Invalid parameter exception
       */
      QStringList toStringListWithExp( const QString &exp = "\\(([^()]+)\\)" ) const;

      /**
       * Converts the parameter into a rectangle.
       * \returns A rectangle
       * \throws QgsBadRequestException Invalid parameter exception
       */
      QgsRectangle toRectangle() const;

      /**
       * Raises an error in case of an invalid conversion.
       * \throws QgsBadRequestException Invalid parameter exception
       */
      void raiseError() const;

      /**
       * Converts a parameter's name into its string representation.
       */
      static QString name( const QgsWfsParameter::Name );

      /**
       * Converts a string into a parameter's name (UNKNOWN in case of an
       * invalid string).
       */
      static QgsWfsParameter::Name name( const QString &name );

      QgsWfsParameter::Name mName;
  };

  /**
   * \ingroup server
   * \class QgsWfs::QgsWfsParameters
   * \brief Provides an interface to retrieve and manipulate WFS parameters received from the client.
   * \since QGIS 3.0
   */
  class QgsWfsParameters : public QgsServerParameters
  {
      Q_GADGET

    public:

      //! Output format for the response
      enum Format
      {
        NONE,
        GML2,
        GML3,
        GeoJSON
      };

      //! Type of results
      enum ResultType
      {
        RESULTS,
        HITS
      };

      /**
       * Constructor for WFS parameters with specific values.
       * \param parameters Map of parameters where keys are parameters' names.
       */
      QgsWfsParameters( const QgsServerParameters &parameters );

      /**
       * Constructor for WFS parameters with default values only.
       */
      QgsWfsParameters();

      /**
       * Default destructor for QgsWfsParameters.
       */
      virtual ~QgsWfsParameters() = default;

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
       * Returns OUTPUTFORMAT parameter as a string.
       * \returns outputFormat parameter as string
       */
      QString outputFormatAsString() const;

      /**
       * Returns format. If the OUTPUTFORMAT parameter is not used, then the
       * default value is GML2 or GML3.
       * \returns format
       */
      Format outputFormat() const;

      /**
       * Returns RESULTTYPE parameter as a string.
       * \returns resultType parameter as string
       */
      QString resultTypeAsString() const;

      /**
       * Returns resultType. If the RESULTTYPE parameter is not used, then the
       * default value is RESULTS.
       * \returns resultType
       */
      ResultType resultType() const;

      /**
       * Returns PROPERTYNAME parameter as list.
       * \returns propertyName parameter as list
       */
      QStringList propertyNames() const;

      /**
       * Returns MAXFEATURES parameter as a string.
       * \returns maxFeatures parameter as string
       */
      QString maxFeatures() const;

      /**
       * Returns MAXFEATURES parameter as an int or its default value if not
       * defined. An exception is raised if I is defined and cannot be
       *  converted.
       * \returns maxFeatures parameter
       * \throws QgsBadRequestException
       */
      int maxFeaturesAsInt() const;

      /**
       * Returns STARTINDEX parameter as a string.
       * \returns startIndex parameter as string
       */
      QString startIndex() const;

      /**
       * Returns STARTINDEX parameter as an int or its default value if not
       * defined. An exception is raised if I is defined and cannot be
       * converted.
       * \returns startIndex parameter
       * \throws QgsBadRequestException
       */
      int startIndexAsInt() const;

      /**
       * Returns SRSNAME parameter as a string.
       * \returns srsName parameter as string
       */
      QString srsName() const;

      /**
       * Returns TYPENAME parameter as list.
       * \returns typeName parameter as list
       */
      QStringList typeNames() const;

      /**
       * Returns FEATUREID parameter as list.
       * \returns featureId parameter as list
       */
      QStringList featureIds() const;

      /**
       * Returns FILTER parameter as list.
       * \returns filter parameter as list
       */
      QStringList filters() const;

      /**
       * Returns BBOX if defined or an empty string.
       * \returns bbox parameter
       */
      QString bbox() const;

      /**
       * Returns BBOX as a rectangle if defined and valid. An exception is
       * raised if the BBOX string cannot be converted into a rectangle.
       * \returns bbox as rectangle
       * \throws QgsBadRequestException
       */
      QgsRectangle bboxAsRectangle() const;

      /**
       * Returns SORTBY parameter as list.
       * \returns sortBy parameter as list
       */
      QStringList sortBy() const;

      /**
       * Returns EXP_FILTER parameter as list.
       * \returns expFilters parameter as list
       */
      QStringList expFilters() const;

      /**
       * Returns GEOMETRYNAME parameter as a string.
       * \returns geometryName parameter as string
       */
      QString geometryNameAsString() const;

    private:
      bool loadParameter( const QString &key, const QString &value ) override;
      void save( const QgsWfsParameter &parameter );

      void log( const QString &msg ) const;

      QList<QgsProjectVersion> mVersions;
      QMap<QgsWfsParameter::Name, QgsWfsParameter> mWfsParameters;
  };
}

#endif
