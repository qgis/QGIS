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
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(raster)
 */
class CORE_EXPORT QgsProcessingParameterTypeRasterLayer : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRasterLayer( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A raster layer parameter." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Layer" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "raster" );
    }
};

/**
 * A vector layer parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(vector)
 */
class CORE_EXPORT QgsProcessingParameterTypeVectorLayer : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorLayer( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector layer parameter, e.g. for algorithms which change layer styles, edit layers in place, or other operations which affect an entire layer." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Layer" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "vector" );
    }
};

/**
 * A generic map layer parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(maplayer)
 */
class CORE_EXPORT QgsProcessingParameterTypeMapLayer : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMapLayer( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A generic map layer parameter, which accepts either vector or raster layers." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Map Layer" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "maplayer" );
    }
};

/**
 * A boolean parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(boolean)
 */
class CORE_EXPORT QgsProcessingParameterTypeBoolean : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterBoolean( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A boolean parameter, for true/false values." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Boolean" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "boolean" );
    }
};

/**
 * An expression parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(expression)
 */
class CORE_EXPORT QgsProcessingParameterTypeExpression : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterExpression( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An expression parameter, to add custom expressions based on layer fields." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Expression" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "expression" );
    }
};

/**
 * A crs parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(crs)
 */
class CORE_EXPORT QgsProcessingParameterTypeCrs : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterCrs( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A coordinate reference system (CRS) input parameter." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "CRS" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "crs" );
    }
};

/**
 * A numeric range parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(range)
 */
class CORE_EXPORT QgsProcessingParameterTypeRange : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRange( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric range parameter for processing algorithms." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Range" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "range" );
    }
};

/**
 * A point parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(point)
 */
class CORE_EXPORT QgsProcessingParameterTypePoint : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterPoint( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A geographic point parameter." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Point" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "point" );
    }
};

/**
 * An enum based parameter for processing algorithms, allowing for selection from predefined values.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(enum)
 */
class CORE_EXPORT QgsProcessingParameterTypeEnum : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterEnum( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "TODO." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Enum" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "enum" );
    }
};

/**
 * A rectangular map extent parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(extent)
 */
class CORE_EXPORT QgsProcessingParameterTypeExtent : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterExtent( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A map extent parameter." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Extent" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "extent" );
    }
};

/**
 * A table (matrix) parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(matrix)
 */
class CORE_EXPORT QgsProcessingParameterTypeMatrix : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMatrix( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A table (matrix) parameter for processing algorithms." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Matrix" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "matrix" );
    }
};

/**
 * An input file or folder parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(file)
 */
class CORE_EXPORT QgsProcessingParameterTypeFile : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFile( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A file parameter, for use with non-map layer file sources." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "File" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "file" );
    }
};

/**
 * A vector layer or feature source field parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(field)
 */
class CORE_EXPORT QgsProcessingParameterTypeField : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterField( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector field parameter, for selecting an existing field from a vector source." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Field" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "field" );
    }
};

/**
 * A vector layer destination parameter, for specifying the destination path for a vector layer
 * created by the algorithm.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(vectorDestination)
 */
class CORE_EXPORT QgsProcessingParameterTypeVectorDestination : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorDestination( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector layer destination parameter." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Destination" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "vectorDestination" );
    }

    virtual ParameterFlags flags() const override
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
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(fileDestination)
 */
class CORE_EXPORT QgsProcessingParameterTypeFileDestination : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFileDestination( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A generic file based destination parameter." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "File Destination" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "fileDestination" );
    }

    virtual ParameterFlags flags() const override
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
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(folderDestination)
 */
class CORE_EXPORT QgsProcessingParameterTypeFolderDestination : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFolderDestination( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A folder destination parameter." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Folder Destination" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "folderDestination" );
    }

    virtual ParameterFlags flags() const override
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
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(rasterDestination)
 */
class CORE_EXPORT QgsProcessingParameterTypeRasterDestination : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRasterDestination( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A raster layer destination parameter." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Destination" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "rasterDestination" );
    }

    virtual ParameterFlags flags() const override
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
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(string)
 */
class CORE_EXPORT QgsProcessingParameterTypeString : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterString( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A freeform string parameter." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "String" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "string" );
    }
};

/**
 * A parameter for processing algorithms which accepts multiple map layers.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(multilayer)
 */
class CORE_EXPORT QgsProcessingParameterTypeMultipleLayers : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMultipleLayers( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An input allowing selection of multiple sources, including multiple map layers or file sources." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Multiple Input" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "multilayer" );
    }
};

/**
 * An input feature source (such as vector layers) parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(source)
 */
class CORE_EXPORT QgsProcessingParameterTypeFeatureSource : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFeatureSource( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector feature parameter, e.g. for algorithms which operate on the features within a layer." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Features" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "source" );
    }
};

/**
 * A numeric parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(number)
 */
class CORE_EXPORT QgsProcessingParameterTypeNumber : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterNumber( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter, including float or integer values." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Number" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "number" );
    }
};

/**
 * A distance parameter for processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(distance)
 */
class CORE_EXPORT QgsProcessingParameterTypeDistance : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterDistance( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A numeric parameter representing a distance measure." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Distance" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "distance" );
    }
};

/**
 * A raster band parameter for Processing algorithms.
 *
 * \since QGIS 3.2
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType(band)
 */
class CORE_EXPORT QgsProcessingParameterTypeBand : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterBand( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A raster band parameter, for selecting an existing band from a raster source." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Band" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "band" );
    }
};

#endif // QGSPROCESSINGPARAMETERTYPEIMPL_H
