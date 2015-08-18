#include "TornadoPlot.h"

TornadoPlot::TornadoPlot() {};

void TornadoPlot::Initialize()
{
  parseSettings();

  fShelve= fShelveVector[0];
  fBoard = fShelve->fBoardVector[0];
  fFe    = fBoard->fModuleVector[0];
  uint32_t cFeId = fFe->getFeId();
  
  CbcRegWriter cWriterPlus( fCbcInterface, "Vplus", fVplus );
  this->accept( cWriterPlus );
  CbcRegWriter cWriterThreshold( fCbcInterface, "VCth" , fVCth );
  this->accept( cWriterThreshold );
  
  TString cCanvasName = Form( "FE%d Online Canvas", cFeId );
  TCanvas* cCanvas = new TCanvas( cCanvasName , cCanvasName , 1100 , 600 );
  cCanvas->Divide(2);
  fCanvasMap[fFe] = cCanvas;
  
  for ( auto& cCbc : fFe->fCbcVector )
    {
      uint32_t cCbcId = cCbc->getCbcId();
      
      // one 2D histogram per CBC
      uint32_t AmpRange = fAmpMax - fAmpMin;
      uint32_t DelayRange = fDelayMax - fDelayMin;

      TString cHistName = Form( "Hist2D_Cbc%d" , cCbcId );
      TH2F* cDelAmp = new TH2F(cHistName,cHistName, fDelayMax-fDelayMin,fDelayMin,fDelayMax , AmpRange/fAmpStep,fAmpMin,fAmpMax);
      cDelAmp->GetYaxis()->SetTitle( "Pulse Amplitude" );
      cDelAmp->GetXaxis()->SetTitle( "Pulse Delay" );
      cDelAmp->SetMinimum(-0.01);
      cDelAmp->SetMaximum(1);
      gStyle->SetPalette(57);
      gStyle->SetNumberContours(50);
      fHistMap[cCbc] = cDelAmp;
    }     
}


void TornadoPlot::ScanTestPulseDelay(uint32_t cCoarseDelay)
{
  for (uint32_t cFineDelay = fDelayMin; cFineDelay < fDelayMax; cFineDelay++ )
    {
      uint32_t cDelay = cCoarseDelay + cFineDelay;
      setDelayAndTestGroup( cDelay );
      ScanTestPulseAmplitude(cFineDelay);
    }
}


void TornadoPlot::ScanTestPulseAmplitude(uint32_t cFineDelay)
{
  for (uint32_t cAmp = fAmpMin; cAmp<fAmpMax; cAmp+=fAmpStep)
    {
      setSystemTestPulse( cAmp , fChannel );
      vector1D ResultVec = GetEfficiency(cFineDelay,cAmp);  // data taken here
      auto cCanvas = fCanvasMap.find(fFe)->second;
      for (auto& cCbc : fFe->fCbcVector)
	{
	  uint32_t cCbcId = cCbc->getCbcId();
	  cCanvas->cd(cCbcId+1);
	  auto c2DHist = fHistMap.find(cCbc)->second;
	  c2DHist->Fill( cFineDelay , cAmp , ResultVec[cCbcId] );
	  c2DHist->Draw("colz");
	}
      cCanvas->Update();
    }
}

vector1D TornadoPlot::GetEfficiency( uint32_t cFineDelay , uint32_t cAmp)
{  
        
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
  return EventAvg;
}


vector1D TornadoPlot::EventAveraging(vector2D EventRes)
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
 

void TornadoPlot::setDelayAndTestGroup( uint32_t pDelay )
{
  this->fTestGroup = findTestGroup( fChannel );
  uint8_t cCoarseDelay = floor( pDelay  / 25 );
  uint8_t cFineDelay = ( cCoarseDelay * 25 ) + 24 - pDelay;

  std::cout << "Current Time: " << pDelay<< std::endl;
  BeBoardRegWriter cBeBoardWriter( fBeBoardInterface, DELAY_AF_TEST_PULSE, cCoarseDelay );
  this->accept( cBeBoardWriter );
  CbcRegWriter cWriter( fCbcInterface, "SelTestPulseDel&ChanGroup", to_reg( cFineDelay, fTestGroup ) );
  this->accept( cWriter );
}


void TornadoPlot::parseSettings()
{
  // now read the settings from the map
  auto cSetting = fSettingsMap.find( "Nevents" );
  fNumEvents = cSetting->second;

  cSetting = fSettingsMap.find( "VCth" );
  fVCth = cSetting->second;
  
  cSetting = fSettingsMap.find( "Vplus" );
  fVplus = cSetting->second;

  cSetting = fSettingsMap.find( "Channel" );
  fChannel = cSetting->second;
  
  cSetting = fSettingsMap.find( "AmpMax" );
  fAmpMax = cSetting->second;
  
  cSetting = fSettingsMap.find( "AmpMin" );
  fAmpMin = cSetting->second;
  
  cSetting = fSettingsMap.find( "AmpStep" );
  fAmpStep = cSetting->second;
  
  cSetting = fSettingsMap.find( "DelayMax" );
  fDelayMax = cSetting->second;
  
  cSetting = fSettingsMap.find( "DelayMin" );
  fDelayMin = cSetting->second;


  std::cout << "Parsed the following settings:"          <<std::endl;
  std::cout << "	Nevents    = " << fNumEvents     <<std::endl;
  std::cout << "        VCth       = " << int( fVCth )   <<std::endl;
  std::cout << "	Vplus      = " << int( fVplus )  <<std::endl;
  std::cout << "	Channel    = " << int( fChannel )<<std::endl;
  std::cout << "        AmpMax     = " << int( fAmpMax ) <<std::endl;
  std::cout << "        AmpMin     = " << int( fAmpMin ) <<std::endl;
}


void TornadoPlot::setSystemTestPulse( uint8_t pTPAmplitude, uint8_t pChannelId )
{
  // translate the channel id to a test group
  std::vector<std::pair<std::string, uint8_t>> cRegVec;
  
  //calculate the right test group
  this->fTestGroup = findTestGroup( pChannelId );

  cRegVec.push_back( std::make_pair( "MiscTestPulseCtrl&AnalogMux", 0xD1 ) );
  cRegVec.push_back( std::make_pair( "TestPulsePot", pTPAmplitude ) );
  cRegVec.push_back( std::make_pair( "Vplus",  fVplus ) );
  
  CbcMultiRegWriter cWriter( fCbcInterface, cRegVec );
  this->accept( cWriter );
}


int TornadoPlot::findTestGroup( uint32_t pChannelId )
{
  int cGrp = -1;
  for ( int cChIndex = 0; cChIndex < 16; cChIndex++ )
    {
      uint32_t cResult = pChannelId / 2 - cChIndex * 8;
      if ( cResult < 8 ) cGrp = cResult;
    }
  return cGrp;
}
