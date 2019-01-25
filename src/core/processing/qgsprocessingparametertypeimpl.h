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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterRasterLayer" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterRasterLayer" );
    }

    QString id() const override
    {
      return QStringLiteral( "raster" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsRasterLayer" );
    }
};

/**
 * A mesh layer parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('mesh')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeMeshLayer : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMeshLayer( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A mesh layer parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Mesh Layer" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMeshLayer" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMeshLayer" );
    }

    QString id() const override
    {
      return QStringLiteral( "mesh" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsMeshLayer" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterVectorLayer" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterVectorLayer" );
    }

    QString id() const override
    {
      return QStringLiteral( "vector" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsVectorLayer" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMapLayer" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMapLayer" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsMapLayer" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsRasterLayer" )
             << QStringLiteral( "QgsVectorLayer" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterBoolean" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterBoolean" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "bool" )
             << QStringLiteral( "int" )
             << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterExpression" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterExpression" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterCrs" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterCrs" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList()
             << QStringLiteral( "str: 'ProjectCrs'" )
             << QObject::tr( "str: CRS auth ID (e.g. 'EPSG:3111')" )
             << QObject::tr( "str: CRS PROJ4 (e.g. 'PROJ4:…')" )
             << QObject::tr( "str: CRS WKT (e.g. 'WKT:…')" )
             << QObject::tr( "str: layer ID. CRS of layer is used." )
             << QObject::tr( "str: layer name. CRS of layer is used." )
             << QObject::tr( "str: layer source. CRS of layer is used." )
             << QObject::tr( "QgsCoordinateReferenceSystem" )
             << QObject::tr( "QgsMapLayer: CRS of layer is used" )
             << QObject::tr( "QgsProcessingFeatureSourceDefinition: CRS of source is used" )
             << QStringLiteral( "QgsProperty" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterRange" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterRange" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[float]: list of 2 float values" )
             << QObject::tr( "list[str]: list of strings representing floats" )
             << QObject::tr( "str: as two comma delimited floats, e.g. '1,10'" )
             << QStringLiteral( "QgsProperty" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterPoint" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterPoint" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as an 'x,y' string, e.g. '1.5,10.1'" )
             << QStringLiteral( "QgsPointXY" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsReferencedPointXY" );
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
      return QCoreApplication::translate( "Processing", "An enumerated type parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Enum" );
    }

    QString id() const override
    {
      return QStringLiteral( "enum" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterEnum" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterEnum" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QObject::tr( "str: as string representation of int, e.g. '1'" )
             << QStringLiteral( "QgsProperty" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterExtent" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterExtent" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as comma delimited list of x min, x max, y min, y max. E.g. '4,10,101,105'" )
             << QObject::tr( "str: layer ID. Extent of layer is used." )
             << QObject::tr( "str: layer name. Extent of layer is used." )
             << QObject::tr( "str: layer source. Extent of layer is used." )
             << QObject::tr( "QgsMapLayer: Extent of layer is used" )
             << QObject::tr( "QgsProcessingFeatureSourceDefinition: Extent of source is used" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsRectangle" )
             << QStringLiteral( "QgsReferencedRectangle" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMatrix" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMatrix" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: as comma delimited list of values" )
             << QStringLiteral( "list" )
             << QStringLiteral( "QgsProperty" );
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
      return QCoreApplication::translate( "Processing", "A file or folder parameter, for use with non-map layer file sources or folders." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "File/Folder" );
    }

    QString id() const override
    {
      return QStringLiteral( "file" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFile" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFile" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterField" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterField" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterVectorDestination" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterVectorDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsProcessingOutputLayerDefinition" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFileDestination" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFileDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFolderDestination" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFolderDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterRasterDestination" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterRasterDestination" );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
      return flags;
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsProcessingOutputLayerDefinition" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterString" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterString" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" )
             << QStringLiteral( "QgsProperty" );
    }
};

/**
 * A authentication configuration parameter for processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('authcfg')
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsProcessingParameterTypeAuthConfig : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterAuthConfig( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A authentication configuration parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Authentication Configuration" );
    }

    QString id() const override
    {
      return QStringLiteral( "authcfg" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterAuthConfig" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterAuthConfig" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "str" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMultipleLayers" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMultipleLayers" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[str]: list of layer IDs" )
             << QObject::tr( "list[str]: list of layer names" )
             << QObject::tr( "list[str]: list of layer sources" )
             << QStringLiteral( "list[QgsMapLayer]" )
             << QStringLiteral( "QgsProperty" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFeatureSource" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFeatureSource" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: layer ID" )
             << QObject::tr( "str: layer name" )
             << QObject::tr( "str: layer source" )
             << QStringLiteral( "QgsProcessingFeatureSourceDefinition" )
             << QStringLiteral( "QgsProperty" )
             << QStringLiteral( "QgsVectorLayer" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterNumber" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterNumber" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "float" )
             << QStringLiteral( "QgsProperty" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterDistance" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterDistance" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "float" )
             << QStringLiteral( "QgsProperty" );
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

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterBand" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterBand" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QStringLiteral( "int" )
             << QStringLiteral( "QgsProperty" );
    }

};

/**
 * A feature sink parameter for Processing algorithms.
 *
 * \ingroup core
 * \note No Python bindings available. Get your copy from QgsApplication.processingRegistry().parameterType('band')
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterTypeFeatureSink : public QgsProcessingParameterType
{
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFeatureSink( name );
    }

    ParameterFlags flags() const override
    {
      ParameterFlags flags = QgsProcessingParameterType::flags();
      flags.setFlag( ParameterFlag::ExposeToModeler, false );
      return flags;
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A feature sink destination parameter." );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Feature Sink" );
    }

    QString id() const override
    {
      return QStringLiteral( "sink" );
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterFeatureSink" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterFeatureSink" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "str: destination vector file, e.g. 'd:/test.shp'" )
             << QObject::tr( "str: 'memory:' to store result in temporary memory layer" )
             << QObject::tr( "str: using vector provider ID prefix and destination URI, e.g. 'postgres:…' to store result in PostGIS table" )
             << QStringLiteral( "QgsProcessingOutputLayerDefinition" )
             << QStringLiteral( "QgsProperty" );
    }

};

#endif // QGSPROCESSINGPARAMETERTYPEIMPL_H
