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

#include <QIcon>

#include "qgis_sip.h"
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
#include "qgsclassificationfixedinterval.h"
% End
#endif



/**
 * \ingroup core
 * \brief QgsClassificationRange contains the information about a classification range
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsClassificationRange
{
  public:
    //! Constructor
    QgsClassificationRange( const QString &label, double lowerBound, double upperBound )
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

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsClassificationRange: '%1'>" ).arg( sipCpp->label() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:
    QString mLabel;
    double mLowerBound;
    double mUpperBound;
};



/**
 * \ingroup core
 * \brief QgsClassificationMethod is an abstract class for implementations of classification methods
 * \see QgsClassificationMethodRegistry
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
    else if ( dynamic_cast<QgsClassificationFixedInterval *>( sipCpp ) )
      sipType = sipType_QgsClassificationFixedInterval;
    else
      sipType = 0;
    SIP_END
#endif

  public:

    //! Flags for the classification method
    enum MethodProperty
    {
      NoFlag                 = 0,       //!< No flag
      ValuesNotRequired      = 1 << 1,  //!< Deprecated since QGIS 3.12
      SymmetricModeAvailable = 1 << 2,  //!< This allows using symmetric classification
      IgnoresClassCount      = 1 << 3,  //!< The classification method does not compute classes based on a class count (since QGIS 3.26)
    };
    Q_DECLARE_FLAGS( MethodProperties, MethodProperty )


    //! Defines the class position
    enum ClassPosition
    {
      LowerBound, //!< The class is at the lower bound
      Inner,      //!< The class is not at a bound
      UpperBound  //!< The class is at the upper bound
    };

    /**
      * Creates a classification method.
      * \param properties The properties of the implemented method
      * \param codeComplexity as the exponent in the big O notation
      */
    explicit QgsClassificationMethod( MethodProperties properties = NoFlag, int codeComplexity = 1 );

    virtual ~QgsClassificationMethod();

    /**
     * Returns a clone of the method.
     * Implementation can take advantage of copyBase method which copies the parameters of the base class
     */
    virtual QgsClassificationMethod *clone() const = 0 SIP_FACTORY;

    //! The readable and translate name of the method
    virtual QString name() const = 0;

    //! The id of the method as saved in the project, must be unique in registry
    virtual QString id() const = 0;

    //! The icon of the method
    virtual QIcon icon() const {return QIcon();}

    /**
     * Returns the classification flags.
     *
     * \since QGIS 3.26
     */
    QgsClassificationMethod::MethodProperties flags() const { return mFlags; }

    /**
     * Returns the label for a range
     */
    virtual QString labelForRange( double lowerValue, double upperValue, ClassPosition position = Inner ) const;


    //! Writes extra information about the method
    virtual void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const {Q_UNUSED( element ); Q_UNUSED( context )}
    //! Reads extra information to apply it to the method
    virtual void readXml( const QDomElement &element, const QgsReadWriteContext &context ) {Q_UNUSED( element ); Q_UNUSED( context )}

    /**
     * Returns if the method requires values to calculate the classes
     * If not, bounds are sufficient
     */
    virtual bool valuesRequired() const {return true;}


    // *******************
    // non-virtual methods

    //! Code complexity as the exponent in Big O notation
    int codeComplexity() const {return mCodeComplexity;}

    /**
     * Returns if the method supports symmetric calculation
     */
    bool symmetricModeAvailable() const {return mFlags.testFlag( SymmetricModeAvailable );}

    /**
     * Returns if the symmetric mode is enabled
     */
    bool symmetricModeEnabled() const {return symmetricModeAvailable() && mSymmetricEnabled;}

    /**
     * Returns the symmetry point for symmetric mode
     */
    double symmetryPoint() const {return mSymmetryPoint;}

    /**
     * Returns if the symmetric mode is astride
     * if TRUE, it will remove the symmetry point break so that the 2 classes form only one
     */
    bool symmetryAstride() const {return mSymmetryAstride;}

    /**
     * Defines if the symmetric mode is enables and configures its parameters.
     * If the symmetric mode is not available in the current implementation, calling this method has no effect.
     * \param enabled if the symmetric mode is enabled
     * \param symmetryPoint the value of the symmetry point
     * \param symmetryAstride if TRUE, it will remove the symmetry point break so that the 2 classes form only one
     */
    void setSymmetricMode( bool enabled, double symmetryPoint = 0, bool symmetryAstride = false );

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
    static QList<double> rangesToBreaks( const QList<QgsClassificationRange> &classes );

    /**
     * This will calculate the classes for a given layer to define the classes.
     * \param layer The vector layer
     * \param expression The name of the field on which the classes are calculated
     * \param nclasses The number of classes to be returned
     */
    QList<QgsClassificationRange> classes( const QgsVectorLayer *layer, const QString &expression, int nclasses );

    /**
     * This will calculate the classes for a list of values.
     * \param values The list of values
     * \param nclasses The number of classes to be returned
     */
    QList<QgsClassificationRange> classes( const QList<double> &values, int nclasses );

    /**
     * This will calculate the classes for defined bounds without any values.
     * \warning If the method implementation requires values, this will return an empty list.
     * \param minimum The minimum value for the breaks
     * \param maximum The maximum value for the breaks
     * \param nclasses The number of classes to be returned
     */
    QList<QgsClassificationRange> classes( double minimum, double maximum, int nclasses );

    /**
     * Saves the method to a DOM element and return it
     * \param doc the DOM document
     * \param context the read/write context
     */
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const;

    /**
     * Reads the DOM element and return a new classification method from it
     * \param element the DOM element
     * \param context the read/write context
     */
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

    /**
     * Returns the parameter from its name
     * \since QGIS 3.12
     */
    const QgsProcessingParameterDefinition *parameterDefinition( const QString &parameterName ) const;

    /**
     * Returns the list of parameters
     * \since QGIS 3.12
     */
    QgsProcessingParameterDefinitions parameterDefinitions() const {return mParameters;}

    /**
     * Defines the values of the additional parameters
     * \since QGIS 3.12
     */
    void setParameterValues( const QVariantMap &values );

    /**
     * Returns the values of the processing parameters.
     * One could use QgsProcessingParameters::parameterAsXxxx to retrieve the actual value of a parameter.
     * \since QGIS 3.12
     */
    QVariantMap parameterValues() const {return mParameterValues;}

    static const int MAX_PRECISION;
    static const int MIN_PRECISION;

  protected:

    //! Copy the parameters (shall be used in clone implementation)
    void copyBase( QgsClassificationMethod *c ) const;

    //! Format the number according to label properties
    QString formatNumber( double value ) const;

    /**
     * Add a parameter to the method.
     * The paramaeter is a processing parameter which will allow its configuration in the GUI.
     * \note Only parameters having their widget implementation in C++ are supported. i.e. pure
     * Python parameters are not supported.
     * \since QGIS 3.12
     */
    void addParameter( QgsProcessingParameterDefinition *definition SIP_TRANSFER );

  private:

    /**
     * Calculate the breaks, should be reimplemented, values might be an empty list if they are not required
     * If the symmetric mode is available, the implementation is responsible of applying the symmetry
     * The maximum value is expected to be added at the end of the list, but not the minimum
     */
    virtual QList<double> calculateBreaks( double &minimum, double &maximum,
                                           const QList<double> &values, int nclasses ) = 0;

    //! This is called after calculating the breaks or restoring from XML, so it can rely on private variables
    virtual QString valueToLabel( double value ) const {return formatNumber( value );}

    //! Create a list of ranges from a list of classes
    QList<QgsClassificationRange> breaksToClasses( const QList<double> &breaks ) const;

    // implementation properties (set by initialization)
    MethodProperties mFlags = MethodProperties();
    int mCodeComplexity = 1;

    // parameters (set by setters)
    // if some are added here, they should be handled in the clone method
    bool mSymmetricEnabled = false;
    double mSymmetryPoint = 0;
    bool mSymmetryAstride = false;
    int mLabelPrecision = 4;
    bool mLabelTrimTrailingZeroes = true;
    QString mLabelFormat;

    // values used to manage number formatting - precision and trailing zeroes
    double mLabelNumberScale = 1.0;
    QString mLabelNumberSuffix;

    //! additional parameters
    QgsProcessingParameterDefinitions mParameters;
    QVariantMap mParameterValues;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsClassificationMethod::MethodProperties )

#endif // QGSCLASSIFICATIONMETHOD_H
