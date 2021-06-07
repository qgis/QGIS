/***************************************************************************
    QgsProfileWindowDockWidget.cpp
    --------------------------
    begin                : February 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#include "qgsprofilewindowdockwidget.h"
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


QgsProfileWindowDockWidget::QgsProfileWindowDockWidget(const QString &name, QWidget *parent)
  : QgsDockWidget(parent)
{
  setupUi(this);
  //setAttribute( Qt::WA_DeleteOnClose );
  setWindowFlags(Qt::FramelessWindowHint);
  // QgsDockWidget::setWindowFlags(Qt::FramelessWindowHint);

  this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

  mContents->layout()->setContentsMargins(0, 0, 0, 0);
  mContents->layout()->setMargin(0);
  static_cast<QVBoxLayout *>(mContents->layout())->setSpacing(0);

  setWindowTitle(name);
  mToolbar->setIconSize(QgisApp::instance()->iconSize(true));

  connect(mActionToggleEditing, &QAction::triggered, this, &QgsProfileWindowDockWidget::OnmActionToggleEditingClicked);
  connect(mActionSaveEdits, &QAction::triggered, this, &QgsProfileWindowDockWidget::OnmActionSaveEditsClicked);
  connect(m_selectiononprofile, &QAction::triggered, this, &QgsProfileWindowDockWidget::OnmselectiononprofileClciekd);
  connect(m_drawlieonprofile_2, &QAction::triggered, this, &QgsProfileWindowDockWidget::OndrawlieonprofileClicked2);
  connect(m_drawlieonprofile, &QAction::triggered, this, &QgsProfileWindowDockWidget::OndrawlieonprofileClicked);

  connect(mActionPickPoints, &QAction::triggered, this, &QgsProfileWindowDockWidget::OnmActionPickPoints);
  connect(mActionBrush, &QAction::triggered, this, &QgsProfileWindowDockWidget::OnmActionBrushPoints);
  connect(showshaderparameter, &QAction::triggered, this, &QgsProfileWindowDockWidget::OnmActionsetshaderClicked);
  connect(mActionTurnLeft, &QAction::triggered, this, &QgsProfileWindowDockWidget::rotatePointCloudLeft);
  connect(mActionTurnRight, &QAction::triggered, this, &QgsProfileWindowDockWidget::rotatePointCloudRight);
  connect(mActionPan, &QAction::triggered, this, &QgsProfileWindowDockWidget::OnmActionHandClicked);

  mActionSaveEdits->setDisabled(true);
  m_selectiononprofile->setDisabled(true);
  m_drawlieonprofile->setDisabled(true);
  m_drawlieonprofile_2->setDisabled(true);
  mActionSinglePointPen->setDisabled(true);
  mActionPickPoints->setDisabled(true);
  mActionBrush->setDisabled(true);
}

QgsProfileWinow *QgsProfileWindowDockWidget::getmapCanvas()
{
  return mMapCanvas;
}

void QgsProfileWindowDockWidget::setProfileWindow(QgsProfileWinow * window)
{
  mMapCanvas = window;
  if (profile_widget == nullptr )
  {
    profile_widget = new QWidget(this);
    profile_widget->setMaximumWidth(180);
    profile_widget->setMinimumWidth(100);
  }

  ///--------ui------- 
  mMapCanvas->setShaderParamsUIWidget(profile_widget);
  QFile shaderFile("shaders:m_las_points.glsl");
  shaderFile.open(QIODevice::ReadOnly);
  QByteArray src = shaderFile.readAll();
  mMapCanvas->shaderProgram().setShader(src);
  //mMapCanvas->LockXYZ(4);
  horizontalLayout->addWidget(mMapCanvas);

  mMapCanvas->setOpenHandCursor();
  connect(mActionPan, SIGNAL(triggered()), mMapCanvas, SLOT(setOpenHandCursor()));
  // connect(mActionPan, &QAction::triggered, mMapCanvas,  SLOT(setOpenHandCursor()));
}

void QgsProfileWindowDockWidget::setMain3DWindow(QgsProfileWinow * window)
{
  mMainCanvas = window;
}
void QgsProfileWindowDockWidget::setclassdock(QgsClassSettingWindowDockWidget* dock)
{
  classdock = dock;
}
void QgsProfileWindowDockWidget::OnmActionSaveEditsClicked()
{
  mMapCanvas->applyclass(classdock->getoriginalClass(), classdock->getTargetClass(), m_rule, m_method);
}
void QgsProfileWindowDockWidget::OnmselectiononprofileClciekd()
{
  mMapCanvas->StartInterpretMode(0);
  m_rule = QString("Box");
  m_method = QString("override");
}

void QgsProfileWindowDockWidget::OndrawlieonprofileClicked2()
{
  mMapCanvas->StartInterpretMode(2);//1 ,2 
  m_rule = QString("Line down");
  m_method = QString("override");
}
void QgsProfileWindowDockWidget::OnmActionPickPoints()
{
  mMapCanvas->StartPickingMode();
}
void QgsProfileWindowDockWidget::OnmActionBrushPoints()
{
  //mMapCanvas->StartBrushMode(originalClass, TargetClass);
}

void QgsProfileWindowDockWidget::ApplyButtonClicked()
{
  //int classID = mTable->item(mTable->rowCount() - 1, 1)->data(0).toInt();

  //mMapCanvas->applyclass();
}

void QgsProfileWindowDockWidget::OndrawlieonprofileClicked()
{
  mMapCanvas->StartInterpretMode(1);//1 ,2
  m_rule = QString("Line above");
  m_method = QString("override");
}

void  QgsProfileWindowDockWidget::OnmActionToggleEditingClicked()
{
  Editing = !Editing;
  if (!Editing)
  {
    mActionSaveEdits->setDisabled(true);
    m_selectiononprofile->setDisabled(true);
    m_drawlieonprofile->setDisabled(true);
    m_drawlieonprofile_2->setDisabled(true);
    mActionSinglePointPen->setDisabled(true);
    mActionPickPoints->setDisabled(true);
    mActionBrush->setDisabled(true);
  }
  if (Editing)
  {
    mActionSaveEdits->setEnabled(true);
    m_selectiononprofile->setEnabled(true);
    m_drawlieonprofile->setEnabled(true);
    m_drawlieonprofile_2->setEnabled(true);
    mActionSinglePointPen->setEnabled(true);
    mActionPickPoints->setEnabled(true);
    mActionBrush->setEnabled(true);
  }
}
static bool classforminserted = false;
static bool classtargetforminserted = false;

static bool inserted = false;
void QgsProfileWindowDockWidget::OnmActionsetshaderClicked()
{

  if (!inserted)
  {
    horizontalLayout->addWidget(profile_widget);
    profile_widget->show();
    profile_widget->setVisible(true);

  }
  else
  {
    horizontalLayout->removeWidget(profile_widget);
    profile_widget->hide();
    profile_widget->setVisible(false);
  }

  inserted = !inserted;
}

void QgsProfileWindowDockWidget::OnmActionHandClicked()
{
  mMapCanvas->resetState();
  QCursor mCursor(Qt::PointingHandCursor);
  this->setCursor(mCursor);
 
}


void QgsProfileWindowDockWidget::rotatePointCloudLeft()
{
  //mMapCanvas->m_camera.mouseDrag
  mMapCanvas->RotateCamera(10, true);
}


void QgsProfileWindowDockWidget::rotatePointCloudRight()
{
  mMapCanvas->RotateCamera(10, false);
}


///---------------------------------////

QgsClassSettingWindowDockWidget::QgsClassSettingWindowDockWidget(const QString &name, QWidget *parent)
  : QgsDockWidget(parent)
{
  setupUi(this);
  setWindowFlags(Qt::FramelessWindowHint);
  this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

  mContents->layout()->setContentsMargins(0, 0, 0, 0);
  mContents->layout()->setMargin(0);
  static_cast<QVBoxLayout *>(mContents->layout())->setSpacing(0);


  const QObjectList list = groupBox->children();

  for each (QObject* var in list)
  {
    if (var->metaObject()->className() == QString("QCheckBox"))
    {
      static_cast<QCheckBox*>(var)->setChecked(true);
      QString Name = var->objectName();
      int classID = Name.split("_").at(1).toInt();
      connect(static_cast<QCheckBox*>(var), &QCheckBox::stateChanged, this, &QgsClassSettingWindowDockWidget::OnCheckChanged);
      originalClass.insert(std::pair<int, bool>(classID, true));
    }
  }

  const QObjectList list2 = groupBox_2->children();

  for each (QObject* varc in list2)
  {
    if (varc->metaObject()->className() == QString("QRadioButton"))
    {
      connect(static_cast<QRadioButton*>(varc), SIGNAL(toggled(bool)), this, SLOT(OnCheckChanged2()));
    }
  }


  connect(pushButton, &QAbstractButton::clicked, this, &QgsClassSettingWindowDockWidget::CheckAll);
  connect(pushButton_2, &QAbstractButton::clicked, this, &QgsClassSettingWindowDockWidget::UnCheckAll);
  this->setWindowTitle(tr("类型"));
}

void QgsClassSettingWindowDockWidget::OnCheckChanged()
{
  //QObjectlist groupBox->children();
  const QObjectList list = groupBox->children();

  for each (QObject* var in list)
  {

    if (var->metaObject()->className() == QString("QCheckBox"))
    {

      QString Name = var->objectName();
      int classID = Name.split("_").at(1).toInt();
      originalClass[classID] = static_cast<QCheckBox*>(var)->isChecked();

    }
  }
}

void QgsClassSettingWindowDockWidget::OnCheckChanged2()
{
  //QObjectlist groupBox->children();
  const QObjectList list2 = groupBox_2->children();

  for each (QObject* varc in list2)
  {
    if (varc->metaObject()->className() == QString("QRadioButton") && static_cast<QRadioButton*>(varc)->isChecked())
    {
      QString Name = varc->objectName();
      TargetClass = Name.split("_").at(1);
    }
  }

}


void QgsClassSettingWindowDockWidget::CheckAll()
{

  //QObjectlist groupBox->children();
  const QObjectList list = groupBox->children();

  for each (QObject* var in list)
  {

    if (var->metaObject()->className() == QString("QCheckBox"))
    {
      static_cast<QCheckBox*>(var)->setChecked(true);

    }
  }

}

void QgsClassSettingWindowDockWidget::UnCheckAll()
{
  //QObjectlist groupBox->children();
  const QObjectList list = groupBox->children();

  for each (QObject* var in list)
  {
    if (var->metaObject()->className() == QString("QCheckBox"))
    {
      static_cast<QCheckBox*>(var)->setChecked(false);
      //connect(static_cast<QCheckBox*>(var), &QCheckBox::stateChanged, this, SLOT(OnCheckChanged(classID)));

    }

  }
  originalClass.clear();
}

void  QgsClassSettingWindowDockWidget::myShowDock()
{
  if (!initialized)
  {
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    this->setObjectName(QStringLiteral("ShowProfileWindow"));
    this->setWhatsThis(tr("ShowProfileWindow"));
    this->setAllowedAreas(Qt::AllDockWidgetAreas);
    QgisApp::instance()->addDockWidget(Qt::RightDockWidgetArea, this);
    initialized = true;
  }
  this->show();
}
void  QgsClassSettingWindowDockWidget::myHideDock()
{
  this->hide();
}
