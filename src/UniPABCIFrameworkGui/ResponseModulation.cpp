/*****************************************************************************
* 
* UniPA BCI Framework: an Augmented BCI Framework
* 
* Copyright (C) 2016-2017 Salvatore La Bua (slabua@gmail.com)
* RoboticsLab, Department of Industrial and Digital Innovation (DIID),
* Universita' degli Studi di Palermo, V. delle Scienze, Palermo, Italy.
* 
* http://www.slblabs.com/projects/unipa-bci-framework
* https://github.com/slabua/UniPABCIFramework
* 
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* 
*****************************************************************************/

#include "ResponseModulation.h"
#include "rootpaths.h"
#include "pugixml.hpp"
#include <iostream>
#include <ctime>

using std::cout;
using std::endl;
using std::string;

const char* pathToResultsFile;


ResponseModulation::ResponseModulation(void)
{
}


ResponseModulation::~ResponseModulation(void)
{
}

void ResponseModulation::readConfiguration() {

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file((configFilesRoot + "/configuration.xml").c_str());
	
	if (result) {
		bioHybridMode		= (bool) atoi(doc.child("biohybridmode").first_child().value());	// SLB
		flash_analysis		= atoi(doc.child("flash_analysis").first_child().value());
		selectedID			= atoi(doc.child("selectedID").first_child().value());

		pugi::xml_node tmp	= doc.child("tobiiresultIDs");
		tobiiresultIDs.clear();
		for (pugi::xml_node tool = tmp.child("resultID"); tool; tool = tool.next_sibling("resultID")) {
			tobiiresultIDs.push_back(atoi(tool.first_child().value()));
		}

		epy_min			= atof(doc.child("epy_min").first_child().value());
		epy_max			= atof(doc.child("epy_max").first_child().value());
		egy_min			= atof(doc.child("egy_min").first_child().value());
		egy_max			= atof(doc.child("egy_max").first_child().value());
		epy_norm			= atof(doc.child("epy_norm").first_child().value());	// focus = 1-stress
		egy_norm			= atof(doc.child("egy_norm").first_child().value());	// attention

		rsq_min			= atof(doc.child("rsq_min").first_child().value());
		rsq_max			= atof(doc.child("rsq_max").first_child().value());
		rsq_norm			= atof(doc.child("rsq_norm").first_child().value());	// intention

		username			= doc.child("name").first_child().value();
		binfilename			= doc.child("binfilename").first_child().value();
		flash_precision		= atof(doc.child("flash_precision").first_child().value());			// [0-100]
		tobiiresultPC		= atof(doc.child("tobiiresultPC").first_child().value()) * 100;		// [0-100]

	}
	else
		cout << "Could not find configuration file." << endl;

}

void ResponseModulation::updateConfiguration() {

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file((configFilesRoot + "/configuration.xml").c_str());
	if (result) {
		if (bioHybridMode) {
			pugi::xml_node node = doc.child("threshold_passed");
			std::string threshold_passed = (activation_threshold) ? "1" : "0";
			node.first_child().set_value(threshold_passed.c_str());

			node = doc.child("behaviour_intensity");
			node.first_child().set_value(std::to_string(behaviour_intensity).c_str());
		}
		else {
			pugi::xml_node node = doc.child("threshold_passed");
			node.first_child().set_value("-1");

			node = doc.child("behaviour_intensity");
			node.first_child().set_value("-1");
		}
		doc.save_file((configFilesRoot + "/configuration.xml").c_str());
	}
	else
		cout << "Could not find configuration file." << endl;
}

bool ResponseModulation::isCalibration() {

	readConfiguration();
	string calibteststring = "Calib";
	
	return binfilename.find(calibteststring) != std::string::npos;

}

string ResponseModulation::getResult() {

	string result = "";
	
	if (bioHybridMode) {
		result += "\nR^2\tEntropy\tEnergy\n";
		result += std::to_string(rsq_norm);
		result += "\t";
		result += std::to_string(epy_norm);
		result += "\t";
		result += std::to_string(egy_norm);
		result += "\n";
	}

	if (!isCalibration()) {
		result += "\nAction Selected: "; // SLB TODO Print only if in a spelling session
		result += std::to_string(selectedID);
		result += ".\n";
	}

	if (bioHybridMode) {
		result += "\nBehaviour Intensity: ";
		result += std::to_string(behaviour_intensity);
		result += ".\n";
	}

	return result;

}

void ResponseModulation::writeResult() {

	pathToResults		= outputFilesRoot + "/" + username + "_response.txt";
	pathToResultsFile	= pathToResults.c_str();
	time_t now			= time(NULL);
	char* timeNow		= new char[20];
	strftime(timeNow, 80, "%Y_%m_%d_%H_%M_%S", gmtime(&now));
	string timestring(timeNow);

	string result = "";
	result += timestring;
	result += "\t";
	result += binfilename;
	result += "\t";
	result += std::to_string(selectedID);
	result += "\t";
	result += std::to_string(flash_precision);
	result += "\t";
	result += std::to_string(tobiiresultPC);

	if (bioHybridMode) {
		result += "\t";
		result += std::to_string(rsq_norm);
		result += "\t";
		result += std::to_string(epy_norm);
		result += "\t";
		result += std::to_string(egy_norm);
		result += "\t";
		result += std::to_string(behaviour_intensity);

		result += "\t";
		result += std::to_string(rsq_min);
		result += "\t";
		result += std::to_string(rsq_max);
		result += "\t";
		result += std::to_string(epy_min);
		result += "\t";
		result += std::to_string(epy_max);
		result += "\t";
		result += std::to_string(egy_min);
		result += "\t";
		result += std::to_string(egy_max);
	}
	//result += "\n";

	resultsFile.open(pathToResultsFile, std::ios_base::app | std::ios_base::out);

	resultsFile << result << endl;

	resultsFile.flush();
	resultsFile.close();

}

void ResponseModulation::thresholdModule() {

	if (bioHybridMode) {
		if (rsq_norm < ACTIVATION_THRESHOLD)
			cout << "The R^2 value is below the activation threshold." << endl;

		activation_threshold = (rsq_norm >= ACTIVATION_THRESHOLD);
	}
}

void ResponseModulation::modulationModule() {

	if (bioHybridMode)
		behaviour_intensity = (rsq_norm + epy_norm + egy_norm)  / 3;

}

void ResponseModulation::Process() {

	readConfiguration();

	thresholdModule();
	modulationModule();

	updateConfiguration();

	writeResult();

}

