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

using namespace std;
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
#include "cdpwizardbase.uic.h"
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
      CDPWizard();
      CDPWizard( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
      ~CDPWizard();
    private:
      bool initialise();

      FileReader* fileReader;
      FileGroup* fileGroup;

      //debugging
      bool debugModeFlag;

      //the climate data proccessor does all the work
      ClimateDataProcessor *climateDataProcessor;


      public slots: // Public slots

          /** This method overrides the virtual CDPWizardBase method of the same name. */
          void cboFileType_activated( const QString &myQString );
      /** This method overrides the virtual CDPWizardBase method (slot) of the same name. */
      void formSelected(const QString &thePageNameQString);
};

#endif
