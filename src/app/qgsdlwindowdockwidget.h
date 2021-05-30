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
#ifndef QGSDLWINDOWDOCKWIDGET_H
#define QGSDLWINDOWDOCKWIDGET_H

#include "qgsprofilewindowdockwidget.h"
#include "ui_qgsDLwindowdockwidgetbase.h"
#include "ui_3DpointsPickedDlg.h"
#include "qgsdockwidget.h"
#include "qgspointxy.h"
#include "qgis_app.h"
#include <QWidgetAction>
#include <QTimer>
#include <memory>

#include "View3D.h"
#include <QVector3D>

#include "polynomial3CurveFitter.h"

class polynomial3CurveFitter3;
class QgsScaleComboBox;
class QgsDoubleSpinBox;
class QCheckBox;
class QRadioButton;

typedef View3D QgsProfileWinow;
typedef Imath::V3d Point3D;

enum PointType
{
  Start = 0,
  End = 0x1,
  XuanDuan = 0x2,
  Other = 0x3
};

struct ModelItem
{
  Point3D XYZ;
  PointType type = PointType::Other;
  double error = 0;

  bool compAsendingX(ModelItem firsrt, ModelItem second)
  {
    return firsrt.XYZ.x > second.XYZ.x;
  }

  bool compDesendingX(ModelItem firsrt, ModelItem second)
  {
    return firsrt.XYZ.x < second.XYZ.x;
  }

  bool compAsendingY(ModelItem firsrt, ModelItem second)
  {
    return firsrt.XYZ.y > second.XYZ.y;
  }

  bool compDesendinY(ModelItem firsrt, ModelItem second)
  {
    return firsrt.XYZ.y < second.XYZ.y;
  }
};

class APP_EXPORT QgsDLAttributeTableModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  QgsDLAttributeTableModel(QWidget *parent = nullptr);
  void receivepickedpoints(QVector3D pointxyz);
  void receivepoints(Point3D& pointxyz);
  void sortByColumn(int col);
  void setProfileWindow(QgsProfileWinow *window);
  //自定义导入导出数据的接口
  void setModelData(const std::vector<ModelItem> &datas);
  void ClearModelData();
  std::vector<ModelItem>& getModelData() ;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  bool insertModelData(int row, const ModelItem &datas);
  QVariant data(const QModelIndex &index, int role) const override;
  //单元格的可操作性标志位，如可编辑，可选中等
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

Q_SIGNALS:
  void PointAdded();

private:
  QgsProfileWinow *mMapCanvas = nullptr;
  QWidget *m_parent = nullptr;
  QStringList m_header;
  std::vector<ModelItem> modelData;
  bool isconnected = false;
};

class APP_EXPORT QgsPcdpickeddlgWindowDockWidget : public QgsDockWidget, public Ui::pcdpickeddlg
{
  Q_OBJECT

public:
  explicit QgsPcdpickeddlgWindowDockWidget(const QString &name, QWidget *parent = nullptr);
  void setModel(QAbstractItemModel *model);

private:
  std::shared_ptr<polynomial3CurveFitter3> Fiter_Ptr = nullptr;

  bool insert_pt_table_(Point3D xyz, PointType type)
  {
    //this->alignedPointsTableWidget->setModel();
    return true;
  }

public slots:
  void OnPaiXuClicked(int column);
  // 重置 model 数据 清理
  void OnResetClicked();
  void OnPointAdded();
  void OnNiheButtonClicked();
};

class APP_EXPORT QgsDLWindowDockWidget : public QgsDockWidget, private Ui::QgsDLWindowDockWidgetBase
{
  Q_OBJECT

public:
  explicit QgsDLWindowDockWidget(const QString &name, QWidget *parent = nullptr);

  QgsProfileWinow *getmapCanvas();
  void setProfileWindow(QgsProfileWinow *window);
  void setMain3DWindow(QgsProfileWinow *window);

private slots:
  void OnmActiontiqudianlixianClicked();
  void dockpolynomial_dialog();
  void OnmActionSaveEditsClicked();
  void OnmselectiononprofileClciekd();
  void OnDrawPolygonOnProfileClicked();
  void OnmActionPickPoints();
  void OnmActionBrushPoints();
  void ApplyButtonClicked();
  void OndrawlieonprofileClicked();
  void OnmActionHandClicked();
  void rotatePointCloudLeft();
  void rotatePointCloudRight();
  void GetModelDataFromAoi();
  void OnInterpretPolygonChanged();

private:
  QgsProfileWinow *mMapCanvas = nullptr;
  QgsProfileWinow *mMainCanvas = nullptr;

  QRadioButton *mSyncExtentRadio = nullptr;
  QRadioButton *mSyncSelectionRadio = nullptr;
  QgsScaleComboBox *mScaleCombo = nullptr;
  QgsDoubleSpinBox *mMagnificationEdit = nullptr;
  QgsDoubleSpinBox *mScaleFactorWidget = nullptr;
  QCheckBox *mSyncScaleCheckBox = nullptr;
  bool Editing = false;
  bool isconnetwith_mMapCanvas = false;
  QString m_rule;
  QString m_method;
  QgsPcdpickeddlgWindowDockWidget *polynomial_dialog_widget = nullptr;
  std::shared_ptr<QgsDLAttributeTableModel> dltable = nullptr;
  std::vector<ModelItem> modeldatas; // 选中的电力线点
};

#endif // QGSDLWINDOWDOCKWIDGET_H
