/***************************************************************************
                            qgschartregistry.h
                            ------------------------
    begin                : June 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCHARTREGISTRY_H
#define QGSCHARTREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsplot.h"

/**
 * \ingroup core
 * \brief Stores metadata about a chart class.
 *
 * \note In C++ you can use QgsChartAbstractMetadata convenience class.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsChartAbstractMetadata
{
  public:

    /**
     * Constructor for QgsChartAbstractMetadata with the specified class \a type.
     */
    QgsChartAbstractMetadata( const QString &type, const QString &visibleName )
      : mType( type )
      , mVisibleName( visibleName )
    {}

    virtual ~QgsChartAbstractMetadata() = default;

    /**
     * Returns the unique type code for the chart class.
     */
    QString type() const { return mType; }

    /**
     * Returns a translated, user visible name for the chart class.
     */
    QString visibleName() const { return mVisibleName; }

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotations here, that's not
     * the case.
     * As per Phil Thomson's advice on https://www.riverbankcomputing.com/pipermail/pyqt/2017-July/039450.html:
     *
     * "
     * /Factory/ is used when the instance returned is guaranteed to be new to Python.
     * In this case it isn't because it has already been seen when being returned by QgsProcessingAlgorithm::createInstance()
     * (However for a different sub-class implemented in C++ then it would be the first time it was seen
     * by Python so the /Factory/ on create() would be correct.)
     *
     * You might try using /TransferBack/ on create() instead - that might be the best compromise.
     * "
     */

    /**
     * Creates a chart of this class.
     */
    virtual QgsPlot *createChart() = 0 SIP_TRANSFERBACK;

  private:

    QString mType;
    QString mVisibleName;
};

//! Chart creation function
typedef std::function<QgsPlot *()> QgsChartCreateFunc SIP_SKIP;

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief Convenience metadata class that uses static functions to create charts and their configuration widgets.
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsChartMetadata : public QgsChartAbstractMetadata
{
  public:

    /**
     * Constructor for QgsChartMetadata with the specified class \a type.
     */
    QgsChartMetadata( const QString &type, const QString &visibleName,
                      const QgsChartCreateFunc &pfCreate )
      : QgsChartAbstractMetadata( type, visibleName )
      , mCreateFunc( pfCreate )
    {}

    /**
     * Returns the classes' chart creation function.
     */
    QgsChartCreateFunc createFunction() const { return mCreateFunc; }

    QgsPlot *createChart() override { return mCreateFunc ? mCreateFunc() : nullptr; }

  protected:
    QgsChartCreateFunc mCreateFunc = nullptr;

};

#endif

/**
 * \ingroup core
 * \class QgsChartRegistry
 * \brief Registry of available chart types.
 *
 * QgsChartRegistry is not usually directly created, but rather accessed through
 * QgsApplication::chartRegistry().
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsChartRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Creates a new empty item registry.
     *
     * QgsChartRegistry is not usually directly created, but rather accessed through
     * QgsApplication::chartRegistry().
     *
     * \see populate()
    */
    QgsChartRegistry( QObject *parent = nullptr );
    ~QgsChartRegistry() override;

    /**
     * Populates the registry with standard chart types. If called on a non-empty registry
     * then this will have no effect and will return FALSE.
     */
    bool populate();

    QgsChartRegistry( const QgsChartRegistry &rh ) = delete;
    QgsChartRegistry &operator=( const QgsChartRegistry &rh ) = delete;

    /**
     * Returns the metadata for the specified chart \a type. Returns NULLPTR if
     * a corresponding type was not found in the registry.
     */
    QgsChartAbstractMetadata *chartMetadata( const QString &type ) const;

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotations here, that's not
     * the case.
     * As per Phil Thomson's advice on https://www.riverbankcomputing.com/pipermail/pyqt/2017-July/039450.html:
     *
     * "
     * /Factory/ is used when the instance returned is guaranteed to be new to Python.
     * In this case it isn't because it has already been seen when being returned by QgsProcessingAlgorithm::createInstance()
     * (However for a different sub-class implemented in C++ then it would be the first time it was seen
     * by Python so the /Factory/ on create() would be correct.)
     *
     * You might try using /TransferBack/ on create() instead - that might be the best compromise.
     * "
     */

    /**
     * Registers a new chart type.
     * \note Takes ownership of the metadata instance.
     */
    bool addChartType( QgsChartAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Removes a new a chart type from the registry.
     */
    bool removeChartType( const QString &type );

    /**
     * Creates a new instance of a chart  given the \a type.
     */
    QgsPlot *createChart( const QString &type ) const SIP_TRANSFERBACK;

    /**
     * Returns a map of available charts types to translated name.
     */
    QMap<QString, QString> chartTypes() const;

  signals:

    /**
     * Emitted whenever a new chart type is added to the registry, with the specified
     * \a type and visible \a name.
     */
    void chartAdded( const QString &type, const QString &name );

    /**
     * Emitted whenever a new chart type is added to the registry, with the specified
     * \a type and visible \a name.
     */
    void chartAboutToBeRemoved( const QString &type );

  private:

#ifdef SIP_RUN
    QgsChartRegistry( const QgsChartRegistry &rh );
#endif

    QMap<QString, QgsChartAbstractMetadata *> mMetadata;

};

#endif //QGSCHARTREGISTRY_H



