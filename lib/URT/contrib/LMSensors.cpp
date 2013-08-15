/* Copyright 2009-2011 Michael Sechooler
 *
 * This file is part of URT.
 * 
 * URT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * URT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with URT.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LMSensors.h"
#include <iostream>
#include <limits>
#include <cstdlib>
#include <sensors/sensors.h>
#include "../State.h"

using namespace urt::contrib;

LMSensors::LMSensors() {
	if(sensors_init(NULL))
		throw Error("cannot initialize libsensors library");

	//find first chip
	int nr = 0;
	chip = sensors_get_detected_chips(NULL, &nr);
	if(!chip)
		throw Error("no sensor chips available");

	//build subfeature map for chip
	nr = 0; 
	const sensors_feature* feat = sensors_get_features(chip, &nr);
	while(feat) {
		const sensors_subfeature* subfeat;
		if((subfeat = sensors_get_subfeature(chip, feat, SENSORS_SUBFEATURE_IN_INPUT)) ||
		   (subfeat = sensors_get_subfeature(chip, feat, SENSORS_SUBFEATURE_FAN_INPUT)) ||
		   (subfeat = sensors_get_subfeature(chip, feat, SENSORS_SUBFEATURE_TEMP_INPUT)) ||
		   (subfeat = sensors_get_subfeature(chip, feat, SENSORS_SUBFEATURE_POWER_AVERAGE)) ||
		   (subfeat = sensors_get_subfeature(chip, feat, SENSORS_SUBFEATURE_ENERGY_INPUT))) {
			char* label = sensors_get_label(chip, feat);
			subfeatureMap[label] = subfeat->number;
			std::free(label);
		}

		feat = sensors_get_features(chip, &nr);
	}
}
LMSensors& LMSensors::getSingleton() {
	static LMSensors lm;
	return lm;
}
LMSensors::~LMSensors() { sensors_cleanup(); }

double LMSensors::getValue(const std::string& name) {
	double value;
	SubfeatureMap::const_iterator i = subfeatureMap.find(name);
	if(i == subfeatureMap.end())
		return std::numeric_limits<double>::quiet_NaN();
	else if(sensors_get_value(chip, i->second, &value) == 0)
		return value;
	else
		return std::numeric_limits<double>::quiet_NaN();
		
}

void LMSensors::LMSensorFunctor::operator()() {
	double value;
	if(sensors_get_value(getSingleton().chip, subfeature, &value) == 0)
		urt::State::set(key, value);
}
LMSensors::LMSensorFunctor LMSensors::substateUpdater(const std::string& subfeatureName, const std::string& substateKey) {
	SubfeatureMap::const_iterator i = subfeatureMap.find(subfeatureName);
	if(i == subfeatureMap.end())
		throw Error("cannot find subfeature by given name");
	return LMSensorFunctor(i->second, substateKey);
}
