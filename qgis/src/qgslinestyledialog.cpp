#include "qgslinestyledialog.h"
#include "qpushbutton.h"
#include <iostream>
#include "qgssymbologyutils.h"

QgsLineStyleDialog::QgsLineStyleDialog(QWidget* parent, const char* name, bool modal, WFlags fl): QgsLineStyleDialogBase(parent,name,modal,fl)
{
    //load the icons stored in QgsSymbologyUtils.cpp (to avoid redundancy)
    solid->setPixmap(QgsSymbologyUtils::char2LinePixmap("SolidLine"));
    dash->setPixmap(QgsSymbologyUtils::char2LinePixmap("DashLine"));
    dot->setPixmap(QgsSymbologyUtils::char2LinePixmap("DotLine"));
    dashdot->setPixmap(QgsSymbologyUtils::char2LinePixmap("DashDotLine"));
    dashdotdot->setPixmap(QgsSymbologyUtils::char2LinePixmap("DashDotDotLine"));

    QObject::connect(okbutton,SIGNAL(clicked()),this,SLOT(queryStyle()));
    QObject::connect(cancelbutton,SIGNAL(clicked()),this,SLOT(reject()));
    solid->toggle();//solid style is the default
}

Qt::PenStyle QgsLineStyleDialog::style()
{
    return m_style;
}

void QgsLineStyleDialog::queryStyle()
{
    if(solid->isOn())
    {
	m_style=Qt::SolidLine;
    }
    else if(dash->isOn())
    {
	m_style=Qt::DashLine;
    }
    else if(dot->isOn())
    {
	m_style=Qt::DotLine;
    }
    else if(dashdot->isOn())
    {
	m_style=Qt::DashDotLine;
    }
    else if(dashdotdot->isOn())
    {
	m_style=Qt::DashDotDotLine;
    }
    accept();
}

