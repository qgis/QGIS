/***************************************************************************
                         qgscomposeritem.h
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERITEM_H
#define QGSCOMPOSERITEM_H

#include "qgscomposition.h"

class QWidget;
class QCanvas;
class QDomNode;
class QDomDocument;

class QqsComposition;

class QgsComposerItem
{

public:
    QgsComposerItem();
    virtual ~QgsComposerItem(); 
public:
    /** \brief Set plot style */
    void setPlotStyle ( QgsComposition::PlotStyle p );

    /** \brief get plot style */
    QgsComposition::PlotStyle plotStyle ( void );

    /** \brief Set selected, selected item should be highlighted */
    virtual void setSelected( bool s );

    /** \brief Is selected */
    virtual bool selected( void );
    
    /** item id */
    int id ( void );

    /** Update otions and return pointer to options widget */
    virtual QWidget *options ( void );

    /** Create option controls
     * @param parent widget where options are created
     */
    virtual void showOptions ( QWidget * parent );

    /** stores state in project */
    virtual bool writeSettings ( void );

    /** read state from project */
    virtual bool readSettings ( void );

    /** delete settings from project file  */
    virtual bool removeSettings( void ); 

    /** stores state in DOM node
     * @param node is DOM node corresponding to '???' tag
     * @param temp write template file
     */
    virtual bool writeXML( QDomNode & node, QDomDocument & doc, bool templ = false );

    /** sets state from DOM document
     * @param node is DOM node corresponding to '???' tag
     */
    virtual bool readXML( QDomNode & node );

protected:
    QgsComposition::PlotStyle mPlotStyle;
    bool mSelected;
    int mId;

private:
};

#endif
