/***************************************************************************
                          qgslegenditem.cpp  -  description
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
               Romans 3:23=>Romans 6:23=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */

#include <qlabel.h>
#include <qcheckbox.h>

#include "qgssymbol.h"
#include "qgsmaplayer.h"
#include "qgslegenditem.h"
#include "qgsproject.h"


// Construct a top-level QgsLegendItem

QgsLegendItem::QgsLegendItem(QgsMapLayer * lyr, QListView * parent, QAction * actionInOverview)
    : QCheckListItem(parent, lyr->name(), QCheckListItem::CheckBox), 
      m_layer(lyr),
      layerName( lyr->name() ),
      mActionInOverview( actionInOverview )
{
    // activate(); commented out because it was toggling layer visibility on,
    // even if it was off (due to activate() triggering update)
  setOn(lyr->visible());
  setPixmap( 0, *lyr->legendPixmap() );
}


// Construct a QgsLegendItem that is a child of another QgsLegendItem

QgsLegendItem::QgsLegendItem(QString name, QgsLegendItem * parent, QAction * actionInOverview)
    : QCheckListItem( parent, 
                      name, QCheckListItem::CheckBox), 
//    : QCheckListItem( (QCheckListItem*) parent, 
//                      "", QCheckListItem::CheckBox), 
      m_layer( parent->layer() ),
      layerName( name ),
      mActionInOverview( actionInOverview )
{
  //TODO: Make setOn more useful
  setOn( TRUE );
  
#ifdef QGISDEBUG
  std::cout << "QgsLegendItem::QgsLegendItem(subitem):  Adding '" << name << "'." << std::endl;
#endif
  
  
  //TODO: Pixmap (if appropriate for WMS sublayers?)
  //setPixmap( 0, *lyr->legendPixmap() );
}



QgsLegendItem::~QgsLegendItem()
{}


/** Write property of QString layerName. */
void QgsLegendItem::setLayerName(const QString & _newVal)
{
  layerName = _newVal;

  // commented out because this will cause the name to be rendered next to the
  // legend item pixmap, which <em>already</em> contains the layer name
  //setText( 0, _newVal );

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
} // QgsLegendItem::setLayerName()



// /** Write property of QString displayName. */
// void QgsLegendItem::setDisplayName(const QString & _newVal)
// {
//   displayName = _newVal;
// }

void QgsLegendItem::stateChange(ToggleState vis)
{
#ifdef QGISDEBUG
  std::cout << "QgsLegendItem::stateChange: setting visibility to '" << vis << "'." << std::endl;
#endif


  if (0 == depth())
  {
#ifdef QGISDEBUG
  std::cout << "QgsLegendItem::stateChange: depth is 0." << std::endl;
#endif
    if (On == vis)
    {
      m_layer->setVisible(TRUE);
    }
    else
    {
      m_layer->setVisible(FALSE);
    }  
  }
  else if (1 == depth())
  {
#ifdef QGISDEBUG
  std::cout << "QgsLegendItem::stateChange: depth is 1." << std::endl;
#endif
    // Sublayer, e.g. for WMS
    if (On == vis)
    {
      m_layer->setSubLayerVisibility( layerName, TRUE );
    }
    else
    {
      m_layer->setSubLayerVisibility( layerName, FALSE );
    }  
  }
  else
  {
#ifdef QGISDEBUG
  std::cout << "QgsLegendItem::stateChange: depth is other than 0 or 1." << std::endl;
#endif
    // undefined
  }
 
  // notify the project we've made a change
  QgsProject::instance()->dirty(true);

#ifdef QGISDEBUG
  std::cout << "QgsLegendItem::stateChange: exiting." << std::endl;
#endif

}

QgsMapLayer *QgsLegendItem::layer()
{
  return m_layer;
}


QString QgsLegendItem::layerID() const
{
    return m_layer->getLayerID();
} // layerID


void QgsLegendItem::setOn( bool b )
{
#ifdef QGISDEBUG
    std::cerr << __FILE__ << ":" << __LINE__ 
              << " setOn(" << b 
              << ")\n";
#endif
    // commented out because this would cause infinite loop since signals/slots handle this m_layer->setVisible( b );

    QCheckListItem::setOn( b );
} // setOn

