/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
 * Copyright 2008-2013 Pelican Mapping
 * http://osgearth.org
 *
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

namespace osgEarth
{
//    typedef std::list<class osgEarth::Config> ConfigSet;


    // general-purpose name/value pair set.
    struct Properties // : std::map<std::string,std::string>
    {
%TypeHeaderCode
#include <osgEarth/Config>
%End
//        std::string get( const std::string& key ) const;
    };


    /**
     * Config is a general-purpose container for serializable data. You store an object's members
     * to Config, and then translate the Config to a particular format (like XML or JSON). Likewise,
     * the object can de-serialize a Config back into member data. Config support the optional<>
     * template for optional values.
     */
    class Config
    {
%TypeHeaderCode
#include <osgEarth/Config>
%End
    public:
        Config();

//        Config( const std::string& key );

//        Config( const std::string& key, const std::string& value );

        Config( const osgEarth::Config& rhs );

        virtual ~Config();

        /** Context for resolving relative URIs that occur in this Config */
//        void setReferrer( const std::string& value );
//        void inheritReferrer( const std::string& value );
//        const std::string& referrer() const;

//        bool fromXML( std::istream& in );

//        std::string toJSON( bool pretty =false ) const;

//        bool fromJSON( const std::string& json );

        bool empty() const;

