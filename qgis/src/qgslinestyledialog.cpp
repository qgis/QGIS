#include "qgslinestyledialog.h"
#include "qpushbutton.h"
#include <iostream>

QgsLineStyleDialog::QgsLineStyleDialog(QWidget* parent, const char* name, bool modal, WFlags fl): QgsLineStyleDialogBase(parent,name,modal,fl)
{
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

