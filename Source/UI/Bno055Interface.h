/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright (C) 2020 Allen Institute for Brain Science and Open Ephys

	------------------------------------------------------------------

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __BNO055INTERFACE_H__
#define __BNO055INTERFACE_H__

#include <VisualizerEditorHeaders.h>

#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

#include "../Devices/Bno055.h"

class Bno055Interface : public SettingsInterface
{
public:
	/** Constructor */
	Bno055Interface(std::shared_ptr<Bno055> d, OnixSourceEditor* e, OnixSourceCanvas* c);

	/** Destructor */
	~Bno055Interface();

	/** Disables buttons and starts animation if necessary */
	void startAcquisition() override;

	/** Enables buttons and start animation if necessary */
	void stopAcquisition() override;

	/** Save parameters to XML */
	void saveParameters(XmlElement* xml) override;

	/** Load parameters from XML */
	void loadParameters(XmlElement* xml) override;

	/** Updates the info string on the right-hand side of the component */
	void updateInfoString() override;

private:

};

#endif // !__BNO055INTERFACE_H__
