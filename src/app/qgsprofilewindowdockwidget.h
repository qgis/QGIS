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
#include "ui_qgsclasssettingwindowdockwidgetbase.h"
#include "ui_qgsDLwindowdockwidgetbase.h"
#include "ui_3DpointsPickedDlg.h"
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
  PointType type;
  double error;
};

class  QgsDLAttributeTableModel : public QAbstractTableModel
{
  Q_OBJECT
public:

  QgsDLAttributeTableModel(QgsProfileWinow *mapCanvas, QObject *parent = nullptr)
    : QAbstractTableModel(parent)
    , mMapCanvas(mapCanvas)
  {
      m_header.push_back("类型");
      m_header.push_back("X");
      m_header.push_back("Y");
      m_header.push_back("Z");
      modelData.clear();
  }

  //自定义导入导出数据的接口
  void setModelData(const std::vector<ModelItem> &datas)
  {
      beginResetModel();
      modelData = datas;
      endResetModel();
  };

  std::vector<ModelItem> getModelData() const
  {
     return modelData;
  };

  /**
 * Returns the number of rows
 * \param parent parent index
 */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override
  {
    if (parent.isValid())
      return 0;
    return modelData.size();
  };

  /**
 * Returns the number of columns
 * \param parent parent index
 */
  int columnCount(const QModelIndex &parent = QModelIndex()) const override
  {
    if (parent.isValid())
      return 0;
    //返回表格列数
    return  m_header.size();
  };

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
  {
    if (role == Qt::DisplayRole&&orientation == Qt::Horizontal)
      return m_header[section];
    return QAbstractTableModel::headerData(section, orientation, role);
  };

  bool insertModelData(int row, const ModelItem &datas)
  {
      //row为0就是开始，为rowcount就在尾巴
      if (row<0 || row>rowCount())
        return false;
      //需要将操作放到beginInsertRows和endInsertRows两个函数调用之间
      beginInsertRows(QModelIndex(), row, row);
      //在接口对应行插入空数据
      std::vector<ModelItem>::iterator  it = modelData.begin() + row;
      modelData.insert(it, datas);
      endInsertRows();
      return true;
  }

  QVariant data(const QModelIndex &index, int role) const override
  {
    if (!index.isValid())
      return QVariant();
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      const int row = index.row();
      switch (index.column())
      {
          case 0: return modelData.at(row).type;
          case 1: return modelData.at(row).XYZ.x;
          case 2: return  modelData.at(row).XYZ.y;
          case 3: return  modelData.at(row).XYZ.z;
          case 4: return  modelData.at(row).error;
      }
    }
    return QVariant();
  };

  //单元格的可操作性标志位，如可编辑，可选中等
  Qt::ItemFlags flags(const QModelIndex& index) const override
  {
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (index.row() != index.column())
      flags |= Qt::ItemIsEditable;
    return flags;
  };


  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
  {
    //将界面修改的值进行保存
    if (index.isValid() && role == Qt::EditRole)
    {
      const int row = index.row();
      switch (index.column())
      {
          case 0:  modelData.at(row).type = PointType(value.toInt());
          case 1:  modelData.at(row).XYZ.x = value.toDouble() ;
          case 2:  modelData.at(row).XYZ.y = value.toDouble(); 
          case 3:  modelData.at(row).XYZ.z = value.toDouble(); 
          case 4:  modelData.at(row).error =  value.toDouble();
      }
      //发送信号触发刷新
      emit dataChanged(index, index, QVector<int>() << role);
      return true;
    }
    return false;
  };

private:
  QgsProfileWinow * mMapCanvas = nullptr;

  QStringList m_header;
  std::vector<ModelItem> modelData;
};

class  APP_EXPORT  QgsPcdpickeddlgWindowDockWidget : public QgsDockWidget, private Ui::pcdpickeddlg
{
  Q_OBJECT

public:
  explicit QgsPcdpickeddlgWindowDockWidget(const QString &name, QWidget *parent = nullptr)
  {
    setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    this->setWindowTitle(name);
    this->alignedPointsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);// 自适应列宽
    this->alignedPointsTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高
    this->pushButton->setEnabled(false);
  };
private :
  bool insert_pt_table_(Point3D xyz , PointType type)
  {
    //this->alignedPointsTableWidget->setModel();
    return true;
  }

