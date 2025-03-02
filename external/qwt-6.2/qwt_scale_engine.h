/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SCALE_ENGINE_H
#define QWT_SCALE_ENGINE_H

#include "qwt_global.h"
#include "qwt_scale_div.h"

class QwtInterval;
class QwtTransform;

/*!
   \brief Arithmetic including a tolerance
 */
class QWT_EXPORT QwtScaleArithmetic
{
  public:
    static double ceilEps( double value, double intervalSize );
    static double floorEps( double value, double intervalSize );

    static double divideEps( double intervalSize, double numSteps );

    static double divideInterval( double intervalSize,
        int numSteps, uint base );
};

/*!
   \brief Base class for scale engines.

   A scale engine tries to find "reasonable" ranges and step sizes
   for scales.

   The layout of the scale can be varied with setAttribute().

   Qwt offers implementations for logarithmic and linear scales.
 */

class QWT_EXPORT QwtScaleEngine
{
  public:
    /*!
       Layout attributes
       \sa setAttribute(), testAttribute(), reference(),
           lowerMargin(), upperMargin()
     */

    enum Attribute
    {
        //! No attributes
        NoAttribute = 0x00,

        //! Build a scale which includes the reference() value.
        IncludeReference = 0x01,

        //! Build a scale which is symmetric to the reference() value.
        Symmetric = 0x02,

        /*!
           The endpoints of the scale are supposed to be equal the
           outmost included values plus the specified margins
           (see setMargins()).
           If this attribute is *not* set, the endpoints of the scale will
           be integer multiples of the step size.
         */
        Floating = 0x04,

        //! Turn the scale upside down.
        Inverted = 0x08
    };

    Q_DECLARE_FLAGS( Attributes, Attribute )

    explicit QwtScaleEngine( uint base = 10 );
    virtual ~QwtScaleEngine();

    void setBase( uint base );
    uint base() const;

    void setAttribute( Attribute, bool on = true );
    bool testAttribute( Attribute ) const;

    void setAttributes( Attributes );
    Attributes attributes() const;

    void setReference( double );
    double reference() const;

    void setMargins( double lower, double upper );
    double lowerMargin() const;
    double upperMargin() const;

    /*!
       Align and divide an interval

       \param maxNumSteps Max. number of steps
       \param x1 First limit of the interval (In/Out)
       \param x2 Second limit of the interval (In/Out)
       \param stepSize Step size (Return value)
     */
    virtual void autoScale( int maxNumSteps,
        double& x1, double& x2, double& stepSize ) const = 0;

    /*!
       \brief Calculate a scale division

       \param x1 First interval limit
       \param x2 Second interval limit
       \param maxMajorSteps Maximum for the number of major steps
       \param maxMinorSteps Maximum number of minor steps
       \param stepSize Step size. If stepSize == 0.0, the scaleEngine
                   calculates one.

       \return Calculated scale division
     */
    virtual QwtScaleDiv divideScale( double x1, double x2,
        int maxMajorSteps, int maxMinorSteps,
        double stepSize = 0.0 ) const = 0;

    void setTransformation( QwtTransform* );
    QwtTransform* transformation() const;

  protected:
    bool contains( const QwtInterval&, double value ) const;
    QList< double > strip( const QList< double >&, const QwtInterval& ) const;

    double divideInterval( double intervalSize, int numSteps ) const;

    QwtInterval buildInterval( double value ) const;

  private:
    Q_DISABLE_COPY(QwtScaleEngine)

    class PrivateData;
    PrivateData* m_data;
};

/*!
   \brief A scale engine for linear scales

   The step size will fit into the pattern
   \f$\left\{ 1,2,5\right\} \cdot 10^{n}\f$, where n is an integer.
 */

class QWT_EXPORT QwtLinearScaleEngine : public QwtScaleEngine
{
  public:
    explicit QwtLinearScaleEngine( uint base = 10 );
    virtual ~QwtLinearScaleEngine();

    virtual void autoScale( int maxNumSteps,
        double& x1, double& x2, double& stepSize ) const QWT_OVERRIDE;

    virtual QwtScaleDiv divideScale( double x1, double x2,
        int maxMajorSteps, int maxMinorSteps,
        double stepSize = 0.0 ) const QWT_OVERRIDE;


  protected:
    QwtInterval align( const QwtInterval&, double stepSize ) const;

    void buildTicks(
        const QwtInterval&, double stepSize, int maxMinorSteps,
        QList< double > ticks[QwtScaleDiv::NTickTypes] ) const;

    QList< double > buildMajorTicks(
        const QwtInterval& interval, double stepSize ) const;

    void buildMinorTicks( const QList< double >& majorTicks,
        int maxMinorSteps, double stepSize,
        QList< double >& minorTicks, QList< double >& mediumTicks ) const;
};

/*!
   \brief A scale engine for logarithmic scales

   The step size is measured in *decades*
   and the major step size will be adjusted to fit the pattern
   \f$\left\{ 1,2,3,5\right\} \cdot 10^{n}\f$, where n is a natural number
   including zero.

   \warning the step size as well as the margins are measured in *decades*.
 */

class QWT_EXPORT QwtLogScaleEngine : public QwtScaleEngine
{
  public:
    explicit QwtLogScaleEngine( uint base = 10 );
    virtual ~QwtLogScaleEngine();

    virtual void autoScale( int maxNumSteps,
        double& x1, double& x2, double& stepSize ) const QWT_OVERRIDE;

    virtual QwtScaleDiv divideScale( double x1, double x2,
        int maxMajorSteps, int maxMinorSteps,
        double stepSize = 0.0 ) const QWT_OVERRIDE;

  protected:
    QwtInterval align( const QwtInterval&, double stepSize ) const;

    void buildTicks(
        const QwtInterval&, double stepSize, int maxMinorSteps,
        QList< double > ticks[QwtScaleDiv::NTickTypes] ) const;

    QList< double > buildMajorTicks(
        const QwtInterval& interval, double stepSize ) const;

    void buildMinorTicks( const QList< double >& majorTicks,
        int maxMinorSteps, double stepSize,
        QList< double >& minorTicks, QList< double >& mediumTicks ) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtScaleEngine::Attributes )

#endif
