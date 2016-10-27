/** \ingroup core
Undo command to undo/redo all composer item related changes*/
class QgsComposerItemCommand: QUndoCommand
{
%TypeHeaderCode
  #include "qgscomposeritemcommand.h"
%End
  public:
    QgsComposerItemCommand( QgsComposerItem* item, const QString& text, QUndoCommand* parent /TransferThis/ = 0 );
    virtual ~QgsComposerItemCommand();

    /** Reverses the command*/
    void undo();
    /** Replays the command*/
    void redo();

    /** Saves current item state as previous state*/
    void savePreviousState();
    /** Saves current item state as after state*/
    void saveAfterState();

    QDomDocument previousState() const;
    QDomDocument afterState() const;

    /** Returns true if previous state and after state are valid and different*/
    bool containsChange() const;

    /** Returns the target item the command applies to.
     * @returns target composer item
     */
    QgsComposerItem *item() const;

  protected:
    void saveState( QDomDocument& stateDoc ) const;
    void restoreState( QDomDocument& stateDoc ) const;
};

/** A composer command that merges together with other commands having the same context (=id). Keeps the oldest previous state and uses the
  newest after state. The purpose is to avoid too many micro changes in the history*/
class QgsComposerMergeCommand : QgsComposerItemCommand
{
%TypeHeaderCode
  #include "qgscomposeritemcommand.h"
%End
  public:
    enum Context
    {
      Unknown,
      //composer label
      ComposerLabelSetText,
      ComposerLabelSetId,
      ComposerLabelFontColor,
      //composer map
      ComposerMapRotation,
      ComposerMapAnnotationDistance,
      ComposerMapGridFramePenColor,
      ComposerMapGridFrameFill1Color,
      ComposerMapGridFrameFill2Color,
      ComposerMapGridAnnotationFontColor,
      //composer legend
      ComposerLegendText,
      LegendColumnCount,
      LegendSplitLayer,
      LegendEqualColumnWidth,
      LegendSymbolWidth,
      LegendSymbolHeight,
      LegendWmsLegendWidth,
      LegendWmsLegendHeight,
      LegendTitleSpaceBottom,
      LegendGroupSpace,
      LegendLayerSpace,
      LegendSymbolSpace,
      LegendIconSymbolSpace,
      LegendBoxSpace,
      LegendColumnSpace,
      LegendRasterBorderWidth,
      LegendFontColor,
      LegendRasterBorderColor,
      //composer picture
      ComposerPictureRotation,
      ComposerPictureFillColor,
      ComposerPictureOutlineColor,
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
      ScaleBarFontColor,
      ScaleBarFillColor,
      ScaleBarFill2Color,
      ScaleBarStrokeColor,
      // composer table
      TableMaximumFeatures,
      TableMargin,
      TableGridStrokeWidth,
      //composer shape
      ShapeCornerRadius,
      ShapeOutlineWidth,
      //composer arrow
      ArrowOutlineWidth,
      ArrowHeadFillColor,
      ArrowHeadOutlineColor,
      ArrowHeadWidth,
      //item
      ItemOutlineWidth,
      ItemOutlineColor,
      ItemBackgroundColor,
      ItemMove,
      ItemRotation,
      ItemTransparency,
      ItemZoomContent
    };

    QgsComposerMergeCommand( Context c, QgsComposerItem* item, const QString& text );
    ~QgsComposerMergeCommand();

    bool mergeWith( const QUndoCommand * command );
    int id() const;
};
