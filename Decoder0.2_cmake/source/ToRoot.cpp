//============================================================
// Decorder for NIuTPC Logger
//
// Input data format is 
//   HEADER(16byte) + ADCDATA(4000*2*64) + TIMESTAMP(4byte)
//------------------------------------------------------------
// Update : 29. July 2016
// Author : T.Ikeda
//============================================================


#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

//ROOT
#include "TROOT.h"
#include "TStyle.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TFile.h"
// USER
#include "Event.h"

// parameters
#define HEADER_SIZE 16 //byte
#define N_CHANNEL 64
#define N_SAMPLE 4000  // always 4000
#define SAMPLING_HELZ 2.5e6 //default

using namespace std;



int main(int argc,char *argv[]){
  std::cerr << "=========================================" << std::endl;
  std::cerr << " Create ROOT file for NIuTPC output data " << std::endl;
  std::cerr << " Versioin 0.1 " << std::endl;
  std::cerr << " Using : ./main [filename] " << std::endl;
  std::cerr << "=========================================" << std::endl;

  if(argc != 2) {
    std::cout << "<ERROR>    Specify input file name as first argument." 
	      << std::endl;
    return 1;
  }

  string filename=argv[1];
  std::string::size_type index = filename.find(".dat");
  if( index == std::string::npos ) { 
    std::cout << "Failure!!!" << std::endl;
    return 1;
  }

  // ------- get file size ------//
  FILE *fp;
  long file_size;
  char *buffer;
  struct stat stbuf;
  int fd;
  fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) {
    cerr << "ERROR: Cannot get file size" << endl;
    return -1;
  }
  fp = fdopen(fd, "rb");
  if (fp == NULL) {
    cerr << "ERROR: Cannot get file size" << endl;
    return -1;
  }
  if (fstat(fd, &stbuf) == -1) {
    cerr << "ERROR: Cannot get file size" << endl;
    return -1;
  }
  file_size = stbuf.st_size;

  cerr << "Data size: " << file_size << "byte" << endl;

  
  //----------- Creat root file --------//
  std::string outfilename = filename.substr(0, index) + ".root";
  TFile *f=new TFile(outfilename.c_str(),"recreate");
  // ---------- Create Tree ------------//
  TTree* tree= new TTree("Tree","Event Tree");

  // ---------- Create Event Class -----//
  Event *event = new Event();
  tree->Branch("Event", "Event", &event);
  
  //open file
  ifstream fin( filename.c_str(), ios::in | ios::binary );
  if (!fin){
    cerr << "ERROR: Cannot open file" << endl;
    return 1;
  }

  //parameters
  int event_id=0; // add event id infomation
  int current_size=0; 

  //------- Start Read -------//
  while(!fin.eof()){
    if(current_size%100==0) std::cout << "\rFill Data into Tree ... : " 
				      << current_size << "/" << file_size << std::flush;
  // header read
    unsigned char header[HEADER_SIZE];
    fin.read( ( char * ) &header[0], HEADER_SIZE*sizeof( char ) );
    unsigned int* int_p = (unsigned int *)&header[8];
    int length = ntohl(*int_p);// unit?
    int module_num =header[7];
    int_p = (unsigned int *)&header[12];
    int trigger = ntohl(*int_p);

    int data_set = length/2/N_CHANNEL;
    if(data_set > N_SAMPLE){
      cerr << "ERROR: Too large # of sample" << endl;
      return 1;
     }

    //data read
    char *data_s;
    data_s=new char[length];
    fin.read( ( char * ) &data_s[0], length*sizeof(char ) );

    // time stamp read
    int timestamp;
    fin.read( ( char * ) &timestamp, sizeof(int ) );
    //    cerr << "time= " << ntohl(timestamp) << endl;

    
    //    double adc_data[N_CHANNEL][N_SAMPLE];
        vector< vector<double> > adc_data(N_CHANNEL,vector<double>(N_SAMPLE,0));
    for(int i=0;i<data_set;i++){
      for(int ch = 0;ch < N_CHANNEL;ch++){
	int pos = 2*N_CHANNEL*i + 2*ch;
	unsigned short* short_p = (unsigned short *)&data_s[pos];
	adc_data[ch][i] = ntohs(*short_p);
      }
    }  


    //----- Fill -----/
    event = new Event();
    event->SetHeaders(module_num,trigger,timestamp,N_SAMPLE,SAMPLING_HELZ);
    event->SetADC(adc_data);
    event->SetEventID(event_id);
    tree->Fill(); delete event;


    current_size+=HEADER_SIZE + length + 4;
    event_id++;
  }


  tree->Write();
  f->Close(); delete f;



  cerr << endl;



  fin.close();
  
  return 0;


}
