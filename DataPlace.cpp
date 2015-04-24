//DataPlace.cpp

#include "stdafx.h"
#include "DataPlace.h"
#include <fstream>
#include <sstream>
#include <tuple>
#include <unordered_set>
using namespace std;

DataPlace::DataPlace() : LengthNodes(0), heightSC(0)
{
	srand((unsigned int)time(NULL));
}

void DataPlace::Start(const char* p_aux)
{
	parseAuxFile(p_aux);
	parser();
	heightSC = rows[0].h;
}

void DataPlace::parseAuxFile(const char* p_aux)
{
	path_nodes = new char[15];
	path_nets = new char[15];
	path_pl = new char[15];
	path_scl = new char[15];
	char temp[50];

	ifstream f_aux(p_aux, ios::in);
	if(!f_aux)
	{
		cerr<<"Ошибка чтения файла .aux"<<endl;
		system("pause");
		exit(1);
	}
	else
	{
		f_aux.ignore(100, ':');
		f_aux>>path_nodes;
		f_aux>>path_nets;
		f_aux>>temp;
		f_aux>>path_pl;
		f_aux>>path_scl;
	}
	f_aux.close();
}

void DataPlace::parseNodes()
{
	cout<<"Parse .nodes ... | ";
	char temp[50];
	ifstream f_nd(path_nodes, ios::in);
	if(!f_nd)
	{
		cerr<<"Ошибка чтения файла .nodes"<<endl;
		system("pause");
		exit(1);
	}

	while(f_nd)
	{
		f_nd>>temp;
		if( !strcmp(temp,"NumNodes") )
		{
			f_nd.ignore(50,':');
			f_nd>>NumNodes;
			f_nd.get();
			f_nd.ignore(50,':');
			f_nd>>NumTerminals;
			f_nd.ignore(50,'\n');
			f_nd>>temp;
			string name;
			double w, h;
			for(int i=0; i<NumNodes; i++)
			{
				name = temp;
				f_nd>>w;
				f_nd>>h;
				f_nd>>temp;
				if ( !strcmp(temp,"terminal") || !strcmp(temp,"terminal_NI") )
				{
					terminal tr;
					tr.name = name;
					tr.w = w;
					tr.h = h;
					terminals.push_back(tr);
					f_nd.ignore(50,'\n');
					f_nd>>temp;
				}
				else
				{
					node nd;
					nd.name = name;
					nd.w = w;
					nd.h = h;
					//f_nd.seekg(20,ios::cur);
					nodes.push_back(nd);
					nd.i = nodes.size()-1;
					//f_nd.getline(temp,30);
					LengthNodes += w;
				}
			}
			break;
		}
	}
	f_nd.close();

	NumCells = NumNodes - NumTerminals;
}

void DataPlace::parseNets()
{
	cout<<"Parse .nets ... | ";
	char temp[50];
	ifstream f_nt(path_nets, ios::in);
	if(!f_nt)
	{
		cerr<<"Ошибка чтения файла .nets"<<endl;
		system("pause");
		exit(1);
	}

	while(f_nt)
	{
		f_nt>>temp;
		if( !strcmp(temp,"NumNets") )
		{
			f_nt.ignore(10,':');
			f_nt>>NumNets;
			f_nt.get();
			f_nt.ignore(50,':');
			f_nt>>NumPins;
			for (int k=0; k<NumNets; k++)
			{
				f_nt>>temp;
				if( !strcmp(temp,"NetDegree") )
				{
					unsigned n;
					net nt;
					f_nt.ignore(10,':');
					f_nt>>n;
					f_nt.ignore(50,'\n');
					for(int i=0; i<n; i++)
					{
						string t;
						f_nt>>t;
						bool finded = false;

						for(int j=0; j<(NumCells); j++)
							if(nodes[j].name == t)
							{
								finded = true;
								nt.ls.push_back(&nodes[j]);
								nodes[j].nets_of_nodes.push_back(nets.size());
								break;
							}

						if(!finded)
						{
							for(int j=0; j<NumTerminals; j++)
								if(terminals[j].name == t)
								{
									nt.ls.push_back(&terminals[j]);
									terminals[j].nets_of_nodes.push_back(nets.size());
									break;
								}
						}
						f_nt.ignore(50,'\n');
					}
					nets.push_back(nt);
				}
			}
		}
	}
	f_nt.close();
}

