/***************************************************************************
                         qgsprocessingparametertypeimpl.h
                         ------------------------
    begin                : March 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERTYPEIMPL_H
#define QGSPROCESSINGPARAMETERTYPEIMPL_H

#include "qgis.h"
#include "qgis_sip.h"
#include "qgsprocessingparametertype.h"
#include <QCoreApplication>

#define SIP_NO_FILE

/**
 * A raster layer parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('raster')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeRasterLayer : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRasterLayer( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A raster layer parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Layer" );
    }

    QString id() const override
    {
      return QStringLiteral( "raster" );
    }
};

/**
 * A vector layer parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('vector')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeVectorLayer : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorLayer( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector layer parameter, e.g. for algorithms which change layer styles, edit layers in place, or other operations which affect an entire layer." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Layer" );
    }

    QString id() const override
    {
      return QStringLiteral( "vector" );
    }
};

/**
 * A generic map layer parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('maplayer')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeMapLayer : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMapLayer( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A generic map layer parameter, which accepts either vector or raster layers." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Map Layer" );
    }

    QString id() const override
    {
      return QStringLiteral( "maplayer" );
    }
};

/**
 * A boolean parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('boolean')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeBoolean : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterBoolean( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A boolean parameter, for true/false values." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Boolean" );
    }

    QString id() const override
    {
      return QStringLiteral( "boolean" );
    }
};

/**
 * An expression parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('expression')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeExpression : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterExpression( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An expression parameter, to add custom expressions based on layer fields." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Expression" );
    }

    QString id() const override
    {
      return QStringLiteral( "expression" );
    }
};

/**
 * A crs parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('crs')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeCrs : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterCrs( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A coordinate reference system (CRS) input parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "CRS" );
    }

    QString id() const override
    {
      return QStringLiteral( "crs" );
    }
};

/**
 * A numeric range parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('range')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeRange : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRange( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric range parameter for processing algorithms." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Range" );
    }

    QString id() const override
    {
      return QStringLiteral( "range" );
    }
};

/**
 * A point parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('point')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypePoint : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterPoint( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A geographic point parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Point" );
    }

    QString id() const override
    {
      return QStringLiteral( "point" );
    }
};

/**
 * An enum based parameter for processing algorithms, allowing for selection from predefined values.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('enum')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeEnum : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterEnum( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "TODO." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Enum" );
    }

    QString id() const override
    {
      return QStringLiteral( "enum" );
    }
};

/**
 * A rectangular map extent parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('extent')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeExtent : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterExtent( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A map extent parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Extent" );
    }

    QString id() const override
    {
      return QStringLiteral( "extent" );
    }
};

/**
 * A table (matrix) parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('matrix')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeMatrix : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMatrix( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A table (matrix) parameter for processing algorithms." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Matrix" );
    }

    QString id() const override
    {
      return QStringLiteral( "matrix" );
    }
};

/**
 * An input file or folder parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('file')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeFile : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFile( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A file parameter, for use with non-map layer file sources." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "File" );
    }

    QString id() const override
    {
      return QStringLiteral( "file" );
    }
};

/**
 * A vector layer or feature source field parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('field')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeField : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterField( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector field parameter, for selecting an existing field from a vector source." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Field" );
    }

    QString id() const override
    {
      return QStringLiteral( "field" );
    }
};

/**
 * A vector layer destination parameter, for specifying the destination path for a vector layer
 * created by the algorithm.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('vectorDestination')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeVectorDestination : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorDestination( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector layer destination parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Destination" );
    }

    QString id() const override
    {
      return QStringLiteral( "vectorDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();

#if QT_VERSION >= 0x50700
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
#else
      flags &= ~ParameterFlag::ExposeToModeler;
#endif

      return flags;
    }
};

/**
 * A generic file based destination parameter, for specifying the destination path for a file (non-map layer)
 * created by the algorithm.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('fileDestination')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeFileDestination : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFileDestination( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A generic file based destination parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "File Destination" );
    }

    QString id() const override
    {
      return QStringLiteral( "fileDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();

#if QT_VERSION >= 0x50700
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
#else
      flags &= ~ParameterFlag::ExposeToModeler;
#endif

      return flags;
    }
};

/**
 * A folder destination parameter, for specifying the destination path for a folder created
 * by the algorithm or used for creating new files within the algorithm.
 * A folder output parameter.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('folderDestination')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeFolderDestination : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFolderDestination( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A folder destination parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Folder Destination" );
    }

    QString id() const override
    {
      return QStringLiteral( "folderDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();

#if QT_VERSION >= 0x50700
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
#else
      flags &= ~ParameterFlag::ExposeToModeler;
#endif

      return flags;
    }
};

/**
 * A raster layer destination parameter, for specifying the destination path for a raster layer
 * created by the algorithm.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('rasterDestination')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeRasterDestination : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRasterDestination( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A raster layer destination parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Destination" );
    }

    QString id() const override
    {
      return QStringLiteral( "rasterDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();

#if QT_VERSION >= 0x50700
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
#else
      flags &= ~ParameterFlag::ExposeToModeler;
#endif

      return flags;
    }
};

/**
 * A string parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('string')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeString : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterString( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A freeform string parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "String" );
    }

    QString id() const override
    {
      return QStringLiteral( "string" );
    }
};

/**
 * A parameter for processing algorithms which accepts multiple map layers.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('multilayer')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeMultipleLayers : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMultipleLayers( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An input allowing selection of multiple sources, including multiple map layers or file sources." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Multiple Input" );
    }

    QString id() const override
    {
      return QStringLiteral( "multilayer" );
    }
};

/**
 * An input feature source (such as vector layers) parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('source')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeFeatureSource : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFeatureSource( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector feature parameter, e.g. for algorithms which operate on the features within a layer." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Features" );
    }

    QString id() const override
    {
      return QStringLiteral( "source" );
    }
};

/**
 * A numeric parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('number')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeNumber : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterNumber( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter, including float or integer values." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Number" );
    }

    QString id() const override
    {
      return QStringLiteral( "number" );
    }
};

/**
 * A distance parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('distance')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeDistance : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDistance( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter representing a distance measure." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Distance" );
    }

    QString id() const override
    {
      return QStringLiteral( "distance" );
    }
};

/**
 * A raster band parameter for Processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('band')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeBand : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterBand( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A raster band parameter, for selecting an existing band from a raster source." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Band" );
    }

    QString id() const override
    {
      return QStringLiteral( "band" );
    }
};

#endif // QGSPROCESSINGPARAMETERTYPEIMPL_H
