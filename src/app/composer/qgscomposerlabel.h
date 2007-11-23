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

#include "ui_qgscomposerlabelbase.h"
#include "qgscomposeritem.h"
#include <QAbstractGraphicsShapeItem>
#include <QFont>
#include <QPen>
#include <QPolygon>
#include <QRect>
#include <QString>

class QgsComposition;
class QDomNode;
class QDomDocument;

/** \class QgsComposerLabel 
 *  \brief Object representing label. 
 */
// NOTE: QgsComposerLabelBase must be first, otherwise does not compile
//class QgsComposerLabel : public QgsComposerLabelBase, public QCanvasRectangle, public QgsComposerItem
class QgsComposerLabel : public QWidget, private Ui::QgsComposerLabelBase, public QAbstractGraphicsShapeItem, public QgsComposerItem
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
    bool removeSettings ( void );
    bool writeXML( QDomNode & node, QDomDocument & document, bool temp = false );
    bool readXML( QDomNode & node );

    QRectF boundingRect ( void ) const; //errors about overriding things?
     
    /** \brief Reimplementation of QGraphicsItem::paint() - draw on canvas */
    void paint ( QPainter*, const QStyleOptionGraphicsItem*, QWidget* );

    QPolygonF areaPoints() const;
    
    /** \brief Set values in GUI to current values */
    void setOptions ( void );

public slots:
    // Open font dialog
    void on_mFontButton_clicked();

    void on_mTextEdit_textChanged();

    // Box settings changed
    void on_mBoxCheckBox_clicked();
    
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
    // Not used - would it be more effecient if paint() updated the bounding box, and boundingRect returned this?
    //QRect mBoundingRect; 

    // Draw box around the label
    bool mBox;

    // Box buffer
    double mBoxBuffer;
};

#endif
