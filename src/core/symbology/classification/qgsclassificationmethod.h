/***************************************************************************
    qgsclassificationmethod.h
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLASSIFICATIONMETHOD_H
#define QGSCLASSIFICATIONMETHOD_H

#include "qgis_core.h"
#include "qgsprocessingparameters.h"

class QgsVectorLayer;
class QgsRendererRange;


#ifdef SIP_RUN
// This is required for the ConvertToSubClassCode to work properly
// so RTTI for casting is available in the whole module.
% ModuleCode
#include "qgsclassificationequalinterval.h"
#include "qgsclassificationjenks.h"
#include "qgsclassificationprettybreaks.h"
#include "qgsclassificationquantile.h"
#include "qgsclassificationstandarddeviation.h"
% End
#endif



/**
 * \ingroup core
 * QgsClassificationRange contains the information about a classification range
 */
class CORE_EXPORT QgsClassificationRange
{
  public:
    //! Constructor
    QgsClassificationRange( const QString &label, const double &lowerBound, const double &upperBound )
      : mLabel( label )
      , mLowerBound( lowerBound )
      , mUpperBound( upperBound )
    {}
    //! Returns the lower bound
    double lowerBound() const {return mLowerBound;}
    //! Returns the upper bound
    double upperBound() const {return mUpperBound;}

    //! Returns the lower bound
    QString label() const {return mLabel;}

  private:
    QString mLabel;
    double mLowerBound;
    double mUpperBound;
};



