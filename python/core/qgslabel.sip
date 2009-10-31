
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

    struct labelpoint {
      QgsPoint p;
      double angle;
    };


    /** \brief render label
        \note deprecated
      */
    void renderLabel ( QPainter* painter, QgsRectangle& viewExtent, 
                       QgsCoordinateTransform* coordinateTransform,
                       QgsMapToPixel *transform,
                       QgsFeature &feature, bool selected, QgsLabelAttributes *classAttributes=0, double sizeScale = 1, double rasterScaleFactor = 1);
    
    /** \brief render label
     *  \param renderContext renderer context 
     *  \param feature feature to render
     *  \param selected is to be shown selected
     *  \param classAttributes attributes to use for labeling
     *  \note added in 1.2
    */
    void renderLabel ( QgsRenderContext &renderContext, QgsFeature &feature, bool selected, QgsLabelAttributes *classAttributes=0);

    /** Reads the renderer configuration from an XML file
     @param rnode the Dom node to read 
    */
    void readXML(const QDomNode& node);

    /** Writes the contents of the renderer to a configuration file */
    void writeXML( QDomNode & label_node, QDomDocument & document );

    //! add vector of required fields to existing list of fields
    void addRequiredFields ( QList<int> & fields );

    //! Set available fields
    void setFields( const QMap<int, QgsField> & fields  );

    //! Available vector fields
    QMap<int, QgsField> & fields ( );

    /** Pointer to default attributes.
     * @note this will be deprecated in version 2 as it is 
     * badly named. Rather use attributes.
     * @see labelAttributes method rather */
    QgsLabelAttributes *layerAttributes( void );

    /** Pointer to default attributes.
     * @note this replaces the to-be-deprecated layerAttributes method.
     * @note introduced in QGIS 1.4
     */
    QgsLabelAttributes *labelAttributes( void );

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


    /** Accessor and mutator for the minimum scale member */
    void setMinScale( float theMinScale );
    float minScale();

    /** Accessor and mutator for the maximum scale member */
    void setMaxScale( float theMaxScale );
    float maxScale();

    /** Accessor and mutator for the scale based visilibility flag */
    void setScaleBasedVisibility( bool theVisibilityFlag );
    bool scaleBasedVisibility();

};

