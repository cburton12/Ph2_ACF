#include "DelayScan.h"

DelayScan::DelayScan() {};

void DelayScan::Initialize()
{
  parseSettings();

  fShelve= fShelveVector[0];
  fBoard = fShelve->fBoardVector[0];
  fFe    = fBoard->fModuleVector[0];
  uint32_t cFeId = fFe->getFeId();
  
  //uint32_t cDelayAfterPulse = fBeBoardInterface->ReadBoardReg( fBoard, DELAY_AF_TEST_PULSE );
  CbcRegWriter cWriter( fCbcInterface, "Vplus", fVplus );
  this->accept( cWriter );
	      
  for ( auto& cCbc : fFe->fCbcVector )
    {
      uint32_t cCbcId = cCbc->getCbcId();
      
      // One canvas for each CBC
      TString cCanvasName = Form( "FE%dCBC%d  Online Canvas", cFeId, cCbcId );
      TCanvas* ctmpCanvas = new TCanvas( cCanvasName , cCanvasName , 1200 , 700 );
      ctmpCanvas->Divide( 3, 1 );
      fCanvasMap[cCbc] = ctmpCanvas;

      // 2D histogram for the final plot, one per CBC
      uint32_t AmpRange = fAmpMax - fAmpMin;
      uint32_t DelayRange = fDelayMax - fDelayMin;
      //std::cout<<"ampr: "<<AmpRange<<"      "<<"dely: "<<DelayRange<<std::endl;
      TString cHistName = Form( "Hist2D_Cbc%d" , cCbcId );
      TH2F* cAmpDel = new TH2F( cHistName , cHistName , AmpRange/fAmpStep , fAmpMin , fAmpMax ,fDelayMax-fDelayMin , fDelayMin , fDelayMax );
      cAmpDel->GetXaxis()->SetTitle( "Pulse Amplitude" );
      cAmpDel->GetYaxis()->SetTitle( "Pulse Delay" );
      f2DHistMap[cCbc] = cAmpDel;
      
      std::vector<TH1F*> cPulseShapeVec;
      // Create graphs for each CBC 
      TString cName =  Form( "Pulseshape_Cbc%d", cCbcId );
      TObject* cObj = gROOT->FindObject( cName );
      if ( cObj ) delete cObj;
      TH1F* cPulseGraph = new TH1F( cName , cName , fDelayMax-fDelayMin , fDelayMin , fDelayMax );
      cPulseGraph->SetName( cName );
      cPulseGraph->SetMarkerStyle( 3 );
      cPulseGraph->GetXaxis()->SetTitle( "TestPulseDelay [ns]" );
      cPulseGraph->GetYaxis()->SetTitle( "Threshold" );
      cPulseShapeVec.push_back(cPulseGraph);
      
      TString cNameMax = Form( "PulseshapeMax_Cbc%d", cCbcId);
      TH1F* cPulseMax = new TH1F(cNameMax , cNameMax , fDelayMax-fDelayMin , fDelayMin , fDelayMax);
      cPulseMax->SetMarkerStyle(22);
      cPulseShapeVec.push_back(cPulseMax);

      fPulseShapeVecMap[cCbc]=cPulseShapeVec;
    }     
}


void DelayScan::ScanTestPulseDelay(uint32_t cCoarseDelay)
{
  for (uint32_t cFineDelay = fDelayMin; cFineDelay < fDelayMax; cFineDelay++ )
    {
      uint32_t cDelay = cCoarseDelay + cFineDelay;
      setDelayAndTestGroup( cDelay );
      
      ScanTestPulseAmplitude(cFineDelay);
    }
}


