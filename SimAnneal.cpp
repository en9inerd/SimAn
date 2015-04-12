//SimAnneal.cpp

#include "stdafx.h"
#include "SimAnneal.h"

using std::cout;
using std::endl;
using std::setw;

SimAnneal::SimAnneal(DataPlace& rbplace, bool gr, bool det)
	: rb(rbplace), greedy(gr), detailed(det),
		calibMaxIter(500),
		oldCost(0), newCost(0),
		oldPlace(0), newPlace(0),
		lambda(1)
{
	negAcceptCount = posAcceptCount = 0;
	BBox layoutBBox(rb);
	layoutXSize = fabs(layoutBBox.getWidth());
	layoutYSize = fabs(layoutBBox.getHeight());
	layoutArea = layoutXSize * layoutYSize;
	cout << "Placement area: " << layoutXSize << " x " << layoutYSize << endl;
	cout << "Num move_cells: " << rb.NumCells << endl;

	if(!detailed) initPlacement(rb);

	hpwl = rb.evalHPWL();
	overlap = rb.calcOverlap();
	penaltyRow = rb.calcPRow();
	oldCost = cost();

	initTemp = curTemp = hpwl / rb.NumNets;
	stopTemp = curTemp/100;
	unsigned int k = 2;
	maxIter = k * rb.NumCells;

	cout << "Initial:\t Temp: " << curTemp << " Iter: " << maxIter 
		<< " HWPL: " << rb.evalHPWL() << " Over: " << rb.calcOverlap() 
		<< " Cost: " << oldCost << endl;

	double areaPerCell = layoutArea/rb.NumCells;
	double layoutAR = layoutXSize/layoutYSize;
	double widthPerCell = sqrt(areaPerCell*layoutAR);
	double heightPerCell = widthPerCell/layoutAR;

	double minyspan = 2*(rb.rows[1].coord_y-rb.rows[0].coord_y);
	double minxspan = 5*widthPerCell;

	windowfactor = log10(curTemp/stopTemp)/log10(initTemp/stopTemp);
	const double scale = 15;
	xspan = windowfactor*scale*widthPerCell;
	yspan = windowfactor*scale*heightPerCell;

	if(xspan < minxspan)
		xspan = minxspan;
	if(yspan < minyspan)
		yspan = minyspan;

	//calibrate();

	//initTemp = curTemp;
	//stopTemp = curTemp/100;

	anneal();
	rb.checkPRow();
	//rb.print_nodes();

	cout << "Final:\t HWPL: " << rb.evalHPWL() << " Over: " << rb.calcOverlap(true) << endl;
}

void SimAnneal::anneal()
{
	cout << "Beginning temperature decline..." << endl;
	unsigned int j = 0;
	while (curTemp > stopTemp)
	{
		negAcceptCount = posAcceptCount = 0;
		totaloverlap = totalhpwl = 0;

		itCount = 0;
		double avgCost = 0, totalCost = 0;

		cout<<"It#: "<<j<<"\tTemp: "<<curTemp<<endl;

		while (itCount++ <= maxIter)
		{
			totalhpwl += hpwl;
			totaloverlap += overlap;
			if (!(itCount%(maxIter/20)))
			{
				avghpwl = totalhpwl / itCount;
				avgoverlap = totaloverlap / itCount;
				if (avgoverlap == 0)
					lambda = 1;
				else
					lambda = (avghpwl > avgoverlap)? avghpwl/avgoverlap : 1;
				oldCost = hpwl + overlap + 1.4*lambda*penaltyRow;
			}
			generate();

			hpwl -= rb.calcInstHPWL(movables);
			overlap -= rb.calcInstOverlap(movables);
			rb.updateCells(movables, newPlace, penaltyRow);
			newCost = cost();

			if (!accept(newCost, oldCost, curTemp))
			{
				hpwl -= rb.calcInstHPWL(movables);
				overlap -= rb.calcInstOverlap(movables);
				rb.updateCells(movables, oldPlace, penaltyRow);
				oldCost = cost();
			}
			else
			{
				oldCost = newCost;
			}
		}
		update(curTemp);
		j++;
		cout << " HPWL: " << rb.evalHPWL() <<":"<<hpwl<< " Over: " << rb.calcOverlap(true) <<":"<<overlap<< " penRow: " << rb.calcPRow() <<":"<<penaltyRow<<endl;
	}
}

void SimAnneal::generate()
{
	movables.clear();
	oldPlace.clear();
	newPlace.clear();

	size_t randIdx1 = rb.RandomUnsigned(0,rb.NumCells);

	movables.push_back(randIdx1);
	oldPlace.push_back(Point(rb.nodes[randIdx1].pos_x,rb.nodes[randIdx1].pos_y,rb.nodes[randIdx1].lRow));

	unsigned int whichMove = rb.RandomUnsigned(0,5);

	if(whichMove < 4)
	{
		size_t crow = rb.RandomUnsigned(0,rb.NumRows);
		row& cr = rb.rows[crow];
		size_t site = rb.RandomUnsigned(0,cr.num_sites);
		Point randLoc(cr.coord_x+site*cr.site_sp, cr.coord_y, &cr);

		bool foundNode = false;
		for(vector<node* >::const_iterator it = cr.ls.begin(); it != cr.ls.end(); it++)
		{
			if((*it)->pos_x == randLoc.x)
			{
				foundNode = true;
				movables.pop_back();
				oldPlace.pop_back();
				break;
			}
		}
		if(!foundNode) newPlace.push_back(randLoc);
		//if(!foundNode) cout << "displace: " << randIdx1 << "->" <<  crow << "," << site << endl;
	}
	else
	{
		size_t randIdx2;

		do randIdx2 = rb.RandomUnsigned(0,rb.NumCells);
		while(randIdx2 == randIdx1);

		movables.push_back(randIdx2);
		oldPlace.push_back(Point(rb.nodes[randIdx2].pos_x,rb.nodes[randIdx2].pos_y,rb.nodes[randIdx2].lRow));

		newPlace.push_back(Point(rb.nodes[randIdx2].pos_x,rb.nodes[randIdx2].pos_y,rb.nodes[randIdx2].lRow));
		newPlace.push_back(Point(rb.nodes[randIdx1].pos_x,rb.nodes[randIdx1].pos_y,rb.nodes[randIdx1].lRow));
		//cout << "swap:";
		//cout << "\t" << randIdx1 << endl;
		//cout << "\t" << randIdx2 << endl;
	}
}

