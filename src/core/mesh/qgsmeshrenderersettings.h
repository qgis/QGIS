/***************************************************************************
                         qgsmeshrenderersettings.h
                         -------------------------
    begin                : May 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHRENDERERSETTINGS_H
#define QGSMESHRENDERERSETTINGS_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgscolorrampshader.h"
#include "qgsinterpolatedlinerenderer.h"
#include "qgsmesh3daveraging.h"
#include "qgsvectorfieldsettings.h"

#include <QColor>
#include <QDomElement>

/**
 * \ingroup core
 *
 * \brief Represents a mesh renderer settings for mesh objects.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshRendererMeshSettings
{
  public:
    //! Returns whether mesh structure rendering is enabled
    bool isEnabled() const;
    //! Sets whether mesh structure rendering is enabled
    void setEnabled( bool enabled );

    //! Returns line width used for rendering (in millimeters)
    double lineWidth() const;
    //! Sets line width used for rendering (in millimeters)
    void setLineWidth( double lineWidth );

    //! Returns color used for rendering
    QColor color() const;
    //! Sets color used for rendering of the mesh
    void setColor( const QColor &color );

    /**
     * Returns units of the width of the mesh frame
     *
     * \since QGIS 3.14
     */
    Qgis::RenderUnit lineWidthUnit() const;

    /**
     * Sets units of the width of the mesh frame
     *
     * \since QGIS 3.14
     */
    void setLineWidthUnit( Qgis::RenderUnit lineWidthUnit );

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );

  private:
    bool mEnabled = false;
    double mLineWidth = Qgis::DEFAULT_LINE_WIDTH;
    Qgis::RenderUnit mLineWidthUnit = Qgis::RenderUnit::Millimeters;
    QColor mColor = Qt::black;
};

/**
 * \ingroup core
 *
 * \brief Represents a mesh renderer settings for scalar datasets.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshRendererScalarSettings
{
  public:
    /**
     * Resampling of value from dataset
     *
     * - for vertices : does a resampling from values defined on surrounding faces
     * - for faces : does a resampling from values defined on surrounding vertices
     * - for edges : not supported.
     */
    enum DataResamplingMethod
    {

      /**
       * Does not use resampling
       */
      NoResampling = 0,

      /**
       * Does a simple average of values defined for all surrounding faces/vertices
       */
      NeighbourAverage,
    };

    //! Returns color ramp shader function
    QgsColorRampShader colorRampShader() const;
    //! Sets color ramp shader function
    void setColorRampShader( const QgsColorRampShader &shader );

    //! Returns min value used for creation of the color ramp shader
    double classificationMinimum() const;
    //! Returns max value used for creation of the color ramp shader
    double classificationMaximum() const;
    //! Sets min/max values used for creation of the color ramp shader
    void setClassificationMinimumMaximum( double minimum, double maximum );

    //! Returns opacity
    double opacity() const;
    //! Sets opacity
    void setOpacity( double opacity );

    /**
     * Returns the type of interpolation to use to
     * convert face defined datasets to
     * values on vertices
     *
     * \since QGIS 3.12
     */
    DataResamplingMethod dataResamplingMethod() const;

    /**
     * Sets data interpolation method
     *
     * \since QGIS 3.12
     */
    void setDataResamplingMethod( const DataResamplingMethod &dataResamplingMethod );

    /**
     * Returns the stroke width used to render edges scalar dataset
     *
     * \since QGIS 3.14
     */
    QgsInterpolatedLineWidth edgeStrokeWidth() const;

    /**
     * Sets the stroke width used to render edges scalar dataset
     *
     * \since QGIS 3.14
     */
    void setEdgeStrokeWidth( const QgsInterpolatedLineWidth &strokeWidth );

    /**
    *Returns the stroke width unit used to render edges scalar dataset
    *
    * \since QGIS 3.14
    */
    Qgis::RenderUnit edgeStrokeWidthUnit() const;

    /**
     * Sets the stroke width unit used to render edges scalar dataset
     *
     * \since QGIS 3.14
     */
    void setEdgeStrokeWidthUnit( Qgis::RenderUnit edgeStrokeWidthUnit );

    /**
     * Sets the range limits type for minimum maximum calculation
     *
     * \since QGIS 3.42
     */
    void setLimits( Qgis::MeshRangeLimit limits ) { mRangeLimit = limits; }

    /**
     * Returns the range limits type for minimum maximum calculation
     *
     * \since QGIS 3.42
     */
    Qgis::MeshRangeLimit limits() const { return mRangeLimit; }

    /**
     * Sets the mesh extent for minimum maximum calculation
     *
     * \since QGIS 3.42
     */
    void setExtent( Qgis::MeshRangeExtent extent ) { mRangeExtent = extent; }

    /**
     * Returns the mesh extent for minimum maximum calculation
     *
     * \since QGIS 3.42
     */
    Qgis::MeshRangeExtent extent() const { return mRangeExtent; }

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context = QgsReadWriteContext() ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context = QgsReadWriteContext() );

  private:
    void updateShader();

    QgsColorRampShader mColorRampShader;
    DataResamplingMethod mDataResamplingMethod = DataResamplingMethod::NoResampling;
    double mClassificationMinimum = 0;
    double mClassificationMaximum = 0;
    double mOpacity = 1;

    QgsInterpolatedLineWidth mEdgeStrokeWidth;
    Qgis::RenderUnit mEdgeStrokeWidthUnit = Qgis::RenderUnit::Millimeters;

    Qgis::MeshRangeExtent mRangeExtent = Qgis::MeshRangeExtent::WholeMesh;
    Qgis::MeshRangeLimit mRangeLimit = Qgis::MeshRangeLimit::NotSet;
};

