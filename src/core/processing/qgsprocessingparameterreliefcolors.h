/***************************************************************************
  qgsprocessingparameterreliefcolors.h
  ---------------------
  Date                 : June 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERRELIEFCOLORS_H
#define QGSPROCESSINGPARAMETERRELIEFCOLORS_H

#include "qgsprocessingoutputs.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertype.h"
#include "qgsrasterlayerutils.h"

#include <QString>

using namespace Qt::StringLiterals;


/**
 * \brief A parameter for raster relief colors.
 *
 * \ingroup core
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsProcessingParameterReliefColors : public QgsProcessingParameterString
{
  public:
    //! Constructor for QgsProcessingParameterReliefColors.
    QgsProcessingParameterReliefColors( const QString &name, const QString &description = QString(), const QString &parentLayerParameter = QString(), bool optional = true );

    QgsProcessingParameterReliefColors *clone() const override;
    QString type() const override;
    //! Returns the type name for the parameter class.
    static QString typeName() { return u"reliefcolors"_s; } // cppcheck-suppress duplInheritedMember
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass ) const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Returns the linked parent layer parameter name.
     */
    QString parentLayerParameter() const { return mParentLayer; }

    /**
     * Returns the parameter value interpreted as a list of raster relief colors.
     */
    QList< QgsRasterReliefColor> valueAsReliefColors( const QVariant &value, const QgsProcessingContext &context ) const;

    /**
     * Returns a list of raster relief colors encoded as a variant value.
     */
    static QVariant colorsAsVariant( const QList< QgsRasterReliefColor> &colors );

  private:
    QString mParentLayer;
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \brief Parameter type definition for QgsProcessingParameterReliefColors.
 *
 * \ingroup core
 * \note This class is not a part of public API.
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsProcessingParameterTypeReliefColors : public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY { return new QgsProcessingParameterReliefColors( name ); }

    QString description() const override { return QCoreApplication::translate( "Processing", "An input allowing setting the output colors for relief algorithms." ); }

    QString name() const override { return QCoreApplication::translate( "Processing", "Relief colors" ); }

    QString id() const override { return QgsProcessingParameterReliefColors::typeName(); }

    QString pythonImportString() const override { return u"from qgis.core import QgsProcessingParameterReliefColors"_s; }

    QString className() const override { return u"QgsProcessingParameterReliefColors"_s; }

    QStringList acceptedPythonTypes() const override { return QStringList() << u"str"_s; }

    QStringList acceptedStringValues() const override
    {
      return QStringList() << QObject::tr( "Concatenated string value with format of 'min_elevation,max_elevation,red,green,blue;' for each color range" );
    }

    QStringList acceptedParameterTypes() const override { return QStringList() << QgsProcessingParameterString::typeName() << QgsProcessingParameterReliefColors::typeName(); }

    QStringList acceptedOutputTypes() const override { return QStringList() << QgsProcessingOutputVariant::typeName() << QgsProcessingOutputString::typeName(); }

    QColor modelColor() const override { return QColor( 255, 131, 23 ); /* orange */ };
};

///@endcond
#endif

#endif // QGSPROCESSINGPARAMETERRELIEFCOLORS_H
