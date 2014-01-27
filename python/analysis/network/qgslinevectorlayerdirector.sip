/**
* \ingroup networkanalysis
* \class QgsLineVectorLayerDirector
* \brief Determine making the graph from vector line layer
*/
class QgsLineVectorLayerDirector : QgsGraphDirector
{
%TypeHeaderCode
#include <qgslinevectorlayerdirector.h>
%End

  public:
    /**
     * @param myLayer source vector layer
     * @param directionFieldId feield contain road direction value
     * @param directDirectionValue value for one-way road
     * @param reverseDirectionValue value for reverse one-way road
     * @param bothDirectionValue value for road
     * @param defaultDirection 1 - direct direction, 2 - reverse direction, 3 - both direction
     */
    QgsLineVectorLayerDirector( QgsVectorLayer* myLayer,
                                int directionFieldId,
                                const QString& directDirectionValue,
                                const QString& reverseDirectionValue,
                                const QString& bothDirectionValue,
                                int defaultDirection
                              );

    //! Destructor
    virtual ~QgsLineVectorLayerDirector();

    /*
     * MANDATORY DIRECTOR PROPERTY DECLARATION
     */
    void makeGraph( QgsGraphBuilderInterface *builder,
                    const QVector< QgsPoint >& additionalPoints,
                    QVector< QgsPoint>& tiedPoints /Out/ ) const;

    QString name() const;
};

