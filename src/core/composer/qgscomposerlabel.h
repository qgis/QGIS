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

#include "qgscomposeritem.h"

/** \ingroup MapComposer
 * A label that can be placed onto a map composition.
 */
class CORE_EXPORT QgsComposerLabel: public QgsComposerItem
{
  public:
    QgsComposerLabel( QgsComposition *composition );
    ~QgsComposerLabel();

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /**resizes the widget such that the text fits to the item. Keeps top left point*/
    void adjustSizeToText();

    QString text() {return mText;}
    void setText( const QString& text );

    /**Returns the text as it appears on screen (with replaced data field)
      @note this function was added in version 1.2*/
    QString displayText() const;

    QFont font() const;
    void setFont( const QFont& f );
    double margin() {return mMargin;}
    void setMargin( double m ) {mMargin = m;}

    /** stores state in Dom node
       * @param node is Dom node corresponding to 'Composer' tag
       * @param temp write template file
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param node is Dom node corresponding to 'ComposerLabel' tag
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

  private:
    // Text
    QString mText;

    // Font
    QFont mFont;

    // Border between text and fram (in mm)
    double mMargin;

    /**Replaces replace '$CURRENT_DATE<(FORMAT)>' with the current date (e.g. $CURRENT_DATE(d 'June' yyyy)*/
    void replaceDateText( QString& text ) const;
};

#endif


