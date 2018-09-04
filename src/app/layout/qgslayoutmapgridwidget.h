/***************************************************************************
                         qgslayoutmapgridwidget.h
                         ----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTMAPGRIDWIDGET_H
#define QGSLAYOUTMAPGRIDWIDGET_H

#include "ui_qgslayoutmapgridwidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemmapgrid.h"

/**
 * \ingroup app
 * Input widget for the configuration of QgsLayoutItemMapGrids
 * */
class QgsLayoutMapGridWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutMapGridWidgetBase
{
    Q_OBJECT

  public:
    explicit QgsLayoutMapGridWidget( QgsLayoutItemMapGrid *mapGrid, QgsLayoutItemMap *map );

  public slots:

    void setGridItems();
    void mIntervalXSpinBox_editingFinished();
    void mIntervalYSpinBox_editingFinished();
    void mOffsetXSpinBox_valueChanged( double value );
    void mOffsetYSpinBox_valueChanged( double value );
    void mCrossWidthSpinBox_valueChanged( double val );
    void mFrameWidthSpinBox_valueChanged( double val );
    void mGridFrameMarginSpinBox_valueChanged( double val );
    void mFrameStyleComboBox_currentIndexChanged( const QString &text );
    void mGridFramePenSizeSpinBox_valueChanged( double d );
    void mGridFramePenColorButton_colorChanged( const QColor &newColor );
    void mGridFrameFill1ColorButton_colorChanged( const QColor &newColor );
    void mGridFrameFill2ColorButton_colorChanged( const QColor &newColor );
    void mGridTypeComboBox_currentIndexChanged( const QString &text );
    void mMapGridCRSButton_clicked();
    void mMapGridUnitComboBox_currentIndexChanged( const QString &text );
    void mGridBlendComboBox_currentIndexChanged( int index );
    void mCheckGridLeftSide_toggled( bool checked );
    void mCheckGridRightSide_toggled( bool checked );
    void mCheckGridTopSide_toggled( bool checked );
    void mCheckGridBottomSide_toggled( bool checked );

    //frame divisions display
    void mFrameDivisionsLeftComboBox_currentIndexChanged( int index );
    void mFrameDivisionsRightComboBox_currentIndexChanged( int index );
    void mFrameDivisionsTopComboBox_currentIndexChanged( int index );
    void mFrameDivisionsBottomComboBox_currentIndexChanged( int index );

    void mDrawAnnotationGroupBox_toggled( bool state );
    void mAnnotationFormatButton_clicked();

    //annotation display
    void mAnnotationDisplayLeftComboBox_currentIndexChanged( const QString &text );
    void mAnnotationDisplayRightComboBox_currentIndexChanged( const QString &text );
    void mAnnotationDisplayTopComboBox_currentIndexChanged( const QString &text );
    void mAnnotationDisplayBottomComboBox_currentIndexChanged( const QString &text );

    //annotation position
    void mAnnotationPositionLeftComboBox_currentIndexChanged( const QString &text );
    void mAnnotationPositionRightComboBox_currentIndexChanged( const QString &text );
    void mAnnotationPositionTopComboBox_currentIndexChanged( const QString &text );
    void mAnnotationPositionBottomComboBox_currentIndexChanged( const QString &text );

    //annotation direction
    void mAnnotationDirectionComboBoxLeft_currentIndexChanged( int index );
    void mAnnotationDirectionComboBoxRight_currentIndexChanged( int index );
    void mAnnotationDirectionComboBoxTop_currentIndexChanged( int index );
    void mAnnotationDirectionComboBoxBottom_currentIndexChanged( int index );

    void mAnnotationFormatComboBox_currentIndexChanged( int index );
    void mCoordinatePrecisionSpinBox_valueChanged( int value );
    void mDistanceToMapFrameSpinBox_valueChanged( double d );
    void mAnnotationFontColorButton_colorChanged( const QColor &color );

  protected:

    //! Sets the current composer map values to the GUI elements
    virtual void updateGuiElements();

  protected slots:
    //! Initializes data defined buttons to current atlas coverage layer
    void populateDataDefinedButtons();

  private slots:

    //! Sets the GUI elements to the values of mPicture
    void setGuiElementValues();
    void annotationFontChanged();
    void lineSymbolChanged();
    void markerSymbolChanged();

  private:
    QPointer< QgsLayoutItemMap > mMap;
    QPointer< QgsLayoutItemMapGrid > mMapGrid;

    //! Blocks / unblocks the signals of all GUI elements
    void blockAllSignals( bool b );

    void handleChangedFrameDisplay( QgsLayoutItemMapGrid::BorderSide border, QgsLayoutItemMapGrid::DisplayMode mode );
    void handleChangedAnnotationDisplay( QgsLayoutItemMapGrid::BorderSide border, const QString &text );
    void handleChangedAnnotationPosition( QgsLayoutItemMapGrid::BorderSide border, const QString &text );
    void handleChangedAnnotationDirection( QgsLayoutItemMapGrid::BorderSide border, QgsLayoutItemMapGrid::AnnotationDirection direction );

    void insertFrameDisplayEntries( QComboBox *c );
    void insertAnnotationDisplayEntries( QComboBox *c );
    void insertAnnotationPositionEntries( QComboBox *c );
    void insertAnnotationDirectionEntries( QComboBox *c );

    void initFrameDisplayBox( QComboBox *c, QgsLayoutItemMapGrid::DisplayMode display );
    void initAnnotationDisplayBox( QComboBox *c, QgsLayoutItemMapGrid::DisplayMode display );
    void initAnnotationPositionBox( QComboBox *c, QgsLayoutItemMapGrid::AnnotationPosition pos );
    void initAnnotationDirectionBox( QComboBox *c, QgsLayoutItemMapGrid::AnnotationDirection dir );

    //! Enables/disables grid frame related controls
    void toggleFrameControls( bool frameEnabled, bool frameFillEnabled, bool frameSizeEnabled );

    //! Is there some predefined scales, globally or as project's options ?
    bool hasPredefinedScales() const;

};

#endif //QGSLAYOUTMAPGRIDWIDGET_H
