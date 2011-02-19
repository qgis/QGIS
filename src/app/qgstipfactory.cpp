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


#include "qgstipfactory.h"
#include <QTime>
//for rand & srand
#include <cstdlib>


QgsTipFactory::QgsTipFactory() : QObject()
{
  // Im just doing this in a simple way so
  // its easy for translators...later
  // it its worth the time Ill move this data
  // into a sqlite database...
  QgsTip myTip;
  myTip.setTitle(tr("Quantum GIS is open source"));
  myTip.setContent(tr("Quantum GIS is open source software."
        " This means that the software source code can be freely viewed "
        " and modified. The GPL places a restriction that any modifications "
        " you make must be made available to the Quantum GIS project, and "
        " that you can not create a new version of Quantum GIS under a "
        " 'closed source' license. Visit <a href=\"http://qgis.org\">"
        " the QGIS home page (http://qgis.org)</a> for more"
        " information."));
  addGenericTip(myTip);
  //
  myTip.setTitle(tr("QGIS Publications"));
  myTip.setContent(tr("If you write a scientific paper or any other article"
        " that refers to QGIS we would love to include your work"
        " in the <a href=\"http://www.qgis.org/en/community/qgis-case-studies.html\">case studies section</a> of"
        " the Quantum GIS home page (http://http://www.qgis.org/en/community/qgis-case-studies.html)."
        ));
  addGenericTip(myTip);
  myTip.setTitle(tr("Become an QGIS translator"));
  myTip.setContent(tr("Would you like to see QGIS"
        " in your native language? We are looking for more translators"
        " and would appreciate your help! The translation process is "
        " fairly straight forward - instructions are available in the "
        " QGIS wiki"
        " <a href=\"http://www.qgis.org/wiki/GUI_Translation\">translator's page (http://www.qgis.org/wiki/GUI_Translation).</a>"
        ));
  addGuiTip(myTip);
  myTip.setTitle(tr("QGIS Mailing lists"));
  myTip.setContent(tr("If you need help using QGIS"
        " we have a 'users'  mailing list where users help each other with issues"
        " related to using our sofware. We also have a 'developers' mailing list."
        " for those wanting help and to discuss things relating the the QGIS code base."
        " Details on how to subscribe are in the <a href=\"http://www.qgis.org/en/community/mailing-lists.html\">community section</a> of"
        " the QGIS home page (http://www.qgis.org/en/community/mailing-lists.html)."
        ));
  addGuiTip(myTip);
  myTip.setTitle(tr("Is it 'QGIS' or 'Quantum GIS'?"));
  myTip.setContent(tr("Both are correct. For articles we suggest you write 'Quantum GIS (QGIS) is ....'"
        " and then refer to it as QGIS thereafter."
        ));
  addGenericTip(myTip);
  myTip.setTitle(tr("How do I refer to Quantum GIS?"));
  myTip.setContent(tr("QGIS is spelled in all caps."
        " We have various subprojects of the QGIS project "
        " and it will help to avoid confusion if you refer to each by"
        " its name:"
        "<ul>"
        "<li>QGIS Library - this is the C++ library that contains"
        " the core logic that is used to build the QGIS user interface and other applications.</li>"
        "<li>QGIS Application - this is the desktop application that you know and love so much :-).</li>"
        "<li>QGIS Mapserver - this is a server-side application based on the QGIS Library"
        " that will serve up your .qgs projects using the WMS protocol.</li>"
        "</ul>"
        ));
  addGenericTip(myTip);
  /* Template for adding more tips
  myTip.setTitle(tr(""));
  myTip.setContent(tr(""
        ));
  addGenericTip(myTip);
  */
}

QgsTipFactory::~QgsTipFactory()
{

}
//private helper method
void QgsTipFactory::addGuiTip(QgsTip theTip)
{
  mGuiTips << theTip;
  mAllTips << theTip;
}
//private helper method
void QgsTipFactory::addGenericTip(QgsTip theTip)
{
  mGenericTips << theTip;
  mAllTips << theTip;
}
QgsTip QgsTipFactory::getTip()
{
  srand(QTime::currentTime().msec());
  int myRand = rand();
  int myValue = static_cast<int> (myRand % mAllTips.count()); //range [0,(count-1)]
  QgsTip myTip = mAllTips.at(myValue);
  return myTip;
}
QgsTip QgsTipFactory::getTip(int thePosition)
{
  QgsTip myTip = mAllTips.at(thePosition);
  return myTip;
}
QgsTip QgsTipFactory::getGenericTip()
{
  srand(QTime::currentTime().msec());
  int myRand = rand();
  int myValue = static_cast<int> (myRand % mGenericTips.count()); //range [0,(count-1)]
  QgsTip myTip = mGenericTips.at(myValue);
  return myTip;
}
QgsTip QgsTipFactory::getGuiTip()
{
  srand(QTime::currentTime().msec());
  int myRand = rand();
  int myValue = static_cast<int> (myRand % mGuiTips.count()); //range [0,(count-1)]
  QgsTip myTip = mGuiTips.at(myValue);
  return myTip;
}
int QgsTipFactory::randomNumber(int theMax)
{
  return 0;
}

