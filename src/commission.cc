#include <cstring>

#include "../Utils/Utilities.h"
#include "../tools/Commissioning.h"
#include <TApplication.h>
#include "../Utils/argvparser.h"
#include "TROOT.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

using namespace CommandLineProcessing;


int main( int argc, char* argv[] )
{
	Commissioning cCommissioning;
	ArgvParser cmd;

	// init
	cmd.setIntroductoryDescription( "CMS Ph2_ACF  Commissioning tool to perform the following procedures:\n-Timing / Latency scan\n-Threshold Scan\n-Stub Latency Scan" );
	// error codes
	cmd.addErrorCode( 0, "Success" );
	cmd.addErrorCode( 1, "Error" );
	// options
	cmd.setHelpOption( "h", "help", "Print this help page" );

	cmd.defineOption( "file", "Hw Description File . Default value: settings/Commission_2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "file", "f" );

	cmd.defineOption( "latency", "scan the trigger latency", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "latency", "l" );

	cmd.defineOption( "threshold", "scan the CBC comparator threshold", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "threshold", "t" );

	cmd.defineOption( "stublatency", "scan the stub latency", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "stublatency", "s" );

	cmd.defineOption( "minimum", "minimum value for latency scan", ArgvParser::OptionRequiresValue );
	cmd.defineOptionAlternative( "minimum", "m" );

	cmd.defineOption( "range", "range in clock cycles for latency scan", ArgvParser::OptionRequiresValue );
	cmd.defineOptionAlternative( "range", "r" );

	cmd.defineOption( "pedestal", "scan the CBC pedestals in addition with the threshold scan", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "pedestal", "p" );

	cmd.defineOption( "output", "Output Directory . Default value: Results/", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "output", "o" );

	cmd.defineOption( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "batch", "b" );

	cmd.defineOption( "gui", "option only suitable when launching from gui", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "gui", "g" );

	int result = cmd.parse( argc, argv );
	if ( result != ArgvParser::NoParserError )
	{
		std::cout << cmd.parseErrorDescription( result );
		exit( 1 );
	}

	// now query the parsing results
	std::string cHWFile = ( cmd.foundOption( "file" ) ) ? cmd.optionValue( "file" ) : "settings/Commissioning.xml";
	bool cLatency = ( cmd.foundOption( "latency" ) ) ? true : false;
	bool cStubLatency = ( cmd.foundOption( "stublatency" ) ) ? true : false;
	bool cThreshold = ( cmd.foundOption( "threshold" ) ) ? true : false;
	bool cScanPedestal = ( cmd.foundOption( "pedestal" ) ) ? true : false;
	std::string cDirectory = ( cmd.foundOption( "output" ) ) ? cmd.optionValue( "output" ) : "Results/";
	cDirectory += "Commissioning";
	bool batchMode = ( cmd.foundOption( "batch" ) ) ? true : false;
	bool gui = ( cmd.foundOption( "gui" ) ) ? true : false;

	uint8_t cStartLatency = ( cmd.foundOption( "minimum" ) ) ? convertAnyInt( cmd.optionValue( "minimum" ).c_str() ) :  0;
	uint8_t cLatencyRange = ( cmd.foundOption( "range" ) ) ?  convertAnyInt( cmd.optionValue( "range" ).c_str() ) :  10;
   
	uint8_t puppies = 100;
	int pp =10000;
	std::cout<<"minimum:  "<<int(cStartLatency)<<std::endl;
	std::cout<<"range:    "<<int(cLatencyRange)<<std::endl;
	std::cout<<"test:     "<<int(puppies)<<std::endl;
	std::cout<<"anothertest"<<pp<<std::endl;
	//std::cout<<"cmd.foundoption( minimum): "<<cmd.foundoption( "minimum")<<std::endl;
	//std::cout<<"cmd.foundoption( range):   "<<cmd.foundoption( "range")<<std::endl;
	TApplication cApp( "Root Application", &argc, argv );
	if ( batchMode ) gROOT->SetBatch( true );
	else TQObject::Connect( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

	cCommissioning.InitializeHw( cHWFile );
	cCommissioning.InitializeSettings( cHWFile );
	cCommissioning.Initialize( );
	cCommissioning.CreateResultDirectory( cDirectory );
	std::string cResultfile;
	if ( cLatency || cStubLatency ) cResultfile = "Latency";
	else if ( cThreshold ) cResultfile = "Threshold";
	else cResultfile = "Commissioning";
	cCommissioning.InitResultFile( cResultfile );
	//if ( !gui ) cCommissioning.ConfigureHw();
        std::cout<<"minimum:  "<<int(cStartLatency)<<std::endl;
	std::cout<<"range:    "<<int(cLatencyRange)<<std::endl;
	// Here comes our Part:
	std::cout<<"our part"<<std::endl;
	if ( cLatency ) cCommissioning.ScanLatency( cStartLatency, cLatencyRange );
	if ( cStubLatency ) cCommissioning.ScanStubLatency( cStartLatency, cLatencyRange );
	if ( cThreshold ) cCommissioning.ScanThreshold( cScanPedestal );
	cCommissioning.SaveResults();


	if ( !batchMode ) cApp.Run();

	return 0;

}

