// SimAnneal.cpp

#include "SimAnneal.h"
#include <iostream>
#include <iomanip>
#include <cfloat>

using std::cout;
using std::endl;
using std::setw;

SimAnneal::SimAnneal(DataPlace& rbplace, bool gr, bool det)
    : rb(rbplace),
      greedy(gr),
      detailed(det),
      calibMaxIter(500),
      oldCost(0),
      newCost(0),
      oldPlace(0),
      newPlace(0),
      lambda(1),
      lambdaP(1),
      layoutBBox(rb) {
    acceptCount = 0;
    curRate = 1.0;
    maxI = 150;
    I = 0;
    maxUpdate = 25;

    negAcceptCount = posAcceptCount = 0;
    layoutXSize = layoutBBox.getWidth();
    layoutYSize = layoutBBox.getHeight();
    layoutArea = layoutXSize * layoutYSize;
    cout << "Placement area: " << layoutXSize << " x " << layoutYSize << endl;
    cout << "Num move_cells: " << rb.NumCells << endl;

    if (!detailed) {
        initPlacement(rb);
    } else {
        fillingRows(rb);
    }

    hpwl = rb.evalHPWL();
    overlap = rb.calcOverlap(detailed);
    penaltyRow = rb.evalPRow();
    oldCost = cost();

    double areaPerCell = layoutArea / rb.NumCells;
    
    if (detailed) {
        initTemp = curTemp = (hpwl > 0) ? hpwl / rb.NumNets : areaPerCell / 10.0;
    } else {
        initTemp = curTemp = oldCost;
    }
    if (curTemp <= 0 || curTemp != curTemp) {
        curTemp = areaPerCell / 10.0;
        initTemp = curTemp;
    }
    stopTemp = curTemp / 100;
    if (stopTemp <= 0) stopTemp = curTemp / 1000;
    k = 1;  // specify user
    maxIter = k * rb.NumCells;

    cout << "Initial:\t Temp: " << curTemp << " Iter: " << maxIter
         << " HPWL: " << rb.evalHPWL() << " Over: " << rb.calcOverlap(detailed)
         << " Cost: " << oldCost << endl;

    dynamic_window();
    cout << windowfactor << " : " << xspan << " , " << yspan << endl;

    if (!greedy && detailed) calibrate();

    initTemp = curTemp;
    stopTemp = curTemp / 100;

    if (detailed)
        anneal_detailed();
    else
        anneal_global();
    rb.checkPRow();

    cout << "Final:\t HPWL: " << rb.evalHPWL()
         << " Over: " << rb.calcOverlap(true) << endl;
}

/* For Detailed Placement */
void SimAnneal::anneal_detailed() {
    movables.clear();
    oldPlace.clear();
    newPlace.clear();

    hpwl = rb.evalHPWL();
    overlap = rb.calcOverlap();
    penaltyRow = rb.evalPRow();
    oldCost = cost();
    lambda = 1;

    cout << "Beginning temperature decline..." << endl;
    unsigned int j = 0;

    while (curTemp > stopTemp) {
        negAcceptCount = posAcceptCount = 0;
        totaloverlap = totalhpwl = 0;

        itCount = 0;

        cout << "It#: " << j << "\tTemp: " << curTemp << endl;

        while (itCount++ <= maxIter) {
            totalhpwl += hpwl;
            totaloverlap += overlap;
            if (!(itCount % (maxIter / 20))) {
                avghpwl = totalhpwl / itCount;
                avgoverlap = totaloverlap / itCount;
                if (avgoverlap == 0)
                    lambda = 1;
                else
                    lambda = (avghpwl > avgoverlap) ? avghpwl / avgoverlap : 1;

                oldCost = hpwl + lambda * overlap;
            }
            generate();

            hpwl -= rb.calcInstHPWL(movables);
            overlap -= rb.calcInstOverlap(movables);
            rb.updateCells(movables, newPlace, penaltyRow);
            newCost = cost();

            if (!accept(newCost, oldCost, curTemp)) {
                hpwl -= rb.calcInstHPWL(movables);
                overlap -= rb.calcInstOverlap(movables);
                rb.updateCells(movables, oldPlace, penaltyRow);
                oldCost = cost();
            } else {
                oldCost = newCost;
            }
        }
        update(curTemp);
        j++;
        cout << " HPWL: " << hpwl << " Over: " << overlap
             << " penRow: " << penaltyRow << endl;
    }
}

