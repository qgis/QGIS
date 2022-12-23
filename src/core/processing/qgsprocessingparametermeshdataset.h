/***************************************************************************
  qgsprocessingparametermeshdataset.h
  ---------------------
  Date                 : October 2020
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

#ifndef QGSPROCESSINGPARAMETERMESHDATASET_H
#define QGSPROCESSINGPARAMETERMESHDATASET_H

#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertype.h"
#include "qgsmeshdataset.h"

/**
 * \brief A parameter for processing algorithms that need a list of mesh dataset groups.
 *
 * A valid value for this parameter is a list (QVariantList) of dataset groups index in the mesh layer scope
 * Dataset group index can be evaluated with the method valueAsDatasetGroup()
 *
 * \note This parameter is dependent on a mesh layer parameter (see QgsProcessingParameterMeshLayer)
 *
 * \ingroup core
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsProcessingParameterMeshDatasetGroups : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor
     * \param name name of the parameter
     * \param description description of the parameter
     * \param meshLayerParameterName name of the associated mesh layer parameter
     * \param supportedDataType a set of QgsMeshDatasetGroupMetadata::DataType values for data types supported by the parameter
     * \param optional whether the parameter is optional
     */
    QgsProcessingParameterMeshDatasetGroups( const QString &name,
        const QString &description = QString(),
        const QString &meshLayerParameterName = QString(),
        QSet<int> supportedDataType = QSet<int>(),
        bool optional = false );

    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QStringList dependsOnOtherParameters() const override;
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    //! Returns the type name for the parameter class.
    static QString typeName() { return QStringLiteral( "meshdatasetgroups" ); }

    //! Returns the name of the mesh layer parameter
    QString meshLayerParameterName() const;

    //! Returns whether the data type is supported by the parameter
    bool isDataTypeSupported( QgsMeshDatasetGroupMetadata::DataType dataType ) const;

    //! Returns the \a value as a list if dataset group indexes
    static QList<int> valueAsDatasetGroup( const QVariant &value );

  private:
    QString mMeshLayerParameterName;
    QSet<int> mSupportedDataType;

    static bool valueIsAcceptable( const QVariant &input, bool allowEmpty );
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \brief Parameter type definition for QgsProcessingParameterMeshDatasetGroups.
 *
 * \ingroup core
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsProcessingParameterTypeMeshDatasetGroups : public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMeshDatasetGroups( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An input allowing selection dataset groups from a mesh layer" );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Mesh Dataset Groups" );
    }

    QString id() const override
    {
      return QgsProcessingParameterMeshDatasetGroups::typeName();
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMeshDatasetGroups" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMeshDatasetGroups" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "list[int]: list of dataset group indexes, see QgsProcessingParameterMeshDatasetGroups docs" );
    }
};

///@endcond
#endif //SIP_RUN

/**
 * \brief A parameter for processing algorithms that need a list of mesh dataset index from time parameter.
 *
 * A valid value for this parameter is a map (QVariantMap) with in this form:
 *
 * - "type" : the type of time settings "current-context-time", "defined-date-time", "dataset-time-step" or "none" if all the dataset groups are static
 * - "value" : nothing if type is "static" or "current-context-time", QDateTime if "defined-date-time" or, for "dataset_time_step",  list of two int representing the dataset index that is the reference for the time step
 *
 * \note This parameter is dependent on a mesh layer parameter (\see QgsProcessingParameterMeshLayer)
 * and on mesh datast group parameter (\see QgsProcessingParameterMeshDatasetGroups)
 *
 * \ingroup core
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsProcessingParameterMeshDatasetTime : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor
     * \param name name of the parameter
     * \param description description of the parameter
     * \param meshLayerParameterName name of the associated mesh layer parameter (\see QgsProcessingParameterMeshLayer)
     * \param datasetGroupParameterName name of the associated dataset group parameter (\see QgsProcessingParameterMeshDatasetGroups)
     */
    QgsProcessingParameterMeshDatasetTime(
      const QString &name,
      const QString &description = QString(),
      const QString &meshLayerParameterName = QString(),
      const QString &datasetGroupParameterName = QString() );

    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QStringList dependsOnOtherParameters() const override;
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    //! Returns the type name for the parameter class.
    static QString typeName() { return QStringLiteral( "meshdatasettime" ); }

    //! Returns the name of the mesh layer parameter
    QString meshLayerParameterName() const;

    //! Returns the name of the dataset groups parameter
    QString datasetGroupParameterName() const;

    /**
     * Returns the \a dataset value time type as a string :
     * current-context-time : the time is store in the processing context (e.g. current canvas time), in this case the value does not contain any time value
     * defined-date-time : absolute time of type QDateTime
     * dataset-time-step : a time step of existing dataset, in this case the time takes the form of a QMeshDatasetIndex with value to the corresponding dataset index
     * static : dataset groups are all static, in this case the value does not contain any time value
     */
    static QString valueAsTimeType( const QVariant &value );

    /**
     * Returns the \a value as a QgsMeshDatasetIndex if the value has "dataset-time-step" type.
     * If the value has the wrong type return an invalid dataset index
     *
     * \see valueAsTimeType()
     */
    static QgsMeshDatasetIndex timeValueAsDatasetIndex( const QVariant &value );

    /**
     * Returns the \a value as a QDateTime if the value has "defined-date-time" type.
     * If the value has the wrong type return an invalid QDatetime
     *
     * \see valueAsTimeType()
     */
    static QDateTime timeValueAsDefinedDateTime( const QVariant &value );

  private:
    QString mMeshLayerParameterName;
    QString mDatasetGroupParameterName;

    static bool valueIsAcceptable( const QVariant &input, bool allowEmpty );
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \brief Parameter type definition for QgsProcessingParameterMeshDatasetTime.
 *
 * \ingroup core
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsProcessingParameterTypeMeshDatasetTime: public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY
    {
      return new QgsProcessingParameterMeshDatasetTime( name );
    }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An input allowing selection of dataset index from a mesh layer by time setting" );
    }

    QString name() const override
    {
      return QCoreApplication::translate( "Processing", "Mesh Dataset Time" );
    }

    QString id() const override
    {
      return QgsProcessingParameterMeshDatasetTime::typeName();
    }

    QString pythonImportString() const override
    {
      return QStringLiteral( "from qgis.core import QgsProcessingParameterMeshDatasetTime" );
    }

    QString className() const override
    {
      return QStringLiteral( "QgsProcessingParameterMeshDatasetTime" );
    }

    QStringList acceptedPythonTypes() const override
    {
      return QStringList() << QObject::tr( "dict{}: dictionary, see QgsProcessingParameterMeshDatasetTime docs" );
    }
};

///@endcond
#endif //SIP_RUN


#endif // QGSPROCESSINGPARAMETERMESHDATASET_H
