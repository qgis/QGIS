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

#include <QColor>
#include <QDomElement>
#include <limits>

#include "qgis_core.h"
#include "qgis.h"
#include "qgscolorrampshader.h"
#include "qgsmeshdataprovider.h"
#include "qgsmesh3daveraging.h"
#include "qgsinterpolatedlinerenderer.h"

/**
 * \ingroup core
 *
 * \brief Represents a mesh renderer settings for mesh object
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
    QgsUnitTypes::RenderUnit lineWidthUnit() const;

    /**
     * Sets units of the width of the mesh frame
     *
     * \since QGIS 3.14
     */
    void setLineWidthUnit( const QgsUnitTypes::RenderUnit &lineWidthUnit );

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );

  private:
    bool mEnabled = false;
    double mLineWidth = DEFAULT_LINE_WIDTH;
    QgsUnitTypes::RenderUnit mLineWidthUnit = QgsUnitTypes::RenderMillimeters;
    QColor mColor = Qt::black;
};

/**
 * \ingroup core
 *
 * \brief Represents a mesh renderer settings for scalar datasets
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
      None = 0,

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
    QgsUnitTypes::RenderUnit edgeStrokeWidthUnit() const;

    /**
     * Sets the stroke width unit used to render edges scalar dataset
     *
     * \since QGIS 3.14
     */
    void setEdgeStrokeWidthUnit( const QgsUnitTypes::RenderUnit &edgeStrokeWidthUnit );

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context = QgsReadWriteContext() ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context = QgsReadWriteContext() );

  private:
    QgsColorRampShader mColorRampShader;
    DataResamplingMethod mDataResamplingMethod = DataResamplingMethod::None;
    double mClassificationMinimum = 0;
    double mClassificationMaximum = 0;
    double mOpacity = 1;

    QgsInterpolatedLineWidth mEdgeStrokeWidth;
    QgsUnitTypes::RenderUnit mEdgeStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;
};

/**
 * \ingroup core
 *
 * \brief Represents a mesh renderer settings for vector datasets displayed with arrows
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMeshRendererVectorArrowSettings
{
  public:

    //! Algorithm how to transform vector magnitude to length of arrow on the device in pixels
    enum ArrowScalingMethod
    {

      /**
       * Scale vector magnitude linearly to fit in range of vectorFilterMin() and vectorFilterMax()
       */
      MinMax = 0,

      /**
       * Scale vector magnitude by factor scaleFactor()
       */
      Scaled,

      /**
       * Use fixed length fixedShaftLength() regardless of vector's magnitude
       */
      Fixed
    };

    //! Returns method used for drawing arrows
    QgsMeshRendererVectorArrowSettings::ArrowScalingMethod shaftLengthMethod() const;
    //! Sets method used for drawing arrows
    void setShaftLengthMethod( ArrowScalingMethod shaftLengthMethod );

    /**
     * Returns mininimum shaft length (in millimeters)
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::MinMax
     */
    double minShaftLength() const;

    /**
     * Sets mininimum shaft length (in millimeters)
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::MinMax
     */
    void setMinShaftLength( double minShaftLength );

    /**
     * Returns maximum shaft length (in millimeters)
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::MinMax
     */
    double maxShaftLength() const;

    /**
     * Sets maximum shaft length (in millimeters)
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::MinMax
     */
    void setMaxShaftLength( double maxShaftLength );

    /**
     * Returns scale factor
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::Scaled
     */
    double scaleFactor() const;

    /**
     * Sets scale factor
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::Scaled
     */
    void setScaleFactor( double scaleFactor );

    /**
     * Returns fixed arrow length (in millimeters)
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::Fixed
     */
    double fixedShaftLength() const;

    /**
     * Sets fixed length  (in millimeters)
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::Fixed
     */
    void setFixedShaftLength( double fixedShaftLength );

    //! Returns ratio of the head width of the arrow (range 0-1)
    double arrowHeadWidthRatio() const;
    //! Sets ratio of the head width of the arrow (range 0-1)
    void setArrowHeadWidthRatio( double arrowHeadWidthRatio );

    //! Returns ratio of the head length of the arrow (range 0-1)
    double arrowHeadLengthRatio() const;
    //! Sets ratio of the head length of the arrow (range 0-1)
    void setArrowHeadLengthRatio( double arrowHeadLengthRatio );

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );

  private:
    QgsMeshRendererVectorArrowSettings::ArrowScalingMethod mShaftLengthMethod = QgsMeshRendererVectorArrowSettings::ArrowScalingMethod::MinMax;
    double mMinShaftLength = 0.8; //in millimeters
    double mMaxShaftLength = 10; //in millimeters
    double mScaleFactor = 10;
    double mFixedShaftLength = 20; //in millimeters
    double mArrowHeadWidthRatio = 0.15;
    double mArrowHeadLengthRatio = 0.40;
};