/* For Global Placement */
void SimAnneal::anneal_global() {
    movables.clear();
    oldPlace.clear();
    newPlace.clear();

    hpwl = rb.evalHPWL();
    overlap = rb.calcOverlap();
    penaltyRow = rb.evalPRow();
    oldCost = cost();
    lambda = 1;
    lambdaP = 1;

    cout << "Beginning temperature decline..." << endl;

    while (I++ < maxI) {
        acceptCount = 0;
        totalCount = 0;
        updateCount = 0;

        if (I < 0.15 * maxI)
            curRate *= exp(log(0.44) / (0.15 * maxI));
        else if (I < 0.68 * maxI)
            curRate = 0.44;
        else
            curRate *= exp(log(0.01 / 0.44) / (0.35 * maxI));

        negAcceptCount = posAcceptCount = 0;
        totaloverlap = totalhpwl = 0;

        itCount = 0;

        while (itCount++ <= maxIter) {
            updateCount++;
            if (updateCount == maxUpdate) {
                updateCount = 0;
                update_Lam(curTemp);
            }

            totalhpwl += hpwl;
            totaloverlap += overlap;
            if (!(itCount % (maxIter / 20))) {
                avghpwl = totalhpwl / itCount;
                avgoverlap = totaloverlap / itCount;
                if (avgoverlap == 0)
                    lambda = 1;
                else
                    lambda = (avghpwl > avgoverlap) ? avghpwl / avgoverlap : 1;
                lambdaP = 6.1 * lambda;

                oldCost = hpwl + 1.5 * overlap + lambdaP * penaltyRow;
            }
            generate();

            hpwl -= rb.calcInstHPWL(movables);
            overlap -= rb.calcInstOverlap(movables);
            rb.updateCells(movables, newPlace, penaltyRow);
            newCost = cost();

            if (!accept(newCost, oldCost, curTemp)) {
                hpwl -= rb.calcInstHPWL(movables);
                overlap -= rb.calcInstOverlap(movables);
                rb.updateCells(movables, oldPlace, penaltyRow);
                oldCost = cost();
            } else {
                acceptCount++;
                oldCost = newCost;
            }
        }
        dynamic_window();
        cout << windowfactor << " : " << xspan << " , " << yspan << endl;
        cout << "Iter: " << I << " HPWL: " << hpwl << " Over: " << overlap
             << " penRow: " << penaltyRow << endl;
        cout << "\tCR: " << curRate
             << " AR: " << double(acceptCount) / double(itCount) << endl;
    }
}

