#include "DCHistogramHandler.hh"
//#include <TROOT.h>
#include <TStyle.h>
#include <iostream>
#include <sstream>
#include "DCUtils.hh"
#include "TSystem.h"
#include <iomanip>
#include <new>


#include "TBufferXML.h"
DCHistogramHandler* DCHistogramHandler::_me = 0 ;


DCHistogramHandler* DCHistogramHandler::instance() {
  
  if( _me == 0 )
    {
    _me =new DCHistogramHandler ;
    }
  return _me ;
}  


DCHistogramHandler::DCHistogramHandler()
{
  theFile_=0;//new TFile("/dev/shm/DCHistograms.root","RECREATE");
  TH1::AddDirectory(kFALSE);
  mapH1.clear();
  mapH2.clear();
  //top = new AbsTreeNode("Top");
  sem_init(&theMutex_,0,1);
  _me=this;
 }

void DCHistogramHandler::Lock()
{
  sem_wait(&theMutex_);
}
void DCHistogramHandler::UnLock()
{
  sem_post(&theMutex_);
}
TH1* DCHistogramHandler::GetTH1(std::string name) 
{ 
  std::map<std::string,TH1*>::iterator ih=mapH1.find(name);
  
if (ih!=mapH1.end()) 
  { 
    //printf("Histo %s found %x \n",ih->first.c_str(),ih->second);
    return ih->second; 
  }
  else 
    return NULL;
} 


  //! Return a pointer to the TH2
 /** 
      @param name Name of the histogram
      @return a Th2* ptr
   */
TH2* DCHistogramHandler::GetTH2(std::string name) 
{ 
 std::map<std::string,TH2*>::iterator ih=mapH2.find(name);
  
if (ih!=mapH2.end()) 
  { 
    //printf("Histo %s found %x \n",ih->first.c_str(),ih->second);
      return ih->second; 
  }
  else 
    return NULL;
}
TH1* DCHistogramHandler::AccessTH1(std::string name,int nbinx,double xmin,double xmax,std::string sub)
{
  //TH1* p=GetTH1(sub+name);
  auto ih=mapH1.find(sub+name);

  if (ih==mapH1.end())
    return BookTH1(sub+name,nbinx,xmin,xmax);
  else
    return ih->second;
}
TH1* DCHistogramHandler::BookTH1(std::string name,int nbinx,double xmin,double xmax)
{
  //@ Test top->addFullName(name);
  TH1F* h=NULL;
  try 
    {
      h =  new TH1F(name.c_str(),name.c_str(),nbinx,xmin,xmax);
    }
  catch (std::bad_alloc& ba)
    {
      std::cerr << "bad_alloc caught: " << ba.what() << '\n';
      return 0;
    }

  //h->SetDirectory(theFile_);
  std::pair<std::string,TH1*> pr(name,h);
  mapH1.insert(pr);
  //std::cout<<name<<" Booked"<<std::endl;
  return h;
}

