#include "qgspatterndialog.h"
#include "qpushbutton.h"

QgsPatternDialog::QgsPatternDialog(QWidget* parent, const char* name, bool modal, WFlags fl): QgsPatternDialogBase(parent, name, modal,fl)
{
    QObject::connect(okbutton,SIGNAL(clicked()),this,SLOT(queryPattern()));
    QObject::connect(cancelbutton,SIGNAL(clicked()),this,SLOT(reject()));
    solid->toggle();//solid pattern is the default
}

Qt::BrushStyle QgsPatternDialog::pattern()
{
    return m_pattern;
}

void QgsPatternDialog::queryPattern()
{
    if(solid->isOn())
    {
	m_pattern=Qt::SolidPattern;
    }
    else if(fdiag->isOn())
    {
	m_pattern=Qt::FDiagPattern;
    }
    else if(dense4->isOn())
    {
	m_pattern=Qt::Dense4Pattern;
    }
    else if(horizontal->isOn())
    {
	m_pattern=Qt::HorPattern;
    }
    else if(dense5->isOn())
    {
	m_pattern=Qt::Dense5Pattern;
    }
    else if(diagcross->isOn())
    {
	m_pattern=Qt::DiagCrossPattern;
    }
    else if(dense1->isOn())
    {
	m_pattern=Qt::Dense1Pattern;
    }
    else if(dense6->isOn())
    {
	m_pattern=Qt::Dense6Pattern;
    }
    else if(vertical->isOn())
    {
	m_pattern=Qt::VerPattern;
    }
    else if(dense7->isOn())
    {
	m_pattern=Qt::Dense7Pattern;
    }
    else if(cross->isOn())
    {
	m_pattern=Qt::CrossPattern;
    }
    else if(dense2->isOn())
    {
	m_pattern=Qt::Dense2Pattern;
    }
    else if(bdiag->isOn())
    {
	m_pattern=Qt::BDiagPattern;
    }
    else if(dense3->isOn())
    {
	m_pattern=Qt::Dense3Pattern;
    }
    accept();
}
