// simulated annealing.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include <iostream>
#include <locale>
#include "SimAnneal.h"
#include "DataPlace.h"
#include "paramproc.h"
#include "visualization.h"

using std::cerr;
using std::cout;
using std::ostream;
using std::istream;
using std::endl;

DataPlace rbplace;

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "Russian");
	//NoParams	noParams		(argc,argv);
	//BoolParam	helpRequest		("help",argc,argv);
	//BoolParam	helpRequest1	("h",argc,argv);
	//StringParam	auxFileName		("f",argc,argv);
	//BoolParam	save			("save",argc,argv);
	//BoolParam	keepOverlaps	("skipLegal",argc,argv);
	//BoolParam	greedy			("greedy",argc,argv);
	//BoolParam	viewer			("viewer",argc,argv);

	//if (noParams.found() || helpRequest.found() || helpRequest1.found())
	//{
	//	cout << "  Use '-help' or '-f filename.aux' " << endl;
	//	cout <<"Options\n"<<"-save \n"<<"-greedy (no annealing, just greedy moves)\n\n ";
	//	exit(0);
	//}

	const char* aux = "ibm_test.aux";
	rbplace.Start(aux);
	//DataPlace rbplace;

	//if(!keepOverlaps.found())
	//	rbplace.remOverlaps(); //легализация

	double initHPWL = rbplace.evalHPWL();

	cout << " ====== Launching Global Placement ... " << endl;
	cout <<setprecision(10)<<"Initial Center-to-Center WL: "<<initHPWL<<endl;

	//Timer SATimer;  
	SimAnneal SA(rbplace);
	//rbplace.remOverlaps();
	//SATimer.stop();

	//double HPWLafter = rbplace.evalHPWL();

	//cout << " Final Center-to-Center WL: "<<HPWLafter<<endl;

	//cout<<"  % improvement in HPWL is "<<(initHPWL - HPWLafter)*100/HPWLafter<<endl;
	//cout<<"Time taken = "<<SATimer.getUserTime()<<" seconds\n";

	bool viewer = true;

	if(viewer)
	{
		cout<<"View visualization .pl"<<endl;
		opengl_control(argc,argv);
	}

	//if(save.found())
	//{
	//	cout << "Saving out.pl" << endl;
	//	//DataPlace.savePlacement("out.pl");
 //   }

	return 0;
}