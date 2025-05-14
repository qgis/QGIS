/***************************************************************************
                         qgsprocessingoutputs.h
                         -------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGOUTPUTS_H
#define QGSPROCESSINGOUTPUTS_H

#include <QColor>
#include "qgis_core.h"
#include "qgis.h"

class QgsProcessingContext;

//
// Output definitions
//

/**
 * \class QgsProcessingOutputDefinition
 * \ingroup core
 *
 * \brief Base class for the definition of processing outputs.
 *
 * Output definitions encapsulate the properties regarding the outputs from algorithms, such
 * as generated layers or calculated values.
 *
 */

class CORE_EXPORT QgsProcessingOutputDefinition
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type() == QgsProcessingOutputVectorLayer::typeName() )
      sipType = sipType_QgsProcessingOutputVectorLayer;
    else if ( sipCpp->type() == QgsProcessingOutputRasterLayer::typeName() )
      sipType = sipType_QgsProcessingOutputRasterLayer;
    else if ( sipCpp->type() == QgsProcessingOutputPointCloudLayer::typeName() )
      sipType = sipType_QgsProcessingOutputPointCloudLayer;
    else if ( sipCpp->type() == QgsProcessingOutputVectorTileLayer::typeName() )
      sipType = sipType_QgsProcessingOutputVectorTileLayer;
    else if ( sipCpp->type() == QgsProcessingOutputMapLayer::typeName() )
      sipType = sipType_QgsProcessingOutputMapLayer;
    else if ( sipCpp->type() == QgsProcessingOutputMultipleLayers::typeName() )
      sipType = sipType_QgsProcessingOutputMultipleLayers;
    else if ( sipCpp->type() == QgsProcessingOutputHtml::typeName() )
      sipType = sipType_QgsProcessingOutputHtml;
    else if ( sipCpp->type() == QgsProcessingOutputNumber::typeName() )
      sipType = sipType_QgsProcessingOutputNumber;
    else if ( sipCpp->type() == QgsProcessingOutputString::typeName() )
      sipType = sipType_QgsProcessingOutputString;
    else if ( sipCpp->type() == QgsProcessingOutputBoolean::typeName() )
      sipType = sipType_QgsProcessingOutputBoolean;
    else if ( sipCpp->type() == QgsProcessingOutputFolder::typeName() )
      sipType = sipType_QgsProcessingOutputFolder;
    else if ( sipCpp->type() == QgsProcessingOutputFile::typeName() )
      sipType = sipType_QgsProcessingOutputFile;
    else if ( sipCpp->type() == QgsProcessingOutputConditionalBranch::typeName() )
      sipType = sipType_QgsProcessingOutputConditionalBranch;
    else if ( sipCpp->type() == QgsProcessingOutputVariant::typeName() )
      sipType = sipType_QgsProcessingOutputVariant;
    else
      sipType = nullptr;
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsProcessingOutputDefinition.
     */
    QgsProcessingOutputDefinition( const QString &name, const QString &description = QString() );

    virtual ~QgsProcessingOutputDefinition() = default;

    /**
     * A fallback color to represent a processing output
     */
    virtual QColor getColor() const;

    /**
     * Unique output type name.
     */
    virtual QString type() const = 0;

    /**
     * Returns the name of the output. This is the internal identifier by which
     * algorithms access this output.
     * \see setName()
     */
    QString name() const { return mName; }

    /**
     * Sets the \a name of the output. This is the internal identifier by which
     * algorithms access this output.
     * \see name()
     */
    void setName( const QString &name ) { mName = name; }

    /**
     * Returns the description for the output. This is the user-visible string
     * used to identify this output.
     * \see setDescription()
     */
    QString description() const { return mDescription; }

    /**
     * Sets the \a description for the output. This is the user-visible string
     * used to identify this output.
     * \see description()
     */
    void setDescription( const QString &description ) { mDescription = description; }

    /**
     * Sets whether an output was automatically created when adding a parameter.
     * \param autoCreated set to TRUE if the output is to be considered as automatically created.
     * \see autoCreated()
     * \since QGIS 3.14
     */
    void setAutoCreated( bool autoCreated ) { mAutoCreated = autoCreated; }

    /**
     * Returns TRUE if the output was automatically created when adding a parameter.
     * \see setAutoCreated()
     * \since QGIS 3.14
     */
    bool autoCreated() const { return mAutoCreated; }

    /**
     * Returns a string version of the parameter output \a value (if possible).
     *
     * \param value value to convert
     * \param context processing context
     * \param ok will be set to TRUE if value could be represented as a string.
     * \returns value converted to string
     *
     * \see valueAsFormattedString()
     *
     * \since QGIS 3.36
     */
    virtual QString valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const;

    /**
     * Returns a HTML string version of the parameter output \a value (if possible).
     *
     * By default this will return the same value as valueAsString().
     *
     * \param value value to convert
     * \param context processing context
     * \param ok will be set to TRUE if value could be represented as a string.
     * \returns value converted to string
     *
     * \see valueAsString()
     *
     * \since QGIS 3.36
     */
    virtual QString valueAsFormattedString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const;

  protected:

    //! Output name
    QString mName;

    //! Output description
    QString mDescription;

    bool mAutoCreated = false;
};

