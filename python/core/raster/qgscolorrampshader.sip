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
      bool operator<( const QgsColorRampShader::ColorRampItem& other ) const;
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
    QgsColorRampShader::ColorRamp_TYPE colorRampType() const;

    /** \brief Get the color ramp type as a string */
    QString colorRampTypeAsQString();

    /** \brief Get the maximum size the color cache can be*/
    int maximumColorCacheSize();

    /** \brief Set custom colormap */
    void setColorRampItemList( const QList<QgsColorRampShader::ColorRampItem>& theList ); //TODO: sort on set

    /** \brief Set the color ramp type*/
    void setColorRampType( QgsColorRampShader::ColorRamp_TYPE theColorRampType );

    /** \brief Set the color ramp type*/
    void setColorRampType( QString );

    /** \brief Set the maximum size the color cache can be */
    void setMaximumColorCacheSize( int theSize );

    /** \brief Generates and new RGB value based on one input value */
    bool shade( double, int* /Out/, int* /Out/, int* /Out/, int* /Out/ );

    /** \brief Generates and new RGB value based on original RGB value */
    bool shade( double, double, double, double, int* /Out/, int* /Out/, int* /Out/, int* /Out/);

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;
};
