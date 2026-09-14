/***************************************************************************
    qgsvectorfieldstreamfield.h
    ---------------------------
    begin                : November 2019
    copyright            : (C) 2019 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORFIELDSTREAMFIELD_H
#define QGSVECTORFIELDSTREAMFIELD_H

#include <list>
#include <memory>

#include "qgsinterpolatedlinerenderer.h"
#include "qgsmaptopixel.h"
#include "qgsrendercontext.h"
#include "qgsvectorfieldvaluesource.h"

#include <QImage>
#include <QPainter>
#include <QSize>
#include <QVector>

#define SIP_NO_FILE

class QgsRasterBlockFeedback;

///@cond PRIVATE

/**
 * \ingroup core
 *
 * \brief Abstract class used to handle information about a stream field.
 *
 * The field is a raster of low resolution pixels covering the zone of interest, in which traces
 * are integrated by walking the vector field cell by cell. Subclasses decide what is stored per
 * pixel and how the result is drawn.
 *
 * This class is data agnostic: everything it knows about the vector field comes from the
 * QgsVectorFieldValueSource it is given.
 *
 * \note not available in Python bindings
 * \since QGIS 4.4
 */
class QgsVectorFieldStreamField
{
  public:
    struct FieldData
    {
        double magnitude;
        float time;
        int directionX;
        int directionY;
    };

    //! Constructor
    QgsVectorFieldStreamField( std::unique_ptr<QgsVectorFieldValueSource> source, const QgsRenderContext &rendererContext, const QgsInterpolatedLineColor &vectorColoring, int resolution = 1 );

    QgsVectorFieldStreamField( const QgsVectorFieldStreamField &other );
    virtual ~QgsVectorFieldStreamField();

    /**
    * Updates the size of the field and the QgsMapToPixel instance to retrieve map point
    * from pixel in the field depending on the resolution of the device
    * If the extent of renderer context and the resolution are not changed, do nothing
    * else, updates the size and cleans
    */
    void updateSize( const QgsRenderContext &renderContext );

    /**
    * Updates the size of the field and the QgsMapToPixel instance to retrieve map point
    * from pixel in the field depending on the resolution of the device
    */
    void updateSize( const QgsRenderContext &renderContext, int resolution );

    //! Returns true if the field is valid
    bool isValid() const;

    //! Returns the size of the field
    QSize size() const;

    //! Returns the topLeft of the field in the device coordinate
    QPoint topLeft() const;

    //! Adds a trace in the field from a start pixel
    void addTrace( QPoint startPixel );

    //! Adds a trace in the field from a map point
    void addTrace( QgsPointXY startPoint );

    //! Adds random traces in the field from random start points, the number of traces depends on the max filling density
    void addRandomTraces();

    //! Adds a trace in the field from one random start point
    void addRandomTrace();

    //! Adds traces in the field from gridded start points, pixelSpace is the space between points in pixel field
    void addGriddedTraces( int dx, int dy );

    /**
     * Adds traces in the field from the natural positions of the data, that is the points returned
     * by QgsVectorFieldValueSource::seedPoints() for \a extent.
     *
     * Adds nothing if the source has no natural seeding positions.
     */
    void addTracesOnDataPoints( const QgsRectangle &extent );

    //! Sets the resolution of the field
    void setResolution( int width );

    //! Returns the width of particle
    int resolution() const;

    //! Returns the size of the image that represents the trace field
    QSize imageSize() const;

    //! Returns the current render image of the field
    virtual QImage image() const;

    //! Sets the maximum pixel filling, eg, the rate of number pixel that can be filled with way.
    void setPixelFillingDensity( double maxFilling );

    //! Sets  color of the streamlines
    void setColor( QColor color );

    //! Sets line width of the streamlines (in px)
    void setLineWidth( double width );

    //! Sets min/max filter
    void setFilter( double min, double max );

    //! Sets if the size of the field has to be minimized of all the data is in the device
    void setMinimizeFieldSize( bool minimizeFieldSize );