/**
 * \ingroup core
 *
 * \brief Represents a streamline renderer settings for vector datasets displayed by streamlines
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMeshRendererVectorStreamlineSettings
{
  public:
    //! Method used to define start points that are used to draw streamlines
    enum SeedingStartPointsMethod
    {

      /**
       * Seeds start points on the vertices mesh or user regular grid
       */
      MeshGridded = 0,

      /**
       * Seeds start points randomly on the mesh
       */
      Random
    };

    //! Returns the method used for seeding start points of strealines
    SeedingStartPointsMethod seedingMethod() const;
    //! Sets the method used for seeding start points of strealines
    void setSeedingMethod( const SeedingStartPointsMethod &seedingMethod );
    //! Returns the density used for seeding start points
    double seedingDensity() const;
    //! Sets the density used for seeding start points
    void setSeedingDensity( double seedingDensity );
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );
    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;

  private:

    QgsMeshRendererVectorStreamlineSettings::SeedingStartPointsMethod mSeedingMethod = MeshGridded;
    double mSeedingDensity = 0.15;
};

/**
 * \ingroup core
 *
 * \brief Represents a trace renderer settings for vector datasets displayed by particle traces
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMeshRendererVectorTracesSettings
{
  public:

    //! Returns the maximum tail length
    double maximumTailLength() const;
    //! Sets the maximums tail length
    void setMaximumTailLength( double maximumTailLength );
    //! Returns particles count
    int particlesCount() const;
    //! Sets particles count
    void setParticlesCount( int value );
    //! Returns the maximum tail length unit
    QgsUnitTypes::RenderUnit maximumTailLengthUnit() const;
    //! Sets the maximum tail length unit
    void setMaximumTailLengthUnit( const QgsUnitTypes::RenderUnit &maximumTailLengthUnit );

    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );
    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;



  private:
    int mParticlesCount = 1000;
    double mMaximumTailLength = 100;
    QgsUnitTypes::RenderUnit mMaximumTailLengthUnit = QgsUnitTypes::RenderMillimeters;

};

/**
 * \ingroup core
 *
 * \brief Represents a renderer settings for vector datasets
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshRendererVectorSettings
{
  public:

    /**
     * Defines the symbology of vector rendering
     * \since QGIS 3.12
     */
    enum Symbology
    {
      //! Displaying vector dataset with arrows
      Arrows = 0,
      //! Displaying vector dataset with streamlines
      Streamlines,
      //! Displaying vector dataset with particle traces
      Traces
    };

    //! Returns line width of the arrow (in millimeters)
    double lineWidth() const;
    //! Sets line width of the arrow in pixels (in millimeters)
    void setLineWidth( double lineWidth );

    //! Returns color used for drawing arrows
    QColor color() const;
    //! Sets color used for drawing arrows
    void setColor( const QColor &color );

    /**
     * Returns filter value for vector magnitudes.
     *
     * If magnitude of the vector is lower than this value, the vector is not
     * drawn. -1 represents that filtering is not active.
     */
    double filterMin() const;

    /**
     * Sets filter value for vector magnitudes.
     * \see filterMin()
     */
    void setFilterMin( double filterMin );

    /**
     * Returns filter value for vector magnitudes.
     *
     * If magnitude of the vector is higher than this value, the vector is not
     * drawn. -1 represents that filtering is not active.
     */
    double filterMax() const;

    /**
     * Sets filter value for vector magnitudes.
     * \see filterMax()
     */
    void setFilterMax( double filterMax );

    //! Returns whether vectors are drawn on user-defined grid
    bool isOnUserDefinedGrid() const;
    //! Toggles drawing of vectors on user defined grid
    void setOnUserDefinedGrid( bool enabled );
    //! Returns width in pixels of user grid cell
    int userGridCellWidth() const;
    //! Sets width of user grid cell (in pixels)
    void setUserGridCellWidth( int width );
    //! Returns height in pixels of user grid cell
    int userGridCellHeight() const;
    //! Sets height of user grid cell (in pixels)
    void setUserGridCellHeight( int height );

    /**
    * Returns the displaying method used to render vector datasets
    * \since QGIS 3.12
    */
    Symbology symbology() const;

    /**
     * Sets the displaying method used to render vector datasets
     * \since QGIS 3.12
     */
    void setSymbology( const Symbology &symbology );

    /**
     * Returns the coloring method used to render vector datasets
     * \since QGIS 3.14
     */
    QgsInterpolatedLineColor::ColoringMethod coloringMethod() const;

    /**
     * Sets the coloring method used to render vector datasets
     * \since QGIS 3.14
     */
    void setColoringMethod( const QgsInterpolatedLineColor::ColoringMethod &coloringMethod );

    /**
     * Sets the color ramp shader used to render vector datasets
     * \since QGIS 3.14
     */
    QgsColorRampShader colorRampShader() const;

    /**
     * Returns the color ramp shader used to render vector datasets
     * \since QGIS 3.14
     */
    void setColorRampShader( const QgsColorRampShader &colorRampShader );

    /**
     * Returns the stroke coloring used to render vector datasets
     * \since QGIS 3.14
     */
    QgsInterpolatedLineColor vectorStrokeColoring() const;

    /**
    * Returns settings for vector rendered with arrows
    * \since QGIS 3.12
    */
    QgsMeshRendererVectorArrowSettings arrowSettings() const;

    /**
     * Sets settings for vector rendered with arrows
     * \since QGIS 3.12
     */
    void setArrowsSettings( const QgsMeshRendererVectorArrowSettings &arrowSettings );

    /**
     * Returns settings for vector rendered with streamlines
     * \since QGIS 3.12
     */
    QgsMeshRendererVectorStreamlineSettings streamLinesSettings() const;

    /**
     * Sets settings for vector rendered with streamlines
     * \since QGIS 3.12
     */
    void setStreamLinesSettings( const QgsMeshRendererVectorStreamlineSettings &streamLinesSettings );

    /**
     * Returns settings for vector rendered with traces
     * \since QGIS 3.12
     */
    QgsMeshRendererVectorTracesSettings tracesSettings() const;

    /**
     * Sets settings for vector rendered with traces
     * \since QGIS 3.12
     */
    void setTracesSettings( const QgsMeshRendererVectorTracesSettings &tracesSettings );

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context = QgsReadWriteContext() ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context = QgsReadWriteContext() );

  private:

    Symbology mDisplayingMethod = Arrows;

    double mLineWidth = DEFAULT_LINE_WIDTH; //in millimeters
    QgsColorRampShader mColorRampShader;
    QColor mColor = Qt::black;
    QgsInterpolatedLineColor::ColoringMethod mColoringMethod = QgsInterpolatedLineColor::SingleColor;
    double mFilterMin = -1; //disabled
    double mFilterMax = -1; //disabled
    int mUserGridCellWidth = 10; // in pixels
    int mUserGridCellHeight = 10; // in pixels
    bool mOnUserDefinedGrid = false;

    QgsMeshRendererVectorArrowSettings mArrowsSettings;
    QgsMeshRendererVectorStreamlineSettings mStreamLinesSettings;
    QgsMeshRendererVectorTracesSettings mTracesSettings;
};

