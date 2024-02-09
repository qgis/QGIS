/***************************************************************************
  qgsprocessingparameteralignrasterlayers.h
  ---------------------
  Date                 : July 2023
  Copyright            : (C) 2023 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERALIGNRASTERLAYERS_H
#define QGSPROCESSINGPARAMETERALIGNRASTERLAYERS_H

#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertype.h"
#include "qgsalignrasterdata.h"

/**
 * \brief A parameter for Processing algorithms that need a list of input raster
 * layers to align - this parameter provides Processing framework's adapter for
 * QList<QgsAlignRaster::Item>.
 *
 * A valid value for this parameter is a list (QVariantList), where each
 * item is a map (QVariantMap) in this form:
 * {
 *   'inputFile':  string,
 *   'outputFile': string,
 *   'resampleMethod': int,
 *   'rescale': bool,
 * }
 *
 * Also it can accept lists (either string lists or QgsMapLayer list)
 * as well as individual layer object or string representing layer source.
 *
 * Static functions parametersAsItems(), variantMapAsItem(), itemAsVariantMap()
 * provide conversion between QgsAlignRaster::Item representation and QVariant
 * representation.
 *
 * \ingroup core
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsProcessingParameterAlignRasterLayers : public QgsProcessingParameterDefinition
{
  public:
    //! Constructor for QgsProcessingParameterAlignRasterLayers.
    QgsProcessingParameterAlignRasterLayers( const QString &name, const QString &description = QString() );

    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass ) const override;
    QString valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const override;
    QVariant valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const override;

    //! Returns the type name for the parameter class.
    static QString typeName() { return QStringLiteral( "alignrasterlayers" ); }
    //! Converts a QVariant value (a QVariantList) to a list of input layers
    static QList<QgsAlignRasterData::RasterItem> parameterAsItems( const QVariant &layersVariant, QgsProcessingContext &context );
    //! Converts a QVariant value (a QVariantMap) to a single input layer
    static QgsAlignRasterData::RasterItem variantMapAsItem( const QVariantMap &layerVariantMap, QgsProcessingContext &context );
    //! Converts a single input layer to QVariant representation (a QVariantMap)
    static QVariantMap itemAsVariantMap( const QgsAlignRasterData::RasterItem &item );
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \brief Parameter type definition for QgsProcessingParameterAlignRasterLayers.
 *
 * \ingroup core
 * \note This class is not a part of public API.
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsProcessingParameterTypeAlignRasterLayers : public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterAlignRasterLayers( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An input allowing selection of multiple raster layers to align." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Align raster Layers" );
    }

    QString id() const override
    {
      return QgsProcessingParameterAlignRasterLayers::typeName();
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterAlignRasterLayers" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterAlignRasterLayers" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[dict]: list of input layers as dictionaries, see QgsProcessingParameterAlignRasterLayers docs" )
             << QObject::tr( "list[str]: list of layer IDs" )
             << QObject::tr( "list[str]: list of layer names" )
             << QObject::tr( "list[str]: list of layer sources" )
             << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "list[QgsMapLayer]" )
             << QStringLiteral( "QgsRasterLayer" );
    }
};

///@endcond
#endif

#endif // QGSPROCESSINGPARAMETERALIGNRASTERLAYERS_H
