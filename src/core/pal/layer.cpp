/*
 *   libpal - Automated Placement of Labels Library
 *
 *   Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                      University of Applied Sciences, Western Switzerland
 *                      http://www.hes-so.ch
 *
 *   Contact:
 *      maxence.laurent <at> heig-vd <dot> ch
 *    or
 *      eric.taillard <at> heig-vd <dot> ch
 *
 * This file is part of libpal.
 *
 * libpal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libpal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpal.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _CRT_SECURE_NO_DEPRECATE

#include <stddef.h>
#include <geos_c.h>

#include <iostream>
#include <cstring>
#include <cmath>


#include <pal/pal.h>
#include <pal/layer.h>
#include <pal/palexception.h>

#include "linkedlist.hpp"
#include "hashtable.hpp"

#include "feature.h"
#include "geomfunction.h"
#include "util.h"

#include "simplemutex.h"

namespace pal {

    Layer::Layer (const char *lyrName, double min_scale, double max_scale, Arrangement arrangement, Units label_unit, double defaultPriority, bool obstacle, bool active, bool toLabel, Pal *pal) :  pal (pal), obstacle (obstacle), active (active), toLabel (toLabel), label_unit (label_unit), min_scale (min_scale), max_scale (max_scale), arrangement (arrangement) {

        this->name = new char[strlen (lyrName) +1];
        strcpy (this->name, lyrName);

        modMutex = new SimpleMutex();

        //rtreeFile = new char[strlen(lyrName)+7];
        //sprintf (rtreeFile, "%s.rtree", lyrName);

        rtree = new RTree<Feature*, double, 2, double>();
        hashtable = new HashTable<Cell<Feature*>*> (5281);

        if (defaultPriority < 0.0001)
            this->defaultPriority = 0.0001;
        else if (defaultPriority > 1.0)
            this->defaultPriority = 1.0;
        else
            this->defaultPriority = defaultPriority;

        features = new LinkedList<Feature*> (ptrFeatureCompare);
    }

    Layer::~Layer() {
        modMutex->lock();

        if (features) {
            while (features->size()) {
                delete features->pop_front();
            }
            delete features;
        }

        if (name)
            delete[] name;

        delete rtree;

        delete hashtable;
        delete modMutex;
    }

    /*
    Feature *Layer::getFeature(int i){
       if (i<0 || i >=int(features->size()))
          return NULL;
       return features->at(i);
    }*/


    bool Layer::isScaleValid (double scale) {
        return (scale >= min_scale || min_scale == -1)
               && (scale <= max_scale || max_scale == -1);
    }


// PUBLIC method !
    int Layer::getNbFeatures() {
        return features->size();
    }

// TODO avoid :
// first -> name = layer->getName()
// then  -> layer->rename("...");
// so -> name is broken !!!
    void Layer::rename (char *name) {
        if (name && strlen (this->name) > 0) {
            delete[] this->name;
            name = new char[strlen (name) +1];
            strcpy (this->name, name);
        }
    }

// TODO see rename()
    const char *Layer::getName() {
        return name;
    }

    Arrangement Layer::getArrangement() {
        return arrangement;
    }

    void Layer::setArrangement (Arrangement arrangement) {
        this->arrangement = arrangement;
    }


    bool Layer::isObstacle() {
        return obstacle;
    }

    bool Layer::isToLabel() {
        return toLabel;
    }

    bool Layer::isActive() {
        return active;
    }


    double Layer::getMinScale() {
        return min_scale;
    }

    double Layer::getMaxScale() {
        return max_scale;
    }

    double Layer::getPriority() {
        return defaultPriority;
    }

    void Layer::setObstacle (bool obstacle) {
        this->obstacle = obstacle;
    }

    void Layer::setActive (bool active) {
        this->active = active;
    }

    void Layer::setToLabel (bool toLabel) {
        this->toLabel = toLabel;
    }

    void Layer::setMinScale (double min_scale) {
        this->min_scale = min_scale;
    }

    void Layer::setMaxScale (double max_scale) {
        this->max_scale = max_scale;
    }

    void Layer::setPriority (double priority) {
        if (priority >= 1.0) // low priority
            defaultPriority = 1.0;
        else if (priority <= 0.0001)
            defaultPriority = 0.0001; // high priority
        else
            defaultPriority = priority;
    }

#if 0
//inline Feat *splitButterflyPolygon (Feat *f, int pt_a, int pt_b, double cx, double cy){
    int i, k;
    Feat *new_feat = new Feat();

#ifdef _DEBUG_
    // std::cout << "splitButterFly " << f->nbPoints << " " << pt_a << " " << pt_b <<  std::endl;
#endif

    new_feat->geom = f->geom;

    new_feat->id = f->id;
    new_feat->type = geos::geom::GEOS_POLYGON;

    if (pt_a < pt_b)
        new_feat->nbPoints = pt_b - pt_a + 1;
    else
        new_feat->nbPoints = f->nbPoints - pt_a + pt_b + 1;


