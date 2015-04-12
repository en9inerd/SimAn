//SimAnneal.h

#ifndef _SIMANNEAL_H_
#define _SIMANNEAL_H_

#include "DataPlace.h"
#include "BBox.h"

using namespace std;

class SimAnneal
{
private: 
	void generate();
	void calibrate();
	double cost();
	bool accept(double newCost, double oldCost, double curTemp);
	void update(double& curTemp);
	void anneal();

	double windowfactor;
	double xspan, yspan;

	DataPlace &rb;
	double layoutXSize, layoutYSize, layoutArea;
	size_t maxIter;
	size_t calibMaxIter;
	double initTemp;
	double curTemp;
	double stopTemp;

	bool greedy, detailed;
	double hpwl, avghpwl, totalhpwl;
	double overlap, avgoverlap, totaloverlap;
	double penaltyRow;
	double oldCost, newCost;
	double lambda;

	vector<size_t> movables;
	vector<Point> oldPlace;
	vector<Point> newPlace;

	double posAcceptCount, negAcceptCount, totalCount;
	size_t itCount;

	void initPlacement(DataPlace& );
public:
	SimAnneal(DataPlace& rbplace, bool gr, bool det);
};

#endif