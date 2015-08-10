#include "PulseNoiseStudy.h"


PulseNoiseStudy::PulseNoiseStudy() {};


void PulseNoiseStudy::Initialize()
{
  ParseSettings();
    
  fShelve = fShelveVector[0]; //only one shelve, board, and module
  fBoard = fShelve->fBoardVector[0];
  fFe = fBoard->fModuleVector[0];
  uint32_t BoardId=fBoard->getBeId();
  uint32_t FeId=fFe->getFeId();
    
  uint32_t pDelayMin = 5000;
  uint32_t pDelayMax = 5025;
    
  // initialize canvases/histograms here...
  for (auto& cCbc : fFe->fCbcVector)
    {
      uint32_t cCbcId = cCbc->getCbcId();
      TString cCanvasName = Form( "FE%dCBC%d_Canvas" , 0 , cCbcId);
      TCanvas* ctmpCanvas = new TCanvas( cCanvasName , cCanvasName );
      ctmpCanvas->Divide(2,1);
      fCanvasMap[cCbc] = ctmpCanvas;
        
      uint32_t AmpRange = fAmpMax - fAmpMin;
      TString cHistName = Form( "FE%dCBC%d_histogram" , 0 , cCbcId);
      TH1F* cHist = new TH1F( cHistName , cHistName , AmpRange/fAmpStep , fAmpMin , fAmpMax);
      cHist->GetXaxis()->SetTitle( "Amplitude" );
      cHist->GetYaxis()->SetTitle( "Efficiency" );
      fHistMap[cCbc] = cHist;
    }
}


void PulseNoiseStudy::ScanAmplitudes()
{
  vector1D cPulsePeakVec = findPulseMax( fAmpMin );
  //setDelayAndTestGroup(cPulsePeak);
  // sets for all cbc's... taking the average here I guess?
  setDelayAndTestGroup( floor( (cPulsePeakVec[0]+cPulsePeakVec[1]) / 2 ) );
    
  for (uint32_t cAmp = fAmpMin+1; cAmp<fAmpMax; cAmp += fAmpStep)
    { // we've already done fAmpMin in "findPulseMax"; proceed to next one.
      setSystemTestPulse( cAmp );
      vector1D cMidPoints = ScanVCthEff( cAmp );
        
      for (auto& cCbc : fFe->fCbcVector)
        {
	  uint32_t cCbcId = cCbc->getCbcId();
	  auto cCanvas = fCanvasMap.find(cCbc)->second;
	  auto cHist = fHistMap.find(cCbc)->second;
        
	  cCanvas->cd(2);
	  cHist->Fill(cAmp,cMidPoints[cCbcId]);
	  cHist->Draw("hist");
	  cCanvas->Update();
        }
    }
}


vector1D PulseNoiseStudy::findPulseMax( uint32_t fAmpMin )
{
  setSystemTestPulse( fAmpMin );
    
  vector2D AmpScanVec;
  for (uint32_t cDelay = pDelayMin; cDelay < pDelayMax; cDelay++)
    {
      setDelayAndTestGroup(cDelay);
      vector1D AmpScanRes = ScanVCthEff(fAmpMin);
      AmpScanVec.push_back( AmpScanRes );
    }
  vector1D cPulsePeakVec;
  for (auto& cCbc : fFe->fCbcVector)
    {
      uint32_t cPulsePeak = 0;
      for (uint32_t ii=1; ii<AmpScanVec.size(); ii++)
	{
	  if ( AmpScanVec[ii] > AmpScanVec[cPulsePeak] )
	    cPulsePeak=ii;
	}
      cPulsePeakVec.push_back(cPulsePeak);
    }
  return cPulsePeakVec;
}