#ifdef _DEBUG_FULL_
    std::cout << "nbpoints:" << new_feat->nbPoints << std::endl;
#endif

    new_feat->x = new double[new_feat->nbPoints];
    new_feat->y = new double[new_feat->nbPoints];

    new_feat->minmax[0] = cx;
    new_feat->minmax[1] = cy;
    new_feat->minmax[2] = cx;
    new_feat->minmax[3] = cy;

    new_feat->x[0] = cx;
    new_feat->y[0] = cy;

#ifdef _DEBUG_FULL_
    std::cout << new_feat->x[0] << ";" << new_feat->y[0] << std::endl;
#endif

    for (i = pt_a, k = 1;i != pt_b;i = (i + 1) % f->nbPoints, k++) {
        new_feat->x[k] = f->x[i];
        new_feat->y[k] = f->y[i];

        new_feat->minmax[0] = new_feat->x[k] < new_feat->minmax[0] ? new_feat->x[k] : new_feat->minmax[0];
        new_feat->minmax[2] = new_feat->x[k] > new_feat->minmax[2] ? new_feat->x[k] : new_feat->minmax[2];

        new_feat->minmax[1] = new_feat->y[k] < new_feat->minmax[1] ? new_feat->y[k] : new_feat->minmax[1];
        new_feat->minmax[3] = new_feat->y[k] > new_feat->minmax[3] ? new_feat->y[k] : new_feat->minmax[3];
#ifdef _DEBUG_FULL_
        std::cout << new_feat->x[k] << ";" << new_feat->y[k] << std::endl;
#endif
    }

    if (f->nbHoles > 0) {
        // TODO check in which part are each hole !
        new_feat->nbHoles = f->nbHoles;
        new_feat->holes = new PointSet*[f->nbHoles];
        for (i = 0;i < f->nbHoles;i++) {
            new_feat->holes[i] = new PointSet (*f->holes[i]);
        }
    } else {
        new_feat->nbHoles = 0;
        new_feat->holes = NULL;
    }

#ifdef _DEBUG_FULL_
    std::cout << " go to reorder ..." << std::endl;
#endif
    reorderPolygon (new_feat->nbPoints, new_feat->x, new_feat->y);

#ifdef _DEBUG_FULL_
    std::cout << "split ok ..." << std::endl;
#endif


    return new_feat;
}
#endif

void Layer::registerFeature (const char *geom_id, PalGeometry *userGeom, double label_x, double label_y) {
    int j;

    if (geom_id && label_x >= 0 && label_y >= 0) {
        modMutex->lock();
        j = features->size();

        if (hashtable->find (geom_id)) {
            modMutex->unlock();
            throw new PalException::FeatureExists();
            return;
        }

        /* Split MULTI GEOM and Collection in simple geometries*/
        GEOSGeometry *the_geom = userGeom->getGeosGeometry();

        LinkedList<Feat*> *finalQueue = splitGeom (the_geom, geom_id);

        int nGeom = finalQueue->size();
        int part = 0;

        bool first_feat = true;

        while (finalQueue->size() > 0) {
            Feat *f = finalQueue->pop_front();
#ifdef _DEBUG_FULL_
            std::cout << "f-> popped" << std::endl;
#endif
            Feature *ft;

            switch (f->type) {
            case GEOS_POINT:
            case GEOS_LINESTRING:
            case GEOS_POLYGON:
                //case geos::geom::GEOS_POINT:
                //case geos::geom::GEOS_LINESTRING:
                //case geos::geom::GEOS_POLYGON:
#ifdef _DEBUG_FULL_
                std::cout << "Create Feat" << std::endl;
#endif
                ft = new Feature (f, this, part, nGeom, userGeom);
                ft->deleteCoord();
#ifdef _DEBUG_FULL_
                std::cout << "Feature created" << std::endl;
#endif
                break;
            default:
#ifdef _VERBOSE_
                std::cerr << "Wrong geometry type, should never occurs !!" << std::endl;
#endif
                exit (-1);
            }

            double bmin[2];
            double bmax[2];
            bmin[0] = ft->xmin;
            bmin[1] = ft->ymin;

            bmax[0] = ft->xmax;
            bmax[1] = ft->ymax;

            ft->label_x = label_x;
            ft->label_y = label_y;

            features->push_back (ft);

            if (first_feat) {
                hashtable->insertItem (geom_id, features->last);
                first_feat = false;
            }

#ifdef _DEBUG_FULL_
            std::cout << "Feat box : " << bmin[0] << " " <<  bmin[1] << " " <<  bmax[0] << " " << bmax[1] << std::endl;
#endif

            rtree->Insert (bmin, bmax, ft);

#ifdef _DEBUG_FULL_
            std::cout << "feature inserted :-)" << std::endl;
#endif

            delete f;
#ifdef _DEBUG_FULL_
            std::cout << "f deleted..." << std::endl;
#endif
            part++;
        }
        delete finalQueue;

        userGeom->releaseGeosGeometry (the_geom);
    }
    modMutex->unlock();
}


