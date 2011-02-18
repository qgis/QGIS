/***************************************************************************
 *   Copyright (C) 2007 by Tim Sutton   *
 *   tim@linfiniti.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "omgtipfactory.h"
#include <QTime>
//for rand & srand
#include <cstdlib> 


OmgTipFactory::OmgTipFactory() : QObject()
{
  // Im just doing this in a simple way so 
  // its easy for translators...later
  // it its worth the time Ill move this data
  // into a sqlite database...
  OmgTip myTip;
  myTip.setTitle(tr("openModeller is open source"));
  myTip.setContent(tr("openModeller is open source software."
        " This means that the software source code can be freely viewed "
        " and modified. The GPL places a restriction that any modifications "
        " you make must be made available to the openModeller project, and "
        " that you can not create a new version of openModeller under a "
        " 'closed source' license. Visit <a href=\"http://openModeller.sf.net\">"
        " the openModeller home page (http://openModeller.sf.net)</a> for more"
        " information."));
  addGenericTip(myTip);
  //
  myTip.setTitle(tr("openModeller Publications"));
  myTip.setContent(tr("If you write a scientific paper or any other article"
        " that refers to openModeller we would love to include your work"
        " in the references section of "
        " the openModeller home page (http://openModeller.sf.net).</a>"
        ));
  addGenericTip(myTip);
  myTip.setTitle(tr("Become an openModeller Desktop translator"));
  myTip.setContent(tr("Would you like to see openModeller Desktop"
        " in your native language? We are looking for more translators"
        " and would appreciate your help! The translation process is "
        " fairly straight forward - instructions are available in the "
        " resources section of "
        " the openModeller home page (http://openModeller.sf.net).</a>"
        ));
  addGuiTip(myTip);
  myTip.setTitle(tr("openModeller Mailing lists"));
  myTip.setContent(tr("If you need help using openModeller Desktop"
        " we have a mailing list where users help each other with issues"
        " related to niche modelling and using openModeller Desktop."
        " Details on how to subscribe are in the resources section of"
        " the openModeller home page (http://openModeller.sf.net).</a>"
        ));
  addGuiTip(myTip);
  myTip.setTitle(tr("Is it 'modelling' or 'modeling'?"));
  myTip.setContent(tr("Both spellings are correct. For openModeller"
        " we use the former spelling."
        ));
  addGenericTip(myTip);
  myTip.setTitle(tr("How do I refer to openModeller?"));
  myTip.setContent(tr("openModeller is spelled with a lower case"
        " 'o' at the start of the word - even if its the beginning"
        " of a sentance. We have various subprojects of the openModeller "
        " project and it will help to avoid confusion if you refer to each by"
        " its name:"
        "<ul>"
        "<li>openModeller Library - this is the C++ library that contains"
        " the core logic for carrying out niche modelling"
        "<li>openModeller Console - these are a collection of command"
        " line tools that allow you to run niche models from a unix or"
        " DOS shell, or from a scripting environment."
        "<li>openModeller Web Service - The openModeller Web Service"
        " allows for remote execution of niche models."
        "<li>openModeller Desktop - the openModeller Desktop provides"
        " a graphical user interface for the openModeller Library. It"
        " also includes the capability to run models using the"
        " openModeller Web Service (though this is still considered"
        " experimental)."
        "</ul>"
        ));
  addGenericTip(myTip);
  myTip.setTitle(tr("How can I improve model execution times?"));
  myTip.setContent(tr("Model processing time is typically determined by"
        "<ul>"
        "<li>the algorithm you select,</li>"
        "<li>the number, extents and spatial resolution of your format and environmental layers,</li>" 
        "<li>the number of cells excluded by your mask (if any),</li>"
        "<li>in some cases the number of presence and absence points (e.g. distance algs),</li>"
        "<li>the speed of the computer the model is running on (CPU, disk access etc).</li>"
        "</ul>"
        "So if you want to improve model processing times you need to adjust "
        "one of these variables. One thing noticed quite commonly is that people "
        "use extremely high resolution datasets that often carry little "
        "additional information over lower resolution equivalents. For example "
        "interpolating widely dispersed weather station data to produce a 50cm "
        "raster probably carries little additional value over for example using "
        "10m2 pixels.<br>"
        "Another area of performance improvement you can look at is "
        "preselecting environmental variables using techniques such as Chi "
        "Square test. Future versions of openModeller will integrate the ability "
        "to do this type of preselection."
        ));
  addGenericTip(myTip);
  /* Template for adding more tips
  myTip.setTitle(tr(""));
  myTip.setContent(tr(""
        ));
  addGenericTip(myTip);
  */
}

OmgTipFactory::~OmgTipFactory()
{

}
//private helper method
void OmgTipFactory::addGuiTip(OmgTip theTip)
{
  mGuiTips << theTip;
  mAllTips << theTip;
}
//private helper method
void OmgTipFactory::addGenericTip(OmgTip theTip)
{
  mGenericTips << theTip;
  mAllTips << theTip;
}
OmgTip OmgTipFactory::getTip()
{
  srand(QTime::currentTime().msec());
  int myRand = rand();
  int myValue = static_cast<int> (myRand % mAllTips.count()); //range [0,(count-1)]
  OmgTip myTip = mAllTips.at(myValue);
  return myTip;
}
OmgTip OmgTipFactory::getTip(int thePosition)
{
  OmgTip myTip = mAllTips.at(thePosition);
  return myTip;
}
OmgTip OmgTipFactory::getGenericTip()
{
  srand(QTime::currentTime().msec());
  int myRand = rand();
  int myValue = static_cast<int> (myRand % mGenericTips.count()); //range [0,(count-1)]
  OmgTip myTip = mGenericTips.at(myValue);
  return myTip;
}
OmgTip OmgTipFactory::getGuiTip()
{
  srand(QTime::currentTime().msec());
  int myRand = rand();
  int myValue = static_cast<int> (myRand % mGuiTips.count()); //range [0,(count-1)]
  OmgTip myTip = mGuiTips.at(myValue);
  return myTip;
}
int OmgTipFactory::randomNumber(int theMax)
{
  return 0;
}

