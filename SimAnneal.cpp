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
		lambda(1), lambdaP(1), layoutBBox(rb)
{
	negAcceptCount = posAcceptCount = 0;
	layoutXSize = layoutBBox.getWidth();
	layoutYSize = layoutBBox.getHeight();
	layoutArea = layoutXSize * layoutYSize;
	cout<<"Placement area: "<<layoutXSize<<" x "<<layoutYSize<<endl;
	cout<<"Num move_cells: "<<rb.NumCells<<endl;

	if(!detailed)
	{
		initPlacement(rb);
	}
	else
	{
		fillingRows(rb);
	}

	hpwl = rb.evalHPWL();
	overlap = rb.calcOverlap(detailed);
	penaltyRow = rb.evalPRow();
	oldCost = cost();

	initTemp = curTemp = hpwl / rb.NumNets; //initial HPWL per net
	stopTemp = curTemp/100;
	unsigned int k = 1; //specify user
	maxIter = k * rb.NumCells;

	cout<<"Initial:\t Temp: "<<curTemp<<" Iter: "<<maxIter 
		<<" HPWL: "<<rb.evalHPWL()<<" Over: "<<rb.calcOverlap(detailed) 
		<<" Cost: "<<oldCost<<endl;

	dynamic_window();

	if(!greedy && detailed)
		calibrate();

	initTemp = curTemp;
	stopTemp = curTemp/100;

	anneal();
	rb.checkPRow();
	//for(vector<node*>::const_iterator it = rb.rows[0].ls.begin(); it != rb.rows[0].ls.end(); it++)
	//{
	//	cout<<(*it)->pos_x<<", ";
	//}
	//cout<<endl;

	cout<<"Final:\t HPWL: "<<rb.evalHPWL()<<" Over: "<<rb.calcOverlap(true)<<endl;
}

void SimAnneal::anneal()
{
	movables.clear();
	oldPlace.clear();
	newPlace.clear();

	hpwl = rb.evalHPWL();
	overlap = rb.calcOverlap();
	penaltyRow = rb.evalPRow();
	oldCost = cost();
	lambda = 1;
	lambdaP = 1;

	cout<<"Beginning temperature decline..."<<endl;
	unsigned int j = 0;

	while (curTemp > stopTemp)
	{
		negAcceptCount = posAcceptCount = 0;
		totaloverlap = totalhpwl = 0;

		itCount = 0;

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
				lambdaP = 4.3 * lambda;

				oldCost = (!detailed) ? (hpwl + overlap + lambdaP*penaltyRow) : (hpwl + lambda*overlap);
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
		cout<<" HPWL: "<<rb.evalHPWL()<<":"<<hpwl<<" Over: "<<rb.calcOverlap(true)<<":"<<overlap<<" penRow: "<<rb.evalPRow()<<":"<<penaltyRow<<endl;
	}
}

