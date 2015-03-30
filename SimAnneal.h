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
	double cost();
	bool accept(double newCost, double oldCost, double curTemp);
	void update(double& curTemp);
	void anneal();

	//double windowfactor;
	//double xspan, yspan;

	DataPlace &rb;
	double layoutXSize, layoutYSize, layoutArea;
	unsigned int maxIter;
	double initTemp;
	double curTemp;
	double stopTemp;

	double hpwl;
	double overlap;
	double penaltyRow;
	double oldCost, newCost;

	vector<size_t> movables;
	vector<Point> oldPlace;
	vector<Point> newPlace;

	double posAcceptCount, negAcceptCount, totalCount;
	size_t itCount;

	void initPlacement(DataPlace& );
public:
	SimAnneal(DataPlace& rbplace);
};

#endif