//! List of processing parameters
typedef QList< const QgsProcessingOutputDefinition * > QgsProcessingOutputDefinitions;

/**
 * \class QgsProcessingOutputMapLayer
 * \ingroup core
 * \brief A map layer output for processing algorithms, where layers may be either vector or raster.
 *
 * If the actual layer output type is known (e.g. always vector or always raster), use
 * QgsProcessingOutputVectorLayer or QgsProcessingOutputRasterLayer instead.
 *
 */
class CORE_EXPORT QgsProcessingOutputMapLayer : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputMapLayer.
     */
    QgsProcessingOutputMapLayer( const QString &name, const QString &description = QString() );

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputLayer" ); }

    QString type() const override;

};

/**
 * \class QgsProcessingOutputVectorLayer
 * \ingroup core
 * \brief A vector layer output for processing algorithms.
 */
class CORE_EXPORT QgsProcessingOutputVectorLayer : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputVectorLayer.
     */
    QgsProcessingOutputVectorLayer( const QString &name, const QString &description = QString(), Qgis::ProcessingSourceType type = Qgis::ProcessingSourceType::VectorAnyGeometry );

    /**
     * A color to represent a vector layer ouput
     */
    QColor getColor() const override { return QColor( 0, 255, 0 ); /* green */ };

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputVector" ); }
    QString type() const override { return typeName(); }

    /**
     * Returns the layer type for the output layer.
     * \see setDataType()
     */
    Qgis::ProcessingSourceType dataType() const;

    /**
     * Sets the layer \a type for the output layer.
     * \see dataType()
     */
    void setDataType( Qgis::ProcessingSourceType type );

  private:

    Qgis::ProcessingSourceType mDataType = Qgis::ProcessingSourceType::VectorAnyGeometry;
};

/**
 * \class QgsProcessingOutputRasterLayer
 * \ingroup core
 * \brief A raster layer output for processing algorithms.
 */
class CORE_EXPORT QgsProcessingOutputRasterLayer : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputRasterLayer.
     */
    QgsProcessingOutputRasterLayer( const QString &name, const QString &description = QString() );

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputRaster" ); }
    QString type() const override { return typeName(); }

    /**
     * A color to represent a raster layer output
     */
    QColor getColor() const override { return QColor( 0, 180, 180 ); /* turquoise */ };
};

/**
 * \class QgsProcessingOutputPointCloudLayer
 * \ingroup core
 * \brief A pointcloud layer output for processing algorithms.
  * \since QGIS 3.24
 */
class CORE_EXPORT QgsProcessingOutputPointCloudLayer : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputPointCloudLayer.
     */
    QgsProcessingOutputPointCloudLayer( const QString &name, const QString &description = QString() );

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputPointCloud" ); }
    QString type() const override { return typeName(); }
};

/**
 * \class QgsProcessingOutputMultipleLayers
 * \ingroup core
 * \brief A multi-layer output for processing algorithms which create map layers, when
 * the number and nature of the output layers is not predefined.
 *
 * \note Always prefer to explicitly define QgsProcessingOutputVectorLayer,
 * QgsProcessingOutputRasterLayer or QgsProcessingOutputMapLayer where possible. QgsProcessingOutputMultipleLayers
 * should only ever be used when the number of output layers is not
 * fixed - e.g. as a result of processing all layers in a specified
 * folder.
 */
class CORE_EXPORT QgsProcessingOutputMultipleLayers : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputMultipleLayers.
     */
    QgsProcessingOutputMultipleLayers( const QString &name, const QString &description = QString() );

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputMultilayer" ); }
    QString type() const override;
    QString valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const override;

    /**
     * A color to represent a multiple layer output
     */
    QColor getColor() const override { return QColor( 237, 142, 0 ); /* orange */ };
};

/**
 * \class QgsProcessingOutputHtml
 * \ingroup core
 * \brief A HTML file output for processing algorithms.
 */
class CORE_EXPORT QgsProcessingOutputHtml : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputHtml.
     */
    QgsProcessingOutputHtml( const QString &name, const QString &description = QString() );

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputHtml" ); }
    QString type() const override { return typeName(); }
    QString valueAsFormattedString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const override;

    /**
     * A color to represent an HTML output
     */
    QColor getColor() const override { return QColor( 100, 100, 255 ); /* slate blueish */ };
};