/**
 * \ingroup core
 * QgsClassification is an abstract class for implementations of classification methods
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsClassificationMethod SIP_ABSTRACT
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsClassificationEqualInterval *>( sipCpp ) )
      sipType = sipType_QgsClassificationEqualInterval;
    else if ( dynamic_cast<QgsClassificationJenks *>( sipCpp ) )
      sipType = sipType_QgsClassificationJenks;
    else if ( dynamic_cast<QgsClassificationPrettyBreaks *>( sipCpp ) )
      sipType = sipType_QgsClassificationPrettyBreaks;
    else if ( dynamic_cast<QgsClassificationQuantile *>( sipCpp ) )
      sipType = sipType_QgsClassificationQuantile;
    else if ( dynamic_cast<QgsClassificationStandardDeviation *>( sipCpp ) )
      sipType = sipType_QgsClassificationStandardDeviation;
    else
      sipType = 0;
    SIP_END
#endif

  public:

    //! Defines the class position
    enum ClassPosition
    {
      LowerBound,
      Inner,
      UpperBound
    };

    /**
      * Creates a classification method.
      * \param valuesRequired if TRUE, this means that the method requires a set of values to determine the classes
      * \param symmetricModeAvailable if TRUE, this allows using symmetric classification
      * \param codeCommplexity as the exponent in the big O notation
      * \param
      */
    explicit QgsClassificationMethod( bool valuesRequired, bool symmetricModeAvailable, int codeComplexity = 1 );

    virtual ~QgsClassificationMethod() = default;

    virtual QgsClassificationMethod *clone() const = 0 SIP_FACTORY;

    //! The readable and translate name of the method
    virtual QString name() const = 0;

    //! The id of the method
    virtual QString id() const = 0; // as saved in the project, must be unique in registry

    /**
     * Returns the label for a range
     */
    virtual QString labelForRange( const double &lowerValue, const double &upperValue, ClassPosition position = Inner ) const;


    //! Writes extra information about the method
    virtual void saveExtra( QDomElement &element, const QgsReadWriteContext &context ) const {Q_UNUSED( element ); Q_UNUSED( context )}
    //! Reads extra information to apply it to the method
    virtual void readExtra( const QDomElement &element, const QgsReadWriteContext &context ) {Q_UNUSED( element ); Q_UNUSED( context )}

    // *******************
    // non-virtual methods

    /**
     * Returns if the method requires values to calculate the classes
     * If not, bounds are sufficient
     */
    bool valuesRequired() const {return mValuesRequired;}

    //! Code complexity as the exponent in Big O notation
    int codeComplexity() const {return mCodeComplexity;}

    /**
     * Returns if the method supports symmetric calculation
     */
    bool symmetricModeAvailable() const {return mSymmetricModeAvailable;}

    /**
     * Returns if the symmetric mode is enabled
     */
    bool symmetricModeEnabled() const {return mSymmetricModeAvailable && mSymmetricEnabled;}

    /**
     * Returns the symmetry point for symmetric mode
     */
    double symmetryPoint() const {return mSymmetryPoint;}

    /**
     * Returns if the symmetric mode is astride
     * if TRUE, it will remove the symmetry point break so that the 2 classes form only one
     */
    bool astride() const {return mAstride;}

    /**
     * Defines if the symmetric mode is enables and configures its parameters.
     * If the symmetric mode is not available in the current implementation, calling this method has no effect.
     * \param enabled if the symmetric mode is enabled
     * \param symmetryPoint the value of the symmetry point
     * \param astride if TRUE, it will remove the symmetry point break so that the 2 classes form only one
     */
    void setSymmetricMode( bool enabled, double symmetryPoint = 0, bool astride = false );

    // Label properties
    //! Returns the format of the label for the classes
    QString labelFormat() const { return mLabelFormat; }
    //! Defines the format of the labels for the classes, using %1 and %2 for the bounds
    void setLabelFormat( const QString &format ) { mLabelFormat = format; }
    //! Returns the precision for the formatting of the labels
    int labelPrecision() const { return mLabelPrecision; }
    //! Defines the precision for the formatting of the labels
    void setLabelPrecision( int labelPrecision );
    //! Returns if the trailing 0 are trimmed in the label
    bool labelTrimTrailingZeroes() const { return mLabelTrimTrailingZeroes; }
    //! Defines if the trailing 0 are trimmed in the label
    void setLabelTrimTrailingZeroes( bool trimTrailingZeroes ) { mLabelTrimTrailingZeroes = trimTrailingZeroes; }

    //! Transforms a list of classes to a list of breaks
    static QList<double> listToValues( const QList<QgsClassificationRange> classes );

    /**
     * This will calculate the breaks for a given layer to define the classes.
     * The breaks do not contain the uppper and lower bounds (minimum and maximum values).
     * \param vl The vector layer
     * \param fieldName The name of the field on which the classes are calculated
     * \param numberOfClasses The number of classes to be returned
     */
    QList<QgsClassificationRange> classes( const QgsVectorLayer *vl, const QString &expression, int numberOfClasses );

    /**
     * This will calculate the breaks for a list of values.
     * The breaks do not contain the uppper and lower bounds (minimum and maximum values)
     * \param values The list of values
     * \param numberOfClasses The number of classes to be returned
     */
    QList<QgsClassificationRange> classes( const QList<double> &values, int numberOfClasses );

    /**
     * This will calculate the classes for defined bounds without any values.
     * The breaks do not contain the uppper and lower bounds (minimum and maximum values)
     * \warning If the method implementation requires values, this will return an empty list.
     * \param values The list of values
     * \param numberOfClasses The number of classes to be returned
     */
    QList<QgsClassificationRange> classes( double minimum, double maximum, int numberOfClasses );

    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const;
    static QgsClassificationMethod *create( const QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Remove the breaks that are above the existing opposite sign classes to keep colors symmetrically balanced around symmetryPoint
     * Does not put a break on the symmetryPoint. This is done before.
     * \param breaks The breaks of an already-done classification
     * \param symmetryPoint The point around which we want a symmetry
     * \param astride A bool indicating if the symmetry is made astride the symmetryPoint or not ( [-1,1] vs. [-1,0][0,1] )
     */
    static void makeBreaksSymmetric( QList<double> &breaks SIP_INOUT, double symmetryPoint, bool astride );

    /**
     * Returns the label for a range
     */
    QString labelForRange( const QgsRendererRange &range, ClassPosition position = Inner ) const;



    static const int MAX_PRECISION;
    static const int MIN_PRECISION;

  protected:

    //! Copy the parameters (shall be used in clone implementation)
    void copyBase( QgsClassificationMethod *c ) const;

    //! Format the number according to label properties
    QString formatNumber( double value ) const;

    // parameters (set by setters)
    // if some are added here, they should be handled in the clone method
    bool mSymmetricEnabled = false;
    double mSymmetryPoint = 0;
    bool mAstride = false;
    QString mLabelFormat;
    int mLabelPrecision = 4;
    bool mLabelTrimTrailingZeroes = true;


  private:

    /**
     * Calculate the breaks, should be reimplemented, values might be an empty list
     * If the symmetric mode is available, the implementation is responsible of applying the symmetry
     * The maximum value is expected to be added at the end of the list, but not the minimum
     */
    virtual QList<double> calculateBreaks( double minimum, double maximum,
                                           const QList<double> &values, int numberOfClasses ) = 0;

    //! This is called after calculating the breaks or restoring from XML, so it can rely on private variables
    virtual QString valueToLabel( const double &value ) const {return formatNumber( value );}


    //! Create a list of ranges from a list of classes
    QList<QgsClassificationRange> breaksToClasses( const QList<double> &breaks ) const;

    // implementation properties (set by initialization)
    bool mValuesRequired; // if all values are required to calculate breaks
    bool mSymmetricModeAvailable;
    int mCodeComplexity;

    // values used to manage number formatting - precision and trailing zeroes
    double mLabelNumberScale = 1.0;
    QString mLabelNumberSuffix;
};

#endif // QGSCLASSIFICATIONMETHOD_H
