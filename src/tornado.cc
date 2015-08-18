#include <cstring>
#include "../tools/TornadoPlot.h"
#include <TApplication.h>
#include "../Utils/argvparser.h"
#include "TROOT.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

int main( int argc, char* argv[] )
{
  TornadoPlot cTornadoPlot;
  ArgvParser cmd;
  
  // init
  cmd.setIntroductoryDescription( "CMS:Ph2_ACF tornado tool can be used to decimate small towns in Oklahoma." );
  // options
  cmd.setHelpOption( "h", "help", "Print this help page" );
  cmd.defineOption( "channel", "Scan the channel", ArgvParser::OptionRequiresValue );
  cmd.defineOptionAlternative( "channel", "c" );

  int result = cmd.parse( argc, argv );
  if ( result != ArgvParser::NoParserError )
    {
      std::cout << cmd.parseErrorDescription( result );
      exit( 1 );
    }
  
  // now query the parsing results
  std::string cHWFile = "settings/DelayTest.xml";
  uint8_t cSelectedChannel = ( cmd.foundOption( "channel" ) ) ? convertAnyInt( cmd.optionValue( "channel" ).c_str() ) : 1;

  TApplication cApp( "Root Application", &argc, argv );
  TQObject::Connect( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

  // from System Controller
  cTornadoPlot.InitializeHw( cHWFile );
  cTornadoPlot.InitializeSettings( cHWFile );
  cTornadoPlot.Initialize();
  cTornadoPlot.ConfigureHw();

  cTornadoPlot.ScanTestPulseDelay(5000);

  cApp.Run();
  return 0;
}