        bool isSimple() const;

//        std::string& key();

//        const std::string& value() const;

//        const ConfigSet& children() const;

//        const ConfigSet children( const std::string& key ) const {
//            ConfigSet r;
//            for(ConfigSet::const_iterator i = _children.begin(); i != _children.end(); i++ ) {
//                if ( i->key() == key )
//                    r.push_back( *i );
//            }
//            return r;
//        }
//
//        bool hasChild( const std::string& key ) const {
//            for(ConfigSet::const_iterator i = _children.begin(); i != _children.end(); i++ )
//                if ( i->key() == key )
//                    return true;
//            return false;
//        }
//
//        void remove( const std::string& key ) {
//            for(ConfigSet::iterator i = _children.begin(); i != _children.end(); ) {
//                if ( i->key() == key )
//                    i = _children.erase( i );
//                else
//                    ++i;
//            }
//        }
//
//        osgEarth::Config child( const std::string& key ) const;
//
//        const osgEarth::Config* child_ptr( const std::string& key ) const;
//
//        osgEarth::Config* mutable_child( const std::string& key );
//
//        void merge( const osgEarth::Config& rhs );
//
//        osgEarth::Config* find( const std::string& key, bool checkThis =true );
//        const osgEarth::Config* find( const std::string& key, bool checkThis =true) const;
//
//        template<typename T>
//        void addIfSet( const std::string& key, const optional<T>& opt ) {
//            if ( opt.isSet() ) {
//                add( key, osgEarth::toString<T>( opt.value() ) );
//            }
//        }
//
//        template<typename T>
//        void addObjIfSet( const std::string& key, const osg::ref_ptr<T>& opt ) {
//            if ( opt.valid() ) {
//                osgEarth::Config conf = opt->getConfig();
//                conf.key() = key;
//                add( conf );
//            }
//        }
//
//        template<typename T>
//        void addObjIfSet( const std::string& key, const optional<T>& obj ) {
//            if ( obj.isSet() ) {
//                osgEarth::Config conf = obj->getConfig();
//                conf.key() = key;
//                add( conf );
//            }
//        }
//
//        template<typename X, typename Y>
//        void addIfSet( const std::string& key, const std::string& val, const optional<X>& target, const Y& targetValue ) {
//            if ( target.isSetTo( targetValue ) )
//                add( key, val );
//        }
//
//        template<typename T>
//        void add( const std::string& key, const T& value ) {
//            _children.push_back( osgEarth::Config(key, Stringify() << value) );
//            //_children.back().setReferrer( _referrer );
//            _children.back().inheritReferrer( _referrer );
//        }
//
//        void add( const Config& conf ) {
//            _children.push_back( conf );
//            //_children.back().setReferrer( _referrer );
//            _children.back().inheritReferrer( _referrer );
//        }
//
//        void add( const std::string& key, const Config& conf ) {
//            Config temp = conf;
//            temp.key() = key;
//            add( temp );
//        }
//
//        void add( const ConfigSet& set ) {
//            for( ConfigSet::const_iterator i = set.begin(); i != set.end(); i++ )
//                add( *i );
//        }
//
//        template<typename T>
//        void addObj( const std::string& key, const T& value ) {
//            Config conf = value.getConfig();
//            conf.key() = key;
//            add( conf );
//        }
//
//        template<typename T>
//        void updateIfSet( const std::string& key, const optional<T>& opt ) {
//            if ( opt.isSet() ) {
//                remove(key);
//                add( key, osgEarth::toString<T>( opt.value() ) );
//            }
//        }
//
//        template<typename T>
//        void updateObjIfSet( const std::string& key, const osg::ref_ptr<T>& opt ) {
//            if ( opt.valid() ) {
//                remove(key);
//                Config conf = opt->getConfig();
//                conf.key() = key;
//                add( conf );
//            }
//        }
//
//        template<typename T>
//        void updateObjIfSet( const std::string& key, const optional<T>& obj ) {
//            if ( obj.isSet() ) {
//                remove(key);
//                Config conf = obj->getConfig();
//                conf.key() = key;
//                add( conf );
//            }
//        }
//
//        template<typename X, typename Y>
//        void updateIfSet( const std::string& key, const std::string& val, const optional<X>& target, const Y& targetValue ) {
//            if ( target.isSetTo( targetValue ) ) {
//                remove(key);
//                add( key, val );
//            }
//        }
//
//        template<typename T>
//        void update( const std::string& key, const T& value ) {
//            update( Config(key, Stringify() << value) );
//        }
//
//        void update( const Config& conf ) {
//            remove(conf.key());
//            add( conf );
//        }
//
//        template<typename T>
//        void updateObj( const std::string& key, const T& value ) {
//            remove(key);
//            Config conf = value.getConfig();
//            conf.key() = key;
//            add( conf );
//        }
//
//        template<typename T>
//        void set( const std::string& key, const T& value ) {
//            update( key, value );
//        }
//
//        bool hasValue( const std::string& key ) const {
//            return !value(key).empty();
//        }
//
//        const std::string value( const std::string& key ) const {
//            std::string r = trim(child(key).value());
//            if ( r.empty() && _key == key )
//                r = _defaultValue;
//            return r;
//        }
//
//        const std::string referrer( const std::string& key ) const {
//            return child(key).referrer();
//        }
//
//        // populates a primitive value.
//        template<typename T>
//        T value( const std::string& key, T fallback ) const {
//            std::string r;
//            if ( hasChild( key ) )
//                r = child(key).value();
//            return osgEarth::as<T>( r, fallback );
//        }
//
//        bool boolValue( bool fallback ) const {
//            return osgEarth::as<bool>( _defaultValue, fallback );
//        }
//
//        // populates the output value iff the Config exists.
//        template<typename T>
//        bool getIfSet( const std::string& key, optional<T>& output ) const {
//            std::string r;
//            if ( hasChild(key) )
//                r = child(key).value();
//            if ( !r.empty() ) {
//                output = osgEarth::as<T>( r, output.defaultValue() );
//                return true;
//            }
//            else
//                return false;
//        }
//
//        // for Configurable's
//        template<typename T>
//        bool getObjIfSet( const std::string& key, optional<T>& output ) const {
//            if ( hasChild( key ) ) {
//                output = T( child(key) );
//                return true;
//            }
//            else
//                return false;
//        }
//
//        // populates a Referenced that takes a Config in the constructor.
//        template<typename T>
//        bool getObjIfSet( const std::string& key, osg::ref_ptr<T>& output ) const {
//            if ( hasChild( key ) ) {
//                output = new T( child(key) );
//                return true;
//            }
//            else
//                return false;
//        }
//
//        template<typename T>
//        bool getObjIfSet( const std::string& key, T& output ) const {
//            if ( hasChild(key) ) {
//                output = T( child(key) );
//                return true;
//            }
//            return false;
//        }
//
//        template<typename X, typename Y>
//        bool getIfSet( const std::string& key, const std::string& val, optional<X>& target, const Y& targetValue ) const {
//            if ( hasValue( key ) && value( key ) == val ) {
//                target = targetValue;
//                return true;
//            }
//            else
//                return false;
//        }
//
//        template<typename X, typename Y>
//        bool getIfSet( const std::string& key, const std::string& val, X& target, const Y& targetValue ) const {
//            if ( hasValue(key) && value(key) == val ) {
//                target = targetValue;
//                return true;
//            }
//            return false;
//        }
//
//        template<typename T>
//        bool getIfSet( const std::string& key, T& output ) const {
//            if ( hasValue(key) ) {
//                output = value<T>(key, output);
//                return true;
//            }
//            return false;
//        }
//
//        /** support for conveying non-serializable objects in a Config (in memory only) */
//
//        typedef std::map<std::string, osg::ref_ptr<osg::Referenced> > RefMap;
//
//        void addNonSerializable( const std::string& key, osg::Referenced* obj ) {
//            _refMap[key] = obj;
//        }
//
//        void updateNonSerializable( const std::string& key, osg::Referenced* obj ) {
//            _refMap[key] = obj;
//        }
//
//        template<typename X>
//        X* getNonSerializable( const std::string& key ) const {
//            RefMap::const_iterator i = _refMap.find(key);
//            return i == _refMap.end() ? 0 : dynamic_cast<X*>( i->second.get() );
//        }
//
//        // remove everything from (this) that also appears in rhs
//        Config operator - ( const Config& rhs ) const;
//
//    protected:
//        std::string _key;
//        std::string _defaultValue;
//        ConfigSet   _children;
//        std::string _referrer;
//        Config*     _emptyConfig;
//
//        RefMap _refMap;
    };



//    // specialization for Config
//    template <> inline
//    void Config::addIfSet<Config>( const std::string& key, const optional<Config>& opt ) {
//        if ( opt.isSet() ) {
//            Config conf = opt.value();
//            conf.key() = key;
//            add( conf );
//        }
//    }
//
//    template<> inline
//    void Config::updateIfSet<Config>( const std::string& key, const optional<Config>& opt ) {
//        if ( opt.isSet() ) {
//            remove(key);
//            Config conf = opt.value();
//            conf.key() = key;
//            add( conf );
//        }
//    }
//
//    template<> inline
//    bool Config::getIfSet<Config>( const std::string& key, optional<Config>& output ) const {
//        if ( hasChild( key ) ) {
//            output = child(key);
//            return true;
//        }
//        else
//            return false;
//    }
//
//    template<> inline
//    void Config::add<std::string>( const std::string& key, const std::string& value ) {
//        _children.push_back( Config( key, value ) );
//        //_children.back().setReferrer( _referrer );
//        _children.back().inheritReferrer( _referrer );
//    }
//
//    template<> inline
//    void Config::update<std::string>( const std::string& key, const std::string& value ) {
//        remove(key);
//        add( Config(key, value) );
//    }
//
//    template<> inline
//    void Config::update<Config>( const std::string& key, const Config& conf ) {
//        remove(key);
//        Config temp = conf;
//        temp.key() = key;
//        add( temp );
//    }
//
//    template<> inline
//    void Config::add<float>( const std::string& key, const float& value ) {
//        add( key, Stringify() << std::setprecision(8) << value );
//        //add( key, Stringify() << std::fixed << std::setprecision(8) << value );
//    }
//
//    template<> inline
//    void Config::add<double>( const std::string& key, const double& value ) {
//        add( key, Stringify() << std::setprecision(16) << value );
//        //add( key, Stringify() << std::fixed << std::setprecision(16) << value );
//    }
//
//    template<> inline
//    void Config::update<float>( const std::string& key, const float& value ) {
//        update( key, Stringify() << std::setprecision(8) << value );
//        //update( key, Stringify() << std::fixed << std::setprecision(8) << value );
//    }
//
//    template<> inline
//    void Config::update<double>( const std::string& key, const double& value ) {
//        update( key, Stringify() << std::setprecision(16) << value );
//    }