    //! Assignment operator
    QgsVectorFieldStreamField &operator=( const QgsVectorFieldStreamField &other );

  protected:
    virtual void initImage();
    QPointF fieldToDevice( const QPoint &pixel ) const;

    /**
     * Returns TRUE if \a value passes the min/max magnitude filter set with setFilter().
     *
     * \note Nothing calls this yet: magnitude filtering has never been applied to streamlines and
     * traces, unlike arrows and wind barbs. Wiring it in changes the rendered output, so it is left
     * to a dedicated fix rather than done as part of a refactor.
     */
    bool filterMag( double value ) const;
    bool isTraceOutside( const QPoint &pixel ) const;

  private:
    QgsPointXY positionToMapCoordinates( const QPoint &pixelPosition, const QgsPointXY &positionInPixel );
    bool addPixelToChunkTrace( QPoint &pixel, QgsVectorFieldStreamField::FieldData &data, std::list<QPair<QPoint, QgsVectorFieldStreamField::FieldData> > &chunkTrace );
    void setChunkTrace( std::list<QPair<QPoint, FieldData>> &chunkTrace );
    virtual void drawTrace( const QPoint & ) const {}
    void clearChunkTrace( std::list<QPair<QPoint, FieldData>> &chunkTrace );
    virtual void storeInField( const QPair<QPoint, FieldData> pixelData ) = 0;
    virtual void initField() = 0;
    void simplifyChunkTrace( std::list<QPair<QPoint, FieldData>> &shunkTrace );

    virtual bool isTraceExists( const QPoint &pixel ) const = 0;

  protected:
    QSize mFieldSize;
    std::unique_ptr<QPainter> mPainter = std::unique_ptr<QPainter>( nullptr );
    int mFieldResolution = 1;
    QPen mPen;
    QImage mTraceImage;

    QgsMapToPixel mMapToFieldPixel;
    QgsRectangle mOutputExtent = QgsRectangle();
    QgsInterpolatedLineColor mVectorColoring;

    /*the direction for a pixel is defined with a char value
     *
     *     1  2  3
     *     4  5  6
     *     7  8  9
     *
     *     convenient to retrieve the indexes of the next pixel from the direction d:
     *     Xnext= (d-1)%3-1
     *     Ynext = (d-1)/3-1
     *
     *     and the direction is defined by :
     *     d=incX + 2 + (incY+1)*3
     */
    QVector<unsigned char> mDirectionField;
    QgsRenderContext mRenderContext;

    std::unique_ptr<QgsVectorFieldValueSource> mSource;

  private:
    int mPixelFillingCount = 0;
    int mMaxPixelFillingCount = 0;
    QgsRectangle mMapExtent;
    QPoint mFieldTopLeftInDeviceCoordinates;
    bool mValid = false;
    double mPixelFillingDensity = 0;
    double mMinMagFilter = -1;
    double mMaxMagFilter = -1;
    bool mMinimizeFieldSize = true; //
};

/**
 * \ingroup core
 *
 * \brief Class used to draw streamlines from a vector field
 *
 * \note not available in Python bindings
 * \since QGIS 4.4
 */
class QgsVectorFieldStreamlinesField : public QgsVectorFieldStreamField
{
  public:
    //! Constructor
    QgsVectorFieldStreamlinesField(
      std::unique_ptr<QgsVectorFieldValueSource> source, QgsRenderContext &rendererContext, const QgsInterpolatedLineColor &vectorColoring, QgsRasterBlockFeedback *feedBack = nullptr
    );

    void compose();

  private:
    void storeInField( const QPair<QPoint, FieldData> pixelData ) override;
    void initField() override;
    void initImage() override;
    bool isTraceExists( const QPoint &pixel ) const override;
    void drawTrace( const QPoint &start ) const override;

    QVector<bool> mField;
    QImage mDrawingTraceImage;
    std::unique_ptr<QPainter> mDrawingTracePainter;

