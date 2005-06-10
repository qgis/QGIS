//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef OMGUIMAIN_H
#define OMGUIMAIN_H

#include <qlabel.h>
#include <qstring.h>

#ifdef WIN32
  #include "omguireportbase.h"
  #include <omguimainbase.h>
#else
  #include "omguireportbase.uic.h"
  #include <omguimainbase.uic.h>
#endif

/**
@author Tim Sutton
*/
class OmGuiMain :public OmGuiMainBase
{
  Q_OBJECT
    public:
      OmGuiMain();

      ~OmGuiMain();
      void fileExit();
      void runWizard();
      void saveMapAsImage();
      public slots:
          void drawModelImage (QString theFileName);
      //! Log emitted from wizard when modek is done
      void modelDone(QString);

    private:
      QLabel * mPictureWidget;
      OmGuiReportBase *  mReport;
};

#endif
