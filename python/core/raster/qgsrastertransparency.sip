
class QgsRasterTransparency
{
%TypeHeaderCode
#include <qgsrastertransparency.h>
%End

  public:
    QgsRasterTransparency();

    //
    // Structs to hold transparent pixel vlaues
    //
    struct TransparentThreeValuePixel
    {
      double red;
      double green;
      double blue;
      double percentTransparent;
    };

    struct TransparentSingleValuePixel
    {
      double min;
      double max;
      double percentTransparent;
    };

    //
    // Initializer, Accessor and mutator for transparency tables.
    //
    /** \brief Accessor for transparentSingleValuePixelList */
    QList<QgsRasterTransparency::TransparentSingleValuePixel> transparentSingleValuePixelList() const;

    /** \brief Accessor for transparentThreeValuePixelList */
    QList<QgsRasterTransparency::TransparentThreeValuePixel> transparentThreeValuePixelList() const;

    /** \brief Reset to the transparency list to a single value */
    void initializeTransparentPixelList( double );

    /** \brief Reset to the transparency list to a single value */
    void initializeTransparentPixelList( double, double, double );

    /** \brief Mutator for transparentSingleValuePixelList */
    void setTransparentSingleValuePixelList( const QList<QgsRasterTransparency::TransparentSingleValuePixel>& /Transfer/);

    /** \brief Mutator for transparentThreeValuePixelList */
    void setTransparentThreeValuePixelList( const QList<QgsRasterTransparency::TransparentThreeValuePixel>& /Transfer/);

    /** \brief Returns the transparency value for a single value Pixel */
    int alphaValue( double, int theGlobalTransparency = 255 ) const;

    /** \brief Return the transparency value for a RGB Pixel */
    int alphaValue( double, double, double, int theGlobalTransparency = 255 ) const;

    /** True if there are no entries in the pixel lists except the nodata value*/
    bool isEmpty() const;

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void readXML( const QDomElement& elem );
};
