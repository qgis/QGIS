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
      //! convenience constructor
      ColorRampItem( double val, const QColor& col, const QString& lbl = QString() );

      QString label;
      double value;
      QColor color;

      // compare operator for sorting
      bool operator<( const QgsColorRampShader::ColorRampItem& other ) const;
    };

    /** Supported methods for color interpolation. */
    enum ColorRamp_TYPE
    {
      INTERPOLATED, //!< Interpolates the color between two class breaks linearly.
      DISCRETE,     //!< Assigns the color of the higher class for every pixel between two class breaks.
      EXACT         //!< Assigns the color of the exact matching value in the color ramp item list
    };

    /** \brief Get the custom colormap*/
    QList<QgsColorRampShader::ColorRampItem> colorRampItemList() const;

    /** \brief Get the color ramp type */
    QgsColorRampShader::ColorRamp_TYPE colorRampType() const;

    /** \brief Get the color ramp type as a string */
    QString colorRampTypeAsQString();

    /** \brief Get the maximum size the color cache can be
     * @deprecated will be removed in QGIS 3.0. Color cache is not used anymore.
     */
    int maximumColorCacheSize();

    /** \brief Set custom colormap */
    void setColorRampItemList( const QList<QgsColorRampShader::ColorRampItem>& theList ); //TODO: sort on set

    /** \brief Set the color ramp type*/
    void setColorRampType( QgsColorRampShader::ColorRamp_TYPE theColorRampType );

    /** \brief Set the color ramp type*/
    void setColorRampType( const QString& theType );

    /** \brief Set the maximum size the color cache can be
     * @deprecated will be removed in QGIS 3.0. Color cache is not used anymore.
     */
    void setMaximumColorCacheSize( int theSize );

    /** \brief Generates and new RGB value based on one input value */
    bool shade( double, int* /Out/, int* /Out/, int* /Out/, int* /Out/ );

    /** \brief Generates and new RGB value based on original RGB value */
    bool shade( double, double, double, double, int* /Out/, int* /Out/, int* /Out/, int* /Out/);

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;

    /** Sets whether the shader should not render values out of range.
     * @param clip set to true to clip values which are out of range.
     * @see clip()
     */
    void setClip( bool clip );

    /** Returns whether the shader will clip values which are out of range.
     * @see setClip()
     */
    bool clip() const;
};