void DataPlace::parseScl()
{
	cout<<"Parse .scl ... | ";
	char temp[50];
	ifstream f_scl(path_scl, ios::in);
	if(!f_scl)
	{
		cerr<<"Ошибка чтения файла .scl"<<endl;
		system("pause");
		exit(1);
	}

	while(f_scl)
	{
		f_scl>>temp;
		if( !strcmp(_strlwr(temp),"numrows") )
		{
			f_scl.ignore(10,':');
			f_scl>>NumRows;
			for(int i=0; i<NumRows; i++)
			{
				row rw;
				f_scl.ignore(50,':');
				f_scl>>rw.coord_y;
				f_scl.ignore(50,':');
				f_scl>>rw.h;
				f_scl.ignore(50,':');
				f_scl.ignore(50,':');
				f_scl>>rw.site_sp;
 				f_scl.ignore(50,':');
				f_scl.ignore(50,':');
				f_scl.ignore(50,':');
				f_scl>>rw.coord_x;
				f_scl.ignore(50,':');
				f_scl>>rw.num_sites;
				rows.push_back(rw);
			}
			break;
		}
	}
	f_scl.close();
}

void DataPlace::parsePl()
{
	cout<<"Parse .pl ... ";
	ifstream f_pl(path_pl, ios::in);
	if(!f_pl)
	{
		cerr<<"Ошибка чтения файла .pl"<<endl;
		system("pause");
		exit(1);
	}

	string s;
	while(getline(f_pl, s))
	{
		stringstream ss(s);
		vector<string> words(5);

		for(int i=0; i<5; i++)
			ss >> words[i];

		if(words[3] == ":")
		{
			bool finded = false;

			for(int j=0; j<NumCells; j++)
				if(nodes[j].name == words[0])
				{
					finded = true;
					nodes[j].pos_x = stoi(words[1]);
					nodes[j].pos_y = stoi(words[2]);
					break;
				}

			if(!finded)
			{
				for(int j=0; j<NumTerminals; j++)
					if(terminals[j].name == words[0])
					{
						terminals[j].pos_x = stoi(words[1]);
						terminals[j].pos_y = stoi(words[2]);
						break;
					}
			}
		}
	}

	f_pl.close();
}

void DataPlace::parser()
{
	cout<<" << Parser... >> "<<endl;
	cout<<" [ ";
	parseNodes();
	parseNets();
	parseScl();
	parsePl();
	cout<<" ] "<<endl;
	cout<<endl;
}

void DataPlace::print_nodes()
{
	cout<<"Список нодов:"<<endl;
	for(int i=0; i<(NumNodes-NumTerminals); i++)
	{
		cout<<"Нод "<<nodes[i].name<<": номера связей [";
		for(int j=0; j<nodes[i].nets_of_nodes.size(); j++)
			cout<<" "<<nodes[i].nets_of_nodes[j]+1;
		cout<<" ]"<<endl;
	}
	cout<<endl;
    for(int i=0; i<NumTerminals; i++)
	{
		cout<<"Терминал "<<terminals[i].name<<endl;
	}
}

void DataPlace::print_nets()
{
	cout<<"Список цепей:"<<endl;
	for(int i=0; i<NumNets; i++)
	{
		cout<<"Цепь "<<i+1<<": {";
		for(int j=0; j<nets[i].ls.size(); j++)
			cout<<" "<<nets[i].ls[j]->name;
		cout<<" }"<<endl;
	}
	cout<<endl;
}

void DataPlace::print_scl()
{
	cout<<"Список строк:"<<endl;
	for(int i=0; i<NumRows; i++)
	{
		cout<<"Row "<<i+1<<endl;
		cout<<"\t coordinate: "<<rows[i].coord_y<<endl;
		cout<<"\t height: "<<rows[i].h<<endl;
		cout<<"\t sitespacing: "<<rows[i].site_sp<<endl;
		cout<<"\t subrowOrigin: "<<rows[i].coord_x<<endl;
		cout<<"\t numsites: "<<rows[i].num_sites<<endl;
	}
	cout<<endl;
}

void DataPlace::print_pl()
{
	cout<<"Список координат нодов:"<<endl;

	for(int i=0; i<NumNodes-NumTerminals; i++)
	{
		cout<<nodes[i].name<<" : "<<nodes[i].pos_x<<"\t"<<nodes[i].pos_y<<"  Row: "<<nodes[i].lRow->coord_y<<endl;
	}
	cout<<endl;

	//if(!terminals.empty())
	//	for(int i=0; i<NumTerminals; i++)
	//	{
	//		cout<<terminals[i].name<<" : "<<" pos_x "<<terminals[i].pos_x<<" pos_y "<<terminals[i].pos_y<<endl;
	//		cout<<terminals[i].name<<" : "<<" w "<<terminals[i].w<<" h "<<terminals[i].h<<endl;
	//	}
	//	cout<<endl;
}

