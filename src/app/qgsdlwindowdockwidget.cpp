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
#include <stdlib.h>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

using namespace gte;

/////-------------------------------电力线点ViewModel-------------------------------------------

QgsDLAttributeTableModel::QgsDLAttributeTableModel(QWidget *parent )
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

void QgsDLAttributeTableModel::receivepickedpoints(QVector3D pointxyz)
{
  ModelItem temp;
  temp.XYZ = Point3D(pointxyz.x(), pointxyz.y(), pointxyz.z());
  temp.error = 0;
  temp.type = PointType::Other;
  modelData.push_back(temp);
  setModelData(modelData);
  emit PointAdded();
}
void QgsDLAttributeTableModel::receivepoints(Point3D& pointxyz)
{
  ModelItem temp;
  temp.XYZ = pointxyz;
  temp.error = 0;
  temp.type = PointType::Other;
  modelData.push_back(temp);
  setModelData(modelData);
}

void QgsDLAttributeTableModel::setProfileWindow(QgsProfileWinow *window)
{
  mMapCanvas = window;

  if (!isconnected )
  {
    connect(mMapCanvas, &View3D::EmitPointXYZ, this, &QgsDLAttributeTableModel::receivepickedpoints);
    isconnected = true;
  }
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
std::vector<ModelItem>& QgsDLAttributeTableModel::getModelData() 
{
  return modelData;
};

int QgsDLAttributeTableModel::rowCount(const QModelIndex &parent ) const
{
  if (parent.isValid())
    return 0;
  return modelData.size();
};

int QgsDLAttributeTableModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;
  //返回表格列数
  return m_header.size();
};

QVariant QgsDLAttributeTableModel::headerData(int section, Qt::Orientation orientation, int role ) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    return m_header[section];
  return QAbstractTableModel::headerData(section, orientation, role);
};

bool QgsDLAttributeTableModel::insertModelData(int row, const ModelItem &datas)
{
  //row为0就是开始，为rowcount就在尾巴
  if (row < 0 || row > rowCount())
    return false;
  //需要将操作放到beginInsertRows和endInsertRows两个函数调用之间
  beginInsertRows(QModelIndex(), row, row);
  //在接口对应行插入空数据
  std::vector<ModelItem>::iterator it = modelData.begin() + row;
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
    case 0:
      return QString::number(modelData.at(row).XYZ.x, 'f', 3); // .toFloat();
    case 1:
      return QString::number(modelData.at(row).XYZ.y, 'f', 3); // .toFloat();
    case 2:
      return QString::number(modelData.at(row).XYZ.z, 'f', 3); // .toFloat();
    case 3:
      return modelData.at(row).type;
    case 4:
      return QString::number(modelData.at(row).error, 'f', 3); // .toFloat();
    }
  }
  return QVariant();
};

//单元格的可操作性标志位，如可编辑，可选中等
Qt::ItemFlags QgsDLAttributeTableModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);
  if (index.row() != index.column())
    flags |= Qt::ItemIsEditable;
  return flags;
};

