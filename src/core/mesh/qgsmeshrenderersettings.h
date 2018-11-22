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
#include <limits>

#include "qgis_core.h"
#include "qgis.h"
#include "qgscolorrampshader.h"
#include "qgsmeshdataprovider.h"


/**
 * \ingroup core
 *
 * Represents a mesh renderer settings for mesh object
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

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );

  private:
    bool mEnabled = false;
    double mLineWidth = DEFAULT_LINE_WIDTH;
    QColor mColor = Qt::black;
};

/**
 * \ingroup core
 *
 * Represents a mesh renderer settings for scalar datasets
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshRendererScalarSettings
{
  public:
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

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );

  private:
    QgsColorRampShader mColorRampShader;
    double mClassificationMinimum = 0;
    double mClassificationMaximum = 0;
    double mOpacity = 1;
};

/**
 * \ingroup core
 *
 * Represents a mesh renderer settings for vector datasets
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshRendererVectorSettings
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

    //! Returns method used for drawing arrows
    QgsMeshRendererVectorSettings::ArrowScalingMethod shaftLengthMethod() const;
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

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );

  private:
    double mLineWidth = DEFAULT_LINE_WIDTH; //in milimeters
    QColor mColor = Qt::black;
    double mFilterMin = -1; //disabled
    double mFilterMax = -1; //disabled
    QgsMeshRendererVectorSettings::ArrowScalingMethod mShaftLengthMethod = QgsMeshRendererVectorSettings::ArrowScalingMethod::MinMax;
    double mMinShaftLength = 0.8; //in milimeters
    double mMaxShaftLength = 10; //in milimeters
    double mScaleFactor = 10;
    double mFixedShaftLength = 20; //in milimeters
    double mArrowHeadWidthRatio = 0.15;
    double mArrowHeadLengthRatio = 0.40;
    bool mOnUserDefinedGrid = false;
    int mUserGridCellWidth = 10; // in pixels
    int mUserGridCellHeight = 10; // in pixels
};


/**
 * \ingroup core
 *
 * Represents all mesh renderer settings
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsMeshRendererSettings
{
  public:

    //! Returns renderer settings
    QgsMeshRendererMeshSettings nativeMeshSettings() const { return mRendererNativeMeshSettings; }
    //! Sets new renderer settings, triggers repaint
    void setNativeMeshSettings( const QgsMeshRendererMeshSettings &settings ) { mRendererNativeMeshSettings = settings; }

    //! Returns renderer settings
    QgsMeshRendererMeshSettings triangularMeshSettings() const { return mRendererTriangularMeshSettings; }
    //! Sets new renderer settings
    void setTriangularMeshSettings( const QgsMeshRendererMeshSettings &settings ) { mRendererTriangularMeshSettings = settings; }

    //! Returns renderer settings
    QgsMeshRendererScalarSettings scalarSettings( int groupIndex ) const { return mRendererScalarSettings.value( groupIndex ); }
    //! Sets new renderer settings
    void setScalarSettings( int groupIndex, const QgsMeshRendererScalarSettings &settings ) { mRendererScalarSettings[groupIndex] = settings; }

    //! Returns renderer settings
    QgsMeshRendererVectorSettings vectorSettings( int groupIndex ) const { return mRendererVectorSettings.value( groupIndex ); }
    //! Sets new renderer settings
    void setVectorSettings( int groupIndex, const QgsMeshRendererVectorSettings &settings ) { mRendererVectorSettings[groupIndex] = settings; }

    //! Returns active scalar dataset
    QgsMeshDatasetIndex activeScalarDataset() const { return mActiveScalarDataset; }
    //! Sets active scalar dataset for rendering
    void setActiveScalarDataset( QgsMeshDatasetIndex index = QgsMeshDatasetIndex() ) { mActiveScalarDataset = index; }

    //! Returns active vector dataset
    QgsMeshDatasetIndex activeVectorDataset() const { return mActiveVectorDataset; }
    //! Sets active vector dataset for rendering.
    void setActiveVectorDataset( QgsMeshDatasetIndex index = QgsMeshDatasetIndex() ) { mActiveVectorDataset = index; }

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );

  private:
    QgsMeshRendererMeshSettings mRendererNativeMeshSettings;
    QgsMeshRendererMeshSettings mRendererTriangularMeshSettings;

    QHash<int, QgsMeshRendererScalarSettings> mRendererScalarSettings;  //!< Per-group scalar settings
    QHash<int, QgsMeshRendererVectorSettings> mRendererVectorSettings;  //!< Per-group vector settings

    //! index of active scalar dataset
    QgsMeshDatasetIndex mActiveScalarDataset;

    //! index of active vector dataset
    QgsMeshDatasetIndex mActiveVectorDataset;
};

#endif //QGSMESHRENDERERSETTINGS_H
