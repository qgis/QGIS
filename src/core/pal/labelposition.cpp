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

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <cfloat>

#include <pal/layer.h>
#include <pal/pal.h>
#include <pal/label.h>

#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"

#ifndef M_PI
#define M_PI 3.1415926535897931159979634685
#endif


namespace pal {

    LabelPosition::LabelPosition (int id, double x1, double y1, double w, double h, double alpha, double cost, Feature *feature) : id (id), cost (cost), /*workingCost (0),*/ alpha (alpha), feature (feature), nbOverlap (0), w (w), h (h) {


        // alpha take his value bw 0 and 2*pi rad
        while (this->alpha > 2*M_PI)
            this->alpha -= 2 * M_PI;

        while (this->alpha < 0)
            this->alpha += 2 * M_PI;

        register double beta = this->alpha + (M_PI / 2);

        double dx1, dx2, dy1, dy2;

        double tx, ty;

        dx1 = cos (this->alpha) * w;
        dy1 = sin (this->alpha) * w;

        dx2 = cos (beta) * h;
        dy2 = sin (beta) * h;

        x[0] = x1;
        y[0] = y1;

        x[1] = x1 + dx1;
        y[1] = y1 + dy1;

        x[2] = x1 + dx1 + dx2;
        y[2] = y1 + dy1 + dy2;

        x[3] = x1 + dx2;
        y[3] = y1 + dy2;

        // upside down ?
        if (this->alpha > M_PI / 2 && this->alpha <= 3*M_PI / 2) {
            tx = x[0];
            ty = y[0];

            x[0] = x[2];
            y[0] = y[2];

            x[2] = tx;
            y[2] = ty;

            tx = x[1];
            ty = y[1];

            x[1] = x[3];
            y[1] = y[3];

            x[3] = tx;
            y[3] = ty;

            if (this->alpha < M_PI)
                this->alpha += M_PI;
            else
                this->alpha -= M_PI;
        }
    }

    bool LabelPosition::isIn (double *bbox) {
        int i;

        for (i = 0;i < 4;i++) {
            if (x[i] >= bbox[0] && x[i] <= bbox[2] &&
                    y[i] >= bbox[1] && y[i] <= bbox[3])
                return true;
        }

        return false;

    }

    void LabelPosition::print() {
        std::cout << feature->getLayer()->getName() << "/" << feature->getUID() << "/" << id;
        std::cout << " cost: " << cost;
        std::cout << " alpha" << alpha << std::endl;
        std::cout << x[0] << ", " << y[0] << std::endl;
        std::cout << x[1] << ", " << y[1] << std::endl;
        std::cout << x[2] << ", " << y[2] << std::endl;
        std::cout << x[3] << ", " << y[3] << std::endl;
        std::cout << std::endl;
    }

    bool LabelPosition::isInConflict (LabelPosition *ls) {
        int i, i2, j;
        int d1, d2;

        if (this->probFeat == ls->probFeat) // bugfix #1
            return false; // always overlaping itself !

        double cp1, cp2;


        //std::cout << "Check intersect" << std::endl;
        for (i = 0;i < 4;i++) {
            i2 = (i + 1) % 4;
            d1 = -1;
            d2 = -1;
            //std::cout << "new seg..." << std::endl;
            for (j = 0;j < 4;j++) {
                cp1 = cross_product (x[i], y[i], x[i2], y[i2], ls->x[j], ls->y[j]);
                if (cp1 > 0) {
                    d1 = 1;
                    //std::cout << "    cp1: " << cp1 << std::endl;
                }
                cp2 = cross_product (ls->x[i], ls->y[i],
                                     ls->x[i2], ls->y[i2],
                                     x[j], y[j]);

                if (cp2 > 0) {
                    d2 = 1;
                    //std::cout << "     cp2 " << cp2 << std::endl;
                }
            }

            if (d1 == -1 || d2 == -1) // disjoint
                return false;
        }
        return true;
    }

