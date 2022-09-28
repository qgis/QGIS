/***************************************************************************
  qgsprocessingparametertininputlayers.h
  ---------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERTININPUTLAYERS_H
#define QGSPROCESSINGPARAMETERTININPUTLAYERS_H

#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertype.h"

/**
 * \brief A parameter for processing algorithms that need a list of input vector layers to construct a TIN.
 *
 * A valid value for this parameter is a list (QVariantList), where each item is a map (QVariantMap) in this form:
 * {
 *   'source':  string that represents identification of the vector layer,
 *   'type': how the vector layer is used : as vertices, structure lines or break lines
 *   'attributeIndex' : the index of the attribute of the vector layer used to defined the Z value of vertices,
 *    if -1, the Z coordinates of features are used
 * }
 *
 * \ingroup core
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsProcessingParameterTinInputLayers: public QgsProcessingParameterDefinition
{
  public:

    //! Defines the type of input layer
    enum Type
    {
      Vertices, //!< Input that adds only vertices
      StructureLines, //!< Input that adds add structure lines
      BreakLines //!< Input that adds vertices and break lines
    };

    //! Used to store input layer Id and other associated parameters
    struct InputLayer
    {
      QString source; //!The source of the input layer
      Type type; //!The type of the input layer (see Type)
      int attributeIndex; //! The attribute index used for Z value of vertices
    };

    //! Constructor
    QgsProcessingParameterTinInputLayers( const QString &name, const QString &description = QString() );

    QgsProcessingParameterDefinition *clone() const override;
    QString type() const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const override;
    QVariant valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

    //! Returns the type name for the parameter class.
    static QString typeName() { return QStringLiteral( "tininputlayers" ); }
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \brief Parameter type definition for QgsProcessingParameterTinInputLayers.
 *
 * \ingroup core
 * \note This class is not a part of public API.
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsProcessingParameterTypeTinInputLayers : public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterTinInputLayers( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An input allowing selection of multiple layers to create a TIN with vertices and/or break lines" );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "TIN Creation Layers" );
    }

    QString id() const override
    {
      return QgsProcessingParameterTinInputLayers::typeName();
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterTinInputLayers" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterTinInputLayers" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[dict]: list of input layers as dictionaries, see QgsProcessingParameterTinInputLayers docs" );
    }
};

///@endcond
#endif

#endif // QGSPROCESSINGPARAMETERTININPUTLAYERS_H
