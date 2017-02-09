/* -*-c++-*- */
/*
 * osgEarth is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#ifndef QGSGLOBEFEATUREOPTIONS_H
#define QGSGLOBEFEATUREOPTIONS_H

#include <osgEarth/Common>
#include <osgEarthFeatures/FeatureSource>

class QgsVectorLayer;

class QgsGlobeFeatureOptions : public osgEarth::Features::FeatureSourceOptions // NO EXPORT; header only
{
  private:
    template <class T>
    class RefPtr : public osg::Referenced
    {
      public:
        RefPtr( T* ptr ) : mPtr( ptr ) {}
        T* ptr() { return mPtr; }

      private:
        T* mPtr;
    };

  public:
    QgsGlobeFeatureOptions( const ConfigOptions& opt = ConfigOptions() )
        : osgEarth::Features::FeatureSourceOptions( opt )
    {
      // Call the driver declared as "osgearth_feature_qgis"
      setDriver( "qgis" );
      fromConfig( _conf );
    }

    osgEarth::Config getConfig() const override
    {
      osgEarth::Config conf = osgEarth::Features::FeatureSourceOptions::getConfig();
      conf.updateIfSet( "layerId", mLayerId );
      conf.updateNonSerializable( "layer", new RefPtr< QgsVectorLayer >( mLayer ) );
      return conf;
    }

    osgEarth::optional<std::string>& layerId() { return mLayerId; }
    const osgEarth::optional<std::string>& layerId() const { return mLayerId; }

    QgsVectorLayer* layer() const { return mLayer; }
    void setLayer( QgsVectorLayer* layer ) { mLayer = layer; }

  protected:
    void mergeConfig( const osgEarth::Config& conf ) override
    {
      osgEarth::Features::FeatureSourceOptions::mergeConfig( conf );
      fromConfig( conf );
    }

  private:
    void fromConfig( const osgEarth::Config& conf )
    {
      conf.getIfSet( "layerId", mLayerId );
      RefPtr< QgsVectorLayer > *layer_ptr = conf.getNonSerializable< RefPtr< QgsVectorLayer > >( "layer" );
      mLayer = layer_ptr ? layer_ptr->ptr() : 0;
    }

    osgEarth::optional<std::string> mLayerId;
    QgsVectorLayer*       mLayer;
};

#endif // QGSGLOBEFEATUREOPTIONS_H

