#ifndef PulseNoiseStudy_h__
#define PulseNoiseStudy_h__

#include "Tool.h"
#include "Channel.h"
#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/CommonVisitors.h"
#include <map>
#include "TCanvas.h"
#include "TProfile.h"
#include "TString.h"
#include "Channel.h"
#include "TMath.h"
#include "TH1F.h"

typedef std::map<Cbc*,TCanvas*> CanvasMap;
typedef std::map<Cbc*,std::map<uint32_t,TH1F*> > ScurveMap; // indices: Cbc, Amplitude
typedef std::map<Cbc*,TH1F*> HistMap;
typedef std::vector<std::vector<double>> vector2D;
typedef std::vector<double> vector1D;
typedef std::map<uint32_t,std::vector<double>> DataMap; 

class PulseNoiseStudy : public Tool
{
public:
    PulseNoiseStudy();
    
    // methods
    void Initialize();
    void ParseSettings();
    void ScanAmplitudes();
    vector1D findPulseMax( uint32_t fAmpMin );
    vector1D ScanVCth( uint32_t cAmp );
    vector1D MakeScurve( DataMap cDataMap , uint32_t cAmp , uint32_t cVCthMax );
    void setSystemTestPulse( uint32_t cAmp );
    void setDelayAndTestGroup(uint32_t cDelay);
    vector1D EventAveraging( vector2D EventRes );
    int findTestGroup();
    vector1D ScanVCthEff( uint32_t cAmp );
    
    // global
    Shelve*  fShelve;
    BeBoard* fBoard ;
    Module*  fFe    ;
    
    uint32_t fNumEvents, fVplus, fChannel, fAmpMax, fAmpMin, fAmpStep;
    uint32_t pDelayMin, pDelayMax;
    int fTestGroup;
    
    CanvasMap fCanvasMap;
    ScurveMap fScurveMap;
    HistMap   fHistMap;

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