    int LabelPosition::getId() {
        return id;
    }

    double LabelPosition::getX() {
        return x[0];
    }

    double LabelPosition::getY() {
        return y[0];
    }

    double LabelPosition::getAlpha() {
        return alpha;
    }

    double LabelPosition::getCost() {
        return cost;
    }

    Feature * LabelPosition::getFeature() {
        return feature;
    }

    bool xGrow (void *l, void *r) {
        return ( (LabelPosition*) l)->x[0] > ( (LabelPosition*) r)->x[0];
    }

    bool yGrow (void *l, void *r) {
        return ( (LabelPosition*) l)->y[0] > ( (LabelPosition*) r)->y[0];
    }

    bool xShrink (void *l, void *r) {
        return ( (LabelPosition*) l)->x[0] < ( (LabelPosition*) r)->x[0];
    }

    bool yShrink (void *l, void *r) {
        return ( (LabelPosition*) l)->y[0] < ( (LabelPosition*) r)->y[0];
    }

    bool costShrink (void *l, void *r) {
        return ( (LabelPosition*) l)->cost < ( (LabelPosition*) r)->cost;
    }

    bool costGrow (void *l, void *r) {
        return ( (LabelPosition*) l)->cost > ( (LabelPosition*) r)->cost;
    }

    /*
    bool workingCostShrink (void *l, void *r){
       return ((LabelPosition*)l)->workingCost < ((LabelPosition*)r)->workingCost;
    }

    bool workingCostGrow (void *l, void *r){
       return ((LabelPosition*)l)->workingCost > ((LabelPosition*)r)->workingCost;
    }
    */

    Label *LabelPosition::toLabel (bool active) {

        //double x[4], y;

        //x = this->x[0];
        //y = this->y[0];

//#warning retourner les coord projeté ou pas ?
        //feature->layer->pal->proj->getLatLong(this->x[0], this->y[0], &x, &y);

        return new Label (this->x, this->y, alpha, feature->uid, feature->layer->name, feature->userGeom);
    }


    bool obstacleCallback (PointSet *feat, void *ctx) {
        LabelPosition::PolygonCostCalculator *pCost = (LabelPosition::PolygonCostCalculator*) ctx;

        LabelPosition *lp = pCost->getLabel();
        if ( (feat == lp->feature) || (feat->holeOf && feat->holeOf != lp->feature)) {
            return true;
        }

        // if the feature is not a hole we have to fetch corrdinates
        // otherwise holes coordinates are still in memory (feature->selfObs)
        if (feat->holeOf == NULL) {
            ( (Feature*) feat)->fetchCoordinates();
        }

        pCost->update(feat);


        if (feat->holeOf == NULL) {
            ( (Feature*) feat)->releaseCoordinates();
        }


        return true;
    }

    void LabelPosition::setCostFromPolygon (RTree <PointSet*, double, 2, double> *obstacles, double bbx[4], double bby[4]){

        double amin[2];
        double amax[2];


        LabelPosition::PolygonCostCalculator *pCost = new LabelPosition::PolygonCostCalculator(this);
        //cost = getCostFromPolygon (feat, dist_sq);

        // center
        //cost = feat->getDistInside((this->x[0] + this->x[2])/2.0, (this->y[0] + this->y[2])/2.0 );

        feature->fetchCoordinates();
        pCost->update(feature);

        PointSet *extent = new PointSet(4, bbx, bby);

        pCost->update(extent);

        delete extent;

        // TODO Comment
        /*if (cost > (w*w + h*h) / 4.0) {
            double dist = sqrt (cost);
            amin[0] = (x[0] + x[2]) / 2.0 - dist;
            amin[1] = (y[0] + y[2]) / 2.0 - dist;
            amax[0] = amin[0] + 2 * dist;
            amax[1] = amin[1] + 2 * dist;
        } else {*/
            amin[0] = feature->xmin;
            amin[1] = feature->ymin;
            amax[0] = feature->xmax;
            amax[1] = feature->ymax;
        //}

        //std::cout << amin[0] << " " << amin[1] << " " << amax[0] << " " <<  amax[1] << std::endl;
        obstacles->Search (amin, amax, obstacleCallback, pCost);

        cost = pCost->getCost();

        feature->releaseCoordinates();
        delete pCost;
    }