void SimAnneal::generate() {
    movables.clear();
    oldPlace.clear();
    newPlace.clear();

    size_t randIdx1 = rb.RandomUnsigned(0, rb.NumCells);

    node& randNode1 = rb.nodes[randIdx1];
    movables.push_back(randIdx1);
    oldPlace.push_back(Point(randNode1));

    double cellWidth1 = randNode1.w;

    unsigned int directedMove = rb.RandomUnsigned(0, 10);
    unsigned int whichMove = rb.RandomUnsigned(0, 10);

    if (directedMove < 0) {
        if (whichMove < 4) {
            size_t crow = rb.RandomUnsigned(0, rb.NumRows);
            row& cr = rb.rows[crow];
            size_t site = rb.RandomUnsigned(0, cr.num_sites);
            Point randLoc(cr.coord_x + site * cr.site_sp, cr.coord_y, &cr);

            bool check;
            Point newLoc = randLoc;
            if (!detailed)
                check = rb.checkPointInRow(newLoc);
            else
                check = !rb.findClosestWS(randLoc, newLoc, cellWidth1);

            if (check) {
                movables.pop_back();
                oldPlace.pop_back();
            } else {
                newPlace.push_back(newLoc);
            }
        } else {
            size_t randIdx2;

            do
                randIdx2 = rb.RandomUnsigned(0, rb.NumCells);
            while (randIdx2 == randIdx1);

            movables.push_back(randIdx2);
            oldPlace.push_back(Point(rb.nodes[randIdx2]));

            newPlace.push_back(Point(rb.nodes[randIdx2]));
            newPlace.push_back(Point(randNode1));
        }
    } else {
        double rowpitch = rb.rows[1].coord_y - rb.rows[0].coord_y;
        double sitepitch = randNode1.lRow->site_sp;
        double ycellcoordoffset = layoutBBox.yMin;
        double xcellcoordoffset = layoutBBox.xMin;

        Point curLoc = randNode1;

        if (whichMove < 4) {
            Point optLoc = rb.calcMeanLoc(randIdx1);

            optLoc.x =
                (floor((optLoc.x - xcellcoordoffset) / sitepitch)) * sitepitch +
                xcellcoordoffset;
            optLoc.y =
                (floor((optLoc.y - ycellcoordoffset) / rowpitch)) * rowpitch +
                ycellcoordoffset;

            optLoc.x =
                (optLoc.x >= layoutBBox.xMax) ? layoutBBox.xMax : optLoc.x;
            optLoc.x =
                (optLoc.x <= layoutBBox.xMin) ? layoutBBox.xMin : optLoc.x;
            optLoc.y = (optLoc.y >= layoutBBox.yMax)
                           ? layoutBBox.yMax - rb.heightSC
                           : optLoc.y;
            optLoc.y =
                (optLoc.y <= layoutBBox.yMin) ? layoutBBox.yMin : optLoc.y;

            bool check;
            Point newLoc = optLoc;
            if (!detailed) {
                rb.findCoreRow(newLoc);
                check = rb.checkPointInRow(newLoc);
            } else {
                check = !rb.findClosestWS(optLoc, newLoc, cellWidth1);
            }

            if (check) {
                movables.pop_back();
                oldPlace.pop_back();
            } else {
                newPlace.push_back(newLoc);
            }
        } else if (whichMove < 8) {
            Point newLoc;
            double ymin = ((curLoc.y - yspan) <= layoutBBox.yMin)
                              ? layoutBBox.yMin
                              : curLoc.y - yspan;
            double ymax = ((curLoc.y + yspan) >= layoutBBox.yMax)
                              ? layoutBBox.yMax
                              : curLoc.y + yspan;
            newLoc.y = rb.RandomDouble(ymin, ymax);

            double xmin = ((curLoc.x - xspan) <= layoutBBox.xMin)
                              ? layoutBBox.xMin
                              : curLoc.x - xspan;
            double xmax = ((curLoc.x + xspan) >= layoutBBox.xMax)
                              ? layoutBBox.xMax
                              : curLoc.x + xspan;
            newLoc.x = rb.RandomDouble(xmin, xmax);

            newLoc.x =
                (floor((newLoc.x - xcellcoordoffset) / sitepitch)) * sitepitch +
                xcellcoordoffset;
            newLoc.y =
                (floor((newLoc.y - ycellcoordoffset) / rowpitch)) * rowpitch +
                ycellcoordoffset;
            if (newLoc.y == layoutBBox.yMax) newLoc.y -= rb.heightSC;

            bool check;
            Point Loc = newLoc;
            if (!detailed) {
                rb.findCoreRow(Loc);
                check = rb.checkPointInRow(Loc);
            } else {
                check = !rb.findClosestWS(newLoc, Loc, cellWidth1);
            }

            if (check) {
                movables.pop_back();
                oldPlace.pop_back();
            } else {
                newPlace.push_back(Loc);
            }
        } else {
            Point newLoc;
            double ymin = ((curLoc.y - yspan) <= layoutBBox.yMin)
                              ? layoutBBox.yMin
                              : curLoc.y - yspan;
            double ymax = ((curLoc.y + yspan) >= layoutBBox.yMax)
                              ? layoutBBox.yMax
                              : curLoc.y + yspan;

            double xmin = ((curLoc.x - xspan) <= layoutBBox.xMin)
                              ? layoutBBox.xMin
                              : curLoc.x - xspan;
            double xmax = ((curLoc.x + xspan) >= layoutBBox.xMax)
                              ? layoutBBox.xMax
                              : curLoc.x + xspan;

            size_t randIdx2;
            do {
                newLoc.y = rb.RandomDouble(ymin, ymax);
                newLoc.x = rb.RandomDouble(xmin, xmax);
                newLoc.x = (floor((newLoc.x - xcellcoordoffset) / sitepitch)) *
                               sitepitch +
                           xcellcoordoffset;
                newLoc.y = (floor((newLoc.y - ycellcoordoffset) / rowpitch)) *
                               rowpitch +
                           ycellcoordoffset;
                if (newLoc.y == layoutBBox.yMax) newLoc.y -= rb.heightSC;
                randIdx2 = rb.findCellIdx(newLoc);
            } while (randIdx2 == randIdx1);

            if (randIdx2 < rb.NumCells) {
                movables.push_back(randIdx2);
                oldPlace.push_back(Point(rb.nodes[randIdx2]));

                newPlace.push_back(Point(rb.nodes[randIdx2]));
                newPlace.push_back(Point(randNode1));
            } else {
                newPlace.push_back(newLoc);
            }
        }
    }
}

