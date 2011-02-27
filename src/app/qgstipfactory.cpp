/***************************************************************************
 *   Copyright (C) 2007 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
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
  // I'm just doing this in a simple way so
  // its easy for translators...later
  // it its worth the time Ill move this data
  // into a sqlite database...
  QgsTip myTip;
  myTip.setTitle( tr( "Quantum GIS is open source" ) );
  myTip.setContent( tr( "Quantum GIS is open source software."
                        " This means that the software source code can be freely viewed"
                        " and modified. The GPL places a restriction that any modifications"
                        " you make must be made available to the Quantum GIS project, and"
                        " that you can not create a new version of Quantum GIS under a"
                        " 'closed source' license. Visit <a href=\"http://qgis.org\">"
                        " the QGIS home page (http://qgis.org)</a> for more"
                        " information." ) );
  addGenericTip( myTip );
  //
  myTip.setTitle( tr( "QGIS Publications" ) );
  myTip.setContent( tr( "If you write a scientific paper or any other article"
                        " that refers to QGIS we would love to include your work"
                        " in the <a href=\"http://www.qgis.org/en/community/qgis-case-studies.html\">case studies section</a> of"
                        " the Quantum GIS home page (http://http://www.qgis.org/en/community/qgis-case-studies.html)."
                      ) );
  addGenericTip( myTip );
  myTip.setTitle( tr( "Become an QGIS translator" ) );
  myTip.setContent( tr( "Would you like to see QGIS"
                        " in your native language? We are looking for more translators"
                        " and would appreciate your help! The translation process is"
                        " fairly straight forward - instructions are available in the"
                        " QGIS wiki"
                        " <a href=\"http://www.qgis.org/wiki/GUI_Translation\">translator's page (http://www.qgis.org/wiki/GUI_Translation).</a>"
                      ) );
  addGuiTip( myTip );
  myTip.setTitle( tr( "QGIS Mailing lists" ) );
  myTip.setContent( tr( "If you need help using QGIS"
                        " we have a 'users'  mailing list where users help each other with issues"
                        " related to using our sofware. We also have a 'developers' mailing list."
                        " for those wanting help and to discuss things relating the the QGIS code base."
                        " Details on how to subscribe are in the <a href=\"http://www.qgis.org/en/community/mailing-lists.html\">community section</a> of"
                        " the QGIS home page (http://www.qgis.org/en/community/mailing-lists.html)."
                      ) );
  addGuiTip( myTip );
  myTip.setTitle( tr( "Is it 'QGIS' or 'Quantum GIS'?" ) );
  myTip.setContent( tr( "Both are correct. For articles we suggest you write 'Quantum GIS (QGIS) is ....'"
                        " and then refer to it as QGIS thereafter."
                      ) );
  addGenericTip( myTip );
  myTip.setTitle( tr( "How do I refer to Quantum GIS?" ) );
  myTip.setContent( tr( "QGIS is spelled in all caps."
                        " We have various subprojects of the QGIS project"
                        " and it will help to avoid confusion if you refer to each by"
                        " its name:"
                        "<ul>"
                        "<li>QGIS Library - this is the C++ library that contains"
                        " the core logic that is used to build the QGIS user interface and other applications.</li>"
                        "<li>QGIS Application - this is the desktop application that you know and love so much :-).</li>"
                        "<li>QGIS Mapserver - this is a server-side application based on the QGIS Library"
                        " that will serve up your .qgs projects using the WMS protocol.</li>"
                        "</ul>"
                      ) );
  addGenericTip( myTip );
  // This tip contributed by Andreas Neumann
  myTip.setTitle( tr( "Add the current date to a map layout" ) );
  myTip.setContent( tr( "You can add a current date variable to your map"
                        " layout. Create a regular text label and add the string"
                        " $CURRENT_DATE(yyyy-MM-dd) to the text box. See the"
                        " <a href=\"http://doc.qt.nokia.com/latest/qdate.html#toString\">"
                        "QDate::toString format documentation</a> for the possible date formats."
                      ) );
  addGuiTip( myTip );
  myTip.setTitle( tr( "Moving Elements and Maps in the Print Composer" ) );
  myTip.setContent( tr( "In the print composer tool bar you can find two"
                        " buttons for moving elements. The left one (a selection cursor"
                        " with the hand symbol) selects and moves elements in the layout."
                        " After selecting the element with this tool you can also move"
                        " them around with the arrow keys. For accurate positioning use"
                        " the <strong>Position and Size</strong> dialogue, which can be"
                        " found in the tab <strong>Item &rarr; General Options &rarr; Position"
                        " and Size</strong>. For easier positioning you can also set"
                        " specific anchor points of the element within this dialogue."
                        " The other move tool (the globe icon combined with the hand icon)"
                        " allows to move the map content within a map frame."
                      ) );
  addGuiTip( myTip );
  // This  tip contributed by Andreas Neumann
  myTip.setTitle( tr( "Lock an element in the layout view" ) );
  myTip.setContent( tr( "By left clicking an element in the layout view you can"
                        " select it, by right clicking an element you can lock it. A lock symbol"
                        " will appear in the upper left corner of the selected element. This"
                        " prevents the element from accidentally being moved with the mouse. While"
                        " in a locked state, you cannot move an element with the mouse but you can"
                        " still move it with the arrow keys or by absolutely positioning it by"
                        " setting its <strong>Position and Size</strong>."
                      ) );
  addGuiTip( myTip );
  // This  tip contributed by Andreas Neumann
  myTip.setTitle( tr( "Rotating a map and linking a north arrow" ) );
  myTip.setContent( tr( "You can rotate a map by setting its rotation value"
                        " in the <strong>Item tab &rarr; Map</strong> section. To place a north"
                        " arrow in your layout you can use the <strong>Add Image</strong>"
                        " tool, the button with the little camera icon. QGIS comes with a"
                        " selection of north arrows. After the placement of the north arrow"
                        " in the layout you can link it with a specific map frame by activating"
                        " the <strong>Sync with map</strong> checkbox and selecting a map frame."
                        " Whenever you change the rotation value of a linked map, the north arrow"
                        " will now automatically adjust its rotation."
                      ) );
  addGuiTip( myTip );
  // This  tip contributed by Andreas Neumann
  myTip.setTitle( tr( "Numeric scale value in map layout linked to map frame" ) );
  myTip.setContent( tr( "If you want to place a text label as a placeholder for the"
                        " current scale, linked to a map frame, you need to place a scalebar and"
                        " set the style to 'Numeric'. You also need to select the map frame, if there"
                        " is more than one."
                      ) );
  addGuiTip( myTip );
  // by Tim
  myTip.setTitle( tr( "Using the mouse scroll wheel" ) );
  myTip.setContent( tr( "You can use the scroll wheel on your mouse to zoom in,"
                        " out and pan the map. Scroll forwards to zoom in, scroll backwards to"
                        " zoom out and press and hold the scroll wheel down to pan the map. You"
                        " can configure options for scroll wheel behaviour in the Options panel."
                      ) );
  addGuiTip( myTip );
  // by Tim
  myTip.setTitle( tr( "Stopping rendering" ) );
  myTip.setContent( tr( "Sometimes you have a very large dataset which takes ages"
                        " to draw. You can press 'esc' (the escape key), or click the small red"
                        " 'X' icon in the status bar to the bottom right of the window at any"
                        " time to halt rendering. If you are going to be performing several"
                        " actions (e.g. modifying symbology options) and wish to temporarily"
                        " disable map rendering while you do so, you can uncheck the 'Render'"
                        " checkbox in the bottom right of the status bar. Don't forget to check"
                        " it on again when you are ready to have the map draw itself again!"
                      ) );
  addGuiTip( myTip );
  /* Template for adding more tips
  myTip.setTitle(tr(""));
  myTip.setContent(tr(""
        ));
  addGuiTip(myTip);
  */
}