void SimAnneal::generate()
{
	movables.clear();
	oldPlace.clear();
	newPlace.clear();

	size_t randIdx1 = rb.RandomUnsigned(0,rb.NumCells);

	node& randNode1 = rb.nodes[randIdx1];
	movables.push_back(randIdx1);
	oldPlace.push_back(Point(randNode1));

	double cellWidth1 = randNode1.w;

	unsigned int directedMove = rb.RandomUnsigned(0,10);
	unsigned int whichMove = rb.RandomUnsigned(0,10);

	if(directedMove < 0)
	{
		if(whichMove < 4)
		{
			size_t crow = rb.RandomUnsigned(0,rb.NumRows);
			row& cr = rb.rows[crow];
			size_t site = rb.RandomUnsigned(0,cr.num_sites);
			Point randLoc(cr.coord_x+site*cr.site_sp, cr.coord_y, &cr);

			if(rb.checkPointInRow(randLoc) )
			{
				movables.pop_back();
				oldPlace.pop_back();
			}
			else
			{
				newPlace.push_back(randLoc);
			}
		}
		else
		{
			size_t randIdx2;

			do randIdx2 = rb.RandomUnsigned(0,rb.NumCells);
			while(randIdx2 == randIdx1);

			movables.push_back(randIdx2);
			oldPlace.push_back(Point(rb.nodes[randIdx2]));

			newPlace.push_back(Point(rb.nodes[randIdx2]));
			newPlace.push_back(Point(randNode1));
		}
	}
	else
	{
		double rowpitch = rb.rows[1].coord_y - rb.rows[0].coord_y;
		double sitepitch = randNode1.lRow->site_sp;
		double ycellcoordoffset = layoutBBox.yMin;
		double xcellcoordoffset = layoutBBox.xMin;

		Point curLoc = randNode1;

		if(whichMove < 3)
		{
			Point optLoc = rb.calcMeanLoc(randIdx1);

			optLoc.x = (floor((optLoc.x - xcellcoordoffset)/sitepitch))*sitepitch + xcellcoordoffset;
			optLoc.y = (floor((optLoc.y - ycellcoordoffset)/rowpitch))*rowpitch + ycellcoordoffset;

			optLoc.x = (optLoc.x >= layoutBBox.xMax) ? layoutBBox.xMax : optLoc.x;
			optLoc.x = (optLoc.x <= layoutBBox.xMin) ? layoutBBox.xMin : optLoc.x;
			optLoc.y = (optLoc.y >= layoutBBox.yMax - rb.heightSC) ? layoutBBox.yMax - rb.heightSC : optLoc.y;
			optLoc.y = (optLoc.y <= layoutBBox.yMin) ? layoutBBox.yMin : optLoc.y;

			rb.findCoreRow(optLoc);
			if(rb.checkPointInRow(optLoc))
			{
				movables.pop_back();
				oldPlace.pop_back();
			}
			else
			{
				newPlace.push_back(optLoc);
			}
		}
		else if(whichMove < 6)
		{
			Point newLoc;
			double ymin = ((curLoc.y - yspan) <= layoutBBox.yMin) ? layoutBBox.yMin : curLoc.y - yspan;
			double ymax = ((curLoc.y + yspan) >= layoutBBox.yMax) ? layoutBBox.yMax : curLoc.y + yspan;
			newLoc.y = rb.RandomDouble(ymin, ymax);

			double xmin = ((curLoc.x - xspan) <= layoutBBox.xMin) ? layoutBBox.xMin : curLoc.x - xspan;
			double xmax = ((curLoc.x + xspan) >= layoutBBox.xMax) ? layoutBBox.xMax : curLoc.x + xspan;
			newLoc.x = rb.RandomDouble(xmin, xmax);

			newLoc.x = (floor((newLoc.x - xcellcoordoffset)/sitepitch))*sitepitch + xcellcoordoffset;
			newLoc.y = (floor((newLoc.y - ycellcoordoffset)/rowpitch))*rowpitch + ycellcoordoffset;
			if(newLoc.y == layoutBBox.yMax)
				newLoc.y -= rb.heightSC;

			rb.findCoreRow(newLoc);
			if(rb.checkPointInRow(newLoc) )
			{
				movables.pop_back();
				oldPlace.pop_back();
			}
			else
			{
				newPlace.push_back(newLoc);
			}
		}
		else
		{
			Point newLoc;
			double ymin = ((curLoc.y - yspan) <= layoutBBox.yMin) ? layoutBBox.yMin : curLoc.y - yspan;
			double ymax = ((curLoc.y + yspan) >= layoutBBox.yMax) ? layoutBBox.yMax : curLoc.y + yspan;

			double xmin = ((curLoc.x - xspan) <= layoutBBox.xMin) ? layoutBBox.xMin : curLoc.x - xspan;
			double xmax = ((curLoc.x + xspan) >= layoutBBox.xMax) ? layoutBBox.xMax : curLoc.x + xspan;

			size_t randIdx2;
			do
			{
				newLoc.y = rb.RandomDouble(ymin, ymax);
				newLoc.x = rb.RandomDouble(xmin, xmax);
				newLoc.x = (floor((newLoc.x - xcellcoordoffset)/sitepitch))*sitepitch + xcellcoordoffset;
				newLoc.y = (floor((newLoc.y - ycellcoordoffset)/rowpitch))*rowpitch + ycellcoordoffset;
				if(newLoc.y == layoutBBox.yMax)
					newLoc.y -= rb.heightSC;
				randIdx2 = rb.findCellIdx(newLoc);
			}
			while(randIdx2 == randIdx1);

			if(randIdx2 < rb.NumCells)
			{
				movables.push_back(randIdx2);
				oldPlace.push_back(Point(rb.nodes[randIdx2]));

				newPlace.push_back(Point(rb.nodes[randIdx2]));
				newPlace.push_back(Point(randNode1));
			}
			else
			{
				newPlace.push_back(newLoc);
			}
		}
	}
}

