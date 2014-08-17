/***************************************************************************
                         qgscomposermapoverview.h
                         --------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERMAPOVERVIEW_H
#define QGSCOMPOSERMAPOVERVIEW_H

#include "qgscomposermap.h"
#include <QString>
#include <QObject>

class QDomDocument;
class QDomElement;
class QPainter;
class QgsFillSymbolV2;

class CORE_EXPORT QgsComposerMapOverview : public QObject
{
    Q_OBJECT

  public:

    QgsComposerMapOverview( const QString& name, QgsComposerMap* map );
    ~QgsComposerMapOverview();

    void drawOverview( QPainter* painter ) const;

    /** stores state in Dom element
       * @param elem is Dom element corresponding to 'ComposerMap' tag
       * @param doc Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       * @param doc is Dom document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    void setName( const QString& name ) { mName = name; }
    QString name() const { return mName; }

    QString id() const { return mUuid; }

    void setEnabled( const bool enabled ) { mEnabled = enabled; }
    bool enabled() const { return mEnabled; }

    /**Sets overview frame map. -1 disables the overview frame
     * @note: this function was added in version 2.5
    */
    void setFrameMap( const int mapId );

    /**Returns id of overview frame (or -1 if no overfiew frame)
     * @note: this function was added in version 2.5
    */
    int frameMapId() const { return mFrameMapId; }

    void setFrameSymbol( QgsFillSymbolV2* symbol );

    QgsFillSymbolV2* frameSymbol() { return mFrameSymbol; }
    const QgsFillSymbolV2* frameSymbol() const { return mFrameSymbol; }

    /** Returns the overview's blending mode */
    QPainter::CompositionMode blendMode() const { return mBlendMode; }

    /** Sets the overview's blending mode*/
    void setBlendMode( const QPainter::CompositionMode blendMode );

    /** Returns true if the overview frame is inverted */
    bool inverted() const { return mInverted; }

    /** Sets the overview's inversion mode*/
    void setInverted( const bool inverted );

    /** Returns true if the extent is forced to center on the overview */
    bool centered() const { return mCentered; }

    /** Set the overview's centering mode */
    void setCentered( const bool centered );

    /**Reconnects signals for overview map, so that overview correctly follows changes to source
     * map's extent
    */
    void connectSignals();

  public slots:

    void overviewExtentChanged();

  private:
    QgsComposerMapOverview(); //forbidden

    QgsComposerMap* mComposerMap;
    QString mName;
    QString mUuid;

    bool mEnabled;

    /**Id of map which displays its extent rectangle into this composer map (overview map functionality). -1 if not present*/
    int mFrameMapId;

    /**Drawing style for overview farme*/
    QgsFillSymbolV2* mFrameSymbol;

    /**Blend mode for overview*/
    QPainter::CompositionMode mBlendMode;

    bool mInverted;

    /** Centering mode for overview */
    bool mCentered;

    void createDefaultFrameSymbol();
};

#endif // QGSCOMPOSERMAPOVERVIEW_H
