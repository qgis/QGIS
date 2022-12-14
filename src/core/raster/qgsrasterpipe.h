/***************************************************************************
    qgsrasterpipe.h - Internal raster processing modules interface
     --------------------------------------
    Date                 : Jun 21, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERPIPE_H
#define QGSRASTERPIPE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgspropertycollection.h"

#include <QImage>
#include <QMap>
#include <QObject>

class QgsRasterInterface;
class QgsRasterRenderer;
class QgsRasterResampleFilter;
class QgsBrightnessContrastFilter;
class QgsHueSaturationFilter;
class QgsRasterProjector;
class QgsRasterNuller;
class QgsRasterResampleFilter;
class QgsContrastEnhancement;
class QgsRasterDataProvider;

#if defined(Q_OS_WIN)
#undef interface
#endif

/**
 * \ingroup core
 * \brief Contains a pipeline of raster interfaces for sequential raster processing.
 */
class CORE_EXPORT QgsRasterPipe
{
  public:

    /**
     * Data definable properties.
     * \since QGIS 3.22
     */
    enum Property
    {
      RendererOpacity, //!< Raster renderer global opacity
    };

    /**
     * Constructor for an empty QgsRasterPipe.
     */
    QgsRasterPipe() = default;

    /**
     * Copy constructor.
     */
    QgsRasterPipe( const QgsRasterPipe &pipe ) SIP_SKIP;

    ~QgsRasterPipe();

    QgsRasterPipe &operator=( const QgsRasterPipe &rh ) = delete;

    /**
     * Moves the pipe to another \a thread.
     *
     * This effects all QObject derived interfaces in the pipe, and follows the same
     * behavior as QObject::moveToThread. Specifically, it is permitted to PUSH the
     * pipe from the current thread to another thread, but NOT to PULL a pipe
     * from another thread over to the current thread. Pulling is only supported
     * by first pushsing the pipe from its current thread to a NULLPTR thread,
     * and then later pulling to the current thread. See QObject documentation
     * for more details.
     *
     * \since QGIS 3.30
    */
    void moveToThread( QThread *thread );

    /**
     * Attempts to insert interface at specified index and connect
     * if connection would fail, the interface is not inserted and FALSE is returned
     *
     * \see set()
     * \see replace()
    */
    bool insert( int idx, QgsRasterInterface *interface SIP_TRANSFER );
#ifdef SIP_RUN
    % MethodCode
    sipRes = sipCpp->insert( a0, a1 );
    if ( !sipRes )
    {
      // if insertion failed transfer ownership back to python
      PyObject *o = sipGetPyObject( a1, sipType_QgsRasterInterface );
      if ( o )
        sipTransferTo( o, NULL );
    }
    % End
#endif

    /**
     * Attempts to replace the interface at specified index and reconnect the pipe.
     *
     * If the connection would fail, the interface is not inserted and FALSE is returned.
     *
     * \see insert()
     * \see set()
    */
    bool replace( int idx, QgsRasterInterface *interface SIP_TRANSFER );

    /**
     * Inserts a new known interface in default place or replace interface of the same
     * role if it already exists.
     *
     * Known interfaces are:
     *
     * - QgsRasterDataProvider,
     * - QgsRasterRenderer
     * - QgsRasterResampleFilter
     * - QgsRasterProjector
     * - QgsHueSaturationFilter
     * - QgsBrightnessContrastFilter
     *
     * (and their subclasses).
     *
     * For other interfaces the position of the interface in the pipe must be explicitly
     * specified using the insert() method.
     *
     * \see insert()
     * \see replace()
     */
    bool set( QgsRasterInterface *interface SIP_TRANSFER );

    /**
     * Removes and deletes the interface at given index (if possible).
     *
     * Returns TRUE if the interface was successfully removed.
     */
    bool remove( int idx );

    /**
     * Removes and deletes interface from pipe (if possible).
     *
     * Returns TRUE if the interface was successfully removed.
     */
    bool remove( QgsRasterInterface *interface );

    /**
     * Returns the size of the pipe (the number of interfaces contained in the pipe).
     */
    int size() const { return mInterfaces.size(); }

    /**
     * Returns the interface at the specified index.
     */
    QgsRasterInterface *at( int idx ) const { return mInterfaces.at( idx ); }

    /**
     * Returns last interface in the pipe.
     */
    QgsRasterInterface *last() const { return mInterfaces.last(); }

    /**
     * Set whether the interface at the specified index is enabled.
     *
     * Returns TRUE on success.
    */
    bool setOn( int idx, bool on );

