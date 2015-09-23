#ifndef QGSVECTORLAYERLABELING_H
#define QGSVECTORLAYERLABELING_H

class QDomDocument;
class QDomElement;
class QString;

class QgsVectorLayer;
class QgsVectorLayerLabelProvider;

/**
 * Abstract base class - its implementations define different approaches to the labeling of a vector layer.
 *
 * @note added in 2.12
 */
class CORE_EXPORT QgsAbstractVectorLayerLabeling
{
  public:

    virtual ~QgsAbstractVectorLayerLabeling();

    //! Unique type string of the labeling configuration implementation
    virtual QString type() const = 0;

    //! Factory for label provider implementation
    virtual QgsVectorLayerLabelProvider* provider( QgsVectorLayer* layer ) const = 0;

    //! Return labeling configuration as XML element
    virtual QDomElement save( QDomDocument& doc ) const = 0;

    // static stuff

    //! Try to create instance of an implementation based on the XML data
    static QgsAbstractVectorLayerLabeling* create( const QDomElement& element );
};

/**
 * Basic implementation of the labeling interface.
 *
 * The configuration is kept in layer's custom properties for backward compatibility.
 *
 * @note added in 2.12
 */
class CORE_EXPORT QgsVectorLayerSimpleLabeling : public QgsAbstractVectorLayerLabeling
{
  public:

    virtual QString type() const override;
    virtual QgsVectorLayerLabelProvider* provider( QgsVectorLayer* layer ) const override;
    virtual QDomElement save( QDomDocument& doc ) const override;
};

#endif // QGSVECTORLAYERLABELING_H
