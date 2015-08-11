/***************************************************************************
        qgstipfactory.cpp
        ---------------------
    begin                : February 2011
    copyright            : (C) 2007 by Tim Sutton
    email                : tim at linfiniti dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstipfactory.h"
#include <QTime>
//for rand & srand
#include <cstdlib>


QgsTipFactory::QgsTipFactory() : QObject()
{
  // I'm just doing this in a simple way so
  // its easy for translators...later
  // it its worth the time I'll move this data
  // into a sqlite database...
  QgsTip myTip;
  myTip.setTitle( tr( "QGIS is open source" ) );
  myTip.setContent( tr( "QGIS is open source software."
                        " This means that the software source code can be freely viewed"
                        " and modified. The GPL places a restriction that any modifications"
                        " you make must be made available in source form to whoever you give"
                        " modified versions to, and that you can not create a new version of"
                        " QGIS under a 'closed source' license. Visit"
                        " <a href=\"http://qgis.org\"> the QGIS home page (http://qgis.org)</a>"
                        " for more information."
                      ) );
  addGenericTip( myTip );
  //
  myTip.setTitle( tr( "QGIS Publications" ) );
  myTip.setContent( tr( "If you write a scientific paper or any other article"
                        " that refers to QGIS we would love to include your work"
                        " in the <a href=\"http://qgis.org/en/site/about/case_studies/index.html\">case studies section</a> of"
                        " the QGIS home page."
                      ) );
  addGenericTip( myTip );
  myTip.setTitle( tr( "Become a QGIS translator" ) );
  myTip.setContent( tr( "Would you like to see QGIS"
                        " in your native language? We are looking for more translators"
                        " and would appreciate your help! The translation process is"
                        " fairly straight forward - instructions are available in the"
                        " QGIS wiki"
                        " <a href=\"http://qgis.org/en/site/getinvolved/translate.html#howto-translate-gui\">translator's page.</a>"
                      ) );
  addGuiTip( myTip );
  myTip.setTitle( tr( "QGIS Mailing lists" ) );
  myTip.setContent( tr( "If you need help using QGIS"
                        " we have a 'users' mailing list where users help each other with issues"
                        " related to using our sofware. We also have a 'developers' mailing list."
                        " for those wanting help and to discuss things relating to the QGIS code base."
                        " Details on how to subscribe are in the <a href=\"http://qgis.org/en/site/forusers/support.html#mailing-lists\">community section</a> of"
                        " the QGIS home page."
                      ) );
  addGuiTip( myTip );
  myTip.setTitle( tr( "Is it 'QGIS' or 'Quantum GIS'?" ) );
  myTip.setContent( tr( "Both used to be correct, but we recently decided to just use 'QGIS'. For articles we suggest you write 'QGIS is ....'"
                      ) );
  addGenericTip( myTip );
  myTip.setTitle( tr( "How do I refer to QGIS?" ) );
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
                        " <a href=\"http://qt-project.org/doc/qt-5.0/qtcore/qdate.html#toString\">"
                        "QDate::toString format documentation</a> for the possible date formats."
                      ) );
  addGuiTip( myTip );
  myTip.setTitle( tr( "Moving Elements and Maps in the Print Composer" ) );
  myTip.setContent( tr( "In the print composer tool bar you can find two buttons for moving"
                        " elements. The left one (right arrow icon) selects and moves elements"
                        " in the layout. After selecting the element with this tool you can also"
                        " move them around with the arrow keys. For accurate positioning use the"
                        " <strong>Position and Size</strong> section, which can be found in the"
                        " tab <strong>Item Properties -> Position and Size</strong>."
                        " The other move tool (map canvas icon with right arrow) allows one"
                        " to move the map content within a map frame."
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
  myTip.setContent( tr( "In the Print Composer you can rotate a map by setting its rotation value"
                        " in the tab <strong>Item Properties -> Map -> Main properties</strong> section."
                        " To place a north arrow in your layout you can use the"
                        " <strong>Add Image</strong> tool. After the selection and"
                        " placement of the north arrow in the layout you can link it"
                        " with a specific map frame by activating the <strong>Sync with map</strong>"
                        " checkbox and selecting a map frame. Whenever you change the rotation"
                        " value of a linked map, the north arrow will now automatically adjust"
                        " its rotation."
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
                        " to draw. You can press 'Esc' (the escape key), or click the 'Stop"
                        " map rendering' icon in the status bar to the bottom of the window at any"
                        " time to halt rendering. If you are going to be performing several"
                        " actions (e.g. modifying symbology options) and wish to temporarily"
                        " disable map rendering while you do so, you can uncheck the 'Render'"
                        " checkbox in the bottom right of the status bar. Don't forget to check"
                        " it on again when you are ready to have the map draw itself again!"
                      ) );
  addGuiTip( myTip );
  // Tip contributed by Alister Hood
  myTip.setTitle( tr( "Join intersected polylines when rendering" ) );
  myTip.setContent( tr( "When applying layered styles to a polyline layer, you can join"
                        " intersecting lines together simply by enabling symbol levels."
                        " The image below shows a before (left) and after (right) view of"
                        " an intersection when symbol levels are enabled." ) +
                    QString( "<p><center><img src=\":/images/tips/symbol_levels.png\"/></center></p>" )
                  );
  addGuiTip( myTip );
  // by Tim
  myTip.setTitle( tr( "Auto-enable on the fly projection" ) );
  myTip.setContent( tr( "In the options dialog, under the CRS tab, you can set QGIS so that"
                        " whenever you create a new project, 'on the fly projection' is enabled"
                        " automatically and a pre-selected Coordinate Reference System of your"
                        " choice is used."
                      ) );
  addGuiTip( myTip );
  // by Tim
  myTip.setTitle( tr( "Sponsor QGIS" ) );
  myTip.setContent( tr( "If QGIS is saving you money or you like our work and"
                        " have the financial ability to help, please consider sponsoring the"
                        " development of QGIS. We use money from sponsors to pay for"
                        " travel and costs related to our regular hackfest meetings, and to generally"
                        " support the goals of our project. Please see the <a"
                        " href=\"http://qgis.org/en/site/getinvolved/governance/sponsorship/sponsorship.html\">QGIS Sponsorship Web"
                        " Page</a> for more details."
                      ) );
  addGenericTip( myTip );
  // by gsherman
  myTip.setTitle( tr( "QGIS has Plugins!" ) );
  myTip.setContent( tr( "QGIS has plugins that extend its functionality."
                        " QGIS ships with some core plugins you can explore from the"
                        " Plugins->Manage and Install Plugins menu. In addition there are a lot"
                        " of <a href=\"http://plugins.qgis.org/\">Python plugins </a>"
                        " contributed by the user community that can be"
                        " installed via this same menu. Don't miss out on all QGIS has to offer!"
                        " Check out the plugins and see what they can do for you."
                      ) );
  addGenericTip( myTip );
  // by yjacolin
  myTip.setTitle( tr( "Add an action to layer" ) );
  myTip.setContent( tr( "Action in a layer allow user to trigger action when clicking on a geometry"
                        " with 'Run Feature Action' tools."
                        "For example, you can open a HTML page using the field value of the geometry "
                        "as a parameter. Look at the <a href=\"http://docs.qgis.org/latest/en/docs/user_manual/working_with_vector/vector_properties.html?#actions-menu\">documentation</a>."
                      ) );
  addGuiTip( myTip );
  // by yjacolin
  myTip.setTitle( tr( "Copy, paste and cut in QGIS" ) );
  myTip.setContent( tr( "Copy, paste, and cut work as in another applications in QGIS. Select a "
                        "feature (a geometry or an attribut row in the attribute table) and use "
                        "one of this shortcuts: Ctrl+C to copy, Ctrl+X to cut, and Ctrl+V to paste."
                      ) );
  addGuiTip( myTip );
  // by yjacolin
  myTip.setTitle( tr( "Right click with identify tools" ) );
  myTip.setContent( tr( "Right click with the identify tool to show a context menu from which you can "
                        "choose the layer in which to identify a feature. A sub menu will list features "
                        "identified and a third sub-menu will show the action link setup for the layer."
                        "If one of this sub-menu doesn't contain any information, the next sub-menu"
                        "will appear instead. For example, if you have just one layer, and click "
                        "somewhere with several features, the first menu will list the feature list "
                        "instead of layer list."
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
  int myRand = qrand();
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
  int myRand = qrand();
  int myValue = static_cast<int>( myRand % mGenericTips.count() ); //range [0,(count-1)]
  QgsTip myTip = mGenericTips.at( myValue );
  return myTip;
}
QgsTip QgsTipFactory::getGuiTip()
{
  int myRand = qrand();
  int myValue = static_cast<int>( myRand % mGuiTips.count() ); //range [0,(count-1)]
  QgsTip myTip = mGuiTips.at( myValue );
  return myTip;
}

int QgsTipFactory::randomNumber( int theMax )
{
  Q_UNUSED( theMax );
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
