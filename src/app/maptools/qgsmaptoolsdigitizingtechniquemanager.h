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
#include "qgssettingsentry.h"
#include "qgsmaptoolcapture.h"
#include "qgsmaptoolshapeabstract.h"

#include <QWidgetAction>


class QgsSpinBox;

class QAction;
class QToolButton;


template class APP_EXPORT QgsSettingsEntryEnumFlag<QgsMapToolCapture::CaptureTechnique>;


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
    static const inline  QgsSettingsEntryEnumFlag<QgsMapToolCapture::CaptureTechnique> settingsDigitizingTechnique = QgsSettingsEntryEnumFlag<QgsMapToolCapture::CaptureTechnique>( QStringLiteral( "/qgis/digitizing/technique" ), QgsSettings::NoSection, QgsMapToolCapture::CaptureTechnique::StraightSegments ) SIP_SKIP;

    static const inline QgsSettingsEntryString settingMapToolShapeDefaultForShape = QgsSettingsEntryString( QStringLiteral( "/qgis/digitizing/shape-map-tools/%1/default" ), QgsSettings::NoSection, QString(), QObject::tr( "Default map tool for given shape category" ) ) SIP_SKIP;
    static const inline QgsSettingsEntryString settingMapToolShapeCurrent = QgsSettingsEntryString( QStringLiteral( "/qgis/digitizing/shape-map-tools/current" ), QgsSettings::NoSection, QString(), QObject::tr( "Current shape map tool" ) ) SIP_SKIP;

    QgsMapToolsDigitizingTechniqueManager( QObject *parent );
    ~QgsMapToolsDigitizingTechniqueManager();
    void setupToolBars();
    void setupCanvasTools();

  public slots:
    void enableDigitizingTechniqueActions( bool enabled, QAction *triggeredFromToolAction = nullptr );


  private slots:
    void setCaptureTechnique( QgsMapToolCapture::CaptureTechnique technique, bool alsoSetShapeTool = true );
    void setShapeTool( const QString &shapeToolId );

  private:
    QMap<QgsMapToolCapture::CaptureTechnique, QAction *> mTechniqueActions;
    QHash<QString, QAction *> mShapeActions;
    QMap<QgsMapToolShapeAbstract::ShapeCategory, QToolButton *> mShapeCategoryButtons;

    QToolButton *mDigitizeModeToolButton = nullptr;
    QgsStreamDigitizingSettingsAction *mStreamDigitizingSettingsAction = nullptr;


};

#endif // QGSMAPTOOLSDIGITIZINGTECHNIQUEMANAGER_H