bool QgsDLAttributeTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  //将界面修改的值进行保存
  if (index.isValid() && role == Qt::EditRole)
  {
    const int row = index.row();
    switch (index.column())
    {
    case 0:
      modelData.at(row).XYZ.x = value.toDouble();
    case 1:
      modelData.at(row).XYZ.y = value.toDouble();
    case 2:
      modelData.at(row).XYZ.z = value.toDouble();
    case 3:
      modelData.at(row).type = PointType(value.toInt());
    case 4:
      modelData.at(row).error = value.toDouble();
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
  connect(m_selectiononprofile, &QAction::triggered, this, &QgsDLWindowDockWidget::OnmselectiononprofileClciekd);
  connect(action_polygonselectiononprofile, &QAction::triggered, this, &QgsDLWindowDockWidget::OnDrawPolygonOnProfileClicked);
  connect(m_multipolygonselectiononprofile, &QAction::triggered, this, &QgsDLWindowDockWidget::OnMultiPolygonSelectiononprofileClciekd);

  mActionSaveEdits->setDisabled(true);
  mActionPickPoints->setDisabled(true);
  polynomial_dialog_widget = new QgsPcdpickeddlgWindowDockWidget("电力线拟合", QgisApp::instance());
  polynomial_dialog_widget->hide();
  polynomial_dialog_widget->setVisible(false);

  dltable = std::make_shared<QgsDLAttributeTableModel>(this);
  polynomial_dialog_widget->alignedPointsTableView->setModel(dltable.get());
  connect(dltable.get(), &QgsDLAttributeTableModel::PointAdded, polynomial_dialog_widget, &QgsPcdpickeddlgWindowDockWidget::OnPointAdded);

  //QHeaderView *headerGoods = polynomial_dialog_widget->alignedPointsTableView->horizontalHeader();
  //connect(headerGoods, SIGNAL(sectionClicked(int)), dltable.get(), SLOT(sortByColumn(int)));
}

QgsProfileWinow *QgsDLWindowDockWidget::getmapCanvas()
{
  return mMapCanvas;
}

void QgsDLWindowDockWidget::setProfileWindow(QgsProfileWinow *window)
{
  mMapCanvas = window;
  horizontalLayout->addWidget(mMapCanvas);
  mMapCanvas->setOpenHandCursor();
  if (! isconnetwith_mMapCanvas)
  {
    connect(mActionPan, SIGNAL(triggered()), mMapCanvas, SLOT(setOpenHandCursor()));
    connect(mMapCanvas, SIGNAL(Interpretpolygonchanged()), this, SLOT(OnInterpretPolygonChanged()));
    isconnetwith_mMapCanvas = true;
  }

  if (!(dltable == nullptr))
  {
    dltable->setProfileWindow(window);
  }
}

void QgsDLWindowDockWidget::setMain3DWindow(QgsProfileWinow *window)
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
    m_selectiononprofile->setDisabled(true);
    OnmActionHandClicked();
    //mMapCanvas->resetState();
  }
  if (Editing)
  {
    mActionSaveEdits->setEnabled(true);
    mActionPickPoints->setEnabled(true);
    m_selectiononprofile->setEnabled(true);
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
  mMapCanvas->resetState();
  mMapCanvas->StartProfileviewMode(false);
  mMapCanvas->StartInterpretMode(0);
  m_rule = QString("Box");

}

void QgsDLWindowDockWidget::OnMultiPolygonSelectiononprofileClciekd()
{
  //m_multipolygonselectiononprofile

  mMapCanvas->resetState();
  mMapCanvas->StartProfileviewMode(false);
  mMapCanvas->StartInterpretMode(3); //1 ,2
  m_rule = QString("multipolygon");

}

void QgsDLWindowDockWidget::OnDrawPolygonOnProfileClicked()
{
  mMapCanvas->resetState();
  mMapCanvas->StartProfileviewMode();
  mMapCanvas->StartInterpretMode(3); //1 ,2
  m_rule = QString("polygon");

}
void QgsDLWindowDockWidget::OnmActionPickPoints()
{
  mMapCanvas->resetState();
  mMapCanvas->StartProfileviewMode();
  mMapCanvas->StartPickingMode();
}
void QgsDLWindowDockWidget::OnmActionBrushPoints()
{
}

void QgsDLWindowDockWidget::ApplyButtonClicked()
{
}