    /**
     * Returns TRUE if the interface at the specified index may be switched on or off.
     */
    bool canSetOn( int idx, bool on );

    // Getters for special types of interfaces

    /**
     * Returns the data provider interface, or NULLPTR if no data provider is present in the pipe.
     */
    QgsRasterDataProvider *provider() const;

    /**
     * Returns the raster renderer interface, or NULLPTR if no raster renderer is present in the pipe.
     */
    QgsRasterRenderer *renderer() const;

    /**
     * Returns the resample filter interface, or NULLPTR if no resample filter is present in the pipe.
     */
    QgsRasterResampleFilter *resampleFilter() const;

    /**
     * Returns the brightness filter interface, or NULLPTR if no brightness filter is present in the pipe.
     */
    QgsBrightnessContrastFilter *brightnessFilter() const;

    /**
     * Returns the hue/saturation interface, or NULLPTR if no hue/saturation filter is present in the pipe.
     */
    QgsHueSaturationFilter *hueSaturationFilter() const;

    /**
     * Returns the projector interface, or NULLPTR if no projector is present in the pipe.
     */
    QgsRasterProjector *projector() const;

    /**
     * Returns the raster nuller interface, or NULLPTR if no raster nuller is present in the pipe.
     */
    QgsRasterNuller *nuller() const;

    /**
     * Sets which stage of the pipe should apply resampling.
     *
     * Provider resampling is only supported if provider sets
     * ProviderHintCanPerformProviderResampling in providerCapabilities().
     *
     * \see resamplingStage()
     * \since QGIS 3.16
     */
    void setResamplingStage( Qgis::RasterResamplingStage stage );

    /**
     * Returns which stage of the pipe should apply resampling
     *
     * \see setResamplingStage()
     * \since QGIS 3.16
     */
    Qgis::RasterResamplingStage resamplingStage() const { return mResamplingStage; }

    /**
     * Returns a reference to the pipe's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \since QGIS 3.22
     */
    QgsPropertyCollection &dataDefinedProperties() { return mDataDefinedProperties; }

    /**
     * Returns a reference to the pipe's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \see Property
     * \note not available in Python bindings
     * \since QGIS 3.22
     */
    const QgsPropertyCollection &dataDefinedProperties() const SIP_SKIP { return mDataDefinedProperties; }

    /**
     * Sets the pipe's property \a collection, used for data defined overrides.
     *
     * Any existing properties will be discarded.
     *
     * \see dataDefinedProperties()
     * \see Property
     * \since QGIS 3.22
     */
    void setDataDefinedProperties( const QgsPropertyCollection &collection ) { mDataDefinedProperties = collection; }

    /**
     * Evaluates any data defined properties set on the pipe, applying their results
     * to the corresponding interfaces in place.
     *
     * \since QGIS 3.22
     */
    void evaluateDataDefinedProperties( QgsExpressionContext &context );

    /**
     * Returns the definitions for data defined properties available for use in raster pipes.
     *
     * \since QGIS 3.22
     */
    static QgsPropertiesDefinition propertyDefinitions();

  private:
#ifdef SIP_RUN
    QgsRasterPipe( const QgsRasterPipe &pipe );
#endif

    //! Gets known parent type_info of interface parent
    Qgis::RasterPipeInterfaceRole interfaceRole( QgsRasterInterface *iface ) const;

    // Interfaces in pipe, the first is always provider
    QVector<QgsRasterInterface *> mInterfaces;

    QMap<Qgis::RasterPipeInterfaceRole, int> mRoleMap;

    // Set role in mRoleMap
    void setRole( QgsRasterInterface *interface, int idx );

    // Unset role in mRoleMap
    void unsetRole( QgsRasterInterface *interface );

    // Check if index is in bounds
    bool checkBounds( int idx ) const;

    //! Gets known interface by role
    QgsRasterInterface *interface( Qgis::RasterPipeInterfaceRole role ) const;

    /**
     * \brief Try to connect interfaces in pipe and to the provider at beginning.
     * Returns TRUE if connected or false if connection failed
    */
    bool connect( QVector<QgsRasterInterface *> interfaces );

    Qgis::RasterResamplingStage mResamplingStage = Qgis::RasterResamplingStage::ResampleFilter;

    //! Property collection for data defined raster pipe settings
    QgsPropertyCollection mDataDefinedProperties;

    //! Property definitions
    static QgsPropertiesDefinition sPropertyDefinitions;

    static void initPropertyDefinitions();
};

#endif


