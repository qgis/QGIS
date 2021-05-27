/***************************************************************************
    QgsProfileWindowDockWidget.cpp
    --------------------------
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
 ***************************************************************************/
#include "qgsdlwindowdockwidget.h"

#include "qgsscalecombobox.h"
#include "qgsdoublespinbox.h"
#include "qgssettings.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include <QMessageBox>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QRadioButton>

#ifdef GTE_MATH
#include <Mathematics/ApprPolynomial3.h>
using namespace gte;
#endif // DEBUG





 /////-------------------------------电力线点ViewModel-------------------------------------------

QgsDLAttributeTableModel::QgsDLAttributeTableModel(QWidget *parent = nullptr)
  : QAbstractTableModel(parent)
{
  m_header.push_back("X");
  m_header.push_back("Y");
  m_header.push_back("Z");
  m_header.push_back("类型");
  m_header.push_back("误差");
  modelData.clear();
  m_parent = parent;
}


void  QgsDLAttributeTableModel::receivepickedpoints(QVector3D pointxyz)
{
  ModelItem temp;
  temp.XYZ = Point3D(pointxyz.x(), pointxyz.y(), pointxyz.z());
  temp.error = 0;
  temp.type = PointType::Other;
  modelData.push_back(temp);
  setModelData(modelData);
  emit PointAdded();
}

void QgsDLAttributeTableModel::setProfileWindow(QgsProfileWinow * window)
{
  mMapCanvas = window;

  connect(mMapCanvas, &View3D::EmitPointXYZ, this, &QgsDLAttributeTableModel::receivepickedpoints);
  //connect(mMapCanvas,);
}

void QgsDLAttributeTableModel::sortByColumn(int col)
{
  switch (col)
  {
    /*
     case 0: std::sort(modelData.begin(), modelData.end(), &ModelItem::compAsendingX); break;
     case 1: std::sort(modelData.begin(), modelData.end(), &ModelItem::compAsendingY); break;
     default: std::sort(modelData.begin(), modelData.end(), &ModelItem::compAsendingX); break;
     */
  }
  setModelData(modelData);
}

void QgsDLAttributeTableModel::setModelData(const std::vector<ModelItem> &datas)
{
  beginResetModel();
  modelData = datas;
  endResetModel();
};

void QgsDLAttributeTableModel::ClearModelData()
{
  beginResetModel();
  modelData.clear();
  endResetModel();
}
std::vector<ModelItem> QgsDLAttributeTableModel::getModelData() const
{
  return modelData;
};

int QgsDLAttributeTableModel::rowCount(const QModelIndex &parent = QModelIndex()) const 
{
  if (parent.isValid())
    return 0;
  return modelData.size();
};


int QgsDLAttributeTableModel::columnCount(const QModelIndex &parent = QModelIndex()) const 
{
  if (parent.isValid())
    return 0;
  //返回表格列数
  return  m_header.size();
};

QVariant  QgsDLAttributeTableModel:: headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
{
  if (role == Qt::DisplayRole&&orientation == Qt::Horizontal)
    return m_header[section];
  return QAbstractTableModel::headerData(section, orientation, role);
};

bool QgsDLAttributeTableModel::insertModelData(int row, const ModelItem &datas)
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

QVariant QgsDLAttributeTableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    const int row = index.row();
    switch (index.column())
    {
    case 0: return  QString::number(modelData.at(row).XYZ.x, 'f', 3);// .toFloat();
    case 1: return  QString::number(modelData.at(row).XYZ.y, 'f', 3);// .toFloat();
    case 2: return  QString::number(modelData.at(row).XYZ.z, 'f', 3);// .toFloat();
    case 3: return  modelData.at(row).type;
    case 4: return  QString::number(modelData.at(row).error, 'f', 3);// .toFloat();
    }
  }
  return QVariant();
};

//单元格的可操作性标志位，如可编辑，可选中等
Qt::ItemFlags QgsDLAttributeTableModel::flags(const QModelIndex& index) const 
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);
  if (index.row() != index.column())
    flags |= Qt::ItemIsEditable;
  return flags;
};


bool QgsDLAttributeTableModel::setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
{
  //将界面修改的值进行保存
  if (index.isValid() && role == Qt::EditRole)
  {
    const int row = index.row();
    switch (index.column())
    {
    case 0:  modelData.at(row).XYZ.x = value.toDouble();
    case 1:  modelData.at(row).XYZ.y = value.toDouble();
    case 2:  modelData.at(row).XYZ.z = value.toDouble();
    case 3:  modelData.at(row).type = PointType(value.toInt());
    case 4:  modelData.at(row).error = value.toDouble();
    }
    //发送信号触发刷新
    emit dataChanged(index, index, QVector<int>() << role);
    return true;
  }
  return false;
};
/////-------------------------------电力线工具-------------------------------------------