/**
 * \ingroup core
 *
 * \brief Represents a mesh renderer settings for vector datasets displayed with arrows.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.12
 * \deprecated QGIS 4.4. Use QgsVectorFieldArrowSettings instead.
 */
class CORE_EXPORT QgsMeshRendererVectorArrowSettings : public QgsVectorFieldArrowSettings
{
    // exists for backwards compatibility only
};

/**
 * \ingroup core
 *
 * \brief Represents a streamline renderer settings for vector datasets displayed by streamlines.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.12
 * \deprecated QGIS 4.4. Use QgsVectorFieldStreamlineSettings instead.
 */
class CORE_EXPORT QgsMeshRendererVectorStreamlineSettings : public QgsVectorFieldStreamlineSettings
{
    // exists for backwards compatibility only
};

/**
 * \ingroup core
 *
 * \brief Represents a trace renderer settings for vector datasets displayed by particle traces.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.12
 * \deprecated QGIS 4.4. Use QgsVectorFieldTracesSettings instead.
 */
class CORE_EXPORT QgsMeshRendererVectorTracesSettings : public QgsVectorFieldTracesSettings
{
    // exists for backwards compatibility only
};

/**
 * \ingroup core
 *
 * \brief Represents a mesh renderer settings for vector datasets displayed with wind barbs.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.38
 * \deprecated QGIS 4.4. Use QgsVectorFieldWindBarbSettings instead.
 */
class CORE_EXPORT QgsMeshRendererVectorWindBarbSettings : public QgsVectorFieldWindBarbSettings
{
    // exists for backwards compatibility only
};

/**
 * \ingroup core
 *
 * \brief Represents a renderer settings for vector datasets.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 * \deprecated QGIS 4.4. Use QgsVectorFieldSettings instead.
 */
class CORE_EXPORT QgsMeshRendererVectorSettings : public QgsVectorFieldSettings
{
    // exists for backwards compatibility only
};

