
class QgsRasterShaderFunction
{
%TypeHeaderCode
#include <qgsrastershaderfunction.h>
#include <qgscolorrampshader.h>
#include <qgsfreakoutshader.h>
#include <qgspseudocolorshader.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsColorRampShader*>(sipCpp) != NULL)
    sipClass = sipClass_QgsColorRampShader;
  else if (dynamic_cast<QgsFreakOutShader*>(sipCpp) != NULL)
    sipClass = sipClass_QgsFreakOutShader;
  else if (dynamic_cast<QgsPseudoColorShader*>(sipCpp) != NULL)
    sipClass = sipClass_QgsPseudoColorShader;
  else
    sipClass = 0;
%End


public:
  QgsRasterShaderFunction(double theMinimumValue = 0.0, double theMaximumValue = 255.0);
  virtual ~QgsRasterShaderFunction();
   
  /** \brief generates and new RGB value based on one input value */
  virtual bool shade(double, int* /Out/, int* /Out/, int* /Out/);
  /** \brief generates and new RGB value based on original RGB value */
  virtual bool shade(double, double, double, int* /Out/, int* /Out/, int* /Out/);
  
  /** \brief Set the maximum value */
  virtual void setMaximumValue(double);
  /** \brief Return the minimum value */
  virtual void setMinimumValue(double);
};

//////////////

class QgsColorRampShader : QgsRasterShaderFunction
{
%TypeHeaderCode
#include <qgscolorrampshader.h>
%End

  public:
    QgsColorRampShader( double theMinimumValue = 0.0, double theMaximumValue = 255.0 );

    //An entry for classification based upon value.
    //Such a classification is typically used for
    //single band layers where a pixel value represents
    //not a color but a quantity, e.g. temperature or elevation
    struct ColorRampItem
    {
      //! default constructor
      ColorRampItem();
      //! convenience constructor - added in v1.6
      ColorRampItem( double val, QColor col, QString lbl = QString() );
      
      QString label;
      double value;
      QColor color;

      // compare operator for sorting
      //bool operator<( const ColorRampItem& other ) const;
    };

    enum ColorRamp_TYPE
    {
      INTERPOLATED,
      DISCRETE,
      EXACT
    };

    /** \brief Get the custom colormap*/
    QList<QgsColorRampShader::ColorRampItem> colorRampItemList() const;

    /** \brief Get the color ramp type */
    QgsColorRampShader::ColorRamp_TYPE colorRampType();

    /** \brief Get the color ramp type as a string */
    QString colorRampTypeAsQString();

    /** \brief Get the maximum size the color cache can be*/
    int maximumColorCacheSize();

    /** \brief Set custom colormap */
    void setColorRampItemList( const QList<QgsColorRampShader::ColorRampItem>& theList );

    /** \brief Set the color ramp type*/
    void setColorRampType( QgsColorRampShader::ColorRamp_TYPE theColorRampType );

    /** \brief Set the color ramp type*/
    void setColorRampType( QString );

    /** \brief Set the maximum size the color cache can be */
    void setMaximumColorCacheSize( int theSize );

    /** \brief Generates and new RGB value based on one input value */
    bool shade( double, int* /Out/, int* /Out/, int* /Out/);

    /** \brief Generates and new RGB value based on original RGB value */
    bool shade( double, double, double, int* /Out/, int* /Out/, int* /Out/);

};

//////////////

class QgsFreakOutShader : QgsRasterShaderFunction
{
%TypeHeaderCode
#include <qgsfreakoutshader.h>
%End

  public:
    QgsFreakOutShader( double theMinimumValue = 0.0, double theMaximumValue = 255.0 );

    /** \brief generates and new RGB value based on one input value */
    bool shade( double, int* /Out/, int* /Out/, int* /Out/);

    /** \brief generates and new RGB value based on original RGB value */
    bool shade( double, double, double, int* /Out/, int* /Out/, int* /Out/);

    /** \brief Set the maximum value */
    void setMaximumValue( double );

    /** \brief Return the minimum value */
    void setMinimumValue( double );
};

//////////////

class QgsPseudoColorShader : QgsRasterShaderFunction
{
%TypeHeaderCode
#include <qgspseudocolorshader.h>
%End

  public:
    QgsPseudoColorShader( double theMinimumValue = 0.0, double theMaximumValue = 255.0 );

    /** \brief generates and new RGB value based on one input value */
    bool shade( double, int* /Out/, int* /Out/, int* /Out/);

    /** \brief generates and new RGB value based on original RGB value */
    bool shade( double, double, double, int* /Out/, int* /Out/, int* /Out/);

    /** \brief Set the maximum value */
    void setMaximumValue( double );

    /** \brief Return the minimum value */
    void setMinimumValue( double );

};
