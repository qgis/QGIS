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
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"
#include "qgsmaptoolcapture.h"
#include "qgsmaptoolshapeabstract.h"
#include "qgsmaptoolshapecircle2points.h"


#include <QWidgetAction>


class QgsSpinBox;

class QAction;
class QToolButton;


class APP_EXPORT QgsStreamDigitizingSettingsAction: public QWidgetAction
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
    static inline  QgsSettingsEntryEnumFlag<Qgis::CaptureTechnique> *settingsDigitizingTechnique = new QgsSettingsEntryEnumFlag<Qgis::CaptureTechnique>( QStringLiteral( "technique" ), QgsSettings::sTreeDigitizing, Qgis::CaptureTechnique::StraightSegments, QObject::tr( "Current digitizing technique" ), Qgis::SettingsOption::SaveFormerValue ) SIP_SKIP;

    static inline QgsSettingsTreeElement *sTreeShapeMapTools = QgsSettings::sTreeDigitizing->createChildElement( QStringLiteral( "shape-map-tools" ) );
    static const inline QgsSettingsEntryString *settingMapToolShapeDefaultForShape = new QgsSettingsEntryString( QStringLiteral( "%1/default" ), sTreeShapeMapTools, QString(), QObject::tr( "Default map tool for given shape category" ) ) SIP_SKIP;
    static const inline QgsSettingsEntryString *settingMapToolShapeCurrent = new QgsSettingsEntryString( QStringLiteral( "current" ), sTreeShapeMapTools, QgsMapToolShapeCircle2PointsMetadata::TOOL_ID, QObject::tr( "Current shape map tool" ) ) SIP_SKIP;

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

    QMap<Qgis::CaptureTechnique, QAction *> mTechniqueActions;
    QHash<QString, QAction *> mShapeActions;
    QMap<QgsMapToolShapeAbstract::ShapeCategory, QToolButton *> mShapeCategoryButtons;

    QSet< QgsMapTool * > mInitializedTools;

    QToolButton *mDigitizeModeToolButton = nullptr;
    QgsStreamDigitizingSettingsAction *mStreamDigitizingSettingsAction = nullptr;


};

#endif // QGSMAPTOOLSDIGITIZINGTECHNIQUEMANAGER_H