public  slots:
  void OnStartPointPicked(Point3D xyz)
  {

  }
  void OnEndPointPvicked(Point3D xyz)
  {

  }
  void OnXuanDuanPointPvicked(Point3D xyz)
  {
  }
  void OnOtherPointPvicked(Point3D xyz)
  {
  }
  void OnNiheButtonClicked(Point3D xyz)
  {

  }
  void OnBuDianButtonClicked(Point3D xyz)
  {
  }
};
class APP_EXPORT QgsClassSettingWindowDockWidget : public QgsDockWidget, private Ui::QgsClassSettingWindowDockWidgetBase
{
  Q_OBJECT
public:
  explicit QgsClassSettingWindowDockWidget(const QString &name, QWidget *parent = nullptr);
  std::map<int, bool> getoriginalClass()
  {
    return originalClass;
  }
  QString getTargetClass()
  {
    return TargetClass;
  }
private slots:
  void OnCheckChanged();
  void OnCheckChanged2();
  void CheckAll();
  void UnCheckAll();
  void myShowDock();
  void myHideDock();
private:
  bool initialized;
  QWidget* profile_widget;
  std::map<int, bool> originalClass;
  QString TargetClass;
};

class APP_EXPORT QgsProfileWindowDockWidget : public QgsDockWidget, private Ui::QgsProfileWindowDockWidgetBase
{
    Q_OBJECT
      friend QgsClassSettingWindowDockWidget;
  public:
    explicit QgsProfileWindowDockWidget( const QString &name, QWidget *parent = nullptr );

	QgsProfileWinow *getmapCanvas();
	void setProfileWindow(QgsProfileWinow * window);
	void setMain3DWindow(QgsProfileWinow * window);
  void setclassdock(QgsClassSettingWindowDockWidget* dock);

  private slots:
	void OnmActionSaveEditsClicked();
	void OnmselectiononprofileClciekd();

	void OndrawlieonprofileClicked2();

	void OnmActionPickPoints();

	void OnmActionBrushPoints();


	void ApplyButtonClicked();
	void OndrawlieonprofileClicked();
	void OnmActionToggleEditingClicked();
	void OnmActionsetshaderClicked();
	void OnmActionHandClicked();
  void rotatePointCloudLeft();
  void rotatePointCloudRight();

private:
    QgsProfileWinow *mMapCanvas = nullptr;
    QgsProfileWinow *mMainCanvas = nullptr;
    QgsClassSettingWindowDockWidget* classdock = nullptr;

    QRadioButton *mSyncExtentRadio = nullptr;
    QRadioButton *mSyncSelectionRadio = nullptr;
    QgsScaleComboBox *mScaleCombo = nullptr;
    QgsDoubleSpinBox *mMagnificationEdit = nullptr;
    QgsDoubleSpinBox *mScaleFactorWidget = nullptr;
    QCheckBox *mSyncScaleCheckBox = nullptr;
    QWidget* profile_widget = nullptr;
	  bool Editing = false;
	  QString m_rule;
	  QString m_method;
};


class APP_EXPORT QgsDLWindowDockWidget : public QgsDockWidget, private Ui::QgsDLWindowDockWidgetBase
{
  Q_OBJECT
    friend QgsClassSettingWindowDockWidget;
public:
  explicit QgsDLWindowDockWidget(const QString &name, QWidget *parent = nullptr);

  QgsProfileWinow *getmapCanvas();
  void setProfileWindow(QgsProfileWinow * window);
  void setMain3DWindow(QgsProfileWinow * window);
  void setclassdock(QgsClassSettingWindowDockWidget* dock);

private slots:
  void OnmActiontiqudianlixianClicked();
  void dockpolynomial_dialog();
  void OnmActionSaveEditsClicked();
  void OnmselectiononprofileClciekd();
  void OndrawlieonprofileClicked2();
  void OnmActionPickPoints();
  void OnmActionBrushPoints();
  void ApplyButtonClicked();
  void OndrawlieonprofileClicked();
  void OnmActionHandClicked();
  void rotatePointCloudLeft();
  void rotatePointCloudRight();

private:
  QgsProfileWinow *mMapCanvas = nullptr;
  QgsProfileWinow *mMainCanvas = nullptr;
  QgsClassSettingWindowDockWidget* classdock = nullptr;

  QRadioButton *mSyncExtentRadio = nullptr;
  QRadioButton *mSyncSelectionRadio = nullptr;
  QgsScaleComboBox *mScaleCombo = nullptr;
  QgsDoubleSpinBox *mMagnificationEdit = nullptr;
  QgsDoubleSpinBox *mScaleFactorWidget = nullptr;
  QCheckBox *mSyncScaleCheckBox = nullptr;
  bool Editing = false;
  QString m_rule;
  QString m_method;
  QgsPcdpickeddlgWindowDockWidget* polynomial_dialog_widget = nullptr;
};


#endif // QGSPROFILEWINDOWDOCKWIDGET_H