bool DataPlace::checkPointInRow(const Point& point)
{
	node nd;
	nd.pos_x = point.x;
	if(point.lRow->ls.empty())
	{
		cout<<"VECTOR EMPTY!!!!"<<endl;
		return false;
	}

	return binary_search(point.lRow->ls.begin(), point.lRow->ls.end(), &nd,
		[] (node* const& comp1, node* const& comp2) 
		{
			return (comp1->pos_x < comp2->pos_x);
		});
}

void DataPlace::findCoreRow(Point& point)
{
	vector<row>::iterator fIt = lower_bound(rows.begin(), rows.end(), point.y,
		[] (row const& comp, double const& rY) { return comp.coord_y < rY; });
	if(fIt != rows.end() )
	{
		if(fIt->coord_y != point.y)
		{
			cerr<<"ERROR Row!!!!"<<endl;
			exit(1);
		}
		point.lRow = &(*fIt);
	}
	else
	{
		cerr<<"function findCoreRow. ERROR!"<<endl;
		exit(1);
	}
}

size_t DataPlace::findCellIdx(Point& point)
{
	findCoreRow(point);

	//for(vector<node*>::const_iterator itN = l.begin(); itN != l.end(); itN++) //better
	//{
	//	if(point.x == (*itN)->pos_x)
	//		return (*itN)->i;
	//	else if(point.x < (*itN)->pos_x)
	//		break;
	//}

	node nd;
	nd.pos_x = point.x;
	vector<node*>& l = point.lRow->ls;

	pair<vector<node*>::iterator, vector<node*>::iterator> fIt = equal_range(l.begin(), l.end(), &nd,
		[] (node* const& comp1, node* const& comp2) 
		{ 
			return (comp1->pos_x < comp2->pos_x);
		});

	if(fIt.first != fIt.second)
	{
		return (*fIt.first)->i;
	}

	return UINT_MAX;
}

bool DataPlace::findClosestWS(Point& loc, Point& WSLoc, double& width)
{
	if(loc.lRow == NULL)
		findCoreRow(loc);

	vector<node*>::iterator itn, itntemp, itnleft, itnright;

	bool leftRight = false; //false means move left else move right
	double WS = 0;
	double xDiff = 0;

	if(loc.lRow->ls.empty()) //no cells in row
		return(true);

	for(itn = loc.lRow->ls.begin(); itn != loc.lRow->ls.end()-1; ++itn)
	{
		xDiff = 1;
		itntemp = itn++;
		if((*itn)->pos_x <= loc.x && (*itntemp)->pos_x >= loc.x)
			break;
	}

	if(itn != loc.lRow->ls.end()-1)
		xDiff = (*itntemp)->pos_x - ((*itn)->pos_x + (*itn)->w);
	else
		xDiff = (loc.lRow->coord_x + loc.lRow->num_sites * loc.lRow->site_sp) - 
				((*itn)->pos_x + (*itn)->w);
	if(xDiff >= width)
	{
		WSLoc.x = (*itn)->pos_x + (*itn)->w;
		return(true);
	}

	//closest node found, now try to find WS

	itnleft = itnright = itn;

	while( itnleft != (loc.lRow->ls.begin()) || itnright != (loc.lRow->ls.end()-1) )
	{
		//choose the direction of search
		if(leftRight == false && itnright != loc.lRow->ls.end()-1)
			leftRight = true;
		else if(leftRight == true && itnleft != loc.lRow->ls.begin())
			leftRight = false;

		if(leftRight == true) //search right fow WS
		{
			itntemp = itnright + 1;
			WS = (*itntemp)->pos_x - (*itnright)->pos_x - (*itnright)->w;
			if(WS >= width) //WS found
			{
				WSLoc.x = (*itnright)->pos_x + (*itnright)->w;
				return(true);
			}
			++itnright;
		}
		else //search left for WS
		{
			itntemp = itnleft - 1;
			WS = (*itnleft)->pos_x - (*itntemp)->pos_x - (*itntemp)->w;
			if(WS >= width)
			{
				WSLoc.x = (*itnleft)->pos_x - width;
				return(true);
			}
			--itnleft;
		}
	}
	return(false);
}