    void LabelPosition::removeFromIndex (RTree<LabelPosition*, double, 2, double> *index) {
        double amin[2];
        double amax[2];
        int c;

        amin[0] = DBL_MAX;
        amax[0] = -DBL_MAX;
        amin[1] = DBL_MAX;
        amax[1] = -DBL_MAX;
        for (c = 0;c < 4;c++) {
            if (x[c] < amin[0])
                amin[0] = x[c];
            if (x[c] > amax[0])
                amax[0] = x[c];
            if (y[c] < amin[1])
                amin[1] = y[c];
            if (y[c] > amax[1])
                amax[1] = y[c];
        }

        index->Remove (amin, amax, this);
    }


    void LabelPosition::insertIntoIndex (RTree<LabelPosition*, double, 2, double> *index) {
        double amin[2];
        double amax[2];
        int c;

        amin[0] = DBL_MAX;
        amax[0] = -DBL_MAX;
        amin[1] = DBL_MAX;
        amax[1] = -DBL_MAX;
        for (c = 0;c < 4;c++) {
            if (x[c] < amin[0])
                amin[0] = x[c];
            if (x[c] > amax[0])
                amax[0] = x[c];
            if (y[c] < amin[1])
                amin[1] = y[c];
            if (y[c] > amax[1])
                amax[1] = y[c];
        }

        index->Insert (amin, amax, this);
    }


    void LabelPosition::setCost (int nblp, LabelPosition **lPos, int max_p, RTree<PointSet*, double, 2, double> *obstacles, double bbx[4], double bby[4]) {
        //std::cout << "setCost:" << std::endl;
        //clock_t clock_start = clock();

        int i;

        double normalizer;
        // compute raw cost
#ifdef _DEBUG_
        std::cout << "LabelPosition for feat: " << lPos[0]->feature->uid << std::endl;
#endif

        for (i = 0;i < nblp;i++)
            lPos[i]->setCostFromPolygon (obstacles, bbx, bby);

        // lPos with big values came fisrts (value = min distance from label to Polygon's Perimeter)
        //sort ( (void**) lPos, nblp, costGrow);
        sort ( (void**) lPos, nblp, costShrink);


        // define the value's range
        double cost_max = lPos[0]->cost;
        double cost_min = lPos[max_p-1]->cost;

        cost_max -= cost_min;

        if (cost_max > EPSILON){
            normalizer = 0.0020 / cost_max;
        }
        else{
            normalizer = 1;
        }
          
        // adjust cost => the best is 0.0001, the sorst is 0.0021
        // others are set proportionally between best and worst
        for (i = 0;i < max_p;i++) {
#ifdef _DEBUG_
            std::cout << "   lpos[" << i << "] = " << lPos[i]->cost;
#endif
            //if (cost_max - cost_min < EPSILON)
            if (cost_max > EPSILON){
                lPos[i]->cost = 0.0021 - (lPos[i]->cost - cost_min) * normalizer;
            }
            else{
                //lPos[i]->cost = 0.0001 + (lPos[i]->cost - cost_min) * normalizer;
                lPos[i]->cost = 0.0001;
            }

#ifdef _DEBUG_
            std::cout <<  "  ==>  " << lPos[i]->cost << std::endl;
#endif
        }
        //clock_t clock_2;
        //std::cout << "AfterSetCostFromPolygon (" << nblp << "x): " << clock_2 = (clock() - clock_1) << std::endl;
    }

