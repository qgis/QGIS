/***************************************************************************
                          qgscomposeritemcommand.h
                          ------------------------
    begin                : 2010-11-18
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERITEMCOMMAND_H
#define QGSCOMPOSERITEMCOMMAND_H

#include <QUndoCommand>
#include "qgis.h"
#include <QDomDocument>

#include "qgis_core.h"

class QgsComposerItem;
class QgsComposerMultiFrame;

/**
 * \ingroup core
 * Undo command to undo/redo all composer item related changes
*/
class CORE_EXPORT QgsComposerItemCommand: public QUndoCommand
{
  public:
    QgsComposerItemCommand( QgsComposerItem *item, const QString &text, QUndoCommand *parent SIP_TRANSFERTHIS = 0 );

    //! Reverses the command
    void undo() override;
    //! Replays the command
    void redo() override;

    //! Saves current item state as previous state
    void savePreviousState();
    //! Saves current item state as after state
    void saveAfterState();

    QDomDocument previousState() const { return mPreviousState.cloneNode().toDocument(); }
    QDomDocument afterState() const { return mAfterState.cloneNode().toDocument(); }

    //! Returns true if previous state and after state are valid and different
    bool containsChange() const;

    /**
     * Returns the target item the command applies to.
     * \returns target composer item
     */
    QgsComposerItem *item() const;

  protected:
    //! Target item of the command
    QgsComposerItem *mItem = nullptr;
    //! XML that saves the state before executing the command
    QDomDocument mPreviousState;
    //! XML containing the state after executing the command
    QDomDocument mAfterState;

    /**
     * Parameters for frame items
     * Parent multiframe
     */
    QgsComposerMultiFrame *mMultiFrame = nullptr;
    int mFrameNumber;

    //! Flag to prevent the first redo() if the command is pushed to the undo stack
    bool mFirstRun;

    void saveState( QDomDocument &stateDoc ) const;
    void restoreState( QDomDocument &stateDoc ) const;
};

/**
 * \ingroup core
 * A composer command that merges together with other commands having the same context (=id). Keeps the oldest previous state and uses the
  newest after state. The purpose is to avoid too many micro changes in the history
*/
class CORE_EXPORT QgsComposerMergeCommand: public QgsComposerItemCommand
{
  public:
    enum Context
    {
      Unknown = 0,
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
      LegendLineSpacing,
      LegendRasterStrokeWidth,
      LegendFontColor,
      LegendRasterStrokeColor,
      //composer picture
      ComposerPictureRotation,
      ComposerPictureFillColor,
      ComposerPictureStrokeColor,
      ComposerPictureNorthOffset,
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
      ShapeStrokeWidth,
      //composer arrow
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
      ItemOpacity, //!< Item opacity
      ItemZoomContent
    };

    QgsComposerMergeCommand( Context c, QgsComposerItem *item, const QString &text );

    bool mergeWith( const QUndoCommand *command ) override;
    int id() const override { return static_cast< int >( mContext ); }

  private:
    Context mContext;
};

#endif // QGSCOMPOSERITEMCOMMAND_H