Point DataPlace::calcMeanLoc(size_t& cellId)
{
	size_t pinCount = 0;
	Point meanLoc;

	vector<size_t>& edge = nodes[cellId].nets_of_nodes;
	for(vector<size_t>::const_iterator itN = edge.begin(); itN != edge.end(); itN++)
	{
		for(vector<element*>::const_iterator itE = nets[*itN].ls.begin(); itE != nets[*itN].ls.end(); itE++)
		{
				if(!(*itE)->is_terminal() && (*itE)->i == cellId)
					continue;
				meanLoc += Point((*itE)->pos_x, (*itE)->pos_y, NULL);
				pinCount++;
		}
	}
	if(pinCount == 0)
	{
		meanLoc = nodes[cellId];
	}
	else
	{
		meanLoc.x /= pinCount;
		meanLoc.y /= pinCount;
	}
	return meanLoc;
}

double DataPlace::findLimitRow() { return ( LengthNodes / NumRows ); } 

double DataPlace::evalHPWL()
{
	vector<element* >::iterator elMinY, elMaxY;
	vector<element* >::iterator elMinX, elMaxX;

	double totalHPWL = 0;

	for(vector<net>::iterator it = nets.begin(); it != nets.end(); it++)
	{
		tie(elMinY, elMaxY) = minmax_element(it->ls.begin(), it->ls.end(),
			[] (element* const& s1, element* const& s2)
			{
				return (s1->pos_y + 0.5*s1->h) < (s2->pos_y + 0.5*s2->h);
			});

		tie(elMinX, elMaxX) = minmax_element(it->ls.begin(), it->ls.end(),
			[] (element* const& s1, element* const& s2)
			{
				return (s1->pos_x + 0.5*s1->w) < (s2->pos_x + 0.5*s2->w);
			});

		totalHPWL += ( fabs( (*elMaxX)->pos_x + 0.5*(*elMaxX)->w - (*elMinX)->pos_x - 0.5*(*elMinX)->w) 
					 + fabs( (*elMaxY)->pos_y + 0.5*(*elMaxY)->h - (*elMinY)->pos_y - 0.5*(*elMinY)->h) );
	}

	return totalHPWL;
}

double DataPlace::calcOverlap(bool det)
{
	double totalOverlap = 0;
	vector<node* >::iterator next;

	if (det)
	{
		for(vector<row>::iterator itR = rows.begin(); itR != rows.end(); itR++)
		{
			for(vector<node* >::iterator it = itR->ls.begin(); it != itR->ls.end(); it++)
			{
				if(it != itR->ls.end()-1)
				{
					next = it + 1;

					while( ( (*it)->pos_x + (*it)->w ) > (*next)->pos_x )
					{
						if ( ( (*it)->pos_x + (*it)->w ) > ( (*next)->pos_x + (*next)->w ) )
							totalOverlap += (*next)->w * heightSC;
						else
							totalOverlap += fabs( ( (*it)->pos_x + (*it)->w ) - (*next)->pos_x) * heightSC;
						next++;
						if( next == itR->ls.end() ) break;
					}
				}
			}
		}
	}

	return totalOverlap;
}

double DataPlace::calcInstHPWL(vector<size_t>& movables)
{
	unordered_set<size_t> seenNets;
	if(movables.size() == 2)
		seenNets.insert(nodes[movables.front()].nets_of_nodes.begin(), nodes[movables.front()].nets_of_nodes.end());

	double totalHPWL = 0;

	vector<element* >::iterator elMinY, elMaxY;
	vector<element* >::iterator elMinX, elMaxX;

	for(vector<size_t>::iterator it = movables.begin(); it != movables.end(); it++ )
	{
		for(vector<size_t>::iterator itN = nodes[*it].nets_of_nodes.begin(); itN != nodes[*it].nets_of_nodes.end(); itN++)
		{
			if(movables.size() == 2 && it == movables.end()-1)
				if(seenNets.find(*itN) != seenNets.end())
					continue;
			tie(elMinY, elMaxY) = minmax_element(nets[*itN].ls.begin(), nets[*itN].ls.end(),
				[] (element* const& s1, element* const& s2)
				{
					return (s1->pos_y + 0.5*s1->h) < (s2->pos_y + 0.5*s2->h);
				});

			tie(elMinX, elMaxX) = minmax_element(nets[*itN].ls.begin(), nets[*itN].ls.end(),
				[] (element* const& s1, element* const& s2)
				{
					return (s1->pos_x + 0.5*s1->w) < (s2->pos_x + 0.5*s2->w);
				});

			totalHPWL += ( fabs( (*elMaxX)->pos_x + 0.5*(*elMaxX)->w - (*elMinX)->pos_x - 0.5*(*elMinX)->w) 
						 + fabs( (*elMaxY)->pos_y + 0.5*(*elMaxY)->h - (*elMinY)->pos_y - 0.5*(*elMinY)->h) );	
		}
	}

	return totalHPWL;
}