double SimAnneal::cost() {
    hpwl += rb.calcInstHPWL(movables);
    overlap += rb.calcInstOverlap(movables);

    return ((!detailed) ? (hpwl + 1.5 * overlap + lambdaP * penaltyRow)
                        : (hpwl + lambda * overlap));
}

bool SimAnneal::accept(double newCost, double oldCost, double curTemp) {
    if (newCost <= oldCost) {
        posAcceptCount++;
        return (true);
    }

    if (greedy) return (false);

    if (rb.RandomDouble(0, 1) < exp(-fabs(newCost - oldCost) / curTemp)) {
        negAcceptCount++;
        return (true);
    }

    return (false);
}

void SimAnneal::update(double& _curTemp) {
    if (_curTemp > 0.66 * initTemp)
        _curTemp *= 0.8;
    else if (curTemp > 0.33 * initTemp)
        _curTemp *= 0.85;
    else if (_curTemp > 0.1 * initTemp)
        _curTemp *= 0.90;
    else
        _curTemp *= 0.96;

    if (curRate < initTemp && detailed) maxIter = 2 * rb.NumCells;

    dynamic_window();

    cout << windowfactor << " : " << xspan << " , " << yspan << endl;
}

void SimAnneal::update_Lam(double& _curTemp) {
    totalCount = itCount;
    double acceptRate = double(acceptCount) / totalCount;
    double change = (1 - (acceptRate - curRate) / 3);
    _curTemp *= change;
}

void SimAnneal::calibrate() {
    cout << "Performing a warm-up run to calibrate acceptance rates..." << endl;
    cout << "\t" << setw(10) << "Iter" << setw(10) << "\tTemp" << setw(10)
         << "\tNAR" << setw(10) << "\tAR" << setw(10) << "\tCost(HPWL+OV+PR)"
         << endl;

    double oldTemp = DBL_MAX;
    int it = 0;
    int maxIter = 500;

    while (fabs(oldTemp - curTemp) / oldTemp > 0.01 && it < maxIter) {
        ++it;
        oldTemp = curTemp;
        negAcceptCount = posAcceptCount = 0;
        totaloverlap = totalhpwl = 0;
        itCount = 0;
        cout << "[ ";

        while (itCount++ <= calibMaxIter) {
            totalhpwl += hpwl;
            totaloverlap += overlap;
            if (!(itCount % (calibMaxIter / 5))) {
                cout << ".";
                cout.flush();
            }
            if (itCount == 1) {
                avghpwl = totalhpwl / itCount;
                avgoverlap = totaloverlap / itCount;
                if (avgoverlap == 0)
                    lambda = 1;
                else
                    lambda = (avghpwl > avgoverlap) ? avghpwl / avgoverlap : 1;

                oldCost = hpwl + lambda * overlap;
            }
            if (!(itCount % (calibMaxIter / 5)) || itCount == 1) {
                double negAcceptRate = negAcceptCount / itCount;
                curTemp *= (1 - (negAcceptRate - 0.01) / 1);
            }

            generate();

            hpwl -= rb.calcInstHPWL(movables);
            overlap -= rb.calcInstOverlap(movables);
            rb.updateCells(movables, newPlace, penaltyRow);
            newCost = cost();

            accept(newCost, oldCost, curTemp);

            hpwl -= rb.calcInstHPWL(movables);
            overlap -= rb.calcInstOverlap(movables);
            rb.updateCells(movables, oldPlace, penaltyRow);
            oldCost = cost();
        }

        cout << " ]";

        double negAcceptRate = negAcceptCount / calibMaxIter;
        double acceptRate = (posAcceptCount + negAcceptCount) / calibMaxIter;
        cout << setw(10) << it << setw(10) << curTemp << "\t" << setw(10)
             << negAcceptRate << "\t" << setw(10) << acceptRate << "\t"
             << setw(10) << oldCost << endl;
    }
}

