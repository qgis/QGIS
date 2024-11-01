/***************************************************************************
                         qgsmaptoolsdigitizingtechniquemanager.h
                         ----------------------
    begin                : January 2022
    copyright            : (C) 2022 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSDIGITIZINGTECHNIQUEMANAGER_H
#define QGSMAPTOOLSDIGITIZINGTECHNIQUEMANAGER_H

#include "qgis_app.h"
#include "qgsmaptoolcapture.h"
#include "qgsmaptoolshapeabstract.h"
#include "qgssettingstree.h"

#include <QWidgetAction>

class QgsSpinBox;
class QgsSettingsEntryString;
template<class T> class QgsSettingsEntryEnumFlag;

class QAction;
class QToolButton;


class APP_EXPORT QgsStreamDigitizingSettingsAction : public QWidgetAction
{
    Q_OBJECT

  public:
    QgsStreamDigitizingSettingsAction( QWidget *parent = nullptr );
    ~QgsStreamDigitizingSettingsAction() override;

  private:
    QgsSpinBox *mStreamToleranceSpinBox = nullptr;
};

class APP_EXPORT QgsMapToolsDigitizingTechniqueManager : public QObject
{
    Q_OBJECT
  public:
    static const QgsSettingsEntryEnumFlag<Qgis::CaptureTechnique> *settingsDigitizingTechnique;

    static inline QgsSettingsTreeNode *sTreeShapeMapTools = QgsSettingsTree::sTreeDigitizing->createChildNode( QStringLiteral( "shape-map-tools" ) );
    static const QgsSettingsEntryString *settingMapToolShapeCurrent;
    static inline QgsSettingsTreeNamedListNode *sTreeShapeMapToolsCategories = sTreeShapeMapTools->createNamedListNode( QStringLiteral( "categories" ) );
    static const QgsSettingsEntryString *settingMapToolShapeDefaultForCategory;

    QgsMapToolsDigitizingTechniqueManager( QObject *parent );
    ~QgsMapToolsDigitizingTechniqueManager();
    void setupToolBars();
    void setupCanvasTools();

  public slots:
    void enableDigitizingTechniqueActions( bool enabled, QAction *triggeredFromToolAction = nullptr );


  private slots:
    void setCaptureTechnique( Qgis::CaptureTechnique technique, bool alsoSetShapeTool = true );
    void setShapeTool( const QString &shapeToolId );
    void mapToolSet( QgsMapTool *newTool, QgsMapTool * );

  private:
    void setupTool( QgsMapToolCapture *tool );

    void updateDigitizeModeButton( const Qgis::CaptureTechnique technique );

    QMap<Qgis::CaptureTechnique, QAction *> mTechniqueActions;
    QHash<QString, QAction *> mShapeActions;
    QMap<QgsMapToolShapeAbstract::ShapeCategory, QToolButton *> mShapeCategoryButtons;

    QSet<QgsMapTool *> mInitializedTools;

    QToolButton *mDigitizeModeToolButton = nullptr;
    QgsStreamDigitizingSettingsAction *mStreamDigitizingSettingsAction = nullptr;
};

#endif // QGSMAPTOOLSDIGITIZINGTECHNIQUEMANAGER_H
