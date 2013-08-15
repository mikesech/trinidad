#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "Parameters.h"

static bool parseWaypoint(std::istream& is, Waypoints& w) {
	try {
		Waypoint way;
		is >> way.latitude;
		is >> way.longitude;
		is >> way.hasCone;
		w.push_back(way);
	} catch(...) { return false; }
	return true;
}

bool loadConfiguration(Waypoints& waypoints, int argc, char* argv[]) {
	static const std::string DEFAULT_CONFIG = "~/.trinidad";
	boost::program_options::options_description desc("Allowed parameters");
	desc.add_options()
		("help,h", "produce help message")
		("config,c", boost::program_options::value<std::string>()->implicit_value(DEFAULT_CONFIG), "select configuration file")
	;
	boost::program_options::variables_map vm;
	try {
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);
	} catch(boost::program_options::error& e) {
		std::cerr<<"Invalid invocation: "<<e.what()<<'\n'<<desc<<'\n';
		return false;
	}
	
	if(vm.count("help")) {
		std::cerr<<desc<<'\n';
		return false;
	}
	
	if(vm.count("config")) {
		std::ifstream configFile(vm["config"].as<std::string>().c_str());
		if(!configFile) {
			std::cerr<<"Unable to read configuration file "<<vm["config"].as<std::string>()<<".\n";
			return false;
		} else { 
			while(configFile.good()) {
				if(!parseWaypoint(configFile, waypoints)) {
					std::cerr<<"Invalid syntax in configuration file.\n";
					return false;
				}
			} waypoints.pop_back();
		}
	} else {
		std::cerr<<"Please specifiy waypoint file to load.\n";
		return false;
	}
	
	return true;
}