QgsDLWindowDockWidget::QgsDLWindowDockWidget(const QString &name, QWidget *parent)
  : QgsDockWidget(parent)
{
  setupUi(this);
  setWindowFlags(Qt::FramelessWindowHint);
  this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
  mContents->layout()->setContentsMargins(0, 0, 0, 0);
  mContents->layout()->setMargin(0);
  static_cast<QVBoxLayout *>(mContents->layout())->setSpacing(0);

  setWindowTitle(name);
  mToolbar->setIconSize(QgisApp::instance()->iconSize(true));
  connect(action_tiqudianlixian, &QAction::triggered, this, &QgsDLWindowDockWidget::OnmActiontiqudianlixianClicked);
  connect(mActionSaveEdits, &QAction::triggered, this, &QgsDLWindowDockWidget::OnmActionSaveEditsClicked);
  connect(mActionPickPoints, &QAction::triggered, this, &QgsDLWindowDockWidget::OnmActionPickPoints);
  connect(mActionTurnLeft, &QAction::triggered, this, &QgsDLWindowDockWidget::rotatePointCloudLeft);
  connect(mActionTurnRight, &QAction::triggered, this, &QgsDLWindowDockWidget::rotatePointCloudRight);

  mActionSaveEdits->setDisabled(true);
  mActionPickPoints->setDisabled(true);
  polynomial_dialog_widget = new QgsPcdpickeddlgWindowDockWidget("电力线拟合",QgisApp::instance());
  polynomial_dialog_widget->hide();
  polynomial_dialog_widget->setVisible(false);

  dltable = std::make_shared< QgsDLAttributeTableModel>(this);
  polynomial_dialog_widget->alignedPointsTableView->setModel(dltable.get());
  connect(dltable.get(), &QgsDLAttributeTableModel::PointAdded, polynomial_dialog_widget, &QgsPcdpickeddlgWindowDockWidget::OnPointAdded );

  //QHeaderView *headerGoods = polynomial_dialog_widget->alignedPointsTableView->horizontalHeader();
  //connect(headerGoods, SIGNAL(sectionClicked(int)), dltable.get(), SLOT(sortByColumn(int)));

}

QgsProfileWinow *QgsDLWindowDockWidget::getmapCanvas()
{
  return mMapCanvas;
}

void QgsDLWindowDockWidget::setProfileWindow(QgsProfileWinow * window)
{
  mMapCanvas = window;
  horizontalLayout->addWidget(mMapCanvas);
  mMapCanvas->setOpenHandCursor();
  connect(mActionPan, SIGNAL(triggered()), mMapCanvas, SLOT(setOpenHandCursor()));
 
  if (!(dltable ==nullptr))
  {
    dltable->setProfileWindow(window);
  }
}

void QgsDLWindowDockWidget::setMain3DWindow(QgsProfileWinow * window)
{
  mMainCanvas = window;
}

void QgsDLWindowDockWidget::OnmActiontiqudianlixianClicked()
{
  Editing = !Editing;
  if (!Editing)
  {
    mActionSaveEdits->setDisabled(true);
    mActionPickPoints->setDisabled(true);
    OnmActionHandClicked();
    mMapCanvas->resetState();
  }
  if (Editing)
  {
    mActionSaveEdits->setEnabled(true);
    mActionPickPoints->setEnabled(true);
  }
  dockpolynomial_dialog();
}

static bool polynomial_dialoginserted = false;
void QgsDLWindowDockWidget::dockpolynomial_dialog()
{
  if (!polynomial_dialoginserted)
  {
    QgisApp::instance()->addDockWidget(Qt::RightDockWidgetArea, polynomial_dialog_widget);
    polynomial_dialog_widget->show();
    polynomial_dialog_widget->setVisible(true);
    polynomial_dialog_widget->setAutoFillBackground(true);
  }
  else
  {
    QgisApp::instance()->removeDockWidget(polynomial_dialog_widget);
    polynomial_dialog_widget->hide();
    polynomial_dialog_widget->setVisible(false);
  }

  polynomial_dialoginserted = !polynomial_dialoginserted;
}

void QgsDLWindowDockWidget::OnmActionSaveEditsClicked()
{
 // mMapCanvas->applyclass(classdock->getoriginalClass(), classdock->getTargetClass(), m_rule, m_method);
}
void QgsDLWindowDockWidget::OnmselectiononprofileClciekd()
{
  mMapCanvas->StartInterpretMode(0);
  m_rule = QString("Box");
  m_method = QString("override");
}

void QgsDLWindowDockWidget::OndrawlieonprofileClicked2()
{
  mMapCanvas->StartInterpretMode(2);//1 ,2 
  m_rule = QString("Line down");
  m_method = QString("override");
}
void QgsDLWindowDockWidget::OnmActionPickPoints()
{
  mMapCanvas->StartPickingMode();
  /*
   if (mMapCanvas->ViewStateID == View3D::ViewState::PickingPoint)
  {
    connect(mMapCanvas, &View3D::EmitPointXYZ, dltable.get(), &QgsDLAttributeTableModel::receivepickedpoints);
  }
  else
  {
    disconnect(mMapCanvas, &View3D::EmitPointXYZ, dltable.get(), &QgsDLAttributeTableModel::receivepickedpoints);
  }
  */
}
void QgsDLWindowDockWidget::OnmActionBrushPoints()
{
}

void QgsDLWindowDockWidget::ApplyButtonClicked()
{
}

void QgsDLWindowDockWidget::OndrawlieonprofileClicked()
{
  mMapCanvas->StartInterpretMode(1);//1 ,2
  m_rule = QString("Line above");
  m_method = QString("override");
}



void QgsDLWindowDockWidget::OnmActionHandClicked()
{
  QCursor mCursor(Qt::PointingHandCursor);
  this->setCursor(mCursor);
}


void QgsDLWindowDockWidget::rotatePointCloudLeft()
{
  //mMapCanvas->m_camera.mouseDrag
  mMapCanvas->RotateCamera(10, true);
}


void QgsDLWindowDockWidget::rotatePointCloudRight()
{
  mMapCanvas->RotateCamera(10, false);
}

void QgsDLWindowDockWidget::GetModelDataFromAoi()
{
  modeldatas.clear();
  // 从aoi中获取 相应的点 构成 datas

}
