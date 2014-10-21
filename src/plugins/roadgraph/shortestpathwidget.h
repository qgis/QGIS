/***************************************************************************
 *   Copyright (C) 2009 by Sergey Yakushev                                 *
 *   yakushevs<at>list.ru                                                  *
 *                                                                         *
 *   This is file define vrp plugins settings                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef ROADGRAPHPLUGIN_SHORTESTPATHDLG_H
#define ROADGRAPHPLUGIN_SHORTESTPATHDLG_H

// QT includes
#include <QDockWidget>

// Qgis includes
#include <qgspoint.h>

// standard includes

// forward declaration

class QComboBox;
class QLineEdit;
class QPushButton;

class QgsRubberBand;
class QgsMapToolEmitPoint;
class QgsMapCanvas;

class RoadGraphPlugin;

class QgsGraph;

/**
@author Sergey Yakushev
*/
/**
 * \class VrpPluginShortestPathDlg
 * \brief This class implement user interface for finding shortest path between two points.
 */
class RgShortestPathWidget : public QDockWidget
{
    Q_OBJECT
  public:
    /**
     * Standard constructor
     */
    RgShortestPathWidget( QWidget *, RoadGraphPlugin * );

    /**
     * destructor
     */
    ~RgShortestPathWidget();

  private slots:
    /**
     * export path
     */
    void exportPath();

    /**
     * update rubberbands where extents changed
     */
    void mapCanvasExtentsChanged();

    /**
     * on canvas click mouse button
     */
    void setFrontPoint( const QgsPoint& );

    /**
     * on canvas click mouse button
     */
    void setBackPoint( const QgsPoint& );

    /**
     * Activate map tool for coordinate capture
     */
    void onSelectFrontPoint();

    /**
     * Activate map tool for coordinate capture
     */
    void onSelectBackPoint();

    /**
     * finding path
     */
    void findingPath();

    /**
     * clear
     */
    void clear();

    /**
     * help requested
     */
    void helpRequested();

  private:
    /**
     * return path as a graph
     */
    QgsGraph* getPath( QgsPoint& p1, QgsPoint& p2 );

    /**
     * This line edit show front points coordinates
     */
    QLineEdit *mFrontPointLineEdit;

    /**
     * This line edit show back points coordinates
     */
    QLineEdit *mBackPointLineEdit;

    /**
     * This combobox conteined criterion name
     */
    QComboBox *mCriterionName;

    /**
     * This line edit show length calculated path
     */
    QLineEdit *mPathCostLineEdit;

    /**
     * This line edit show time calculated path
     */
    QLineEdit *mPathTimeLineEdit;

    /**
     * this button called to find shortest path
     */
    QPushButton *mCalculate;

    /**
     * this button called to clear line edits and clar current path
     */
    QPushButton *mClear;

    /**
     * this map tool use for select coordinates
     */
    QgsMapToolEmitPoint *mFrontPointMapTool;

    /**
     * this map tool use for select coordinates
     */
    QgsMapToolEmitPoint *mBackPointMapTool;

    /**
     * pointer to Plugin
     */
    RoadGraphPlugin *mPlugin;

    /**
     * Front point
     */
    QgsPoint mFrontPoint;

    /**
     * Back point
     */
    QgsPoint mBackPoint;

    /**
     * show front point
     */
    QgsRubberBand *mrbFrontPoint;

    /**
     * show back point
     */
    QgsRubberBand *mrbBackPoint;

    /**
     * show shortest path
     */
    QgsRubberBand *mrbPath;
};
#endif