    QgsRasterBlockFeedback *mFeedBack = nullptr;
};

/**
 * \ingroup core
 *
 * \brief Used to simulate a moving particle
 *
 * \note not available in Python bindings
 * \since QGIS 4.4
 */
struct QgsVectorFieldTraceParticle
{
    double lifeTime = 0;
    QPoint position;
    std::list<QPoint> tail;
    double remainingTime = 0; //time remaining to spend in the current pixel at the end of the time step
};

/**
 * \ingroup core
 *
 * \brief Class used to draw particle traces from a vector field
 *
 * \note not available in Python bindings
 * \since QGIS 4.4
 */
class QgsVectorFieldParticleTracesField : public QgsVectorFieldStreamField
{
  public:
    //! Constructor
    QgsVectorFieldParticleTracesField( std::unique_ptr<QgsVectorFieldValueSource> source, const QgsRenderContext &rendererContext, const QgsInterpolatedLineColor &vectorColoring );

    QgsVectorFieldParticleTracesField( const QgsVectorFieldParticleTracesField &other );

    //! Adds a particle in the vector field from a start point (pixel) with a specified life time
    void addParticle( const QPoint &startPoint, double lifeTime );

    //! Adds a particle in the vector field from a start point (map point) with a specified life time
    void addParticleXY( const QgsPointXY &startPoint, double lifeTime );

    //! Adds particle randomly (position and life time
    void addRandomParticles();

    //! Moves all the particles with a displacement corresponding to a nondimensional time
    void moveParticles();

    //! Returns the current image of the particles
    QImage imageRendered() const;

    //! Sets the total number of particles generated randomly
    void setParticlesCount( int particlesCount );

    //! Sets the maximum life time (nondimensional) of particle generated
    void setParticlesLifeTime( double particlesLifeTime );

    //! Stumps particles image and leave a persistent effect
    void stump();

    /**
     * Sets stump factor from 0 to 255 :
     * 0, stump completely, no persistence
     * 255, no stump,  total persistence
     */
    void setStumpFactor( int sf );

    //! Sets the time step
    void setTimeStep( double timeStep );

    //! Sets particles size (in px)
    void setParticleSize( double particleSize );

    //! Sets the tail factor
    void setTailFactor( double tailFactor );

    //! Sets the minimum tail length
    void setMinTailLength( int minTailLength );

    //! Sets if the particle has to be stumped dependiong on liketime
    void setStumpParticleWithLifeTime( bool stumpParticleWithLifeTime );

    //! Sets the color of the particles, overwrite the color provided by vector settings
    void setParticlesColor( const QColor &c );

    QgsVectorFieldParticleTracesField &operator=( const QgsVectorFieldParticleTracesField &other );

  private:
    QPoint direction( QPoint position ) const;

    float time( QPoint position ) const;
    float magnitude( QPoint position ) const;

    void drawParticleTrace( const QgsVectorFieldTraceParticle &particle );

    void storeInField( const QPair<QPoint, FieldData> pixelData ) override;
    void initField() override;
    bool isTraceExists( const QPoint &pixel ) const override;

    /* Nondimensional time
     * This field store the time spent by the particle in the pixel
     *
     * This time is nondimensional and value 1 is equivalent to the time spent by the particle in a pixel
     * for Vmax, the maximum magnitude of the vector field.
     *
     */
    QVector<float> mTimeField;
    QVector<float> mMagnitudeField;

    QList<QgsVectorFieldTraceParticle> mParticles;
    QImage mStumpImage;

    double mTimeStep = 200;
    double mParticlesLifeTime = 5000;
    int mParticlesCount = 1000;
    double mTailFactor = 5;
    int mMinTailLength = 3;
    QColor mParticleColor = Qt::white;
    double mParticleSize = 2.5;
    int mStumpFactor = 50;
    bool mStumpParticleWithLifeTime = true;
};

///@endcond

#endif // QGSVECTORFIELDSTREAMFIELD_H
