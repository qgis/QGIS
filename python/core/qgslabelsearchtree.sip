class QgsLabelSearchTree
{
%TypeHeaderCode
#include <qgslabelsearchtree.h>
%End

  public:
    QgsLabelSearchTree();
    ~QgsLabelSearchTree();

    /**Removes and deletes all the entries*/
    void clear();

    /**Returns label position(s) at a given point. QgsLabelSearchTree keeps ownership, don't delete the LabelPositions*/
    // void label( const QgsPoint& p, QList<QgsLabelPosition*>& posList );

    /**Returns label position(s) in given rectangle. QgsLabelSearchTree keeps ownership, don't delete the LabelPositions*/
    // void labelsInRect( const QgsRectangle& r, QList<QgsLabelPosition*>& posList );

    /**Inserts label position. Does not take ownership of labelPos
      @return true in case of success*/
    // bool insertLabel( LabelPosition* labelPos, int featureId, const QString& layerName, bool diagram = false, bool pinned = false );
};
