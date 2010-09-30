/***************************************************************************
                              qgssldrule.h
                              ---------------------
  begin                : Feb 1, 2008
  copyright            : (C) 2008 by Marco Hugentobler
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

#ifndef QGSSLDRULE_H
#define QGSSLDRULE_H

#include "qgssymbol.h"
#include <QSet>

class QgsFeature;
class QgsFilter;
class QgsVectorDataProvider;
class QgsVectorLayer;
class QPainter;
class QDomElement;
class QTemporaryFile;

/**Represents a <Rule> in Styled Layer Descriptor, that is a filter to apply for a feature, min/max scale denominator
 and the symbology*/
class QgsSLDRule
{
  public:
    QgsSLDRule( double minDenom = 0, double maxDenom = 0, const QgsSymbol& s = QgsSymbol(), const QgsFilter* f = 0 );
    ~QgsSLDRule();
    int applySymbology( QPainter* p, QgsFeature& f, QImage* pic, bool selected, double widthScale, double rasterScaleFactor, QGis::GeometryType type, const QColor& selectionColor );
    /**Evaluates if this rule applies to a feature
       @param f the feature
       @param currentScaleDenominator. The current scale denominator of the map*/
    bool evaluate( const QgsFeature& f, double currentScaleDenominator ) const;
    /**Sets symbol, scale denominator and filter from xml <Rule> elements
       @param vl vector layer involved
       @param list of temporary files that are removed after each request
     @return 0 in case of success*/
    int setFromXml( const QDomElement& ruleElement, QgsVectorLayer* vl, QList<QTemporaryFile*>& filesToRemove );

    const QgsFilter* filter() const {return mFilter;}

    /**Returns a set of attribute indices needed in this rule (filter indices and if any\
        rotation attribute index and scaling attribute index */
    QSet<int> attributeIndices() const;

  private:
    /**Stores the symbology. Note that only pen/brush/point symbols are used from QgsSymbol
     and not information about lower/upper value. The validity of the symbol for a feature is only
     tested by mFilter*/
    QgsSymbol mSymbol;
    /**Minimum scale denominator. 0 if not set*/
    double mMinScaleDenominator;
    /**Maximum scale denominator. 0 if not set*/
    double mMaxScaleDenominator;
    /**(Optional) thematic feature filter*/
    QgsFilter* mFilter;

    /**Classification indices that are independent from the filter object (e.g. for data dependent rotation and scaling)*/
    QSet<int> mFilterIndependentClassificationIndices;

    //helper functions
    int symbologyFromLineSymbolizer( const QDomElement& lineSymbolizerElement, QPen& pen ) const;
    /**Provides a point symbol name, a pen and brush (for hardcoded markers) and a size. Returns 0 in case of success*/
    int symbologyFromPointSymbolizer( const QDomElement& pointSymbolizerElement, QgsSymbol& symbol, const QgsVectorDataProvider* dp, QList<QTemporaryFile*>& filesToRemove );
    /*
    int symbologyFromPointSymbolizer(const QDomElement& pointSymbolizerElement, QString& symbolName, QPen& pen
         QBrush& brush, int& size) const; */
    /**Sets a pen and a brush according to the contents of a polygon symbolizer. Returns 0 in case of success.*/
    int symbologyFromPolygonSymbolizer( const QDomElement& polySymbolizerElement, QPen& pen, QBrush& brush ) const;
    /**Applies the settings of a <Fill> tag to a QBrush*/
    int brushFromFillElement( const QDomElement& fillElement, QBrush& brush ) const;
    /**Sets brush properties from <SvgParameter> tags. Returns 0 in case of success*/
    int brushFromSvgParameters( const QDomElement& fillElement, QBrush& brush ) const;
    /**Sets brush image from svg pattern. Returns 0 in case of success*/
    int brushFromSvgPattern( const QDomElement& svgPatternElement, QBrush& brush ) const;

    /**Applies the settings of a <Stroke> tag to a QPen*/
    int penFromStrokeElement( const QDomElement& strokeElement, QPen& pen ) const;
};

#endif //QGSSLDRULE_H
