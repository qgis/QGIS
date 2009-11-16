#ifndef QGSGRADUATEDSYMBOLRENDERERV2_H
#define QGSGRADUATEDSYMBOLRENDERERV2_H

#include "qgsrendererv2.h"

class CORE_EXPORT QgsRendererRangeV2
{
  public:
    QgsRendererRangeV2( double lowerValue, double upperValue, QgsSymbolV2* symbol, QString label );
    QgsRendererRangeV2( const QgsRendererRangeV2& range );

    ~QgsRendererRangeV2();

    double lowerValue() const;
    double upperValue() const;

    QgsSymbolV2* symbol() const;
    QString label() const;

    void setSymbol( QgsSymbolV2* s );
    void setLabel( QString label );

    // debugging
    QString dump();

  protected:
    double mLowerValue, mUpperValue;
    QgsSymbolV2* mSymbol;
    QString mLabel;
};

typedef QList<QgsRendererRangeV2> QgsRangeList;

class QgsVectorLayer;
class QgsVectorColorRampV2;

class CORE_EXPORT QgsGraduatedSymbolRendererV2 : public QgsFeatureRendererV2
{
  public:
    QgsGraduatedSymbolRendererV2( QString attrName = QString(), QgsRangeList ranges = QgsRangeList() );

    virtual ~QgsGraduatedSymbolRendererV2();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );

    virtual void startRender( QgsRenderContext& context, const QgsFieldMap& fields );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    virtual QString dump();

    virtual QgsFeatureRendererV2* clone();

    virtual QgsSymbolV2List symbols();

    QString classAttribute() const { return mAttrName; }
    void setClassAttribute( QString attr ) { mAttrName = attr; }

    const QgsRangeList& ranges() { return mRanges; }

    bool updateRangeSymbol( int rangeIndex, QgsSymbolV2* symbol );
    bool updateRangeLabel( int rangeIndex, QString label );

    enum Mode
    {
      EqualInterval,
      Quantile,
      Custom
    };

    Mode mode() const { return mMode; }
    void setMode( Mode mode ) { mMode = mode; }

    static QgsGraduatedSymbolRendererV2* createRenderer(
      QgsVectorLayer* vlayer,
      QString attrName,
      int classes,
      Mode mode,
      QgsSymbolV2* symbol,
      QgsVectorColorRampV2* ramp );

    //! create renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& element );

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    QgsSymbolV2* sourceSymbol();
    void setSourceSymbol( QgsSymbolV2* sym );

    QgsVectorColorRampV2* sourceColorRamp();
    void setSourceColorRamp( QgsVectorColorRampV2* ramp );

  protected:
    QString mAttrName;
    QgsRangeList mRanges;
    Mode mMode;
    QgsSymbolV2* mSourceSymbol;
    QgsVectorColorRampV2* mSourceColorRamp;

    //! attribute index (derived from attribute name in startRender)
    int mAttrNum;

    QgsSymbolV2* symbolForValue( double value );
};

#endif // QGSGRADUATEDSYMBOLRENDERERV2_H
