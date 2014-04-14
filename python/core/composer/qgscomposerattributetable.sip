/** \ingroup MapComposer
 * Attribute table item for map composer.
 */

class QgsComposerAttributeTableCompare
{
%TypeHeaderCode
#include <qgscomposerattributetable.h>
%End

  public:
    QgsComposerAttributeTableCompare();
    // bool operator()( const QgsAttributeMap& m1, const QgsAttributeMap& m2 );
    void setSortColumn( int col );
    void setAscending( bool asc );
};

/**A table class that displays a vector attribute table*/
class QgsComposerAttributeTable : QgsComposerTable
{
%TypeHeaderCode
#include <qgscomposerattributetable.h>
%End

  public:
    QgsComposerAttributeTable( QgsComposition* composition /TransferThis/ );
    ~QgsComposerAttributeTable();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint*/
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    void setVectorLayer( QgsVectorLayer* vl );
    QgsVectorLayer* vectorLayer() const;

    void setComposerMap( const QgsComposerMap* map /TransferThis/ );
    const QgsComposerMap* composerMap() const;

    void setMaximumNumberOfFeatures( int nr );
    int maximumNumberOfFeatures() const;

    void setDisplayOnlyVisibleFeatures( bool b );
    bool displayOnlyVisibleFeatures() const;
    
    /*Returns true if a feature filter is active on the attribute table
     * @returns bool state of the feature filter
     * @note added in 2.3
     * @see setFilterFeatures
     * @see featureFilter
    */
    bool filterFeatures() const;
    /**Sets whether the feature filter is active for the attribute table
     * @param filter Set to true to enable the feature filter
     * @note added in 2.3
     * @see filterFeatures
     * @see setFeatureFilter
    */
    void setFilterFeatures( bool filter );

    /*Returns the current expression used to filter features for the table. The filter is only
     * active if filterFeatures() is true.
     * @returns feature filter expression
     * @note added in 2.3
     * @see setFeatureFilter
     * @see filterFeatures
    */
    QString featureFilter() const;
    /**Sets the expression used for filtering features in the table. The filter is only
     * active if filterFeatures() is set to true.
     * @param expression filter to use for selecting which features to display in the table
     * @note added in 2.3
     * @see featureFilter
     * @see setFilterFeatures
    */
    void setFeatureFilter( const QString& expression );

    QSet<int> displayAttributes() const;
    void setDisplayAttributes( const QSet<int>& attr );

    QMap<int, QString> fieldAliasMap() const;
    void setFieldAliasMap( const QMap<int, QString>& map );

    /**Adapts mMaximumNumberOfFeatures depending on the rectangle height*/
    void setSceneRect( const QRectF& rectangle );

    // @note not available in python bindings
    // void setSortAttributes( const QList<QPair<int, bool> > att );

    // @note not available in python bindings
    // QList<QPair<int, bool> > sortAttributes() const;

  protected:
    virtual QMap<int, QString> getHeaderLabels() const;

  signals:
    /**This signal is emitted if the maximum number of feature changes (interactively)*/
    void maximumNumberOfFeaturesChanged( int n );
};