TH2* DCHistogramHandler::AccessTH2(std::string name,int nbinx,double xmin,double xmax,int nbiny,double ymin,double ymax,std::string sub)
{
  auto ih=mapH2.find(sub+name);

  if (ih==mapH2.end())
    return BookTH2(sub+name,nbinx,xmin,xmax,nbiny,ymin,ymax);
  else
    return ih->second;

}
TH2* DCHistogramHandler::BookTH2(std::string name,int nbinx,double xmin,double xmax,int nbiny,double ymin,double ymax)
{
  //@ Test top->addFullName(name);
  TH2F* h=NULL;
  try 
    {
      h = new TH2F(name.c_str(),name.c_str(),nbinx,xmin,xmax,nbiny,ymin,ymax);
    }
  catch (std::bad_alloc& ba)
    {
      std::cerr << "bad_alloc caught: " << ba.what() << '\n';
      return 0;
    }
  //h->SetDirectory(theFile_);
  std::pair<std::string,TH2*> pr(name,h);
  mapH2.insert(pr);
  //std::cout<<name<<" Booked"<<std::endl;
  return h;
}
TH3* DCHistogramHandler::AccessTH3(std::string name,int nbinx,double xmin,double xmax,int nbiny,double ymin,double ymax,int nbinz,double zmin,double zmax,std::string sub)
{
  auto ih=mapH3.find(sub+name);

  if (ih==mapH3.end())
    return BookTH3(sub+name,nbinx,xmin,xmax,nbiny,ymin,ymax,nbinz, zmin, zmax);
  else
    return ih->second;

}
TH3* DCHistogramHandler::BookTH3(std::string name,int nbinx,double xmin,double xmax,int nbiny,double ymin,double ymax,int nbinz,double zmin,double zmax)
{
  //@ Test top->addFullName(name);
  TH3C* h =  NULL;
 try 
    {
      h = new TH3C(name.c_str(),name.c_str(),nbinx,xmin,xmax,nbiny,ymin,ymax,nbinz, zmin, zmax);
    }
  catch (std::bad_alloc& ba)
    {
      std::cerr << "bad_alloc caught: " << ba.what() << '\n';
      return 0;
    }
  h->SetDirectory(theFile_);
  std::pair<std::string,TH3*> pr(name,h);
  mapH3.insert(pr);
  //std::cout<<name<<" Booked"<<std::endl;
  return h;
}
TProfile* DCHistogramHandler::AccessProfile(std::string name,int nbinx,double xmin,double xmax,double ymin,double ymax,std::string sub)
{
  TProfile* p=(TProfile*) GetTH1(sub+name);
  if (p==NULL)
    p=BookProfile(sub+name,nbinx,xmin,xmax,ymin,ymax);
  return p;
}

TProfile* DCHistogramHandler::BookProfile(std::string name,int nbinx,double xmin,double xmax,double ymin,double ymax)
{
  //@ Test top->addFullName(name);
  TProfile* h=NULL;
  try {
    if (ymin<ymax) h=  new TProfile(name.c_str(),name.c_str(),nbinx,xmin,xmax,ymin,ymax);
    else  h=  new TProfile(name.c_str(),name.c_str(),nbinx,xmin,xmax);
  }
   catch (std::bad_alloc& ba)
    {
      std::cerr << "bad_alloc caught: " << ba.what() << '\n';
      return 0;
    }
  //  h->SetDirectory(theFile_);
  h->Sumw2();
  std::pair<std::string,TH1*> pr(name,h);
  //std::cout<<name<<" Booked"<<std::endl;
  mapH1.insert(pr);
  return h;
}
TProfile2D* DCHistogramHandler::BookProfile2D(std::string name,int nbinx,double xmin,double xmax,int nbiny, double ymin,double ymax, double zmin, double zmax)
{
  //@ Test top->addFullName(name);
  TProfile2D* h=NULL;
  try {
    if (zmin<zmax)h=  new TProfile2D(name.c_str(),name.c_str(),nbinx,xmin,xmax,nbiny,ymin,ymax,zmin,zmax);
    else h=new TProfile2D(name.c_str(),name.c_str(),nbinx,xmin,xmax,nbiny,ymin,ymax);
  }
   catch (std::bad_alloc& ba)
    {
      std::cerr << "bad_alloc caught: " << ba.what() << '\n';
      return 0;
    }
  //  h->SetDirectory(theFile_);
  h->Sumw2();
  std::pair<std::string,TH2*> pr(name,h);
  //std::cout<<name<<" Booked"<<std::endl;
  mapH2.insert(pr);
  return h;
}


