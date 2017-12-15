/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposeritemcommand.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsComposerItemCommand: QUndoCommand
{
%Docstring
Undo command to undo/redo all composer item related changes
%End

%TypeHeaderCode
#include "qgscomposeritemcommand.h"
%End
  public:
    QgsComposerItemCommand( QgsComposerItem *item, const QString &text, QUndoCommand *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsComposerItemCommand
%End

    virtual void undo();

%Docstring
Reverses the command
%End
    virtual void redo();

%Docstring
Replays the command
%End

    void savePreviousState();
%Docstring
Saves current item state as previous state
%End
    void saveAfterState();
%Docstring
Saves current item state as after state
%End

    QDomDocument previousState() const;
%Docstring

:rtype: QDomDocument
%End
    QDomDocument afterState() const;
%Docstring

:rtype: QDomDocument
%End

    bool containsChange() const;
%Docstring
Returns true if previous state and after state are valid and different

:rtype: bool
%End

    QgsComposerItem *item() const;
%Docstring
Returns the target item the command applies to.

:return: target composer item
:rtype: QgsComposerItem

%End

  protected:



    void saveState( QDomDocument &stateDoc ) const;
%Docstring
Flag to prevent the first redo() if the command is pushed to the undo stack
%End
    void restoreState( QDomDocument &stateDoc ) const;
};

class QgsComposerMergeCommand: QgsComposerItemCommand
{
%Docstring
A composer command that merges together with other commands having the same context (=id). Keeps the oldest previous state and uses the
newest after state. The purpose is to avoid too many micro changes in the history
%End

%TypeHeaderCode
#include "qgscomposeritemcommand.h"
%End
  public:
    enum Context
    {
      Unknown,
      //composer
      ComposerLabelSetText,
      ComposerLabelSetId,
      ComposerLabelFontColor,
      //composer
      ComposerMapRotation,
      ComposerMapAnnotationDistance,
      ComposerMapGridFramePenColor,
      ComposerMapGridFrameFill1Color,
      ComposerMapGridFrameFill2Color,
      ComposerMapGridAnnotationFontColor,
      //composer
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
      LegendLineSpacing,
      LegendRasterStrokeWidth,
      LegendFontColor,
      LegendRasterStrokeColor,
      //composer
      ComposerPictureRotation,
      ComposerPictureFillColor,
      ComposerPictureStrokeColor,
      ComposerPictureNorthOffset,
      // composer
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
      // composer
      TableMaximumFeatures,
      TableMargin,
      TableGridStrokeWidth,
      //composer
      ShapeCornerRadius,
      ShapeStrokeWidth,
      //composer
      ArrowStrokeWidth,
      ArrowHeadFillColor,
      ArrowHeadStrokeColor,
      ArrowHeadWidth,
      //item
      ItemStrokeWidth,
      ItemStrokeColor,
      ItemBackgroundColor,
      ItemMove,
      ItemRotation,
      ItemOpacity,
      ItemZoomContent
    };

    QgsComposerMergeCommand( Context c, QgsComposerItem *item, const QString &text );

    virtual bool mergeWith( const QUndoCommand *command );

    virtual int id() const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposeritemcommand.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
