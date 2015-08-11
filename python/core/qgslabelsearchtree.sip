class QgsLabelSearchTree
{
%TypeHeaderCode
#include <qgslabelsearchtree.h>
%End

  public:
    QgsLabelSearchTree();
    ~QgsLabelSearchTree();

    /** Removes and deletes all the entries*/
    void clear();

    /** Returns label position(s) at a given point. QgsLabelSearchTree keeps ownership, don't delete the LabelPositions
     * @note not available in python bindings
     * TODO: why does this break bindings with QList<QgsLabelPosition>?
     */
    // void label( const QgsPoint& p, QList<QgsLabelPosition*>& posList ) const;

    /** Returns label position(s) in given rectangle. QgsLabelSearchTree keeps ownership, don't delete the LabelPositions
     * @note not available in python bindings
     * TODO: why does this break bindings with QList<QgsLabelPosition>?
     */
    // void labelsInRect( const QgsRectangle& r, QList<QgsLabelPosition*>& posList ) const;

    /** Inserts label position. Does not take ownership of labelPos
     * @return true in case of success
     * @note not available in python bindings
     */
    // bool insertLabel( LabelPosition* labelPos, int featureId, const QString& layerName, bool diagram = false, bool pinned = false );
};
