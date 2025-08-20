/***************************************************************************
                            qgsplotregistry.h
                            -----------------
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
#ifndef QGSPLOTREGISTRY_H
#define QGSPLOTREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsplot.h"

/**
 * \ingroup core
 * \class QgsPlotAbstractMetadata
 * \brief Stores metadata about a plot class.
 *
 * \note In C++ you can use QgsPlotAbstractMetadata convenience class.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsPlotAbstractMetadata
{
  public:

    /**
     * Constructor for QgsPlotAbstractMetadata with the specified class \a type.
     */
    QgsPlotAbstractMetadata( const QString &type, const QString &visibleName )
      : mType( type )
      , mVisibleName( visibleName )
    {}

    virtual ~QgsPlotAbstractMetadata() = default;

    /**
     * Returns the unique type code for the plot class.
     */
    QString type() const { return mType; }

    /**
     * Returns a translated, user visible name for the plot class.
     */
    QString visibleName() const { return mVisibleName; }

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotations here, that's not
     * the case.
     * As per Phil Thomson's advice on https://www.riverbankcomputing.com/pipermail/pyqt/2017-July/039450.html:
     *
     * "
     * /Factory/ is used when the instance returned is guaranteed to be new to Python.
     * In this case it isn't because it has already been seen when being returned by createChart()
     * (However for a different sub-class implemented in C++ then it would be the first time it was seen
     * by Python so the /Factory/ on create() would be correct.)
     *
     * You might try using /TransferBack/ on create() instead - that might be the best compromise.
     * "
     */

    /**
     * Creates a plot of this class.
     */
    virtual QgsPlot *createPlot() = 0 SIP_TRANSFERBACK;

  private:

    QString mType;
    QString mVisibleName;
};

//! Plot creation function
typedef std::function<QgsPlot *()> QgsPlotCreateFunc SIP_SKIP;

#ifndef SIP_RUN

/**
 * \ingroup core
 * \class QgsPlotMetadata
 * \brief Convenience metadata class that uses static functions to create plots.
 * \note not available in Python bindings
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsPlotMetadata : public QgsPlotAbstractMetadata
{
  public:

    /**
     * Constructor for QgsPlotMetadata with the specified class \a type.
     */
    QgsPlotMetadata( const QString &type, const QString &visibleName,
                     const QgsPlotCreateFunc &pfCreate )
      : QgsPlotAbstractMetadata( type, visibleName )
      , mCreateFunc( pfCreate )
    {}

    /**
     * Returns the classes' plot creation function.
     */
    QgsPlotCreateFunc createFunction() const { return mCreateFunc; }

    QgsPlot *createPlot() override { return mCreateFunc ? mCreateFunc() : nullptr; }

  protected:
    QgsPlotCreateFunc mCreateFunc = nullptr;

};

#endif

/**
 * \ingroup core
 * \class QgsPlotRegistry
 * \brief Registry of available plot types.
 *
 * QgsPlotRegistry is not usually directly created, but rather accessed through
 * QgsApplication::plotRegistry().
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsPlotRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Creates a new empty plot registry.
     *
     * QgsPlotRegistry is not usually directly created, but rather accessed through
     * QgsApplication::plotRegistry().
     *
     * \see populate()
    */
    QgsPlotRegistry( QObject *parent = nullptr );
    ~QgsPlotRegistry() override;

    /**
     * Populates the registry with standard plot types. If called on a non-empty registry
     * then this will have no effect and will return FALSE.
     */
    bool populate();

    QgsPlotRegistry( const QgsPlotRegistry &rh ) = delete;
    QgsPlotRegistry &operator=( const QgsPlotRegistry &rh ) = delete;

    /**
     * Returns the metadata for the specified plot \a type. Returns NULLPTR if
     * a corresponding type was not found in the registry.
     */
    QgsPlotAbstractMetadata *plotMetadata( const QString &type ) const;

    /**
     * Registers a new plot type.
     * \note Takes ownership of the metadata instance.
     */
    bool addPlotType( QgsPlotAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Removes a new a plot type from the registry.
     */
    bool removePlotType( const QString &type );

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotations here, that's not
     * the case.
     * As per Phil Thomson's advice on https://www.riverbankcomputing.com/pipermail/pyqt/2017-July/039450.html:
     *
     * "
     * /Factory/ is used when the instance returned is guaranteed to be new to Python.
     * In this case it isn't because it has already been seen when being returned by createChart()
     * (However for a different sub-class implemented in C++ then it would be the first time it was seen
     * by Python so the /Factory/ on create() would be correct.)
     *
     * You might try using /TransferBack/ on create() instead - that might be the best compromise.
     * "
     */

    /**
     * Creates a new instance of a plot given the \a type.
     */
    QgsPlot *createPlot( const QString &type ) const SIP_TRANSFERBACK;

    /**
     * Returns a map of available plot types to translated name.
     */
    QMap<QString, QString> plotTypes() const;

  signals:

    /**
     * Emitted whenever a new plot type is added to the registry, with the specified
     * \a type and visible \a name.
     */
    void plotAdded( const QString &type, const QString &name );

    /**
     * Emitted whenever a plot type is about to be remove from the registry, with the specified
     * \a type and visible \a name.
     */
    void plotAboutToBeRemoved( const QString &type );

  private:

#ifdef SIP_RUN
    QgsPlotRegistry( const QgsPlotRegistry &rh );
#endif

    QMap<QString, QgsPlotAbstractMetadata *> mMetadata;

};

#endif //QGSPLOTREGISTRY_H



