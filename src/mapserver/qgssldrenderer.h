/***************************************************************************
                              qgssldrenderer.h
                 A renderer flexible enough for sld specified symbolisation
                              -------------------
  begin                : May 12, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrenderer.h"
#include <QPixmap>

class QgsSLDRule;

/**A renderer specialised on sld parsing. The symbols can be ranges (lower and upper bounds different)
or values (lower an upper bounds the same) and the bounds can be including and excluding. The class does
also the parsing of the filters from a <Rule> sld element and maps them to */

class QgsSLDRenderer: public QgsRenderer
{
  public:
    QgsSLDRenderer( QGis::GeometryType type );
    virtual ~QgsSLDRenderer();

    /**Adds a new classification rule*/
    void addRule( QgsSLDRule* rule );
    /**Traverses the rule list until a suitable entry is found. If no rule matches, Qt::NoPen and Qt::NoBrush are set*/
    void renderFeature( QgsRenderContext &renderContext, QgsFeature& f, QImage* pic, bool selected, double opacity );
    /**This method is not needed because QgsSLDRenderer is not meant to provide reading from/ storing to project file*/
    int readXML( const QDomNode& rnode, QgsVectorLayer& vl )
    { Q_UNUSED( rnode ); Q_UNUSED( vl ); return 1; }
    /**This method is not needed because QgsSLDRenderer is not meant to provide reading from/ storing to project file*/
    bool writeXML( QDomNode & layer_node, QDomDocument & document, const QgsVectorLayer& vl ) const
    { Q_UNUSED( layer_node ); Q_UNUSED( document ); Q_UNUSED( vl ); return false; }
    /**Has to be decided at runtime depending upon mComparisonOperators and mClassificationFields*/
    bool needsAttributes() const;
    QgsAttributeList classificationAttributes() const;
    const QList<QgsSymbol*> symbols() const;
    /**This method is needed for the getLegendGraphics call*/
    void refreshLegend( std::list< std::pair<QString, QPixmap> >* symbologyList ) const;
    QgsRenderer* clone() const;
    bool containsPixmap() const;
    /**True if the renderer uses transparency on class level*/
    bool usesTransparency() const;
    QString name() const {return "SLD";}
    void setScaleDenominator( double denom ) {mScaleDenominator = denom;}

    //testing class is a friend
    friend class TestQgsSLDRenderer;

  private:

    /**List of SLD Rules in this SLD*/
    QList<QgsSLDRule*> mRules;
    /**The current scale denominator is used for rules with <minScaleDenominator> and/or <maxScaleDenominator> elements.
     This value has to be set by other classes (usually QgsWMSServer). If it is not set, the scale elements are ignored*/
    double mScaleDenominator;
    /**Normally false. Changes to true if QgsSymbols with alpha transparency are inserted*/
    bool mUseTransparencyOnClassLevel;
};
