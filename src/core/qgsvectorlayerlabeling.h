#ifndef QGSVECTORLAYERLABELING_H
#define QGSVECTORLAYERLABELING_H


class QgsPalLayerSettings;
class QgsRuleBasedLabeling;
class QgsVectorLayer;
class QgsVectorLayerLabelProvider;

class CORE_EXPORT QgsVectorLayerLabeling
{
  public:
    enum Mode
    {
      //NoLabels,         //!< the layer does not participate in labeling
      //Obstacles,        //!< no labels are shown, but layer's features act as obstacles for other labels
      SimpleLabels,     //!< the layer is labelled with one style
      RuleBasedLabels   //!< the layer is labelled with multiple styles defined with rules
    };

    //! Defaults to no labels
    QgsVectorLayerLabeling();
    ~QgsVectorLayerLabeling();

    Mode mode() const { return mMode; }
    void setMode( Mode m ) { mMode = m; }

    //QgsPalLayerSettings simpleLabeling();

    //QgsPalLayerSettings* simpleLabeling() const { return mSimpleLabeling; }
    //! Assign simple labeling configuration (takes ownership)
    //void setSimpleLabeling( QgsPalLayerSettings* settings );

    QgsRuleBasedLabeling* ruleBasedLabeling() const { return mRuleBasedLabeling; }
    //! Assign rule-based labeling configuration (takes ownership)
    void setRuleBasedLabeling( QgsRuleBasedLabeling* settings );

    //! Factory for label provider implementation - according to the current mode
    QgsVectorLayerLabelProvider* provider( QgsVectorLayer* layer );

  protected:
    Mode mMode;
    //QgsPalLayerSettings* mSimpleLabeling;
    QgsRuleBasedLabeling* mRuleBasedLabeling;

};


#endif // QGSVECTORLAYERLABELING_H