double DataPlace::calcInstOverlap(vector<size_t>& movables)
{
	double totalOverlap = 0;
	double fall = 0;

	//if(movables.size() == 2)
	//{
	//	node& N1 = nodes[movables[0]];
	//	node& N2 = nodes[movables[1]];
	//	if(N1.lRow == N2.lRow && (N1.pos_x + N1.w > N2.pos_x || N2.pos_x + N2.w > N1.pos_x) )
	//	{
	//		vector<node* >::iterator next;
	//		for(vector<node* >::iterator it = N1.lRow->ls.begin(); it != N1.lRow->ls.end(); it++)
	//		{
	//			if(it != N1.lRow->ls.end()-1)
	//			{
	//				next = it + 1;

	//				while( ( (*it)->pos_x + (*it)->w ) > (*next)->pos_x )
	//				{
	//					if ( ( (*it)->pos_x + (*it)->w ) > ( (*next)->pos_x + (*next)->w ) )
	//						totalOverlap += (*next)->w * heightSC;
	//					else
	//						totalOverlap += fabs( ( (*it)->pos_x + (*it)->w ) - (*next)->pos_x) * heightSC;
	//					next++;
	//					if( next == N1.lRow->ls.end() ) break;
	//				}
	//			}
	//		}
	//		return totalOverlap;
	//	}
	//}

	if(movables.size() == 2)
	{
		node& N1 = nodes[movables[0]];
		node& N2 = nodes[movables[1]];
		if(N1.lRow == N2.lRow && (N1.pos_x + N1.w > N2.pos_x || N2.pos_x + N2.w > N1.pos_x) )
		{
			if ((N1.pos_x >= N2.pos_x) && (N1.pos_x + N1.w <= N2.pos_x + N2.w))
				fall += N1.w * heightSC;
			else if ((N1.pos_x <= N2.pos_x) && (N1.pos_x + N1.w >= N2.pos_x + N2.w))
				fall += N2.w * heightSC;
			else if ((N1.pos_x > N2.pos_x) && (N1.pos_x < N2.pos_x + N2.w))
				fall += fabs(N2.pos_x + N2.w - N1.pos_x) * heightSC;
			else if ((N1.pos_x + N1.w > N2.pos_x) && (N1.pos_x + N1.w < N2.pos_x + N2.w))
				fall += fabs(N1.pos_x + N1.w - N2.pos_x) * heightSC;
		}
	}

	for(vector<size_t>::iterator itM = movables.begin(); itM != movables.end(); itM++)
	{
		row& Row = *nodes[*itM].lRow;
		double mLeftEdge = nodes[*itM].pos_x;
		double mRightEdge = nodes[*itM].pos_x + nodes[*itM].w;
		for(vector<node* >::iterator it = Row.ls.begin(); it !=Row.ls.end(); it++)
		{
			double everyLeftEdge = (*it)->pos_x;
			double everyRightEdge = (*it)->pos_x + (*it)->w;

			if (everyLeftEdge > mRightEdge)
				break;
			if ( &nodes[*itM] == &(**it) )
				continue;

			if ((mLeftEdge >= everyLeftEdge) && (mRightEdge <= everyRightEdge))
				totalOverlap += nodes[*itM].w * heightSC;
			else if ((mLeftEdge <= everyLeftEdge) && (mRightEdge >= everyRightEdge))
				totalOverlap += (*it)->w * heightSC;
			else if ((mLeftEdge > everyLeftEdge) && (mLeftEdge < everyRightEdge))
				totalOverlap += fabs(everyRightEdge - mLeftEdge) * heightSC;
			else if ((mRightEdge > everyLeftEdge) && (mRightEdge < everyRightEdge))
				totalOverlap += fabs(mRightEdge - everyLeftEdge) * heightSC;
		}
	}
	return totalOverlap - fall;
}

double DataPlace::evalPRow()
{
	double totalPRow = 0;
	double limitRow = findLimitRow();

	for(vector<row>::iterator it = rows.begin(); it != rows.end(); it++)
	{
		totalPRow += fabs(it->busySRow - limitRow);
	}
	return totalPRow;
}