    //--------------------------------------------------------------------

    /**
     * Base class for all serializable options classes.
     */
    class ConfigOptions // header-only (no export required)
    {
%TypeHeaderCode
#include <osgEarth/Config>
%End
    public:
        ConfigOptions( const osgEarth::Config& conf =osgEarth::Config() );
        ConfigOptions( const osgEarth::ConfigOptions& rhs );

        virtual ~ConfigOptions();

//        const std::string& referrer() const;

//        ConfigOptions& operator= ( const ConfigOptions& rhs );

        void merge( const osgEarth::ConfigOptions& rhs );

        virtual osgEarth::Config getConfig() const;

        virtual osgEarth::Config getConfig( bool isolate ) const;

        osgEarth::Config newConfig() const;

        bool empty() const;
    };

    /**
     * Base configoptions class for driver options.
     */
    class DriverConfigOptions : osgEarth::ConfigOptions // header-only (no export required)
    {
%TypeHeaderCode
#include <osgEarth/Config>
%End
    public:
        DriverConfigOptions( const osgEarth::ConfigOptions& rhs =osgEarth::ConfigOptions() );

        /** Dtor */
        virtual ~DriverConfigOptions();

        /** Gets or sets the name of the driver to load */
//        void setDriver( const std::string& value );
//        const std::string& getDriver() const;

    public:
        virtual osgEarth::Config getConfig() const;
        virtual osgEarth::Config getConfig( bool isolate );

        virtual void mergeConfig( const osgEarth::Config& conf );

    public:
        void fromConfig( const osgEarth::Config& conf );

    };
};
