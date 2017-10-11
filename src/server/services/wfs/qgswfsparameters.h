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

#include "qgsrectangle.h"
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
      Q_ENUM( ParameterName )

      enum Format
      {
        NONE,
        GML2,
        GML3,
        GeoJSON,
        XSD
      };

      enum ResultType
      {
        RESULTS,
        HITS
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

      /**
       * Returns REQUEST parameter as a string or an empty string if not
       *  defined.
       * \returns request
       */
      QString request() const;

      /**
       * Returns VERSION parameter as a string or an empty string if not
       *  defined.
       * \returns version
       */
      QString version() const;

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
       *  default value is GML2 or GML3.
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
       *  default value is RESULTS.
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
       *  defined. An exception is raised if I is defined and cannot be
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
       *  defined. An exception is raised if I is defined and cannot be
       *  converted.
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
       *  raised if the BBOX string cannot be converted into a rectangle.
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
      QString name( ParameterName name ) const;
      void raiseError( ParameterName name ) const;
      void raiseError( const QString &msg ) const;
      QVariant value( ParameterName name ) const;
      QVariant defaultValue( ParameterName name ) const;
      void log( const QString &msg ) const;
      void save( const Parameter &parameter );
      int toInt( const QVariant &value, const QVariant &defaultValue, bool *error = Q_NULLPTR ) const;
      int toInt( ParameterName name ) const;
      QgsRectangle toRectangle( const QVariant &value, bool *error = Q_NULLPTR ) const;
      QgsRectangle toRectangle( ParameterName p ) const;
      QStringList toStringList( ParameterName p, char delimiter = ',' ) const;
      QStringList toStringListWithExp( ParameterName p, const QString &exp = "\\(([^()]+)\\)" ) const;

      QgsServerRequest::Parameters mRequestParameters;
      QMap<ParameterName, Parameter> mParameters;
      QList<QgsProjectVersion> mVersions;
  };
}

#endif
