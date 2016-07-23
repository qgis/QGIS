/** \ingroup core
 * \class QgsAnnotation
 * \note added in QGIS 3.0
 *
 *  An interface for annotation type items.
 */

class QgsAnnotation
{
%TypeHeaderCode
#include <qgsannotation.h>
%End
  public:

    //! Returns true if the annotation should be shown.
    virtual bool showItem() const = 0;

    /** Returns true if the annotation is attached to a fixed map position, or
     * false if the annotation uses a position relative to the current map
     * extent.
     * @see mapPosition()
     * @see relativePositon()
     */
    //TODO QGIS 3 - rename to hasFixedMapPosition()
    virtual bool mapPositionFixed() const = 0;

    /** Returns the map position of the annotation, if it is attached to a fixed map
     * position.
     * @see mapPositionFixed()
     * @see mapPositionCrs()
     */
    virtual QgsPoint mapPosition() const;

    /** Returns the CRS of the map position, or an invalid CRS if the annotation does
     * not have a fixed map position.
    */
    virtual QgsCoordinateReferenceSystem mapPositionCrs() const;

    /** Returns the relative position of the annotation, if it is not attached to a fixed map
     * position. The coordinates in the return point should be between 0 and 1, and represent
     * the relative percentage for the position compared to the map width and height.
     * @see mapPositionFixed()
     */
    virtual QPointF relativePosition() const;

    /** Returns a scaling factor which should be applied to painters before rendering
     * the item.
     */
    virtual double scaleFactor() const = 0;

    //! deprecated - do not use
    // TODO QGIS 3.0 - remove
    virtual void setItemData( int role, const QVariant& value ) = 0;

    //! Paint the annotation to a destination painter
    virtual void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = nullptr ) = 0;

};