Cell<Feature*>* Layer::getFeatureIt (const char * geom_id) {
    Cell<Feature*>** it = hashtable->find (geom_id);

    if (it)
        return *it;
    else
        return NULL;
}

void Layer::setFeatureDistlabel (const char * geom_id, int distlabel) {
    int i;

    if (distlabel < 0) {
        std::cerr << "setFeatureDistlabel :  invalid size : " << distlabel << std::endl;
        throw new PalException::ValueNotInRange();
        return;
    }

    modMutex->lock();
    Cell<Feature*>* it = getFeatureIt (geom_id);

    if (it) {
        // log
        Feature *feat = it->item;
        int nb = feat->nPart;

        for (i = 0;i < nb;i++) {
            feat = it->item;
            feat->distlabel = distlabel;
            it = it->next;
        }
    } else {
        std::cerr << "setFeatureDistlabel " << geom_id << " not found" << std::endl;
        modMutex->unlock();
        throw new PalException::UnknownFeature();
    }
    modMutex->unlock();
}


int Layer::getFeatureDistlabel (const char *geom_id) {
    modMutex->lock();
    Cell<Feature*>* it = getFeatureIt (geom_id);

    int ret = -1;
    if (it)
        ret = it->item->distlabel;
    else {
        modMutex->unlock();
        throw new PalException::UnknownFeature();
    }

    modMutex->unlock();
    return ret;
}

void Layer::setFeatureLabelSize (const char * geom_id, double label_x, double label_y) {
    int i;

    if (label_x < 0 || label_y < 0) {
        std::cerr << "setFeatureLabelSize :  invalid size : " << label_x << ";" << label_y << std::endl;
        throw new PalException::ValueNotInRange();
        return;
    }

    modMutex->lock();
    Cell<Feature*>* it = getFeatureIt (geom_id);

    if (it) {
        Feature *feat = it->item;
        int nb = feat->nPart;

        for (i = 0;i < nb;i++) {
            feat = it->item;
            feat->label_x = label_x;
            feat->label_y = label_y;
            it = it->next;
        }
    } else {
        std::cerr << "setFeaturelabelSizeFeature " << geom_id << " not found" << std::endl;
        modMutex->unlock();
        throw new PalException::UnknownFeature();
    }
    modMutex->unlock();
}

double Layer::getFeatureLabelHeight (const char *geom_id) {
    modMutex->lock();
    Cell<Feature*>* it = getFeatureIt (geom_id);

    double ret = -1;

    if (it)
        ret = it->item->label_y;
    else {
        modMutex->unlock();
        throw new PalException::UnknownFeature();
    }

    modMutex->unlock();

    return ret;
}

double Layer::getFeatureLabelWidth (const char *geom_id) {
    modMutex->lock();
    Cell<Feature*>* it = getFeatureIt (geom_id);
    double ret = -1;

    if (it)
        ret = it->item->label_x;
    else {
        modMutex->unlock();
        throw new PalException::UnknownFeature();
    }
    modMutex->unlock();
    return ret;
}

    void Layer::setLabelUnit (Units label_unit) {
        if (label_unit == PIXEL || label_unit == METER)
            this->label_unit = label_unit;
    }

    Units Layer::getLabelUnit () {
        return label_unit;
    }



/*
void Layer::setFeatureGeom (const char * geom_id, const char *the_geomHex){


   Cell<Feature*>* it = getFeatureIt (geom_id);

   if (it){
      double bmin[2];
      double bmax[2];

      int oldnPart = it->item->nPart;
      int nPart;

      //geos::io::WKBReader wkbReader =  geos::io::WKBReader();

      geos::io::WKBReader * wkbReader = new geos::io::WKBReader();

      std::string str = the_geomHex;
      std::istream *is = new std::istringstream (str);
      geos::geom::Geometry *initial_geom = wkbReader->readHEX(*is);
      delete is;


      LinkedList<Feat*> *finalFeats = splitGeom (initial_geom, geom_id);

      nPart = finalFeats->size();

      bool pushback = nPart > oldnPart;

      Feature *feat;
      Feat *ft;

      if (!pushback){
         int k = 0
         while (finalFeats->size() > 0){
            feat = it->item;

            ft = finalFeats->pop_front();

            bmin[0] = feat->xmin;
            bmin[1] = feat->ymin;

            bmax[0] = feat->xmax;
            bmax[1] = feat->ymax;

            rtree->Remove (bmin, bmax, feat);

            feat->setXYCoord (ft->geom);
            feat->nPart = nPart;

            bmin[0] = feat->xmin;
            bmin[1] = feat->ymin;

            bmax[0] = feat->xmax;
            bmax[1] = feat->ymax;

            rtree->Insert (bmin, bmax, feat);

            it = it->next;
            k++;
         }

         for (;k<oldnPart;it++){

         }
      }
      else{
        // 1) Backup old value (xPx, etc)
        // 2) remove old Features from old_id
        // 3) append new feauter through addFeature method
      }
   }
}
*/


} // end namespace

