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

#include <qwidget.h>

#ifdef WIN32
#include <omguimainbase.h>
#else
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
  public slots:
    void drawModelImage (QString theFileName);
};

#endif