double SimAnneal::cost()
{
	hpwl += rb.calcInstHPWL(movables);
	overlap += rb.calcInstOverlap(movables);

	return((!detailed) ? (hpwl + overlap + lambdaP*penaltyRow) : (hpwl + lambda*overlap));
}

bool SimAnneal::accept(double newCost, double oldCost, double curTemp)
{
	if(newCost <= oldCost)
	{
		posAcceptCount++;
		return(true);
	}

	if(greedy)
		return(false);

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

	dynamic_window();

	cout<<windowfactor<<" : "<<xspan<<" , "<<yspan<<endl;
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

				lambdaP = 4.3 * lambda;
				oldCost = (!detailed) ? (hpwl + overlap + lambdaP*penaltyRow) : (hpwl + lambda*overlap);
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
			<<"\t"<<setw(10)<<curTemp
			<<"\t"<<setw(10)<<negAcceptRate
			<<"\t"<<setw(10)<<acceptRate
			<<"\t"<<setw(10)<<oldCost
			<<endl;
	}
}

void SimAnneal::initPlacement(DataPlace& _rb)
{
	double limitRow = 1.007 * _rb.findLimitRow();
	double ws = 0; //_rb.rows.begin()->site_sp; 
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
					cerr<<"Ошибка инициализации размещения: недостаточно строк для расположения всех нодов"<<endl;
					system("pause");
					exit(1);
				}
				break;
			}
		}
	}
}

void SimAnneal::fillingRows(DataPlace& _rb)
{
	vector<node>& _nd = _rb.nodes;
	vector<row>& _rw = _rb.rows;

	for(vector<node>::iterator it = rb.nodes.begin(); it != rb.nodes.end(); it++)
	{
		vector<row>::iterator fIt = lower_bound(_rw.begin(), _rw.end(), it->pos_y,
			[] (row const& comp, double const& rY) { return comp.coord_y < rY; });
		if(fIt != _rw.end() )
		{
			if(fIt->coord_y != it->pos_y)
			{
				cerr<<"ERROR Row! fillingRows"<<endl;
				exit(1);
			}
			it->lRow = &(*fIt);
			fIt->ls.push_back(&(*it));
			fIt->busySRow += it->w;
		}
		else
		{
			cerr<<"function fillingRows. ERROR!"<<endl;
			exit(1);
		}
	}

	for(vector<row>::iterator itR = _rw.begin(); itR != _rw.end(); itR++)
	{
		sort(itR->ls.begin(), itR->ls.end(),
			[] (node* const& a, node* const& b) { return a->pos_x < b->pos_x; });
	}
}

void SimAnneal::dynamic_window()
{
	double areaPerCell = layoutArea/rb.NumCells;
	double layoutAR = layoutXSize/layoutYSize;
	double widthPerCell = sqrt(areaPerCell*layoutAR);
	double heightPerCell = widthPerCell/layoutAR;

	double minyspan = 2*(rb.rows[1].coord_y-rb.rows[0].coord_y);
	double minxspan = 5*widthPerCell;

	windowfactor = log10(curTemp/stopTemp)/log10(initTemp/stopTemp);
	const double scale = (detailed) ? 15 : 200;
	xspan = windowfactor*scale*widthPerCell;
	yspan = windowfactor*scale*heightPerCell;

	if(xspan < minxspan)
		xspan = minxspan;
	if(yspan < minyspan)
		yspan = minyspan;
}