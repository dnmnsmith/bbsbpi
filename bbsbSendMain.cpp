#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <utility>
#include <libgen.h>


#include <boost/lexical_cast.hpp>

#include <iostream>
#include <cxxtools/arg.h>

#include "bbsb.h"

// WiringPI output pin, a frankly quite bizarre number.
#define OUTPUT_PIN 2	

char const * program_name = NULL;

void usage();

int main( int argc, char ** argv)
{
 char *progPath = strdup( argv[0] );
 program_name = basename( progPath );

   try
  {
    // read the command line options

      cxxtools::Arg<int> argVerbose(argc,argv,'v',1);
      cxxtools::Arg<std::string> argCommand(argc, argv, 'c', "ON");
      cxxtools::Arg<std::string> argSender(argc, argv, 's', "A" );
      cxxtools::Arg<unsigned int> argReceiver(argc, argv, 'r', 1 );

    if (!(argCommand.isSet() || argSender.isSet() || argReceiver.isSet()))
    {
      std::cerr << "-c -s and -r options must all be set" << std::endl;
      usage();
    }

    if (argReceiver < 1 || argReceiver > 16)
       usage();

    unsigned int receiver = argReceiver;
    receiver -= 1;  

    std::string senderStr = argSender;
    if (senderStr.length() !=1)
	usage();

    if (toupper(senderStr[0]) < 'A' || toupper(senderStr[0]) > 'P')
    {
       std::cerr << "Sender " << senderStr << " out of range" << std::endl;
       usage();
    }

    unsigned int sender = toupper(senderStr[0]) - 'A';

    std::string commandStr = argCommand;
    unsigned int command = 0;

    if ( (commandStr.compare("1") == 0) || 
         (commandStr.compare("on") == 0) ||
         (commandStr.compare("ON") == 0) || 
         (commandStr.compare("On") == 0) )
	command = 1;
    else if ( (commandStr.compare("0") == 0) || 
              (commandStr.compare("off") == 0) ||
              (commandStr.compare("OFF") == 0) || 
              (commandStr.compare("Off") == 0)  )
	command = 0;
    else
    {
        std::cerr << "Command " << commandStr << " out of range" << std::endl;
        usage();
    }

    ByeByeStandby bbsbSender;

    bbsbSender.init( OUTPUT_PIN );

    if( argVerbose.isSet() )
    {
        std::cout << "Sender = " << sender << " receiver = " << receiver << " command = " << command << std::endl;
    }
    bbsbSender.sendSimpleProtocolMessage( sender, receiver, command !=0 );
  }
 catch (const std::exception& e)
 {
   std::cerr << e.what() << std::endl;
 }

 return 0;
}

void usage()
{
  std::cerr << "BBSB sender application" << std::endl;
  std::cerr << "Usage : "<< program_name << " -s <A-P> -r <1-16> -c <0|1|on|off> [-v]" << std::endl;
  std::cerr << "    -v                : Set verbose mode" << std::endl;
  std::cerr << "    -s <A-P>          : Sender, sender code is character A through P" << std::endl;
  std::cerr << "    -r <1-16>         : Receiver, receiver number is 1 through 16" << std::endl;
  std::cerr << "    -c <0|1|on|off>   : Command" << std::endl;

  exit( 1 );
}
