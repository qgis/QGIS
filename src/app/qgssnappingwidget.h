/***************************************************************************
                         qgssnappingwidget.h
    begin                : August 2016
    copyright            : (C) 2016 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSNAPPINGWIDGET_H
#define QGSSNAPPINGWIDGET_H

class QAction;
class QComboBox;
class QDoubleSpinBox;
class QFont;
class QToolButton;
class QTreeView;

class QgsLayerTreeGroup;
class QgsLayerTreeNode;
class QgsLayerTreeView;
class QgsMapCanvas;
class QgsProject;

#include "qgssnappingconfig.h"

#include <QWidget>

/**
  * A widget which lets the user defines settings for snapping on a project
  * The widget can be displayed as a toolbar, in the status bar or as dialog/widget.
  * The display mode is automatically chose based on the parent widget type.
  */
class APP_EXPORT QgsSnappingWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor
     * @param project The project with which this widget configuration will be synchronized
     * @param canvas the map canvas (used for map units)
     * @param parent is the parent widget. Based on the type of parent, it will
     * be displayed a tool bar, in the status bar or as a widget/dialog.
     */
    QgsSnappingWidget( QgsProject* project, QgsMapCanvas* canvas, QWidget* parent = nullptr );

    /** Destructor */
    virtual ~QgsSnappingWidget();

    /**
     * The snapping configuration is what is managed by this widget.
     */
    QgsSnappingConfig config() const;

    /**
     * The snapping configuration is what is managed by this widget.
     */
    void setConfig( const QgsSnappingConfig& config );

  signals:
    void snappingConfigChanged( );

  private slots:
    void projectSnapSettingsChanged();

    void projectTopologicalEditingChanged();

    void enableSnapping( bool checked );

    void changeTolerance( double tolerance );

    void changeUnit( int idx );

    void enableTopologicalEditing( bool enabled );

    void enableIntersectionSnapping( bool enabled );

    void modeButtonTriggered( QAction* action );
    void typeButtonTriggered( QAction* action );

    //! number of decimals of the tolerance spin box depends on map units
    void updateToleranceDecimals();

  private:
    enum DisplayMode
    {
      ToolBar,
      StatusBar,
      Widget
    };
    DisplayMode mDisplayMode;

    //! modeChanged determines if widget are visible or not based on mode
    void modeChanged();

    QgsProject* mProject;
    QgsSnappingConfig mConfig;
    QgsMapCanvas* mCanvas;

    QAction* mEnabledAction;
    QToolButton* mModeButton;
    QAction* mModeAction; // hide widget does not work on toolbar, action needed
    QAction* mAllLayersAction;
    QAction* mActiveLayerAction;
    QAction* mAdvancedModeAction;
    QToolButton* mTypeButton;
    QAction* mTypeAction; // hide widget does not work on toolbar, action needed
    QAction* mVertexAction;
    QAction* mSegmentAction;
    QAction* mVertexAndSegmentAction;
    QDoubleSpinBox* mToleranceSpinBox;
    QAction* mToleranceAction; // hide widget does not work on toolbar, action needed
    QComboBox* mUnitsComboBox;
    QAction* mUnitAction; // hide widget does not work on toolbar, action needed
    QAction* mTopologicalEditingAction;
    QAction* mIntersectionSnappingAction;
    QTreeView* mLayerTreeView;

    void cleanGroup( QgsLayerTreeNode* node );
};

#endif
