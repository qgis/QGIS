/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qstring.h>
#include <sstream>
#include <qfiledialog.h>
#include "filereader.h"

void CDPWizardBase::pbtnMeanTemp_clicked()
{
    QString myFileNameQString = QFileDialog::getOpenFileName  
       ("sample_data/","",this,"Select Mean Temperature","Select Mean Temperature");
    leMeanTemp->setText(myFileNameQString);
}


void CDPWizardBase::pbtnMinTemp_clicked()
{
  QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Minimum Temperature","Select MinumumTemperature");
  leMinTemp->setText(myFileNameQString);
}


void CDPWizardBase::pbtnMaxTemp_clicked()
{
  QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Max Temperature","Select Max Temperature");
  leMaxTemp->setText(myFileNameQString);
}


void CDPWizardBase::pbtnDiurnalTemp_clicked()
{
  QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Diurnal Temperature","Select Diurnal Temperature");
  leDiurnalTemp->setText(myFileNameQString);
}


void CDPWizardBase::pbtnMeanPrecipitation_clicked()
{
  QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Mean Precipitation","Select Mean Precipitation");
  leMeanPrecipitation->setText(myFileNameQString);
}


void CDPWizardBase::pbtnFrostDays_clicked()
{
  QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Frost Days","Select Frost Days");
  leFrostDays->setText(myFileNameQString);
}


void CDPWizardBase::pbtnTotalSolarRad_clicked()
{
  QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Total Solar Radiation","Select Total Solar Radiation");
  leTotalSolarRadiation->setText(myFileNameQString);
}

void CDPWizardBase::pbtnOutputPath_clicked()
{
    QString myFileNameQString = QFileDialog::getExistingDirectory(QString::null,0, QString("select dir"), QString("select dir"), true, true);
  leOutputPath->setText(myFileNameQString);
}


void CDPWizardBase::spinFirstYearToCalc_valueChanged( int theInt)
{
    //make sure the number is valid
    if (theInt < spinFirstYearInFile->value())
    {
	spinFirstYearToCalc->setValue(spinFirstYearInFile->value());
    }
    if (spinFirstYearToCalc->value() > spinLastYearToCalc->value())
    {
	spinLastYearToCalc->setValue(spinFirstYearToCalc->value());
	leEndYearSummary->setText(QString::number(spinLastYearToCalc->value()));
    }
    leStartYearSummary->setText(QString::number(theInt));
}


void CDPWizardBase::spinFirstYearInFile_valueChanged( int theInt)
{
   //make sure the number is valid
    if (theInt > spinFirstYearToCalc->value())
    {
	    spinFirstYearToCalc->setValue(spinFirstYearInFile->value());
	    leStartYearSummary->setText(QString::number(spinFirstYearToCalc->value()));
    }
    if (theInt > spinLastYearToCalc->value())
    {
	    spinLastYearToCalc->setValue(spinFirstYearToCalc->value());
	    leEndYearSummary->setText(QString::number(spinLastYearToCalc->value()));
    }
    if (spinFirstYearToCalc->value() > spinLastYearToCalc->value())
    {
	    spinLastYearToCalc->setValue(spinFirstYearToCalc->value());
	    leEndYearSummary->setText(QString::number(spinLastYearToCalc->value()));
    }
}


void CDPWizardBase::spinLastYearToCalc_valueChanged( int theInt)
{
    //make sure the number is valid
    if (theInt < spinFirstYearToCalc->value())
    {
	    spinLastYearToCalc->setValue(spinFirstYearInFile->value());
    }

    leEndYearSummary->setText(QString::number(spinLastYearToCalc->value()));
}
/* This routine runs each time next is pressed to update the summary page */
void CDPWizardBase::formSelected(const QString  &thePageNameQString)
{

}

void CDPWizardBase::lstVariablesToCalc_selectionChanged()
{
    QString myQString;
    int selectionSizeInt=0;
    for ( unsigned int i = 0; i < lstVariablesToCalc->count(); i++ )
    {
        QListBoxItem *item = lstVariablesToCalc->item( i );
        // if the item is selected...
        if ( item->isSelected() )
        {
            // increment the count of selected items
           selectionSizeInt++;
        }
    }
    myQString.sprintf("<p align=\"right\">(%i) Variables selected </p>",selectionSizeInt);
    lblVariableCount->setText(myQString);

    if (!selectionSizeInt)
    {
	    //setNextEnabled(false); //doesnt work :-(
    }
    else
    {
    	//setNextEnabled(true);  //doesnt work :-(
    }

}




void CDPWizardBase::cboFileType_activated( const QString &myQString )
{
  //this will be implemented by the inherited Wizard class
}

void CDPWizardBase::pushButton9_clicked()
{

}