/**
 * \class QgsProcessingOutputVariant
 * \ingroup core
 * \brief A variant output for processing algorithms, capable of storing any QVariant value.
  * \since QGIS 3.34
 */
class CORE_EXPORT QgsProcessingOutputVariant : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputVariant.
     */
    QgsProcessingOutputVariant( const QString &name, const QString &description = QString() );

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputVariant" ); }

    /**
     * A color to represent a range parameter
     */
    QColor getColor() const override { return QColor( 34, 157, 214 ); /* blue */ };

    QString type() const override;
    QString valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const override;

};

/**
 * \class QgsProcessingOutputNumber
 * \ingroup core
 * \brief A numeric output for processing algorithms.
 */
class CORE_EXPORT QgsProcessingOutputNumber : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputNumber.
     */
    QgsProcessingOutputNumber( const QString &name, const QString &description = QString() );

    /**
    * A color to represent a number output
    */
    QColor getColor() const override { return QColor( 34, 157, 214 ); /* blue */ };

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputNumber" ); }
    QString type() const override { return typeName(); }
    QString valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const override;

};

/**
 * \class QgsProcessingOutputString
 * \ingroup core
 * \brief A string output for processing algorithms.
 */
class CORE_EXPORT QgsProcessingOutputString : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputString.
     */
    QgsProcessingOutputString( const QString &name, const QString &description = QString() );

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputString" ); }
    QString type() const override { return typeName(); }

    /**
     * A color to represent a string output
     */
    QColor getColor() const override { return QColor( 100, 100, 255 ); /* slate blueish */ };
};

/**
 * \class QgsProcessingOutputBoolean
 * \ingroup core
 * \brief A boolean output for processing algorithms.
  * \since QGIS 3.8
 */
class CORE_EXPORT QgsProcessingOutputBoolean : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputNumber.
     */
    QgsProcessingOutputBoolean( const QString &name, const QString &description = QString() );

    /**
     * A color to represent a boolean output
     */
    QColor getColor() const override { return QColor( 51, 201, 28 ); /* green */ };


    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputBoolean" ); }
    QString type() const override { return typeName(); }
    QString valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const override;
};

/**
 * \class QgsProcessingOutputFolder
 * \ingroup core
 * \brief A folder output for processing algorithms.
 */
class CORE_EXPORT QgsProcessingOutputFolder : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputFolder.
     */

    QgsProcessingOutputFolder( const QString &name, const QString &description = QString() );

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputFolder" ); }
    QString type() const override { return typeName(); }
    QString valueAsFormattedString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const override;

    /**
     * A color to represent a folder output
     */
    QColor getColor() const override { return QColor( 80, 80, 80 ); /* dark gray */ };
};

/**
 * \class QgsProcessingOutputFile
 * \ingroup core
 * \brief A file output for processing algorithms.
 */
class CORE_EXPORT QgsProcessingOutputFile : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputFile.
     */
    QgsProcessingOutputFile( const QString &name, const QString &description = QString() );

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputFile" ); }
    QString type() const override { return typeName(); }
    QString valueAsFormattedString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const override;

    /**
     * A color to represent a file output
     */
    QColor getColor() const override { return QColor( 80, 80, 80 ); /* dark gray */ };
};

/**
 * \class QgsProcessingOutputConditionalBranch
 * \ingroup core
 * \brief A conditional branch output for processing algorithms, which represents a possible model logic
 * flow which branches out from this algorithm.
  * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingOutputConditionalBranch : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputConditionalBranch.
     */
    QgsProcessingOutputConditionalBranch( const QString &name, const QString &description = QString() );

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputBranch" ); }
    QString type() const override { return typeName(); }
};

/**
 * \class QgsProcessingOutputVectorTileLayer
 * \ingroup core
 * \brief A vector tile layer output for processing algorithms.
  * \since QGIS 3.32
 */
class CORE_EXPORT QgsProcessingOutputVectorTileLayer : public QgsProcessingOutputDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputVectorTileLayer.
     */
    QgsProcessingOutputVectorTileLayer( const QString &name, const QString &description = QString() );

    /**
     * Returns the type name for the output class.
     */
    static QString typeName() { return QStringLiteral( "outputVectorTile" ); }
    QString type() const override { return typeName(); }

    /**
     * A color to represent a vector tile layer output
     */
    QColor getColor() const override { return QColor( 180, 180, 0 ); /* greenish yellow */ };
};

#endif // QGSPROCESSINGOUTPUTS_H