/**
 * \ingroup core
 *
 * \brief Represents all mesh renderer settings
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
    //! Destructor
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

    //! Returns renderer settings
    QgsMeshRendererVectorSettings vectorSettings( int groupIndex ) const { return mRendererVectorSettings.value( groupIndex ); }
    //! Sets new renderer settings
    void setVectorSettings( int groupIndex, const QgsMeshRendererVectorSettings &settings ) { mRendererVectorSettings[groupIndex] = settings; }

    /**
     * Returns averaging method for conversion of 3d stacked mesh data to 2d data
     *
     * Caller does not own the resulting pointer
     */
    QgsMesh3dAveragingMethod *averagingMethod() const;

    /**
     * Sets averaging method for conversion of 3d stacked mesh data to 2d data
     *
     * Ownership of the method is not transferred.
     */
    void setAveragingMethod( QgsMesh3dAveragingMethod *method );

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

    QHash<int, QgsMeshRendererScalarSettings> mRendererScalarSettings;  //!< Per-group scalar settings
    QHash<int, QgsMeshRendererVectorSettings> mRendererVectorSettings;  //!< Per-group vector settings

    //! index of active scalar dataset group
    int mActiveScalarDatasetGroup = -1;

    //! index of active vector dataset group
    int mActiveVectorDatasetGroup = -1;

    //! Averaging method to get 2D datasets from 3D stacked mesh datasets
    std::shared_ptr<QgsMesh3dAveragingMethod> mAveragingMethod;
};

#endif //QGSMESHRENDERERSETTINGS_H
