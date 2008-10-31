
/** Render class to display labels */
class QgsLabel
{
%TypeHeaderCode
#include <qgslabel.h>
%End

  private:
    QgsLabel (); // pretend that constructor is private for now

public:

    QgsLabel ( const QMap<int, QgsField> & fields  );

    ~QgsLabel();

    /* Fields */
    enum LabelField {
  Text = 0,
  Family,
  Size,
  Bold,
  Italic,
  Underline,
  Color,
  XCoordinate,
  YCoordinate,
  XOffset,
  YOffset,
  Angle,
  Alignment,
  BufferEnabled,
  BufferSize,
  BufferColor,
  BufferBrush,
  BorderWidth,
  BorderColor,
  BorderStyle,
  MultilineEnabled,
  LabelFieldCount
};

    /** \brief render label
     *  \param sizeScale global scale factor for size in pixels, labels in map units are not scaled
    */
    void renderLabel ( QPainter* painter, QgsRect& viewExtent, 
                       QgsCoordinateTransform* coordinateTransform,
                       QgsMapToPixel *transform,
                       QgsFeature &feature, bool selected, QgsLabelAttributes *classAttributes=0, double sizeScale = 1, double rasterScaleFactor = 1);

    /** Reads the renderer configuration from an XML file
     @param rnode the Dom node to read 
    */
    void readXML(const QDomNode& node);

    /** Writes the contents of the renderer to a configuration file */
    // TODO: wrap void writeXML(std::ostream& xml);

    //! add vector of required fields to existing list of fields
    void addRequiredFields ( QList<int> & fields );

    //! Set available fields
    void setFields( const QMap<int, QgsField> & fields  );

    //! Available vector fields
    QMap<int, QgsField> & fields ( );

    //! Pointer to default attributes
    QgsLabelAttributes *layerAttributes ( );

    //! Set label field
    void setLabelField ( int attr, int fieldId );

    //! label field
    QString labelField ( int attr );

    /** Get field value if : 1) field name is not empty
     *                       2) field exists
     *                       3) value is defined
     *  otherwise returns empty string
    */
    QString fieldValue ( int attr, QgsFeature& feature );

};