void DCHistogramHandler::writeHistograms(std::string name)
{
  
  TFile f(name.c_str(),"RECREATE","RAW DATA");
  f.cd();
 
  for (std::map<std::string,TH1*>::iterator iter= mapH1.begin();iter!=mapH1.end();iter++)
    {
      f.cd();
     //std::cout<<" writing "<<iter->first<<std::endl;
     std::vector<std::string> vdir;
     DCUtils::String2Vector(iter->first,vdir,"/");
     std::string dirname(""),lastname("");
     if (vdir.size()>0)
       {
       for (unsigned int i=1;i<vdir.size()-1;i++)
	 {


	   dirname+="/"+vdir[i];
	   f.cd();
	   TDirectory* det=f.GetDirectory(dirname.c_str());
	   if (det==0)
	     {
	       f.cd(lastname.c_str());
	       gDirectory->cd();
	       TDirectory* detn = gDirectory->mkdir(vdir[i].c_str(),vdir[i].c_str());
	       if (detn!=0) detn->cd();
	       det=f.GetDirectory(dirname.c_str());
	     }
	   if (det!=0) det->cd();
	   lastname=dirname;
	 }
       iter->second->SetName(vdir[vdir.size()-1].c_str());       
       }


     iter->second->Write();
     delete iter->second;
    }

  for (std::map<std::string,TH2*>::iterator iter= mapH2.begin();iter!=mapH2.end();iter++)
    {

      f.cd();
     //std::cout<<" writing "<<iter->first<<std::endl;
     std::vector<std::string> vdir;
     DCUtils::String2Vector(iter->first,vdir,"/");
     std::string dirname(""),lastname("");
     if (vdir.size()>0)
       {
       for (unsigned int i=1;i<vdir.size()-1;i++)
	 {


	   dirname+="/"+vdir[i];
	   f.cd();
	   TDirectory* det=f.GetDirectory(dirname.c_str());
	   if (det==0)
	     {
	       f.cd(lastname.c_str());
	       gDirectory->cd();
	       TDirectory* detn = gDirectory->mkdir(vdir[i].c_str(),vdir[i].c_str());
	       if (detn!=0) detn->cd();
	       det=f.GetDirectory(dirname.c_str());
	     }
	   if (det!=0) det->cd();
	   lastname=dirname;
	 }
       iter->second->SetName(vdir[vdir.size()-1].c_str());
       }    
     iter->second->Write();
     delete iter->second;
    }

  for (std::map<std::string,TH3*>::iterator iter= mapH3.begin();iter!=mapH3.end();iter++)
    {

      f.cd();
     //std::cout<<" writing "<<iter->first<<std::endl;
     std::vector<std::string> vdir;
     DCUtils::String2Vector(iter->first,vdir,"/");
     std::string dirname(""),lastname("");
     if (vdir.size()>0)
       {
       for (unsigned int i=1;i<vdir.size()-1;i++)
	 {


	   dirname+="/"+vdir[i];
	   f.cd();
	   TDirectory* det=f.GetDirectory(dirname.c_str());
	   if (det==0)
	     {
	       f.cd(lastname.c_str());
	       gDirectory->cd();
	       TDirectory* detn = gDirectory->mkdir(vdir[i].c_str(),vdir[i].c_str());
	       if (detn!=0) detn->cd();
	       det=f.GetDirectory(dirname.c_str());
	     }
	   if (det!=0) det->cd();
	   lastname=dirname;
	 }
       }







      iter->second->Write();
      delete iter->second;
    }





  mapH1.clear();
  mapH2.clear();
  mapH3.clear();
  f.Write();
  f.Close();
}
 
 
#include <sys/stat.h>
#include <sys/types.h>
#include "TBufferXML.h"
#include "TSQLFile.h"
#include <iostream>
#include <fstream>
void DCHistogramHandler::writeSQL(std::string name)
{
  /*
  const char* dbname = "mysql://lyosdhcal11.in2p3.fr:3306/monitoring";
  const char* username = "mirabito";
  const char* userpass = "braze1";

  // Clean data base and create primary tables
  TSQLFile* f = new TSQLFile(dbname, "recreate", username, userpass);
  // Write with standard I/O functions
  */
  TFile* f = new TFile(name.c_str(),"RECREATE","RAW DATA");
  f->cd();

 
  for (std::map<std::string,TH1*>::iterator iter= mapH1.begin();iter!=mapH1.end();iter++)
    {
      f->cd();
     //std::cout<<" writing "<<iter->first<<std::endl;
     std::vector<std::string> vdir;
     DCUtils::String2Vector(iter->first,vdir,"/");
     std::string dirname(""),lastname("");
     if (vdir.size()>0)
       {
       for (unsigned int i=1;i<vdir.size()-1;i++)
	 {


	   dirname+="/"+vdir[i];
	   f->cd();
	   TDirectory* det=f->GetDirectory(dirname.c_str());
	   if (det==0)
	     {
	       f->cd(lastname.c_str());
	       gDirectory->cd();
	       TDirectory* detn = gDirectory->mkdir(vdir[i].c_str(),vdir[i].c_str());
	       if (detn!=0) detn->cd();
	       det=f->GetDirectory(dirname.c_str());
	     }
	   if (det!=0) det->cd();
	   lastname=dirname;
	 }
       iter->second->SetName(vdir[vdir.size()-1].c_str());       
       }


     iter->second->Write();
    }

  for (std::map<std::string,TH2*>::iterator iter= mapH2.begin();iter!=mapH2.end();iter++)
    {

      f->cd();
     //std::cout<<" writing "<<iter->first<<std::endl;
     std::vector<std::string> vdir;
     DCUtils::String2Vector(iter->first,vdir,"/");
     std::string dirname(""),lastname("");
     if (vdir.size()>0)
       {
       for (unsigned int i=1;i<vdir.size()-1;i++)
	 {


	   dirname+="/"+vdir[i];
	   f->cd();
	   TDirectory* det=f->GetDirectory(dirname.c_str());
	   if (det==0)
	     {
	       f->cd(lastname.c_str());
	       gDirectory->cd();
	       TDirectory* detn = gDirectory->mkdir(vdir[i].c_str(),vdir[i].c_str());
	       if (detn!=0) detn->cd();
	       det=f->GetDirectory(dirname.c_str());
	     }
	   if (det!=0) det->cd();
	   lastname=dirname;
	 }
       iter->second->SetName(vdir[vdir.size()-1].c_str());
       }    
     iter->second->Write();

    }

  for (std::map<std::string,TH3*>::iterator iter= mapH3.begin();iter!=mapH3.end();iter++)
    {

      f->cd();
     //std::cout<<" writing "<<iter->first<<std::endl;
     std::vector<std::string> vdir;
     DCUtils::String2Vector(iter->first,vdir,"/");
     std::string dirname(""),lastname("");
     if (vdir.size()>0)
       {
       for (unsigned int i=1;i<vdir.size()-1;i++)
	 {


	   dirname+="/"+vdir[i];
	   f->cd();
	   TDirectory* det=f->GetDirectory(dirname.c_str());
	   if (det==0)
	     {
	       f->cd(lastname.c_str());
	       gDirectory->cd();
	       TDirectory* detn = gDirectory->mkdir(vdir[i].c_str(),vdir[i].c_str());
	       if (detn!=0) detn->cd();
	       det=f->GetDirectory(dirname.c_str());
	     }
	   if (det!=0) det->cd();
	   lastname=dirname;
	 }
       }







      iter->second->Write();

    }






  f->Write();
  f->Close();

#ifdef OLDWAY

  for (std::map<std::string,TH1*>::iterator iter= mapH1.begin();iter!=mapH1.end();iter++)
    {
      iter->second->Write(iter->first.c_str());
	}

  for (std::map<std::string,TH2*>::iterator iter= mapH2.begin();iter!=mapH2.end();iter++)
    {
      iter->second->Write(iter->first.c_str());
    }

  f->Write();
  f->Close();
#endif

  // Close connection to DB
  delete f;
}
void DCHistogramHandler::writeXML(std::string  path)
{
 	std::string curpath=path;
		std::cout<<path<<std::endl;

	mode_t process_mask = umask(0);
	mkdir(curpath.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);
	umask(process_mask);
 	for (std::map<std::string,TH1*>::iterator iter= mapH1.begin();iter!=mapH1.end();iter++)
	{
		//std::cout<<" writing "<<iter->first<<std::endl;
		std::vector<std::string> vdir;
		DCUtils::String2Vector(iter->first,vdir,"/");
		std::string dirname(""),lastname("");
		std::string curpath=path;
		process_mask = umask(0);
		mkdir(curpath.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);	
		umask(process_mask);
		if (vdir.size()>0)
		{
			for (unsigned int i=1;i<vdir.size()-1;i++)
			{


				curpath+="/"+vdir[i];
				process_mask = umask(0);
				mkdir(curpath.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);	
				umask(process_mask);
				
			}
			lastname=curpath+"/"+vdir[vdir.size()-1];
		}
		else
			lastname=curpath+"/"+iter->first;

		//std::cout<<lastname<<std::endl;
		std::ostringstream xmlstr;
	  
	  xmlstr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	  
	  xmlstr<<TBufferXML::ConvertToXML(iter->second);
	  std::ofstream myfile;
		myfile.open (lastname.c_str());
		myfile << xmlstr.str();
		myfile.close();
	  
		
		
		//iter->second->Write();
		//delete iter->second;
	}
	curpath=path;
	for (std::map<std::string,TH2*>::iterator iter= mapH2.begin();iter!=mapH2.end();iter++)
	{
		//std::cout<<" writing "<<iter->first<<std::endl;
		std::vector<std::string> vdir;
		DCUtils::String2Vector(iter->first,vdir,"/");
		std::string dirname(""),lastname("");
		std::string curpath=path;
		process_mask = umask(0);
		mkdir(curpath.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);	
		umask(process_mask);
		if (vdir.size()>0)
		{
			for (unsigned int i=1;i<vdir.size()-1;i++)
			{


				curpath+="/"+vdir[i];
				process_mask = umask(0);
				mkdir(curpath.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);	
				umask(process_mask);
				
			}
			lastname=curpath+"/"+vdir[vdir.size()-1];
		}
		else
			lastname=curpath+"/"+iter->first;

		//std::cout<<lastname<<std::endl;
		std::ostringstream xmlstr;
	  
	  xmlstr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	  
	  xmlstr<<TBufferXML::ConvertToXML(iter->second);
	  std::ofstream myfile;
		myfile.open (lastname.c_str());
		myfile << xmlstr.str();
		myfile.close();
	  
		
		
		//iter->second->Write();
		//delete iter->second;
	}

 
}