    LabelPosition::PolygonCostCalculator::PolygonCostCalculator (LabelPosition *lp) : lp(lp) {
        int i;
        double hyp = max(lp->feature->xmax - lp->feature->xmin, lp->feature->ymax - lp->feature->ymin);
        hyp *= 10;

        px = (lp->x[0] + lp->x[2]) / 2.0;
        py = (lp->y[0] + lp->y[2]) / 2.0;
        dLp[0] = lp->w / 2;
        dLp[1] = lp->h / 2;
        dLp[2] = dLp[0] / cos (M_PI / 4);

        /*
                   3  2  1
                    \ | /
                  4 --x -- 0
                    / | \
                   5  6  7
        */

        double alpha = lp->getAlpha();
        for (i = 0;i < 8;i++, alpha+=M_PI/4) {
            dist[i] = DBL_MAX;
            ok[i] = false;
            rpx[i] = px + cos(alpha)*hyp;
            rpy[i] = py + sin(alpha)*hyp;
        }
    }

    void LabelPosition::PolygonCostCalculator::update (PointSet *pset) {
        if (pset->type == GEOS_POINT){
            updatePoint(pset);
        }
        else{
            double rx,ry;
            if (pset->getDist (px, py, &rx, &ry) < updateLinePoly(pset)){
                PointSet *point = new PointSet (ry, ry);
                update (point);
                delete point;
            }
        }
    }

    void LabelPosition::PolygonCostCalculator::updatePoint (PointSet *pset) {
        double beta = atan2(pset->y[0] - py, pset->x[0] - px) - lp->getAlpha();

        while (beta < 0){
           beta += 2*M_PI;
        }

        double a45 = M_PI/4;

        int i =(int)(beta/a45);

        for (int j=0;j<2;j++, i=(i+1)%8){
            double rx, ry;
            rx = px - rpy[i] + py;
            ry = py + rpx[i] - px;
            double ix, iy; // the point that we look for
            if (computeLineIntersection(px, py, rpx[i], rpy[i], pset->x[0], pset->y[0], rx, ry, &ix, &iy)){
                double d = dist_euc2d_sq(px, py, ix, iy);
                if (d < dist[i]){
                   dist[i] = d;
                   ok[i] = true;
                }
            }
            else{
               std::cout << "this shouldn't occurs !!!" << std::endl;
            }
        }
    }

    double LabelPosition::PolygonCostCalculator::updateLinePoly (PointSet *pset) {
        int i, j, k;
        int nbP = (pset->type == GEOS_POLYGON ? pset->nbPoints : pset->nbPoints-1);
        double min_dist = DBL_MAX;

        for (i = 0;i < nbP;i++) {
            j = (i + 1) % pset->nbPoints;

            for (k = 0;k < 8;k++) {
                double ix, iy;
                if (computeSegIntersection (px, py, rpx[k], rpy[k], pset->x[i], pset->y[i], pset->x[j], pset->y[j], &ix, &iy)) {
                    double d = dist_euc2d_sq (px, py, ix, iy);
                    if (d < dist[k]) {
                        dist[k] = d;
                        ok[k] = true;
                    }
                    if (d < min_dist){
                        min_dist = d;
                    }
                }
            }
        }
        return min_dist;
    }

    LabelPosition* LabelPosition::PolygonCostCalculator::getLabel () {
        return lp;
    }

    double LabelPosition::PolygonCostCalculator::getCost () {
        int i;

        for (i = 0;i < 8;i++) {
            dist[i] -= ((i%2) ? dLp[2] : ((i==0||i==4)? dLp[0]: dLp[1]));
            if (!ok[i] || dist[i] < 0.1){
                dist[i] = 0.1;
            }
        }

        double a,b,c,d;

        a = min(dist[0], dist[4]);
        b = min(dist[1], dist[5]);
        c = min(dist[2], dist[6]);
        d = min(dist[3], dist[7]);

        //return (a+b+c+d);
        return (a*b*c*d);
    }
} // end namespace

