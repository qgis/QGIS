
/** Render class to display labels */
class QgsLabel
{
%TypeHeaderCode
#include <qgslabel.h>
%End

  public:
    QgsLabel( const QgsFields & fields );

    ~QgsLabel();

    /* Fields */
    enum LabelField
    {
      Text = 0,
      Family,
      Size,
      SizeType,
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
      StrikeOut,     // added in 1.5
      LabelFieldCount
    };

    struct labelpoint
    {
      QgsPoint p;
      double angle;
    };

    /** \brief render label
     *  \param renderContext the render context
     *  \param feature feature to render the label for
     *  \param selected feature is selected
     *  \param classAttributes attributes to create the label from
     *  \note added in 1.2
     */
    void renderLabel( QgsRenderContext &renderContext, QgsFeature &feature, bool selected, QgsLabelAttributes *classAttributes = 0 );

    /** Reads the renderer configuration from an XML file
     @param node the Dom node to read
    */
    void readXML( const QDomNode& node );

    /** Writes the contents of the renderer to a configuration file */
    void writeXML( QDomNode & label_node, QDomDocument & document ) const;

    //! add vector of required fields to existing list of fields
    void addRequiredFields( QList<int> & fields ) const;

    //! Set available fields
    void setFields( const QgsFields & fields );

    //! Available vector fields
    QgsFields & fields();

    /** Pointer to default attributes.
     * @note this replaces the to-be-deprecated layerAttributes method.
     * @note introduced in QGIS 1.4
     */
    QgsLabelAttributes *labelAttributes();

    //! Set label field
    void setLabelField( int attr, int fieldIndex );

    //! label field
    QString labelField( int attr ) const;

    /** Get field value if : 1) field name is not empty
     *                       2) field exists
     *                       3) value is defined
     *  otherwise returns empty string
    */
    QString fieldValue( int attr, QgsFeature& feature );

    /** Accessor and mutator for the minimum scale member */
    void setMinScale( float theMinScale );
    float minScale() const;

    /** Accessor and mutator for the maximum scale member */
    void setMaxScale( float theMaxScale );
    float maxScale() const;

    /** Accessor and mutator for the scale based visilibility flag */
    void setScaleBasedVisibility( bool theVisibilityFlag );
    bool scaleBasedVisibility() const;

  private:
    QgsLabel (); // pretend that constructor is private for now
};