vector1D PulseNoiseStudy::ScanVCth(uint32_t cAmp)
{
  DataMap cDataMap;
  uint32_t cVCth = fVplus;
  uint32_t NZeros = 0;
    
  while (NZeros < 5)
    {
      CbcRegWriter cWriter( fCbcInterface, "VCth", cVCth );
      this->accept( cWriter );
        
      vector2D EventData; // then we take fNumEvents
      uint32_t N = 0;
      uint32_t AcqNum = 0;
        
      fBeBoardInterface->Start( fBoard ); // start of data capture
      while (N < fNumEvents )
        {
	  fBeBoardInterface->ReadData(fBoard,AcqNum,false);
	  const Event* cEvent = fBeBoardInterface->GetNextEvent(fBoard);
	  while (cEvent)
            {
	      vector1D CbcData;
	      for (auto& cCbc : fFe->fCbcVector )
                {
		  uint32_t cCbcId = cCbc->getCbcId();
		  double result = cEvent->DataBit( 0 , cCbcId , fChannel);
		  CbcData.push_back(result);
                }
	      EventData.push_back(CbcData);
	      N++;
	      if (N <= fNumEvents )
		cEvent = fBeBoardInterface->GetNextEvent(fBoard);
	      else break;
            }
	  AcqNum++;
        }
      fBeBoardInterface->Stop( fBoard, AcqNum ); // end of data capture
        
      vector1D EventAvg = EventAveraging(EventData);
      if (EventAvg[0]+EventAvg[1]==0) NZeros++; // stop after threshold high enough
      cDataMap[cVCth] = EventAvg;
      cVCth++;
    }
    
  vector1D cMidPoints = MakeScurve(cDataMap,cAmp,cVCth-1);
  return cMidPoints;
}


vector1D PulseNoiseStudy::MakeScurve( DataMap cDataMap , uint32_t cAmp , uint32_t cVCthMax )
{
  vector1D cMidPoints;
  for (auto& cCbc : fFe->fCbcVector)
    {
      uint32_t cCbcId = cCbc->getCbcId();
      uint32_t cVCthMin = fVplus;
        
      //initialize hists/fits
      TString cScurveName = Form( "FE%dCBC%d_Scurve" , 0 , cCbcId);
      TH1F* cScurve = new TH1F( cScurveName , cScurveName , cVCthMax - cVCthMin , cVCthMin , cVCthMax );
      cScurve->GetXaxis()->SetTitle( "VCth" );
      cScurve->GetYaxis()->SetTitle( "Occupancy" );
        
      TString cFitName = Form( "Fit_Amp%d" , cAmp );
      TF1* cFit = dynamic_cast<TF1*>( gROOT->FindObject(cFitName) );
      cFit = new TF1( cFitName , MyErf , cVCthMin-5 , cVCthMax+5 , 2 );
      cFit->GetYaxis()->SetRange(0,1);
        
      // fill curves
      for (uint32_t cVCth = cVCthMin; cVCth<cVCthMax; cVCth++)
	cScurve->Fill( cVCth , (cDataMap.find(cVCth)->second)[cCbcId] );
        
      // make fit
      double cFirstNon0 = 0;
      double cFirst1 = 0;
      for (int cBin = cScurve->GetNbinsX(); cBin>=1; cBin--)
        {
	  double cContent = cScurve->GetBinContent(cBin);
	  if (!cFirstNon0 )
            {
	      if (cContent)
		cFirstNon0 = cScurve->GetBinCenter(cBin);
            }
	  else if (cContent == 1)
            {
	      cFirst1 = cScurve->GetBinCenter(cBin);
	      break;
            }
        }
        
      double cMiddle = (cFirst1 + cFirstNon0)*0.5;
      double cWidth = (cFirst1 - cFirstNon0)*0.5;
      cFit->SetParameter(0,cMiddle);
      cFit->SetParameter(1,cWidth);
        
      auto cCanvas = fCanvasMap.find(cCbc)->second;
      cCanvas->cd(1);
      cScurve->Fit(cFit , "RNQ+" );
      cCanvas->cd(1);
      cScurve->Draw("hist c");
      cFit->Draw("same");
      cCanvas->Update();
        
      double cMiddleFit = cFit->GetX(0.5 , cFirst1 , cFirstNon0);
      cMidPoints.push_back(cMiddleFit);
    }
  return cMidPoints;
}


void PulseNoiseStudy::setSystemTestPulse(uint32_t cAmp)
{
  this->fTestGroup = findTestGroup();
    
  // vector of registers to edit
  //std::vector<std::pair<std::string,uint32_t>>cRegVec;
  std::vector<std::pair<std::string,uint8_t >> cRegVec;
  cRegVec.push_back( std::make_pair("MiscTestPulseCtrl&AnalogMux",0xD1) );
  cRegVec.push_back( std::make_pair("TestPulsePot",cAmp) );
  cRegVec.push_back( std::make_pair("Vplus",fVplus) );
    
  CbcMultiRegWriter cWriter( fCbcInterface , cRegVec );
  this->accept( cWriter );
}


