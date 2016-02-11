/***************************************************************************
    qgsgrassplugin.h  -  GRASS menu
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSPLUGIN_H
#define QGSGRASSPLUGIN_H
#include "../qgisplugin.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsvectorlayer.h"
#include <QObject>

class QgsGrassTools;
class QgsGrassNewMapset;
class QgsGrassRegion;

class QgsMapCanvas;
class QgsMapLayer;
class QgsMapTool;
class QgsRubberBand;

class QAction;
class QIcon;
class QPainter;
class QToolBar;

/**
* \class QgsGrassPlugin
* \brief OpenModeller plugin for QGIS
*
*/
class QgsGrassPlugin : public QObject, public QgisPlugin
{
    Q_OBJECT

  public:
    /**
     * Constructor for a plugin. The QgisInterface pointer is passed by
     * QGIS when it attempts to instantiate the plugin.
     * @param qI Pointer to the QgisInterface object.
     */
    explicit QgsGrassPlugin( QgisInterface * qI );
    /**
     * Virtual function to return the name of the plugin. The name will be used when presenting a list
     * of installable plugins to the user
     */
    virtual QString name();
    /**
     * Virtual function to return the version of the plugin.
     */
    virtual QString version();
    /**
     * Virtual function to return a description of the plugins functions
     */
    virtual QString description();
    /**
     * Virtual function to return a category of the plugin
     */
    virtual QString category();
    /**
     * Return the plugin type
     */
    virtual int type();
    //! Destructor
    virtual ~QgsGrassPlugin();

    //! Get an icon from the active theme if possible
    static QIcon getThemeIcon( const QString &theName );

  public slots:
    //! init the gui
    virtual void initGui() override;
    //! unload the plugin
    void unload() override;
    //! show the help document
    void help();
    //! Gisbase changed by user
    void onGisbaseChanged();
    //! Display current region
    void displayRegion();
    //! Switch region on/off
    void switchRegion( bool on );
    //! Redraw region
    void redrawRegion( void );
    //! Post render
    void postRender( QPainter * );
    //! Open tools
    void openTools( void );
    //! Create new mapset
    void newMapset();
    //! Open existing mapset and save it to project
    void openMapset();
    //! Close mapset and save it to project
    void closeMapset();
    //! Current mapset changed (opened/closed)
    void mapsetChanged();
    //! Create new vector
    void newVector();
    //! Read project
    void projectRead();
    //! New project
    void newProject();
    //! update plugin icons when the app tells us its theme is changed
    void setCurrentTheme( QString theThemeName );
    void setTransform();
    //! Called when a new layer was added to map registry
    void onLayerWasAdded( QgsMapLayer* theMapLayer );
    //! Called when editing of a layer started
    void onEditingStarted();
    void onEditingStopped();
    void onCurrentLayerChanged( QgsMapLayer* layer );

    void onFieldsChanged();

    // Start editing tools
    void addFeature();

    void onSplitFeaturesTriggered( bool checked );

    // Called when new layer was created in browser
    void onNewLayer( QString uri, QString name );

  private:
    void resetEditActions();

    //! Pointer to our toolbar
    QToolBar *mToolBarPointer;
    //! Pointer to the QGIS interface object
    QgisInterface *qGisInterface;
    //! Pointer to canvas
    QgsMapCanvas *mCanvas;

    //! Pointer to Display region acction
    QAction *mRegionAction;

    // Region rubber band
    QgsRubberBand *mRegionBand;
    //! GRASS tools
    QgsGrassTools *mTools;
    //! Pointer to QgsGrassNewMapset
    QgsGrassNewMapset *mNewMapset;

    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateTransform mCoordinateTransform;

    // Actions
    QAction *mOpenMapsetAction;
    QAction *mNewMapsetAction;
    QAction *mCloseMapsetAction;
    QAction *mOpenToolsAction;
    QAction *mOptionsAction;

    // Editing    static bool mNonInitializable;
    QAction *mAddFeatureAction;
    QAction *mAddPointAction;
    QAction *mAddLineAction;
    QAction *mAddBoundaryAction;
    QAction *mAddCentroidAction;
    QAction *mAddAreaAction;

    QgsMapTool *mAddPoint;
    QgsMapTool *mAddLine;
    QgsMapTool *mAddBoundary;
    QgsMapTool *mAddCentroid;
    QgsMapTool *mAddArea;

    // Names of layer styles before editing started
    QMap<QgsVectorLayer *, QString> mOldStyles;
    // Original layer form suppress
    QMap<QgsVectorLayer *, QgsEditFormConfig::FeatureFormSuppress> mFormSuppress;
};

#endif // QGSGRASSPLUGIN_H
