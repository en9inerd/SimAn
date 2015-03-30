//DataPlace.cpp

#include "stdafx.h"
#include "DataPlace.h"
#include <fstream>
#include <sstream>
#include <tuple>
using namespace std;

DataPlace::DataPlace() : LengthNodes(0), heightSC(0)
{
	//const char* path_aux = "ibm_test.aux";
	//parseAuxFile(path_aux);
	//parser();
	srand((unsigned int)time(NULL));
}

//DataPlace::DataPlace(const char* p_aux)
//{
//	parseAuxFile(p_aux);
//	parser();
//}

void DataPlace::Start(const char* p_aux)
{
	parseAuxFile(p_aux);
	parser();
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

						for(int j=0; j<(NumNodes-NumTerminals); j++)
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

						//nt.ls_sort_x = nt.ls;
						//nt.ls_sort_y = nt.ls;
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
	//char temp[50];
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

			for(int j=0; j<(NumNodes-NumTerminals); j++)
				if(nodes[j].name == words[0])
				{
					finded = true;
					//nodes[j].pos_x = stoi(words[1]);
					//nodes[j].pos_y = stoi(words[2]);
					break;
				}
				//cout<<words[0]<<endl;
			if(!finded)
			{
				for(int j=0; j<NumTerminals; j++)
					if(terminals[j].name == words[0])
					{
						terminals[j].pos_x = stoi(words[1]);
						terminals[j].pos_y = stoi(words[2]);
						//cout<<words[0]<<"\t"<<words[1]<<"\t"<<words[2]<<endl;
						break;
					}
			}
		}
	}
		
	//while(f_pl)
	//{
	//	while( f_pl.peek() != ' ' || f_pl.peek() == '#' )
	//		f_pl.ignore(200, '\n');
	//	f_pl>>temp;
	//	for(int i=0; i<NumNodes; i++)
	//	{
	//		for(int j=0; j<NumNodes; j++)
	//			if(nodes[j].name == temp)
	//			{
	//				if(nodes[j].move == 0)
	//				{
	//					f_pl>>nodes[j].pos_x;
	//					f_pl>>nodes[j].pos_y;
	//					f_pl.ignore(100, '\n');
	//				}
	//				else
	//				{
	//					f_pl.ignore(100, '\n');
	//				}
	//				break;
	//			}
	//		f_pl>>temp;
	//	}
	//}
	f_pl.close();
}

void DataPlace::parser()
{
	cout<<"Parser..."<<endl;
	cout<<" [ ";
	parseNodes();
	parseNets();
	parseScl();
	parsePl();
	cout<<" ] "<<endl;
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
							totalOverlap += (*next)->w;
						else
							totalOverlap += fabs( ( (*it)->pos_x + (*it)->w ) - (*next)->pos_x);
						next++;
						if( next == itR->ls.end() ) break;
					}
				}
			}
		}
	}

	return totalOverlap;
}

double DataPlace::calcInstHPWL(vector<size_t>& movables) // вектор movables содержит номера ячеек (1-2) выбранных для перемещения
{
	double totalHPWL = 0;

	vector<element* >::iterator elMinY, elMaxY;
	vector<element* >::iterator elMinX, elMaxX;

	for(vector<size_t>::iterator it = movables.begin(); it != movables.end(); it++ )
		for(vector<size_t>::iterator itN = nodes[*it].nets_of_nodes.begin(); itN != nodes[*it].nets_of_nodes.end(); itN++)
		{
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

	return totalHPWL;
}

double DataPlace::calcInstOverlap(vector<size_t>& movables)
{
	double totalOverlap = 0;
	//vector<size_t>::iterator iterBegin = movables.begin();
	//vector<size_t>::iterator iterEnd = movables.end();

	//if(movables.size() == 2)
	//{
	//	node& N1 = nodes[movables[0]];
	//	node& N2 = nodes[movables[1]];
	//	if(N1.lRow == N2.lRow)
	//	{
	//		if(N1.pos_x + N1.w > N2.pos_x && N1.pos_x < N2.pos_x)
	//			iterEnd -= 1;
	//		else if(N2.pos_x + N2.w > N1.pos_x && N2.pos_x < N1.pos_x)
	//			iterBegin += 1;
	//	}
	//}

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
				totalOverlap += nodes[*itM].w;
			else if ((mLeftEdge <= everyLeftEdge) && (mRightEdge >= everyRightEdge))
				totalOverlap += (*it)->w;
			else if ((mLeftEdge > everyLeftEdge) && (mLeftEdge < everyRightEdge))
				totalOverlap += fabs(everyRightEdge - mLeftEdge);
			else if ((mRightEdge > everyLeftEdge) && (mRightEdge < everyRightEdge))
				totalOverlap += fabs(mRightEdge - everyLeftEdge);
		}
		//totalOverlap *= Row.h;
	}
	return totalOverlap;
}

void DataPlace::updateCells(const vector<size_t>& movables, const vector<Point>& soln)
{
	for(int i=0; i<movables.size(); i++)
	{
		setLocation(movables[i], soln[i]);
	}
}

void DataPlace::setLocation(size_t id, const Point& pt)
{
	vector<node* >& R = nodes[id].lRow->ls;
	for(vector<node* >::iterator it = R.begin(); it != R.end(); it++)
	{
		if (nodes[id].name == (*it)->name)
		{
			R.erase(it);
			break;
		}
	}

	nodes[id].pos_x = pt.x;
	nodes[id].pos_y = pt.y;
	nodes[id].lRow = pt.lRow;

	for(vector<node* >::iterator it = pt.lRow->ls.begin(); it != pt.lRow->ls.end(); it++)
	{
		if(pt.x <= (*it)->pos_x)
		{
			nodes[id].lRow->ls.insert(it,&nodes[id]);
			break;
		}
		else if ( (it == nodes[id].lRow->ls.end()-1) && (*it)->pos_x <= pt.x )
		{
			nodes[id].lRow->ls.insert(nodes[id].lRow->ls.end(),&nodes[id]);
			break;
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