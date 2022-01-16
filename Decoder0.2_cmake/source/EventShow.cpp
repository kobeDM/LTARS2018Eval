//------------------------------------------------------------------
// Read tree for NIuTPc output data
// Version 0.1
// Update: 29. July 2016
// Author: T.Ikeda
//------------------------------------------------------------------

// STL
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
//
#include <time.h>
// ROOT
#include "TROOT.h"
#include "TStyle.h"
#include "TTree.h"
#include "TH1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TSystem.h"
#include "TClassTable.h"
#include "TApplication.h"
#include "TGraph.h"
#include "TLegend.h"
//USER
#include "Event.h"

#define N_CHANNEL 64
#define N_STRIP 32
#define SAMPLING_HELZ 2.5e6 //Hz

#define DEBUG 0
using namespace std;

//------------- main -------------------------------------------------
int main(int argc,char *argv[]){
	clock_t t1,t2;

	if(argc <4){
		std::cerr << "Usage:" << std::endl;
		std::cerr << "/.main [filename.root] [config.json] [event number]" << std::endl;
		return 1;
	}

	string filename=argv[1];
	std::string::size_type index = filename.find(".root");
	if( index == std::string::npos ) { 
		std::cout << "Failure!!!" << std::endl;
		return 1;
	}

	//json database
	string conffilename=argv[2];
	std::string::size_type index_conf = conffilename.find(".json");
	if( index_conf == std::string::npos ) { 
		std::cout << "Failure!!!" << std::endl;
		return 1;
	}
	boost::property_tree::ptree pt;
	read_json(conffilename,pt);
	int offset_samp = pt.get<int>("config.offset_sampling");

	int ev_num = atoi(argv[3]);

	t1=clock();
	std::cerr << "======================================" << std::endl;
	std::cerr << "Read ROOT file" << std::endl;
	std::cerr << "Visualizer for 0.1c NIuTPC" << std::endl;
	std::cerr << "Version 0.1" << std::endl;
	std::cerr << "======================================" << std::endl;
	std::cerr << "file name: " << filename << endl;
	std::cerr << "config file name: " << conffilename << endl;
	std::cerr << "======================================" << std::endl;
	std::cerr << "offset sampling: " << offset_samp << endl;

	TApplication app("app",&argc,argv);  

	if(!TClassTable::GetDict("Event.so")){
		gSystem->Load("/home/shimada/NEWAGE/LTARS/Decoder0.2_cmake/bin/libEvent.so");
	}

	TFile *f=new TFile(filename.c_str());
	if(!f){
		std::cerr << "ERROR: Cant find file" << std::endl;
		return 1;
	}

	TTree *tree=(TTree*)f->Get("Tree");

	Event *event = new Event();
	int nevent=tree->GetEntries();
	std::cerr << "event number is " << nevent << std::endl;

	TBranch *branch = tree->GetBranch("Event");
	branch->SetAddress(&event);

	tree->GetEntry(ev_num);

	int event_id               = event->GetEventID();
	int sampling_num           = event->GetSamplingNum();
	vector<vector<double>> adc = event->GetADC();
	

	//---------------Fill--------------------
	//Even CH -> High Gain
	//Odd CH  -> Low  Gain
	TGraph* g_hg[N_STRIP];
	TGraph* g_lg[N_STRIP];
	for(int j=0;j<N_STRIP;j++){
		g_hg[j] = new TGraph();
		g_hg[j]->SetName(Form("HGch%d",j));
		g_lg[j] = new TGraph();
		g_lg[j]->SetName(Form("LGch%d",j));
		//g_hg[j]->SetMarkerStyle(8);
		//g_lg[j]->SetMarkerStyle(8);
	}
	int strip_num = 0;
	for(int j=0;j<N_CHANNEL;j++){
		//printf("debug%d\n",j);
		for(int k=0;k<sampling_num;k++){
			if(j%2==0) g_hg[strip_num]->SetPoint(k, k/SAMPLING_HELZ*1e6, adc.at(j).at(k));
			if(j%2==1) g_lg[strip_num]->SetPoint(k, k/SAMPLING_HELZ*1e6, adc.at(j).at(k));
		}
		if(j%2==1)strip_num++;
	}
	/*

	TH2F *h_strip=new TH2F("h_strip","h_strip",N_CHANNEL,0,N_CHANNEL,sampling_num,0,sampling_num);
	h_strip->SetStats(0);
	for(int j=0;j<N_CHANNEL;j++){
		for(int k=0;k<sampling_num;k++){
			h_strip->SetBinContent(j+1,k+1,adc[j][k]);
		}
	}
	*/

	//---------------Draw--------------------
	//TCanvas *c_strip=new TCanvas("c_strip","",800,800);
	//h_strip->Draw("colz");
	TCanvas *c_wave=new TCanvas("c_wave","",1000,1000);
	c_wave->Divide(1,2);
	c_wave->cd(1)->DrawFrame(0,0,1600,4096,"HG waveform;us(4000sampling);ADC(-1~1V/12bit)");
	c_wave->cd(2)->DrawFrame(0,0,1600,4096,"LG waveform;us(4000sampling);ADC(-1~1V/12bit)");
	for(int st=0;st<N_STRIP;st++){
		c_wave->cd(1);g_hg[st]->Draw("pl same");
		c_wave->cd(2);g_lg[st]->Draw("pl same");
	}
	
	//---------------Write--------------------
	stringstream s_eventID; s_eventID << event_id;
	std::string outfilename = filename.substr(0,index) + "_event" + s_eventID.str() + ".root";
	TFile* outfile = new TFile(outfilename.c_str(),"recreate");

	for(int st=0;st<N_STRIP;st++){
		g_hg[st]->Write();
		g_lg[st]->Write();
	}

	outfile->Close();


	//----------- End of output ------------//
	std::cerr << std::endl;
	std::cerr << "======================================" << std::endl;
	t2=clock();
	std::cerr << "execution time: " << (t2-t1)/CLOCKS_PER_SEC << std::endl;
	
	app.Run();

	return 1;
}

