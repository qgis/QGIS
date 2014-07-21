#ifndef QGSLEGENDRENDERER_H
#define QGSLEGENDRENDERER_H

#include <QPointF>

class QRectF;
class QStandardItem;

class QgsLegendModel;
class QgsComposerBaseSymbolItem;
class QgsComposerLegendItem;
class QgsComposerGroupItem;
class QgsComposerLayerItem;
class QgsSymbolV2;

#include "qgslegendsettings.h"

/**
 * @brief The QgsLegendRenderer class handles automatic layout and rendering of legend.
 * The content is given by QgsLegendModel instance. Various layout properties can be configured
 * within QgsLegendRenderer.
 *
 * All spacing and sizes are in millimeters.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsLegendRenderer
{
  public:
    /** Construct legend renderer. The ownership of legend model does not change */
    QgsLegendRenderer( QgsLegendModel* legendModel, const QgsLegendSettings& settings );

    /** Run the layout algorithm and determine the size required for legend */
    QSizeF minimumSize();

    /** Set the preferred resulting legend size. */
    void setLegendSize( QSizeF s ) { mLegendSize = s; }

    /** Find out preferred legend size set by the client. If null, the legend will be drawn with the minimum size */
    QSizeF legendSize() const { return mLegendSize; }

    /** Draw the legend with given painter. It will occupy the area reported in legendSize().
     *  Painter should be scaled beforehand so that units correspond to millimeters.
     */
    void drawLegend( QPainter* painter );

  private:

    /** Nucleon is either group title, layer title or layer child item.
     *  Nucleon is similar to QgsComposerLegendItem but it does not have
     *  the same hierarchy. E.g. layer title nucleon is just title, it does not
     *  include all layer subitems, the same with groups.
     */
    class Nucleon
    {
      public:
        QgsComposerLegendItem* item;
        // Symbol size size without any space around for symbol item
        QSizeF symbolSize;
        // Label size without any space around for symbol item
        QSizeF labelSize;
        QSizeF size;
        // Offset of symbol label, this offset is the same for all symbol labels
        // of the same layer in the same column
        double labelXOffset;
    };

    /** Atom is indivisible set (indivisible into more columns). It may consists
     *  of one or more Nucleon, depending on layer splitting mode:
     *  1) no layer split: [group_title ...] layer_title layer_item [layer_item ...]
     *  2) layer split:    [group_title ...] layer_title layer_item
     *              or:    layer_item
     *  It means that group titles must not be split from layer title and layer title
     *  must not be split from first item, because it would look bad and it would not
     *  be readable to leave group or layer title at the bottom of column.
     */
    class Atom
    {
      public:
        Atom(): size( QSizeF( 0, 0 ) ), column( 0 ) {}
        QList<Nucleon> nucleons;
        // Atom size including nucleons interspaces but without any space around atom.
        QSizeF size;
        int column;
    };

    QSizeF paintAndDetermineSize( QPainter* painter );

    /** Create list of atoms according to current layer splitting mode */
    QList<Atom> createAtomList( QStandardItem* rootItem, bool splitLayer );

    /** Divide atoms to columns and set columns on atoms */
    void setColumns( QList<Atom>& atomList );

    /** Draws title in the legend using the title font and the specified alignment
     * If no painter is specified, function returns the required width/height to draw the title.
     */
    QSizeF drawTitle( QPainter* painter = 0, QPointF point = QPointF(), Qt::AlignmentFlag halignment = Qt::AlignLeft, double legendWidth = 0 );

    double spaceAboveAtom( Atom atom );

    /** Draw atom and return its actual size, the atom is drawn with the space above it
     *  so that first atoms in column are all aligned to the same line regardles their
     * style top space */
    QSizeF drawAtom( Atom atom, QPainter* painter = 0, QPointF point = QPointF() );

    Nucleon drawSymbolItem( QgsComposerBaseSymbolItem* symbolItem, QPainter* painter = 0, QPointF point = QPointF(), double labelXOffset = 0 );

  private:
    QgsLegendModel* mLegendModel;

    QgsLegendSettings mSettings;

    QSizeF mLegendSize;

};

#endif // QGSLEGENDRENDERER_H