void DataPlace::checkPRow()
{
	for(vector<row>::iterator it = rows.begin(); it != rows.end(); it++)
	{
		cout<<it->busySRow<<endl;
		if(it->busySRow > it->num_sites*it->site_sp)
			cout<<it->coord_y<<endl;
	}
}

void DataPlace::updateCells(const vector<size_t>& movables, const vector<Point>& soln, double& prow)
{
	for(int i=0; i<movables.size(); i++)
	{
		setLocation(movables[i], soln[i], prow);
	}
}

void DataPlace::setLocation(const size_t id, const Point& pt, double& prow)
{
	vector<node* >& eraseCell = nodes[id].lRow->ls;

	vector<node*>::iterator fIt = upper_bound(eraseCell.begin(), eraseCell.end(), nodes[id].pos_x,
		[] (double const& rX, node* const& comp) { return rX < comp->pos_x; });
	if((*(fIt-1))->name == nodes[id].name)
	{
		prow -= fabs(nodes[id].lRow->busySRow - findLimitRow());
		nodes[id].lRow->busySRow -= nodes[id].w;
		prow += fabs(nodes[id].lRow->busySRow - findLimitRow());
		eraseCell.erase(fIt-1);
	}
	else
	{
		cerr<<"function setLocation. ERROR - erase element!"<<endl;
		exit(1);
	}

	nodes[id].pos_x = pt.x;
	nodes[id].pos_y = pt.y;
	nodes[id].lRow = pt.lRow;

	if(pt.lRow->ls.empty()) cout<<"ERROR/ VECTOR EMPTY"<<endl;

	vector<node* >& insertCell = nodes[id].lRow->ls;

	if(insertCell.empty() || (*(insertCell.end()-1))->pos_x < pt.x)
	{
		prow -= fabs(nodes[id].lRow->busySRow - findLimitRow());
		nodes[id].lRow->busySRow += nodes[id].w;
		prow += fabs(nodes[id].lRow->busySRow - findLimitRow());
		insertCell.insert(insertCell.end(),&nodes[id]);
	}
	else
	{
		vector<node*>::iterator fIt = lower_bound(insertCell.begin(), insertCell.end(), pt.x,
			[] (node* const& comp, double const& rX) { return comp->pos_x < rX; });
		if(pt.x <= (*fIt)->pos_x)
		{
			prow -= fabs(nodes[id].lRow->busySRow - findLimitRow());
			nodes[id].lRow->busySRow += nodes[id].w;
			prow += fabs(nodes[id].lRow->busySRow - findLimitRow());
			insertCell.insert(fIt,&nodes[id]);
		}
		else
		{
			cerr<<"function setLocation. ERROR - insert element"<<endl;
			exit(1);
		}
	}
}

void DataPlace::remOverlaps()
{
	double offset;
	for(vector<row>::iterator itR = rows.begin(); itR != rows.end(); itR++)
	{
		offset = itR->coord_x + (itR->num_sites*itR->site_sp - itR->busySRow)/2;
		for(vector<node*>::iterator it = itR->ls.begin(); it != itR->ls.end(); it++)
		{
			(*it)->pos_x = offset;
			offset += (*it)->w;
		}
	}
}

double DataPlace::RandomDouble(double min, double max)
{
	return (double)(rand())/RAND_MAX*(max - min) + min;
}

unsigned int DataPlace::RandomUnsigned(unsigned int min, unsigned int max)
{
	return (rand() % (max - min)) + min;
}

void DataPlace::savePlacement(const char* plFileName) const
{
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	ofstream out(plFileName);

	out<<"MIET pl 1.0"<<endl;
	out<<"# Created\t: "<<asctime(timeinfo);
	out<<"# User\t: Loskutov V"<<endl;
	out<<"# Platform\t: Windows 7\n\n"<<endl;

	for(vector<node>::const_iterator it = nodes.begin(); it != nodes.end(); it++)
	{
		out<<setw(8)<<it->name
			<<"  "<<setw(10)<<it->pos_x
			<<"  "<<setw(10)<<it->pos_y<<" : N";
		out<<endl;
	}

	for(vector<terminal>::const_iterator it = terminals.begin(); it != terminals.end(); it++)
	{
		out<<setw(8)<<it->name
			<<"  "<<setw(10)<<it->pos_x
			<<"  "<<setw(10)<<it->pos_y<<" : N";
		out<<endl;
	}

	out.close();
}