void DelayScan::ScanTestPulseAmplitude(uint32_t cFineDelay)
{
  for (uint32_t cAmp = fAmpMin; cAmp<fAmpMax; cAmp+=fAmpStep)
    {
      setSystemTestPulse( cAmp , fChannel );
      vector1D VCthMid = ScanVCth(cFineDelay,cAmp);  // data taken here

      for (auto& cCbc : fFe->fCbcVector)
	{
	  uint8_t cCbcId = cCbc->getCbcId();
	  
	  auto cCanvas = fCanvasMap.find(cCbc)->second;
	  auto c2DHist = f2DHistMap.find(cCbc)->second;

	  auto cPulseMin = (fPulseShapeVecMap.find(cCbc)->second)[0];
	  auto cPulseMax = (fPulseShapeVecMap.find(cCbc)->second)[1];
	  
	  if (cAmp==fAmpMin)
	    {
	      cPulseMin->Fill(cFineDelay , VCthMid[cCbcId] );
	      cCanvas->cd(2);
	      cPulseMin->Draw("hist same p");
	    }
	  if (cAmp>=fAmpMax-fAmpStep)
	    {
	      cPulseMax->Fill(cFineDelay , VCthMid[cCbcId] );
	      cCanvas->cd(2);
	      cPulseMax->Draw("hist same p");
	    }
	  
	  c2DHist->Fill( cAmp , cFineDelay , VCthMid[cCbcId] );
	  cCanvas->cd(3);
	  c2DHist->Draw("colz");
	  
	  cCanvas->Update();
	}
    }
}

vector1D DelayScan::ScanVCth( uint32_t cFineDelay , uint32_t cAmp)
{
  uint32_t cVCth = fVplus-10;
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
	  cVCth = cVCth-6;
        }
      if (EventAvg[0]+EventAvg[1]==0) NZeros++; // stop after threshold high enough
      ThresholdMap[cVCth] = EventAvg;
      cVCth += cVCthStep;
    }

  vector1D cMidPoints = MakeScurve(ThresholdMap,cFineDelay,cAmp,cVCth-1);
  return cMidPoints;
}


vector1D DelayScan::MakeScurve(ThreshMap ThresholdMap , uint32_t cFineDelay , uint32_t cAmp , uint32_t cVCthMax )
{
  vector1D cMidPoints;
  
  for (auto& cCbc : fFe->fCbcVector)
    {
      uint32_t cCbcId = cCbc->getCbcId();
      uint32_t cVCthMin = fVplus-10;

      // Canvas recall
      auto cCanvas = fCanvasMap.find(cCbc)->second;

      // Scurve initialization
      TString cScurveName = Form("Scurve_Del%d_Amp%d" , cFineDelay , cAmp );
      TH1F* cScurve = dynamic_cast<TH1F*>( gROOT->FindObject(cScurveName) );
      if(cScurve) delete cScurve;
      cScurve = new TH1F( cScurveName , cScurveName , cVCthMax - cVCthMin+30 , cVCthMin-15 , cVCthMax+15 );
      cScurve->GetXaxis()->SetTitle("VCth");
      cScurve->GetYaxis()->SetTitle("Occupancy");

      // Fits
      TString cFitName = Form( "Fit_Del%d_Amp%d" , cFineDelay , cAmp );
      TF1* cFit = dynamic_cast<TF1*>( gROOT->FindObject(cFitName) );
      if (cFit) delete cFit;
      cFit = new TF1( cFitName , MyErf , cVCthMin-15 , cVCthMax+15 , 2 );
      cFit->GetYaxis()->SetRange(0,1);

      // fill Scurves
      for (uint32_t cVCth = cVCthMin; cVCth < cVCthMax; cVCth++)
	  cScurve->Fill( cVCth , ThresholdMap.find(cVCth)->second[cCbcId] );

      double cFirstNon0 = 0;
      double cFirst1 = 0;
      for (int cBin = cScurve->GetNbinsX(); cBin>=1 ; cBin--)
	{
	  double cContent = cScurve->GetBinContent( cBin );
	  if ( !cFirstNon0 )
	    {
	      if (cContent)
		cFirstNon0 = cScurve->GetBinCenter( cBin );
	    }
	  else if (cContent == 1)
	    {
	      cFirst1 = cScurve->GetBinCenter( cBin );
	      break;
	    }
	}

      double cMiddle = (cFirst1 + cFirstNon0 ) * 0.5;
      double cWidth  = (cFirst1 - cFirstNon0 ) * 0.5;

      cFit->SetParameter( 0 , cMiddle);
      cFit->SetParameter( 1 , cWidth );

      cScurve->Fit( cFit , "RNQ+" );

      // Drawing left panel
      cCanvas->cd(1);
      cScurve->Draw("hist p");
      cFit->Draw("same");
      
      uint32_t cMiddleFit = cFit->GetX( 0.5 , cFirst1 , cFirstNon0 );
      cMidPoints.push_back(cMiddleFit);
      //std::cout<<"midplot: "<<cMiddleFit<<std::endl;

      cCanvas->Update();
    }
  return cMidPoints;
}


