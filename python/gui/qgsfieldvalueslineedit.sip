/** \class QgsFieldValuesLineEdit
 * \ingroup gui
 * A line edit with an autocompleter which takes unique values from a vector layer's fields.
 * The autocompleter is populated from the vector layer in the background to ensure responsive
 * interaction with the widget.
 * \note added in QGIS 3.0
 */
class QgsFieldValuesLineEdit: QgsFilterLineEdit
{
%TypeHeaderCode
#include <qgsfieldvalueslineedit.h>
%End
  public:

    /** Constructor for QgsFieldValuesLineEdit
     * @param parent parent widget
     */
    QgsFieldValuesLineEdit( QWidget *parent /TransferThis/ = nullptr );

    virtual ~QgsFieldValuesLineEdit();

    /** Sets the layer containing the field that values will be shown from.
     * @param layer vector layer
     * @see layer()
     * @see setAttributeIndex()
     */
    void setLayer( QgsVectorLayer* layer );

    /** Returns the layer containing the field that values will be shown from.
     * @see setLayer()
     * @see attributeIndex()
     */
    QgsVectorLayer* layer() const;

    /** Sets the attribute index for the field containing values to show in the widget.
     * @param index index of attribute
     * @see attributeIndex()
     * @see setLayer()
     */
    void setAttributeIndex( int index );

    /** Returns the attribute index for the field containing values shown in the widget.
     * @see setAttributeIndex()
     * @see layer()
     */
    int attributeIndex() const;

  signals:

    /** Emitted when the layer associated with the widget changes.
     * @param layer vector layer
     */
    void layerChanged( QgsVectorLayer* layer );

    /** Emitted when the field associated with the widget changes.
     * @param index new attribute index for field
     */
    void attributeIndexChanged( int index );
};