/**
 * \ingroup core
 *
 * \brief Represents all mesh renderer settings.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsMeshRendererSettings
{
  public:
    /**
     * Constructs renderer with default single layer averaging method
     */
    QgsMeshRendererSettings();
    QgsMeshRendererSettings( const QgsMeshRendererSettings &other );
    SIP_SKIP QgsMeshRendererSettings( QgsMeshRendererSettings &&other );

    QgsMeshRendererSettings &operator=( const QgsMeshRendererSettings &other );
    QgsMeshRendererSettings &operator=( QgsMeshRendererSettings &&other );

    ~QgsMeshRendererSettings();

    //! Returns native mesh renderer settings
    QgsMeshRendererMeshSettings nativeMeshSettings() const { return mRendererNativeMeshSettings; }
    //! Sets new native mesh  renderer settings, triggers repaint
    void setNativeMeshSettings( const QgsMeshRendererMeshSettings &settings ) { mRendererNativeMeshSettings = settings; }

    //! Returns triangular mesh renderer settings
    QgsMeshRendererMeshSettings triangularMeshSettings() const { return mRendererTriangularMeshSettings; }
    //! Sets new triangular mesh renderer settings
    void setTriangularMeshSettings( const QgsMeshRendererMeshSettings &settings ) { mRendererTriangularMeshSettings = settings; }

    /**
     * Returns edge mesh renderer settings
     * \since QGIS 3.14
     */
    QgsMeshRendererMeshSettings edgeMeshSettings() const { return mRendererEdgeMeshSettings; }

    /**
     * Sets new edge mesh renderer settings
     * \since QGIS 3.14
     */
    void setEdgeMeshSettings( const QgsMeshRendererMeshSettings &settings ) { mRendererEdgeMeshSettings = settings; }

    //! Returns renderer settings
    QgsMeshRendererScalarSettings scalarSettings( int groupIndex ) const { return mRendererScalarSettings.value( groupIndex ); }

    //! Sets new renderer settings
    void setScalarSettings( int groupIndex, const QgsMeshRendererScalarSettings &settings ) { mRendererScalarSettings[groupIndex] = settings; }

    /**
     * Returns whether \a groupIndex has existing scalar settings
     * \since QGIS 3.30.2
     */
    bool hasScalarSettings( int groupIndex ) const { return mRendererScalarSettings.contains( groupIndex ); }

    /**
     * Removes scalar settings with \a groupIndex
     * \since QGIS 3.30.2
     */
    bool removeScalarSettings( int groupIndex ) { return mRendererScalarSettings.remove( groupIndex ); }

    //! Returns renderer settings
    QgsVectorFieldSettings vectorSettings( int groupIndex ) const { return mRendererVectorSettings.value( groupIndex ); }
    //! Sets new renderer settings
    void setVectorSettings( int groupIndex, const QgsVectorFieldSettings &settings ) { mRendererVectorSettings[groupIndex] = settings; }

    /**
     * Returns whether \a groupIndex has existing vector settings
     * \since QGIS 3.30.2
     */
    bool hasVectorSettings( int groupIndex ) const { return mRendererVectorSettings.contains( groupIndex ); }

    /**
     * Removes vector settings for \a groupIndex
     * \since QGIS 3.30.2
     */
    bool removeVectorSettings( int groupIndex ) { return mRendererVectorSettings.remove( groupIndex ); }

    /**
     * Returns averaging method for conversion of 3d stacked mesh data to 2d data
     *
     * Caller does not own the resulting pointer
     */
    QgsMesh3DAveragingMethod *averagingMethod() const;

    /**
     * Sets averaging method for conversion of 3d stacked mesh data to 2d data
     *
     * Ownership of the method is not transferred.
     */
    void setAveragingMethod( QgsMesh3DAveragingMethod *method );

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context = QgsReadWriteContext() ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context = QgsReadWriteContext() );

    /**
     * Returns the active scalar dataset group
     * \since QGIS 3.14
     */
    int activeScalarDatasetGroup() const;

    /**
     * Sets the active scalar dataset group
     * \since QGIS 3.14
     */
    void setActiveScalarDatasetGroup( int activeScalarDatasetGroup );

    /**
     * Returns the active vector dataset group
     * \since QGIS 3.14
     */
    int activeVectorDatasetGroup() const;

    /**
     * Sets the active vector dataset group
     * \since QGIS 3.14
     */
    void setActiveVectorDatasetGroup( int activeVectorDatasetGroup );

    /**
    * Returns whether the group with \a index has render settings (scalar or vector)
    *
    * \since QGIS 3.22
    */
    bool hasSettings( int datasetGroupIndex ) const;

  private:
    QgsMeshRendererMeshSettings mRendererNativeMeshSettings;
    QgsMeshRendererMeshSettings mRendererTriangularMeshSettings;
    QgsMeshRendererMeshSettings mRendererEdgeMeshSettings;

    QHash<int, QgsMeshRendererScalarSettings> mRendererScalarSettings; //!< Per-group scalar settings
    QHash<int, QgsVectorFieldSettings> mRendererVectorSettings;        //!< Per-group vector settings

    //! index of active scalar dataset group
    int mActiveScalarDatasetGroup = -1;

    //! index of active vector dataset group
    int mActiveVectorDatasetGroup = -1;

    //! Averaging method to get 2D datasets from 3D stacked mesh datasets
    std::shared_ptr<QgsMesh3DAveragingMethod> mAveragingMethod;
};

#endif //QGSMESHRENDERERSETTINGS_H