double SimAnneal::cost()
{
	hpwl += rb.calcInstHPWL(movables);
	overlap += rb.calcInstOverlap(movables);

	return(hpwl + overlap + 1.4*lambda*penaltyRow);
}

bool SimAnneal::accept(double newCost, double oldCost, double curTemp)
{
	if(newCost <= oldCost)
	{
		posAcceptCount++;
		return(true);
	}

	if (rb.RandomDouble(0,1) < exp(-fabs(newCost-oldCost)/curTemp))
	{
		negAcceptCount++;
		return(true);
	}

	return(false);
}

void SimAnneal::update(double& _curTemp)
{
	if (_curTemp > 0.66*initTemp)
		_curTemp *=0.8;
	else if (curTemp > 0.33*initTemp)
		_curTemp *= 0.85;
	else if(_curTemp > 0.1*initTemp)
		_curTemp *= 0.90;
	else
		_curTemp *= 0.96;
}

void SimAnneal::calibrate()
{
	cout<<"Performing a warm-up run to calibrate acceptance rates..."<<endl;
	cout<<setw(10)<<"Iter"
		<<setw(10)<<"\tTemp"
		<<setw(10)<<"\tNAR"
		<<setw(10)<<"\tAR"
		<<setw(10)<<"\tCost(HPWL+OV+PR)"<<endl;

	double oldTemp = DBL_MAX;
	int it = 0;
	int maxIter = 500;

	while(fabs(oldTemp-curTemp)/oldTemp > 0.01 && it <maxIter)
	{
		++it;
		oldTemp = curTemp;
		negAcceptCount = posAcceptCount = 0;
		totaloverlap = totalhpwl = 0;
		itCount = 0;
		cout<<"[ ";

		while (itCount++ <= calibMaxIter)
		{
			totalhpwl += hpwl;
			totaloverlap += overlap;
			if( !(itCount%(calibMaxIter/5)) )
			{
				cout<<".";
				cout.flush();
			}
			if(itCount == 1)
			{
				avghpwl = totalhpwl/itCount;
				avgoverlap = totaloverlap/itCount;
				if(avgoverlap == 0 )
					lambda = 1;
				else
					lambda = (avghpwl>avgoverlap)?avghpwl/avgoverlap:1;
				oldCost = hpwl + overlap + 1.4*lambda*penaltyRow;
			}
			if( !(itCount%(calibMaxIter/5)) || itCount == 1 )
			{
				double negAcceptRate = negAcceptCount / itCount;
				curTemp *= (1-(negAcceptRate - 0.01)/1);
			}

			generate();

			hpwl -= rb.calcInstHPWL(movables);
			overlap -= rb.calcInstOverlap(movables);
			rb.updateCells(movables, newPlace, penaltyRow);
			newCost = cost();

			accept(newCost, oldCost, curTemp);

			hpwl -= rb.calcInstHPWL(movables);
			overlap -= rb.calcInstOverlap(movables);
			rb.updateCells(movables, newPlace, penaltyRow);
			oldCost = cost();
		}

		cout<<" ]";

		double negAcceptRate = negAcceptCount / calibMaxIter;
		double acceptRate = (posAcceptCount + negAcceptCount) / calibMaxIter;
		cout<<setw(10)<<it
			<<setw(10)<<curTemp
			<<"\t"<<setw(10)<<negAcceptRate
			<<"\t"<<setw(10)<<acceptRate
			<<"\t"<<setw(10)<<oldCost
			<<endl;
	}
}

void SimAnneal::initPlacement(DataPlace& _rb)
{
	double limitRow = 1.007 * _rb.findLimitRow();
	double ws = _rb.rows.begin()->site_sp; 
	double tab = 15 * ws;   //static_cast<int>( (_rb.rows.begin()->num_sites * _rb.rows.begin()->site_sp - limitRow)/2 );
	double offset;

	vector<node>& _nd = _rb.nodes;
	vector<row>& _rw = _rb.rows;
	vector<node>::iterator it_last = _nd.begin();
	vector<node>::iterator it = it_last;

	for(vector<row>::iterator itR = _rw.begin(); itR != _rw.end(), it != _nd.end(); itR++)
	{
		offset = itR->coord_x;
		for(it = it_last; it != _nd.end(); it++)
		{
			if( limitRow > (itR->busySRow + it->w) )
			{
				it->pos_x = tab + offset;
				it->pos_y = itR->coord_y;
				offset += (ws + it->w);
				itR->busySRow += it->w;
				it->lRow = &(*itR);
				itR->ls.push_back(&(*it));
			}
			else
			{
				it_last = it;
				if ( itR == _rw.end()-1 )
				{
					cerr<<"������ ������������� ����������: ������������ ����� ��� ������������ ���� �����"<<endl;
					system("pause");
					exit(1);
				}
				break;
			}
		}
	}
}


