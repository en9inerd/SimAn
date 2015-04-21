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
	void generate_det();
	void calibrate();
	double cost();
	double cost_det();
	bool accept(double newCost, double oldCost, double curTemp);
	void dynamic_window();
	void update(double& curTemp);
	void anneal();
	void anneal_det();

	double windowfactor;
	double xspan, yspan;

	DataPlace &rb;
	BBox layoutBBox;
	double layoutXSize, layoutYSize, layoutArea;
	size_t maxIter;
	size_t calibMaxIter;
	double initTemp;
	double curTemp;
	double stopTemp;
	double k;

	bool greedy, detailed;
	double hpwl, avghpwl, totalhpwl;
	double overlap, avgoverlap, totaloverlap;
	double penaltyRow;
	double oldCost, newCost;
	double lambda, lambdaP;

	vector<size_t> movables;
	vector<Point> oldPlace;
	vector<Point> newPlace;

	double posAcceptCount, negAcceptCount, totalCount;
	size_t itCount;

	void initPlacement(DataPlace& _rb);
	void fillingRows(DataPlace& _rb);
public:
	SimAnneal(DataPlace& rbplace, bool gr, bool det);
};

#endif