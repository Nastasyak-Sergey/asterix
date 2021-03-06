/*
 *  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
 *
 *  This file is part of Asterix.
 *
 *  Asterix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Asterix is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * AUTHORS:  Damir Salantic, Croatia Control Ltd.
 * 			 Jurica Baricevic, Croatia Control Ltd.
 *			 Zeljko Harjac, Croatia Control Ltd.
 *			 Darko Markus, Croatia Control Ltd.
 *
 */

#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asterix.h"
#include "version.h"
#include "../engine/converterengine.hxx"
#include "../engine/channelfactory.hxx"

bool gVerbose = false;
bool gForceRouting = false;
int gHeartbeat = 0;
const char* gAsterixDefinitionsFile = NULL;

static void DisplayCopyright()
{
    std::cerr << "Asterix " _VERSION_STR " " __DATE__;
#ifdef _DEBUG
    std::cerr << " DEBUG version";
#endif
    std::cerr << "\n\nCopyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)\nThis program comes with ABSOLUTELY NO WARRANTY.\nThis is free software, and you are welcome to redistribute it\nunder certain conditions. See COPYING file for details.";
}

static void show_usage(std::string name)
{
	DisplayCopyright();

	std::cerr << "\n\nReads and parses ASTERIX data from stdin, file or network multicast stream\nand prints it in textual presentation on standard output.\n\n"
			  << "Usage:\n"
			  <<  name << " [-h] [-v] [-P|-O|-R|-F|-H] [-l|-x] [-d filename] -f filename|-i mcastaddress:ipaddress:port"
			  << "\n\nOptions:"
			  << "\n\t-h,--help\tShow this help message"
			  << "\n\t-v,--verbose\tShow more information during program execution"
			  << "\n\t-P,--pcap\tInput is from PCAP file"
			  << "\n\t-O,--oradis\tAsterix packet is encapsulated in ORADIS packet"
			  << "\n\t-R,--oradispcap\tInput is from PCAP file and\n\t\t\tAsterix packet is encapsulated in ORADIS packet"
			  << "\n\t-F,--final\tAsterix packet is encapsulated in FINAL packet."
			  << "\n\t-H,--hdlc\tAsterix packet is encapsulated in HDLC packet."
			  << "\n\t-l,--line\tOutput will be printed as one line per item. This format is suitable for parsing."
			  << "\n\t-x,--xml\tOutput will be printed in XML format.\n\t\t\tFields that are printed are defined in input\n\t\t\tdefinition file as XIDEF."
			  << "\n\t-d,--def\tXML protocol definitions filenames are listed in\n\t\t\tspecified filename.\n\t\t\tBy default are listed in config/asterix.ini"
			  << "\n\t-f filename\tFile generated from libpcap (tcpdump or Wireshark)\n\t\t\tor file in FINAL or HDLC format.\n\t\t\tFor example: -f filename.pcap"
			  << "\n\t-i m:i:p\tMulticast IP address:Interface address:Port.\n\t\t\tFor example: 232.1.1.12:10.17.58.37:21112"
			  << std::endl;
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
    	show_usage("asterix.exe");
        return 1;
    }

	std::string strDefinitions = "config/asterix.ini";
	std::string strFileInput;
	std::string strIPInput;
	std::string strInputFormat = "ASTERIX_RAW";
	std::string strOutputFormat = "ASTERIX_TXT";

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if ((arg == "-h") || (arg == "--help"))
		{
				show_usage(argv[0]);
				return 0;
		}
		else if ((arg == "-v") || (arg == "--verbose"))
		{
			gVerbose = true;
		}
		else if ((arg == "-P") || (arg == "--pcap"))
		{
			strInputFormat = "ASTERIX_PCAP";
		}
		else if ((arg == "-O") || (arg == "--oradis"))
		{
			strInputFormat = "ASTERIX_ORADIS_RAW";
		}
		else if ((arg == "-R") || (arg == "--oradispcap"))
		{
			strInputFormat = "ASTERIX_ORADIS_PCAP";
		}
		else if ((arg == "-F") || (arg == "--final"))
		{
			strInputFormat = "ASTERIX_FINAL";
		}
		else if ((arg == "-H") || (arg == "--hdlc"))
		{
			strInputFormat = "ASTERIX_HDLC";
		}
		else if ((arg == "-l") || (arg == "--line"))
		{
			strOutputFormat = "ASTERIX_OUT";
		}
		else if ((arg == "-x") || (arg == "--xml"))
		{
			strOutputFormat = "ASTERIX_XIDEF";
		}
		else if ((arg == "-k") || (arg == "--kml"))
		{
			strOutputFormat = "ASTERIX_KML";
		}
		else if ((arg == "-d") || (arg == "--definitions"))
		{
			if (i >= argc-1)
			{
				std::cerr << "Error: " + arg + " option requires one argument." << std::endl;
				return 1;
			}

			strDefinitions = argv[++i];
		}
		else if ((arg == "-f"))
		{
			if (i >= argc-1)
			{
				std::cerr << "Error: " + arg + " option requires one argument." << std::endl;
				return 1;
			}
			strFileInput = argv[++i];
		}
		else if ((arg == "-i"))
		{
			if (i >= argc-1)
			{
				std::cerr << "Error: " + arg + " option requires one argument." << std::endl;
				return 1;
			}
			strIPInput = argv[++i];
		}
	}

	// definitions file
	gAsterixDefinitionsFile = strDefinitions.c_str();

	// check for definitions file
	FILE *tmp = fopen(gAsterixDefinitionsFile, "r");
	if (tmp == NULL)
	{
		std::cerr << "Error: Asterix definitions file " +strDefinitions+ " not found." << std::endl;
		exit (2);
	}
	fclose(tmp);

	// Create input string
	std::string strInput = "std 0 ";
	if (!strFileInput.empty() && !strIPInput.empty())
	{
		strInput = "std 0 ASTERIX_RAW";
	}
	if (!strFileInput.empty())
	{
		strInput = "disk " + strFileInput + ":0:1 ";
	}
	else if (!strIPInput.empty())
	{
		strInput = "udp " + strIPInput + ":S ";
	}

	strInput += strInputFormat;

	// Create output string
	std::string strOutput = "std 0 " + strOutputFormat;

    const char         *inputChannel=NULL;
    const char         *outputChannel[CChannelFactory::MAX_OUTPUT_CHANNELS];
    unsigned int 		chFailover = 0;
    unsigned int        nOutput = 1; // Total number of output channels

	inputChannel = strInput.c_str();
	outputChannel[0] = strOutput.c_str();

    // Print out options
    LOGDEBUG(inputChannel, "Input channel description: %s\n", inputChannel);

    for (unsigned int i=0; i<nOutput; i++)
    {
        LOGDEBUG(outputChannel[i], "Output channel %d description: %s\n", i+1, outputChannel[i]);
    }

    gHeartbeat = abs(gHeartbeat); // ignore negative values
//    LOGDEBUG(1, "Heart-beat: %d\n", gHeartbeat);

    // Finally execute converter engine
    if (CConverterEngine::Instance()->Initialize(inputChannel, outputChannel, nOutput, chFailover))
    {
        CConverterEngine::Instance()->Start();
    }
    else
    {
        LOGERROR(1, "Couldn't initialize asterix engine.\n");
        exit(1);
    }

    CConverterEngine::DeleteInstance();

    exit(0);
}


