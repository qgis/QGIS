/***************************************************************************
                         qgscomposerlabel.h
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
#ifndef QGSCOMPOSERLABEL_H
#define QGSCOMPOSERLABEL_H

#include <qwidget.h>
#include <qcanvas.h>
#include <qobject.h>

#include "qgsrect.h"

#include "qgscomposer.h"
#include "qgscomposition.h"
#include "qgscomposeritem.h"

#ifdef WIN32
#include "qgscomposerlabelbase.h"
#else
#include "qgscomposerlabelbase.uic.h"
#endif

class QCanvasItem;
class QCanvasRectangle;
class QPainter;
class QWidget;
class QDomNode;
class QDomDocument;
class QPixmap;
class QImage;
class QFont;
class QPen;
class QRect;


class QgsMapCanvas;
class QgsRect;
class QgsMapToPixel;
class QgsComposition;

/** \class QgsComposerLabel 
 *  \brief Object representing label. 
 */
// NOTE: QgsComposerLabelBase must be first, otherwise does not compile
//class QgsComposerLabel : public QgsComposerLabelBase, public QCanvasRectangle, public QgsComposerItem
class QgsComposerLabel : public QgsComposerLabelBase, public QCanvasPolygonalItem, public QgsComposerItem
{
    Q_OBJECT

public:
    /** \brief Constructor. Settings are written to project.  
     *  \param id object id
     *  \param fontSize font size in typographic points!
     */
    QgsComposerLabel( QgsComposition *composition, int id, int x, int y, QString text, int fontSize = 0 );

    /** \brief Constructor. Settings are read from project.  
     *  \param id object id
     */
    QgsComposerLabel( QgsComposition *composition, int id );

    ~QgsComposerLabel();

    // Reimplement QgsComposerItem:
    void setSelected( bool s );
    bool selected( void );
    QWidget *options ( void );
    bool writeSettings ( void );
    bool readSettings ( void );
    bool writeXML( QDomNode & node, QDomDocument & document, bool temp = false );
    bool readXML( QDomNode & node );

    QRect boundingRect ( void ) const;
     
    /** \brief Reimplementation of QCanvasItem::draw - draw on canvas */
    void draw ( QPainter & painter );

    void drawShape(QPainter&);
    QPointArray areaPoints() const;
    
    /** \brief Set values in GUI to current values */
    void setOptions ( void );

public slots:
    // Open font dialog
    void changeFont ( void );

    void textChanged ( void );
    
private:
    // Pointer to composition
    QgsComposition *mComposition;


    
    // Text 
    QString mText;

    // Font. Font size in typographic points!
    QFont mFont;

    // Pen
    QPen  mPen;

    // Margin in box
    int mMargin;

    // Current bounding box
    QRect mBoundingRect; 
};

#endif
