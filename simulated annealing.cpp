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
	NoParams	noParams		(argc,argv);
	BoolParam	helpRequest		("help",argc,argv);
	BoolParam	helpRequest1	("h",argc,argv);
	StringParam	auxFileName		("f",argc,argv);
	StringParam	outFileName		("save",argc,argv);
	BoolParam	keepOverlaps	("skipLegal",argc,argv);
	//BoolParam	detailed		("detailed",argc,argv);
	BoolParam	greedy			("greedy",argc,argv);
	//BoolParam	viewer			("viewer",argc,argv);

	//if (noParams.found() || helpRequest.found() || helpRequest1.found())
	//{
	//	cout<<"  Use '-help' or '-f filename.aux' "<<endl;
	//	cout<<"Options\n"
	//		<<"-save filename.pl\n"
	//		<<"-detailed (detailed placement, default global)\n"
	//		<<"-greedy (no annealing, just greedy moves)\n\n ";
	//	exit(0);
	//}

	const char* aux = "ibm_test.aux";
	rbplace.Start(aux);

	//rbplace.Start(auxFileName);

	//if(!keepOverlaps.found())
	//	rbplace.remOverlaps();

	bool detailed = false;
	double initHPWL = 0;

	if(detailed)
	{
		initHPWL = rbplace.evalHPWL();
		cout<<" ====== Launching Detailed Placement ... "<<endl;
	}
	else
	{
		cout<<" ====== Launching Global Placement ... "<<endl;
	}
	cout<<setprecision(10)<<" Initial Center-to-Center WL: "<<initHPWL<<endl;

	const clock_t start = clock();
	SimAnneal SA(rbplace, false, false); // (rbplace, greedy, detailed)
	//rbplace.remOverlaps();
	const double GlobalTime = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;

	double HPWLafter = rbplace.evalHPWL();

	cout<<" Final Center-to-Center WL: "<<HPWLafter<<endl;
	cout<<"  % improvement in HPWL is "<<(initHPWL - HPWLafter)*100/HPWLafter<<endl;
	cout<<"Time taken = "<<GlobalTime<<" seconds\n";

	bool viewer = true;

	if(true)
	{
		cout<<"ibm_SA_out.pl"<<endl;
		rbplace.savePlacement("ibm_SA_out.pl");
	}

	if(viewer)
	{
		cout<<"\n\t -<View visualization .pl>-"<<endl;
		opengl_control(argc,argv);
	}

	return 0;
}