vector1D DelayScan::EventAveraging(vector2D EventRes)
{
  vector1D EventAvg;
  for (auto& cCbc : fFe->fCbcVector)
    {
      uint32_t cCbcId = cCbc->getCbcId();
      double sum = 0;
      for (int ii=0; ii<fNumEvents; ii++)
	sum += EventRes[ii][cCbcId];
      //std::cout<<sum/fNumEvents<<std::endl;
      EventAvg.push_back(sum/fNumEvents);
    }
  return EventAvg;
}
 

void DelayScan::setDelayAndTestGroup( uint32_t pDelay )
{
  uint8_t cCoarseDelay = floor( pDelay  / 25 );
  uint8_t cFineDelay = ( cCoarseDelay * 25 ) + 24 - pDelay;

  std::cout << "Current Time: " << pDelay<< std::endl;
  BeBoardRegWriter cBeBoardWriter( fBeBoardInterface, DELAY_AF_TEST_PULSE, cCoarseDelay );
  this->accept( cBeBoardWriter );
  CbcRegWriter cWriter( fCbcInterface, "SelTestPulseDel&ChanGroup", to_reg( cFineDelay, fTestGroup ) );
  this->accept( cWriter );
}


void DelayScan::parseSettings()
{
  // now read the settings from the map
  auto cSetting = fSettingsMap.find( "Nevents" );
  if ( cSetting != std::end( fSettingsMap ) ) fNumEvents = cSetting->second;
  else fNumEvents = 200;

  cSetting = fSettingsMap.find( "Vplus" );
  if ( cSetting != std::end( fSettingsMap ) )  fVplus = cSetting->second;
  else fVplus = 0x6F;
  cSetting = fSettingsMap.find( "ChannelOffset" );
  if ( cSetting != std::end( fSettingsMap ) ) fOffset = cSetting->second;
  else fOffset = 0x05;
  cSetting = fSettingsMap.find( "Channel" );
  if ( cSetting != std::end( fSettingsMap ) ) fChannel = cSetting->second;
  else fChannel = 9;

  cSetting = fSettingsMap.find( "AmpMax" );
  if ( cSetting != std::end( fSettingsMap ) ) fAmpMax = cSetting->second;
  else fAmpMax = 10;
  cSetting = fSettingsMap.find( "AmpMin" );
  if ( cSetting != std::end( fSettingsMap ) ) fAmpMin = cSetting->second;
  else fAmpMin = 5;
  cSetting = fSettingsMap.find( "AmpStep" );
  if ( cSetting != std::end( fSettingsMap ) ) fAmpStep = cSetting->second;
  else fAmpStep = 1;

  cSetting = fSettingsMap.find( "DelayMax" );
  if ( cSetting != std::end( fSettingsMap ) ) fDelayMax = cSetting->second;
  else fDelayMax = 25;
  cSetting = fSettingsMap.find( "DelayMin" );
  if ( cSetting != std::end( fSettingsMap ) ) fDelayMin = cSetting->second;
  else fDelayMin = 0;

  std::cout << "Parsed the following settings:" << std::endl;
  std::cout << "	Nevents    = " << fNumEvents << std::endl;
  std::cout << "	Vplus      = " << int( fVplus ) << std::endl;
  std::cout << "	Channel    = " << int( fChannel ) << std::endl;
  std::cout << "        AmpMax     = " << int( fAmpMax ) << std::endl;
  std::cout << "        AmpMin     = " << int( fAmpMin ) << std::endl;
}


void DelayScan::setSystemTestPulse( uint8_t pTPAmplitude, uint8_t pChannelId )
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


int DelayScan::findTestGroup( uint32_t pChannelId )
{
  int cGrp = -1;
  for ( int cChIndex = 0; cChIndex < 16; cChIndex++ )
    {
      uint32_t cResult = pChannelId / 2 - cChIndex * 8;
      if ( cResult < 8 ) cGrp = cResult;
    }
  return cGrp;
}
