#ifndef QGSPROCESSINGPARAMETERTYPEIMPL_H
#define QGSPROCESSINGPARAMETERTYPEIMPL_H

#include "qgis.h"
#include "qgis_sip.h"
#include "qgsprocessingparametertype.h"
#include <QCoreApplication>

#define SIP_NO_FILE

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
      return QStringLiteral( "Raster Layer" );
    }
};

class CORE_EXPORT QgsProcessingParameterTypeVectorLayer : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorLayer( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A vector layer parameter." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Layer" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "Vector Layer" );
    }
};

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
      return QStringLiteral( "Boolean" );
    }
};

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
      return QStringLiteral( "CRS" );
    }
};

class CORE_EXPORT QgsProcessingParameterTypeRange : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRange( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "TODO." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Range" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "Range" );
    }
};

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
      return QStringLiteral( "Point" );
    }
};

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
      return QStringLiteral( "Enum" );
    }
};

class CORE_EXPORT QgsProcessingParameterTypeExtent : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterExtent( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An extent defines a rectangle." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Extent" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "Extent" );
    }
};

class CORE_EXPORT QgsProcessingParameterTypeMatrix : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMatrix( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "TODO." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Matrix" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "Matrix" );
    }
};

class CORE_EXPORT QgsProcessingParameterTypeFile : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFile( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "A file input parameter." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Extent" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "Extent" );
    }
};

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
      return QCoreApplication::translate( "Processing", "Field" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "Field" );
    }
};

class CORE_EXPORT QgsProcessingParameterTypeVectorDestination : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterVectorDestination( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "TODO." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Vector Destination" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "Vector Destination" );
    }
};

class CORE_EXPORT QgsProcessingParameterTypeFileDestination : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFileDestination( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "TODO." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "File Destination" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "File Destination" );
    }
};

class CORE_EXPORT QgsProcessingParameterTypeFolderDestination : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFolderDestination( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "TODO." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Folder Destination" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "Folder Destination" );
    }
};

class CORE_EXPORT QgsProcessingParameterTypeRasterDestination : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterRasterDestination( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "TODO." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Raster Destination" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "Raster Destination" );
    }
};

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
      return QStringLiteral( "String" );
    }
};

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
      return QCoreApplication::translate( "Processing", "Multiple Layers" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "Multiple Input" );
    }
};

class CORE_EXPORT QgsProcessingParameterTypeFeatureSource : public QgsProcessingParameterType
{
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterFeatureSource( name );
    }

    virtual QString description() const override
    {
      return QCoreApplication::translate( "Processing", "TODO." );
    }

    virtual QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Feature Source" );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "Vector Features" );
    }
};

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
      return QStringLiteral( "Number" );
    }
};


#endif // QGSPROCESSINGPARAMETERTYPEIMPL_H