void QgsDLWindowDockWidget::OndrawlieonprofileClicked()
{
  mMapCanvas->StartInterpretMode(1); //1 ,2
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

void QgsDLWindowDockWidget::OnInterpretPolygonChanged()
{
  if (m_rule == QString("multipolygon") )
  {

    std::vector<ModelItem> previus_models =dltable->getModelData();

    std::vector< V3f> points;

    if (!(dltable == nullptr))
    {
      if (mMapCanvas->GetPointsInPolygon(points));
      {
        std::vector<ModelItem> models;
        for (ModelItem previus_model : previus_models)
        {
          ModelItem model ( previus_model);
          models.push_back(model);
        }
       // models.insert(models.end(), current_models.begin(), current_models.end());
        //models.push_back(current_models);
        for (V3f pt : points)
        {
          ModelItem model;
          model.XYZ = pt;
          models.push_back(model);
        }
        dltable->setModelData(models);
        polynomial_dialog_widget->niheToolButton->setEnabled(true);
      }
    }
  }
  else if (m_rule == QString("polygon"))
  {
    std::vector< V3f> points;
    if (!(dltable == nullptr))
    {
      if (mMapCanvas->GetPointsInPolygon(points));
      {
        std::vector<ModelItem> models;
        for (V3f pt : points)
        {
          ModelItem model;
          model.XYZ = pt;
          models.push_back(model);
        }
        dltable->setModelData(models);
        polynomial_dialog_widget->niheToolButton->setEnabled(true);
      }
    }
  }
}

QgsPcdpickeddlgWindowDockWidget:: QgsPcdpickeddlgWindowDockWidget(const QString &name, QWidget *parent )
:Fiter_Ptr(nullptr)
{
  setupUi(this);
  setWindowFlags(Qt::FramelessWindowHint);
  this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
  this->setWindowTitle(name);
  this->alignedPointsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 自适应列宽
  this->alignedPointsTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);   //自适应行高
  this->pushButton->setEnabled(false);
  this->resetToolButton->setEnabled(true);
  this->alignedPointsTableView->sortByColumn(1, Qt::AscendingOrder); // x 列按照升序排序
  this->alignedPointsTableView->setSortingEnabled(true);
  this->pushButton_save->setEnabled(false);

  connect(niheToolButton, &QToolButton::clicked, this, &QgsPcdpickeddlgWindowDockWidget::OnNiheButtonClicked);
  connect(resetToolButton, &QToolButton::clicked, this, &QgsPcdpickeddlgWindowDockWidget::OnResetClicked);
  connect(pushButton, &QToolButton::clicked, this, &QgsPcdpickeddlgWindowDockWidget::onGenerateData);
  connect(pushButton_queren, &QToolButton::clicked, this, &QgsPcdpickeddlgWindowDockWidget::OnAcceptTemp_jiamidian);
  connect(pushButton_save, &QToolButton::clicked, this, &QgsPcdpickeddlgWindowDockWidget::OnPushButton_save);
  
};

void QgsPcdpickeddlgWindowDockWidget::setModel(QAbstractItemModel *model)
{
  this->alignedPointsTableView->setModel(model);
  //this->resetToolButton->setEnabled(true);
}
void QgsPcdpickeddlgWindowDockWidget::OnPaiXuClicked(int column)
{
  bool ascending = (this->alignedPointsTableView->horizontalHeader()->sortIndicatorSection() == column && this->alignedPointsTableView->horizontalHeader()->sortIndicatorOrder() == Qt::DescendingOrder);
  Qt::SortOrder order = ascending ? Qt::AscendingOrder : Qt::DescendingOrder;
  this->alignedPointsTableView->horizontalHeader()->setSortIndicator(column, order);

  this->alignedPointsTableView->model()->sort(column, order);
  this->alignedPointsTableView->setSortingEnabled(false);
}

void QgsPcdpickeddlgWindowDockWidget::OnResetClicked()
{
  dynamic_cast<QgsDLAttributeTableModel *>(this->alignedPointsTableView->model())->ClearModelData();
  this->alignedPointsTableView->setModel(this->alignedPointsTableView->model());
  this->niheToolButton->setEnabled(false);
}

void QgsPcdpickeddlgWindowDockWidget::OnPointAdded()
{
  if (this->alignedPointsTableView->model()->rowCount() > 9)
  {
    this->niheToolButton->setEnabled(true);
  }
  else
  {
    this->niheToolButton->setEnabled(false);
  }
}

