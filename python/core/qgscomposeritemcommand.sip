/**\ingroup MapComposer
Undo command to undo/redo all composer item related changes*/
class QgsComposerItemCommand: QUndoCommand
{
%TypeHeaderCode
  #include "qgscomposeritemcommand.h"
%End
  public:
    QgsComposerItemCommand( QgsComposerItem* item, const QString& text, QUndoCommand* parent = 0 );
    virtual ~QgsComposerItemCommand();

    /**Reverses the command*/
    void undo();
    /**Replays the command*/
    void redo();

    /**Saves current item state as previous state*/
    void savePreviousState();
    /**Saves current item state as after state*/
    void saveAfterState();

    QDomDocument previousState() const;
    QDomDocument afterState() const;

    /**Returns true if previous state and after state are valid and different*/
    bool containsChange() const;
};

/**\ingroup MapComposer
A composer command that merges together with other commands having the same context (=id). Keeps the oldest previous state and uses the
  newest after state. The purpose is to avoid too many micro changes in the history*/
class QgsComposerMergeCommand: QgsComposerItemCommand
{
%TypeHeaderCode
  #include "qgscomposeritemcommand.h"
%End
  public:
    enum Context
    {
      Unknown = 0,
      //composer label
      ComposerLabelSetText,
      //composer map
      ComposerMapRotation,
      ComposerMapAnnotationDistance,
      //composer legend
      ComposerLegendText,
      LegendSymbolWidth,
      LegendSymbolHeight,
      LegendLayerSpace,
      LegendSymbolSpace,
      LegendIconSymbolSpace,
      LegendBoxSpace,
      //composer picture
      ComposerPictureRotation,
      // composer scalebar
      ScaleBarLineWidth,
      ScaleBarHeight,
      ScaleBarSegmentSize,
      ScaleBarSegmentsLeft,
      ScaleBarNSegments,
      ScaleBarUnitText,
      ScaleBarMapUnitsSegment,
      ScaleBarLabelBarSize,
      ScaleBarBoxContentSpace,
      // composer table
      TableMaximumFeatures,
      TableMargin,
      TableGridStrokeWidth,
      //composer shape
      ShapeRotation,
      ShapeOutlineWidth,
      //composer arrow
      ArrowOutlineWidth,
      ArrowHeadWidth,
      //item
      ItemOutlineWidth
    };

    QgsComposerMergeCommand( Context c, QgsComposerItem* item, const QString& text );
    ~QgsComposerMergeCommand();

    bool mergeWith( const QUndoCommand * command );
    int id() const;
 };