void DCHistogramHandler::ListHisto(std::string reg,int type,std::vector<std::string> &found)
{
  found.clear();
  regex_t preg; int cflags=REG_EXTENDED;cflags=0;
  regcomp(&preg,reg.c_str(),cflags);

  //std::cout<<"On entre dans ListHisto "<<reg<<std::endl;
  //regmatch_t pmatch[256]; int eflags=0;size_t nmatch=0;
  if ( type ==1)
    for (std::map<std::string,TH1*>::const_iterator iter = mapH1.begin(); iter!=mapH1.end();++iter)
      {
	int ier =regexec(&preg, iter->first.c_str(),0,NULL,0);
	if (ier ==0)
	  {
	    //std::cout<<"GetTH1(\""<<iter->first<<"\")"<<std::endl;
	  found.push_back(iter->first);
	  }
      }
  if ( type ==2)
    for (std::map<std::string,TH2*>::const_iterator iter = mapH2.begin(); iter!=mapH2.end();++iter)
      {
	int ier =regexec(&preg, iter->first.c_str(),0,NULL,0);
	if (ier ==0)
	  {
	    //std::cout<<"GetTH2(\""<<iter->first<<"\")"<<std::endl;
	    found.push_back(iter->first);
	  }
      }
  if ( type ==3)
    for (std::map<std::string,TH3*>::const_iterator iter = mapH3.begin(); iter!=mapH3.end();++iter)
      {
	int ier =regexec(&preg, iter->first.c_str(),0,NULL,0);
	if (ier ==0)
	  {
	    //std::cout<<"GetTH2(\""<<iter->first<<"\")"<<std::endl;
	    found.push_back(iter->first);
	  }
      }

  //std::cout<<"On sort de ListHisto"<<std::endl;
}