void PulseNoiseStudy::setDelayAndTestGroup(uint32_t cDelay)
{
  uint32_t cCoarseDelay = floor( cDelay / 25 );
  uint32_t cFineDelay = (cCoarseDelay*25) + 24 - cDelay;
  std::cout<<"Current time: "<<cDelay<<std::endl;
    
  BeBoardRegWriter cBeBoardRegWriter( fBeBoardInterface, DELAY_AF_TEST_PULSE, cCoarseDelay);
  this->accept( cBeBoardRegWriter );
    
  CbcRegWriter cWriter( fCbcInterface , "SelTestPulseDel&ChanGroup" , to_reg( cFineDelay , fTestGroup ) );
  this->accept( cWriter );
}


void PulseNoiseStudy::ParseSettings()
{
  // transfering the settings from the map to the global variables
  auto cSetting = fSettingsMap.find("NumEvents");
  fNumEvents = cSetting->second;
  cSetting = fSettingsMap.find("Vplus");
  fVplus = cSetting->second;
  cSetting = fSettingsMap.find("Channel");
  fChannel = cSetting->second;
  cSetting = fSettingsMap.find("AmpMin");
  fAmpMin = cSetting->second;
  cSetting = fSettingsMap.find("AmpMax");
  fAmpMax = cSetting->second;
  cSetting = fSettingsMap.find("AmpStep");
  fAmpStep = cSetting->second;
}


vector1D PulseNoiseStudy::EventAveraging(vector2D EventRes)
{
  vector1D EventAvg;
  for (auto& cCbc : fFe->fCbcVector)
    {
      uint32_t cCbcId = cCbc->getCbcId();
      double sum = 0;
      for (int ii=0; ii<fNumEvents; ii++)
	sum += EventRes[ii][cCbcId];
      EventAvg.push_back(sum/fNumEvents);
    }
  return EventAvg;
}


int PulseNoiseStudy::findTestGroup()
{
  int cGroup = -1;
  for (int cChInd=0; cChInd<16; cChInd++)
    {
      uint32_t result = fChannel / 2 - cChInd*8;
      if (result<8)
	cGroup = result;
    }
  return cGroup;
}


vector1D PulseNoiseStudy::ScanVCthEff(uint32_t cAmp)
{
  DataMap cDataMap;
  uint32_t cVCth = fVplus;
  uint32_t cVCthStep = 5;
  uint32_t NZeros = 0;
    
  while (NZeros < 5)
    {
      CbcRegWriter cWriter( fCbcInterface, "VCth", cVCth );
      this->accept( cWriter );
        
      vector2D EventData; // then we take fNumEvents
      uint32_t N = 0;
      uint32_t AcqNum = 0;
        
      fBeBoardInterface->Start( fBoard ); // start of data capture
      while (N < fNumEvents )
        {
	  fBeBoardInterface->ReadData(fBoard,AcqNum,false);
	  const Event* cEvent = fBeBoardInterface->GetNextEvent(fBoard);
	  while (cEvent)
            {
	      vector1D CbcData;
	      for (auto& cCbc : fFe->fCbcVector )
                {
		  uint32_t cCbcId = cCbc->getCbcId();
		  double result = cEvent->DataBit( 0 , cCbcId , fChannel);
		  CbcData.push_back(result);
                }
	      EventData.push_back(CbcData);
	      N++;
	      if (N <= fNumEvents )
		cEvent = fBeBoardInterface->GetNextEvent(fBoard);
	      else break;
            }
	  AcqNum++;
        }
      fBeBoardInterface->Stop( fBoard, AcqNum ); // end of data capture
        
      vector1D EventAvg = EventAveraging(EventData);
      if (EventAvg[0]+EventAvg[1] < 1 && cVCthStep==5)
        {
	  cVCthStep = 1;
	  cVCth = cVCth-5;
        }
      if (EventAvg[0]+EventAvg[1]==0) NZeros++; // stop after threshold high enough
      cDataMap[cVCth] = EventAvg;
      cVCth += cVCthStep;
    }
    
  vector1D cMidPoints = MakeScurve(cDataMap,cAmp,cVCth-1);
  return cMidPoints;
}
