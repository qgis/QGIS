/***************************************************************************
  cdpwizard.h  -  description
  -------------------
begin                : Wed May 14 2003
copyright            : (C) 2003 by Tim Sutton
email                : t.sutton@reading.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef WIZARD_H
#define WIZARD_H

// include files for Qt
#include <qwidget.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qlineedit.h>
#include <qfiledialog.h>
// application specific includes
#include "filegroup.h"
#include "climatedataprocessor.h"
#include "dataprocessor.h"
#include "filereader.h"
#include "filewriter.h"

#ifdef WIN32
#include "cdpwizardbase.h"
#else
#include "cdpwizardbase.uic.h"
#endif

//other libs
#include <sstream>
#include <vector>
#include <string>



/**This is the inherited class from the QTDesigner CDPWizardBase.ui
 *@author Tim Sutton
 */

class CDPWizard : public CDPWizardBase
{
  Q_OBJECT
    public: 
    /** */
      CDPWizard();
      CDPWizard( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
      ~CDPWizard();


      void saveDefaults();
      void loadDefaults();
      void accept();
      
	void checkInputFilenames();
	void pbtnMeanTemp_clicked();
	void pbtnMinTemp_clicked();
	void pbtnMaxTemp_clicked();
	void pbtnDiurnalTemp_clicked();
	void pbtnMeanPrecipitation_clicked();
	void pbtnFrostDays_clicked();
	void pbtnTotalSolarRad_clicked();
	void pbtnOutputPath_clicked();
	void spinFirstYearToCalc_valueChanged( int theInt);
	void spinFirstYearInFile_valueChanged( int theInt);
	void spinLastYearToCalc_valueChanged( int theInt);
	void lstVariablesToCalc_selectionChanged();
	void pushButton9_clicked();
	void cboFileType_textChanged( const QString & );
	void leMeanTemp_textChanged( const QString & );
	void leMinTemp_textChanged( const QString & );
	void leMaxTemp_textChanged( const QString & );
	void leDiurnalTemp_textChanged( const QString & );
	void leMeanPrecipitation_textChanged( const QString & );
	void leFrostDays_textChanged( const QString & );
	void leTotalSolarRadiation_textChanged( const QString & );
	void leWindSpeed_textChanged( const QString & );


    private:
      bool initialise();

      FileReader* fileReader;
      FileGroup* fileGroup;

      
      bool debugModeFlag;

      //the climate data proccessor does all the work
      ClimateDataProcessor *climateDataProcessor;

      /**This checks whether any of the file input list boxes
       *are full and if not disables the next button      
       */

      


      public slots: // Public slots

          /** This method overrides the virtual CDPWizardBase method of the same name. */
          void cboFileType_activated( const QString &myQString );
      /** This method overrides the virtual CDPWizardBase method (slot) of the same name. */
      void formSelected(const QString &thePageNameQString);

      
      

  
      
      
      
      
      
};

#endif