QgsTipFactory::~QgsTipFactory()
{

}
//private helper method
void QgsTipFactory::addGuiTip( QgsTip theTip )
{
  mGuiTips << theTip;
  mAllTips << theTip;
}
//private helper method
void QgsTipFactory::addGenericTip( QgsTip theTip )
{
  mGenericTips << theTip;
  mAllTips << theTip;
}
QgsTip QgsTipFactory::getTip()
{
  srand( QTime::currentTime().msec() );
  int myRand = rand();
  int myValue = static_cast<int>( myRand % mAllTips.count() ); //range [0,(count-1)]
  QgsTip myTip = mAllTips.at( myValue );
  return myTip;
}
QgsTip QgsTipFactory::getTip( int thePosition )
{
  QgsTip myTip = mAllTips.at( thePosition );
  return myTip;
}
QgsTip QgsTipFactory::getGenericTip()
{
  srand( QTime::currentTime().msec() );
  int myRand = rand();
  int myValue = static_cast<int>( myRand % mGenericTips.count() ); //range [0,(count-1)]
  QgsTip myTip = mGenericTips.at( myValue );
  return myTip;
}
QgsTip QgsTipFactory::getGuiTip()
{
  srand( QTime::currentTime().msec() );
  int myRand = rand();
  int myValue = static_cast<int>( myRand % mGuiTips.count() ); //range [0,(count-1)]
  QgsTip myTip = mGuiTips.at( myValue );
  return myTip;
}
int QgsTipFactory::randomNumber( int theMax )
{
  return 0;
}

int QgsTipFactory::position( QgsTip tip )
{
  for ( int i = 0; i < mAllTips.count(); ++i )
  {
    QgsTip myTip = mAllTips.at( i );
    if ( myTip.title() == tip.title() )
    {
      return i;
    }
  }
  return -1;
}

int QgsTipFactory::count()
{
  return mAllTips.count();
}