std::string convertDoubleToString(const float value, const int precision = 0)
{
  std::stringstream stream;
  stream.clear();
  stream << std::fixed << std::setprecision(precision) << value;
  return stream.str();
}



void QgsPcdpickeddlgWindowDockWidget::OnNiheButtonClicked()
{

  std::vector<ModelItem> pointsdata= dynamic_cast<QgsDLAttributeTableModel *>(this->alignedPointsTableView->model())->getModelData();
   int sanweijie = this->spinBox->value();
   int erweijie = this->spinBox_2->value();
   double gaizhengshu =this->gaizhengshu->value();

  // this->gaizhegn
   //gaizhengxishu->
   if (this->checkBox_MapToOne->isChecked())
   {
     Fiter_Ptr = std::make_shared<polynomial3CurveFitter3>(sanweijie, erweijie,1, gaizhengshu);
   }
   else
   {
     Fiter_Ptr = std::make_shared<polynomial3CurveFitter3>(sanweijie, erweijie,0, gaizhengshu);
   }

    bool beginok = Fiter_Ptr->BeginReceiveData();
    if (!beginok)
    {
      //TODO::弹出报警框
      return;
    }
    for (ModelItem var : pointsdata)
    {
      std::array<float, 3> point = { var.XYZ.x,  var.XYZ.y , var.XYZ.z };
      Fiter_Ptr->ReceivePointDataXYZ(point);
    }
    bool endok = Fiter_Ptr->EndReceiveData();
    if (!endok)
    {
      //TODO::弹出报警框
      return;
    }

    int numpts = Fiter_Ptr->SetInterVal(0.05);
    if (numpts)
    {
      this->pushButton->setEnabled(true);
    }
   
    if (numpts <1)
    {
      //TODO::弹出报警框
      return;
    }
}
void QgsPcdpickeddlgWindowDockWidget:: onGenerateData()
{
  int numpts_generated = Fiter_Ptr->GenerateXYZSeries();
  if (numpts_generated < 1)
  {
    //TODO::弹出报警框
    return;
  }
  else
  {
     temp_jiamidian.clear();
     temp_jiamidian = Fiter_Ptr->GetGeneratedPoints();
     this->pushButton_queren->setEnabled(true);
     this->pushButton->setEnabled(false);
     this->pushButton_save->setEnabled(true);
  }

  // todo:: 推送到 opengl窗口 进行显示(使用红色） ，暂时不保存。
  //确认接受后，push到 全局加密点  
}

void QgsPcdpickeddlgWindowDockWidget:: OnAcceptTemp_jiamidian()
{
  if (temp_jiamidian.size()>0)
  {
    for (std::array<float, 3> pt : temp_jiamidian)
    {
      global_jiamidian.push_back(pt);
    }
    temp_jiamidian.clear();
    this->pushButton_queren->setEnabled(false);
    this->pushButton_save->setEnabled(false);
  }
  else
  {
    //TODO::弹出 警告框
  }
}

void QgsPcdpickeddlgWindowDockWidget::OnPushButton_save()
{
 // this->cunchu
 QString path = this->cunchulujing->text();
  std::string filename(path.toStdString());
  if (temp_jiamidian.size()>1)
  {
    this->savepoints(temp_jiamidian, filename);
  }
}

void QgsPcdpickeddlgWindowDockWidget::savepoints(std::vector<std::array<float, 3>>& jiamidian, std::string filename)
{
  

  int x = jiamidian.size();
  bool existing;
  std::ofstream fOut(filename);
  // 判断文件是否存在，
  if (existing)
  {
    //std::ofstream(filename); //  追加模式
  }
  else
  {
    //fOut = std::ofstream(filename); /// 新建
  }

  for (std::array<float, 3> pt : jiamidian)
  {
    fOut << convertDoubleToString(pt[0], 3) << "," << convertDoubleToString(pt[1], 3) << "," << convertDoubleToString(pt[2], 3) << std::endl;
  }
  fOut.close();

}
