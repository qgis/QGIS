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
    static const inline  QgsSettingsEntryEnumFlag<Qgis::CaptureTechnique> settingsDigitizingTechnique = QgsSettingsEntryEnumFlag<Qgis::CaptureTechnique>( QStringLiteral( "technique" ), QgsSettings::Prefix::QGIS_DIGITIZING, Qgis::CaptureTechnique::StraightSegments, QObject::tr( "Current digitizing technique" ), Qgis::SettingsOption::SaveFormerValue ) SIP_SKIP;

    static const inline QgsSettingsEntryString settingMapToolShapeDefaultForShape = QgsSettingsEntryString( QStringLiteral( "%1/default" ), QgsSettings::Prefix::QGIS_DIGITIZING_SHAPEMAPTOOLS, QString(), QObject::tr( "Default map tool for given shape category" ) ) SIP_SKIP;
    static const inline QgsSettingsEntryString settingMapToolShapeCurrent = QgsSettingsEntryString( QStringLiteral( "current" ), QgsSettings::Prefix::QGIS_DIGITIZING_SHAPEMAPTOOLS, QString(), QObject::tr( "Current shape map tool" ) ) SIP_SKIP;

    QgsMapToolsDigitizingTechniqueManager( QObject *parent );
    ~QgsMapToolsDigitizingTechniqueManager();
    void setupToolBars();
    void setupCanvasTools();

  public slots:
    void enableDigitizingTechniqueActions( bool enabled, QAction *triggeredFromToolAction = nullptr );


  private slots:
    void setCaptureTechnique( Qgis::CaptureTechnique technique, bool alsoSetShapeTool = true );
    void setShapeTool( const QString &shapeToolId );

  private:
    QMap<Qgis::CaptureTechnique, QAction *> mTechniqueActions;
    QHash<QString, QAction *> mShapeActions;
    QMap<QgsMapToolShapeAbstract::ShapeCategory, QToolButton *> mShapeCategoryButtons;

    QToolButton *mDigitizeModeToolButton = nullptr;
    QgsStreamDigitizingSettingsAction *mStreamDigitizingSettingsAction = nullptr;


};

#endif // QGSMAPTOOLSDIGITIZINGTECHNIQUEMANAGER_H