std::string DCHistogramHandler::getXMLHistoList()
{
  std::ostringstream xmlstr;
  
  xmlstr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  xmlstr << "<response success=\"true\">" << std::endl;
  for (std::map<std::string,TH1*>::const_iterator iter = mapH1.begin(); iter!=mapH1.end();++iter)
    {
      xmlstr<<"<Histo>"<<iter->first<<"</Histo>"<<std::endl;
    }
  for (std::map<std::string,TH2*>::const_iterator iter = mapH2.begin(); iter!=mapH2.end();++iter)
    {
      xmlstr<<"<Histo>"<<iter->first<<"</Histo>"<<std::endl;
    }
  for (std::map<std::string,TH3*>::const_iterator iter = mapH3.begin(); iter!=mapH3.end();++iter)
    {
      xmlstr<<"<Histo>"<<iter->first<<"</Histo>"<<std::endl;
    }
  xmlstr << "</response>" ;
  return  xmlstr.str();
  
  
}
std::string DCHistogramHandler::getJSONHistoList()
{
  std::ostringstream xmlstr;
  
  xmlstr << "{ " << std::endl;
  xmlstr << "\"TH1\":[" << std::endl;
  uint32_t nh1=0;
  for (std::map<std::string,TH1*>::const_iterator iter = mapH1.begin(); iter!=mapH1.end();++iter)
    {
      xmlstr<<"\""<<iter->first<<"\"";
      nh1++;
      if (nh1!=mapH1.size()) 
      xmlstr<<",";  
      
    }
  xmlstr<<"],";

  nh1=0;
  xmlstr << "\"TH2\":[" << std::endl;
  for (std::map<std::string,TH2*>::const_iterator iter = mapH2.begin(); iter!=mapH2.end();++iter)
    {
      xmlstr<<"\""<<iter->first<<"\"";
      nh1++;
      if (nh1!=mapH2.size()) 
      xmlstr<<",";  
      
     
    }
  xmlstr<<"]";

  xmlstr << "}" ;
  return  xmlstr.str();
  
  
}
std::string DCHistogramHandler::getXMLHisto(std::string hname)
{
  std::ostringstream xmlstr;
  //  xmlstr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  TH1* h1= this->GetTH1(hname);
  if (h1!=0)
    xmlstr<<TBufferXML::ConvertToXML(h1);
  else
    {
      TH2* h2= this->GetTH2(hname);
      if (h2!=0)
	xmlstr<<TBufferXML::ConvertToXML(h2);
      else
	{
	  TH3* h3= this->GetTH3(hname);
	  if (h3!=0)
	    xmlstr<<TBufferXML::ConvertToXML(h3);
	}
    }
 return xmlstr.str();
}

std::string DCHistogramHandler::getJSONHisto(std::string hname)
{
  std::ostringstream xmlstr;
  //  xmlstr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  TH1* h1= this->GetTH1(hname);
  if (h1!=0)
    xmlstr<<TBufferJSON::ConvertToJSON(h1);
  else
    {
      TH2* h2= this->GetTH2(hname);
      if (h2!=0)
	xmlstr<<TBufferJSON::ConvertToJSON(h2);
      else
	{
	  TH3* h3= this->GetTH3(hname);
	  if (h3!=0)
	    xmlstr<<TBufferJSON::ConvertToJSON(h3);
	}
    }
 return xmlstr.str();
}
