#ifndef TornadoPlot_h__
#define TornadoPlot_h__

#include "Tool.h"
#include "../Utils/CommonVisitors.h"
#include <map>
#include "TCanvas.h"
#include "TProfile.h"
#include "TString.h"
#include "Channel.h"
#include "TMath.h"
#include "TH2F.h"
#include "TStyle.h"

typedef std::vector<std::vector<double>>  vector2D;
typedef std::vector<double>   vector1D;

class TornadoPlot : public Tool
{
 public:
  TornadoPlot();

  // methods
  void Initialize();
  void ScanTestPulseDelay(uint32_t cCoarseDelay);
  void ScanTestPulseAmplitude(uint32_t cFineDelay);
  vector1D GetEfficiency(uint32_t cFineDelay , uint32_t cAmp);
  vector1D EventAveraging(vector2D EventRes);
  void setDelayAndTestGroup(uint32_t pDelay);
  void parseSettings();
  void setSystemTestPulse(uint8_t pTPAmplitude , uint8_t pChannelId );
  int findTestGroup( uint32_t pChannelId );

  // global
  Shelve*  fShelve;
  BeBoard* fBoard ;
  Module*  fFe    ;
  //TCanvas* fCanvas;
  int fNumEvents, fVCth, fVplus;
  int fOffset, fChannel, fAmpMax, fAmpMin, fAmpStep, fDelayMin, fDelayMax;
  uint8_t fTestGroup;

  std::map<Cbc*,TH2F*> fHistMap;
  std::map<Module*,TCanvas*> fCanvasMap;

 private:
  unsigned char fLookup[16] =
    {
      0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
      0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
    };
  uint8_t reverse( uint8_t n ) {
    return (fLookup[n&0b1111]<<4) | fLookup[n>>4];
  }

  uint8_t to_reg( uint8_t pDelay, uint8_t pGroup )
  {  
    uint8_t cValue = ((reverse(pDelay))&0xF8) | ( ( reverse( pGroup ) ) >> 5 );
    return cValue;
  }
};
#endif
