/***************************************************************************
    qgsmapcanvasdockwidget.h
    ------------------------
    begin                : February 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 **************************************************************************/
#ifndef QGSPROFILEWINDOWDOCKWIDGET_H
#define QGSPROFILEWINDOWDOCKWIDGET_H

#include "ui_qgsprofilewindowdockwidgetbase.h"
#include "ui_pointcloudclassselection.h"
#include "ui_pointcloudtargetclassselection.h"
#include "qgsdockwidget.h"
#include "qgspointxy.h"
#include "qgis_app.h"
#include <QWidgetAction>
#include <QTimer>
#include <memory>

#include "View3D.h"

class QgsScaleComboBox;
class QgsDoubleSpinBox;
class QCheckBox;
class QRadioButton;

typedef View3D QgsProfileWinow;

class APP_EXPORT QgsProfileWindowDockWidget : public QgsDockWidget, private Ui::QgsProfileWindowDockWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsProfileWindowDockWidget( const QString &name, QWidget *parent = nullptr );

	QgsProfileWinow *getmapCanvas();
	void setProfileWindow(QgsProfileWinow * window);
	void setMain3DWindow(QgsProfileWinow * window);

  private slots:
	void OnmActionSaveEditsClicked();
	void OnmselectiononprofileClciekd();

	void OndrawlieonprofileClicked2();

	void OnmActionPickPoints();

	void OnmActionBrushPoints();

	void OnmActionDeleteSelected();
	void ApplyButtonClicked();
	void OndrawlieonprofileClicked();
	void OnmActionToggleEditingClicked();
	void OnmActionsetbeforclassClicked();
	void OnmActionsettargetclassClicked();
	void OnmActionsetshaderClicked();
	void OnmActionHandClicked();

	void OnCheckChanged();
	void OnCheckChanged2();
	void CheckAll();
	void UnCheckAll();
  void rotatePointCloudLeft();
  void rotatePointCloudRight();

private:
    QgsProfileWinow *mMapCanvas = nullptr;
    QgsProfileWinow *mMainCanvas = nullptr;
   
    QRadioButton *mSyncExtentRadio = nullptr;
    QRadioButton *mSyncSelectionRadio = nullptr;
    QgsScaleComboBox *mScaleCombo = nullptr;
    QgsDoubleSpinBox *mMagnificationEdit = nullptr;
    QgsDoubleSpinBox *mScaleFactorWidget = nullptr;
    QCheckBox *mSyncScaleCheckBox = nullptr;
	Ui::pointcloudclassselection  class_form;
	Ui::pointcloudtargetclassselection class_form_target;
	QWidget* class_form_widget;
	QWidget* class_target_widget;
	QWidget* profile_widget;
	std::map<int, bool> originalClass;
	QString TargetClass;

	bool Editing = false;
	QString m_rule;
	QString m_method;
};


#endif // QGSPROFILEWINDOWDOCKWIDGET_H

