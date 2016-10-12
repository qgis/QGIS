/***************************************************************************
                         qgscomposermapgridwidget.h
                         ----------------------
    begin                : October 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#ifndef QGSCOMPOSERMAPGRIDWIDGET_H
#define QGSCOMPOSERMAPGRIDWIDGET_H

#include "ui_qgscomposermapgridwidgetbase.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposermapgrid.h"

/**
 * \ingroup app
 * Input widget for the configuration of QgsComposerMapGrids
 * */
class QgsComposerMapGridWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerMapGridWidgetBase
{
    Q_OBJECT

  public:
    explicit QgsComposerMapGridWidget( QgsComposerMapGrid* mapGrid, QgsComposerMap* composerMap );
    virtual ~QgsComposerMapGridWidget();

  public slots:

    void setGridItems();
    void on_mGridLineStyleButton_clicked();
    void on_mGridMarkerStyleButton_clicked();
    void on_mIntervalXSpinBox_editingFinished();
    void on_mIntervalYSpinBox_editingFinished();
    void on_mOffsetXSpinBox_valueChanged( double value );
    void on_mOffsetYSpinBox_valueChanged( double value );
    void on_mCrossWidthSpinBox_valueChanged( double val );
    void on_mFrameWidthSpinBox_valueChanged( double val );
    void on_mFrameStyleComboBox_currentIndexChanged( const QString& text );
    void on_mGridFramePenSizeSpinBox_valueChanged( double d );
    void on_mGridFramePenColorButton_colorChanged( const QColor& newColor );
    void on_mGridFrameFill1ColorButton_colorChanged( const QColor& newColor );
    void on_mGridFrameFill2ColorButton_colorChanged( const QColor& newColor );
    void on_mGridTypeComboBox_currentIndexChanged( const QString& text );
    void on_mMapGridCRSButton_clicked();
    void on_mMapGridUnitComboBox_currentIndexChanged( const QString& text );
    void on_mGridBlendComboBox_currentIndexChanged( int index );
    void on_mCheckGridLeftSide_toggled( bool checked );
    void on_mCheckGridRightSide_toggled( bool checked );
    void on_mCheckGridTopSide_toggled( bool checked );
    void on_mCheckGridBottomSide_toggled( bool checked );

    //frame divisions display
    void on_mFrameDivisionsLeftComboBox_currentIndexChanged( int index );
    void on_mFrameDivisionsRightComboBox_currentIndexChanged( int index );
    void on_mFrameDivisionsTopComboBox_currentIndexChanged( int index );
    void on_mFrameDivisionsBottomComboBox_currentIndexChanged( int index );

    void on_mDrawAnnotationGroupBox_toggled( bool state );
    void on_mAnnotationFormatButton_clicked();

    //annotation display
    void on_mAnnotationDisplayLeftComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationDisplayRightComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationDisplayTopComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationDisplayBottomComboBox_currentIndexChanged( const QString& text );

    //annotation position
    void on_mAnnotationPositionLeftComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationPositionRightComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationPositionTopComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationPositionBottomComboBox_currentIndexChanged( const QString& text );

    //annotation direction
    void on_mAnnotationDirectionComboBoxLeft_currentIndexChanged( int index );
    void on_mAnnotationDirectionComboBoxRight_currentIndexChanged( int index );
    void on_mAnnotationDirectionComboBoxTop_currentIndexChanged( int index );
    void on_mAnnotationDirectionComboBoxBottom_currentIndexChanged( int index );

    void on_mAnnotationFormatComboBox_currentIndexChanged( int index );
    void on_mCoordinatePrecisionSpinBox_valueChanged( int value );
    void on_mDistanceToMapFrameSpinBox_valueChanged( double d );
    void on_mAnnotationFontButton_clicked();
    void on_mAnnotationFontColorButton_colorChanged( const QColor &color );

  protected:

    /**
     * Sets the current composer map values to the GUI elements*/
    virtual void updateGuiElements();

  protected slots:
    /**
     * Initializes data defined buttons to current atlas coverage layer*/
    void populateDataDefinedButtons();

  private slots:

    /**
     * Sets the GUI elements to the values of mPicture*/
    void setGuiElementValues();

    void updateGridLineStyleFromWidget();
    void cleanUpGridLineStyleSelector( QgsPanelWidget* container );
    void updateGridMarkerStyleFromWidget();
    void cleanUpGridMarkerStyleSelector( QgsPanelWidget* container );

  private:
    QgsComposerMap* mComposerMap;
    QgsComposerMapGrid* mComposerMapGrid;

    /**
     * Blocks / unblocks the signals of all GUI elements*/
    void blockAllSignals( bool b );

    void handleChangedFrameDisplay( QgsComposerMapGrid::BorderSide border, const QgsComposerMapGrid::DisplayMode mode );
    void handleChangedAnnotationDisplay( QgsComposerMapGrid::BorderSide border, const QString& text );
    void handleChangedAnnotationPosition( QgsComposerMapGrid::BorderSide border, const QString& text );
    void handleChangedAnnotationDirection( QgsComposerMapGrid::BorderSide border, QgsComposerMapGrid::AnnotationDirection direction );

    void insertFrameDisplayEntries( QComboBox* c );
    void insertAnnotationDisplayEntries( QComboBox* c );
    void insertAnnotationPositionEntries( QComboBox* c );
    void insertAnnotationDirectionEntries( QComboBox* c );

    void initFrameDisplayBox( QComboBox* c, QgsComposerMapGrid::DisplayMode display );
    void initAnnotationDisplayBox( QComboBox* c, QgsComposerMapGrid::DisplayMode display );
    void initAnnotationPositionBox( QComboBox* c, QgsComposerMapGrid::AnnotationPosition pos );
    void initAnnotationDirectionBox( QComboBox* c, QgsComposerMapGrid::AnnotationDirection dir );

    void updateGridLineSymbolMarker();
    void updateGridMarkerSymbolMarker();

    /**
     * Enables/disables grid frame related controls*/
    void toggleFrameControls( bool frameEnabled, bool frameFillEnabled, bool frameSizeEnabled );

    /**
     * Is there some predefined scales, globally or as project's options ?*/
    bool hasPredefinedScales() const;

};

#endif //QGSCOMPOSERMAPGRIDWIDGET_H