void SimAnneal::initPlacement(DataPlace& _rb) {
    double limitRow = 1.007 * _rb.findLimitRow();
    double ws = 0;         //_rb.rows.begin()->site_sp;
    double tab = 15 * ws;  // static_cast<int>( (_rb.rows.begin()->num_sites *
                           // _rb.rows.begin()->site_sp - limitRow)/2 );
    double offset;

    vector<node>& _nd = _rb.nodes;
    vector<row>& _rw = _rb.rows;
    vector<node>::iterator it_last = _nd.begin();
    vector<node>::iterator it = it_last;

    for (vector<row>::iterator itR = _rw.begin();
         itR != _rw.end(), it != _nd.end(); itR++) {
        offset = itR->coord_x;
        for (it = it_last; it != _nd.end(); it++) {
            if (limitRow > (itR->busySRow + it->w)) {
                it->pos_x = tab + offset;
                it->pos_y = itR->coord_y;
                offset += (ws + it->w);
                itR->busySRow += it->w;
                it->lRow = &(*itR);
                itR->ls.push_back(&(*it));
            } else {
                it_last = it;
                if (itR == _rw.end() - 1) {
                    cerr << "Ошибка инициализации размещения: недостаточно "
                            "строк для расположения всех нодов"
                         << endl;
                    system("pause");
                    exit(1);
                }
                break;
            }
        }
    }
}

void SimAnneal::fillingRows(DataPlace& _rb) {
    vector<node>& _nd = _rb.nodes;
    vector<row>& _rw = _rb.rows;

    for (vector<node>::iterator it = rb.nodes.begin(); it != rb.nodes.end();
         it++) {
        vector<row>::iterator fIt =
            lower_bound(_rw.begin(), _rw.end(), it->pos_y,
                        [](row const& comp, double const& rY) {
                            return comp.coord_y < rY;
                        });
        if (fIt != _rw.end()) {
            if (fIt->coord_y != it->pos_y) {
                cerr << "ERROR Row! fillingRows" << endl;
                exit(1);
            }
            it->lRow = &(*fIt);
            fIt->ls.push_back(&(*it));
            fIt->busySRow += it->w;
        } else {
            cerr << "function fillingRows. ERROR!" << endl;
            exit(1);
        }
    }

    for (vector<row>::iterator itR = _rw.begin(); itR != _rw.end(); itR++) {
        sort(
            itR->ls.begin(), itR->ls.end(),
            [](node* const& a, node* const& b) { return a->pos_x < b->pos_x; });
    }
}

void SimAnneal::dynamic_window() {
    double areaPerCell = layoutArea / rb.NumCells;
    double layoutAR = layoutXSize / layoutYSize;
    double widthPerCell = sqrt(areaPerCell * layoutAR);
    double heightPerCell = widthPerCell / layoutAR;

    double minyspan = 2 * (rb.rows[1].coord_y - rb.rows[0].coord_y);
    double minxspan = 5 * widthPerCell;

    if (detailed) {
        if (curTemp > 0 && stopTemp > 0 && initTemp > stopTemp) {
            windowfactor = log10(curTemp / stopTemp) / log10(initTemp / stopTemp);
        } else {
            windowfactor = 1.0;
        }
    } else {
        if (I < 23)
            windowfactor = 1;
        else if (I <= 97)
            windowfactor = exp(-1.5 * (I - 22) / 75);
        else
            windowfactor = exp(-5.5);
    }

    const double scale = (!detailed) ? 150 : 15;
    xspan = windowfactor * scale * widthPerCell;
    yspan = windowfactor * scale * heightPerCell;

    if (xspan < minxspan) xspan = minxspan;
    if (yspan < minyspan) yspan = minyspan;
}