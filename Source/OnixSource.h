/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2023 Allen Institute for Brain Science and Open Ephys

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


#ifndef __OnixSource_H__
#define __OnixSource_H__

#include <stdio.h>
#include <string.h>

#include <oni.h>

#include <DataThreadHeaders.h>

#include "OnixDevice.h"

class OnixSourceEditor;

/**

	Simulates multiple data streams, for testing purposes.

	@see DataThread, SourceNode

*/

class OnixSource : public DataThread
{

public:

	/** Constructor */
	OnixSource(SourceNode* sn) : DataThread(sn) 
	{ 
		ctx = NULL;
	}

	/** Destructor */
	~OnixSource() 
	{
		if (ctx != NULL)
			oni_destroy_ctx(ctx);
	}

	/** Static method to create DataThread */
	static DataThread* createDataThread(SourceNode* sn);

	/** Creates the custom editor */
	std::unique_ptr<GenericEditor> createEditor(SourceNode* sn);

	/** Not used -- data buffers are updated for each source stream */
	bool updateBuffer() { return true;  }

	/** Returns true if the hardware is connected, false otherwise.*/
	bool foundInputSource();

	/** Initializes data transfer.*/
	bool startAcquisition();

	/** Stops data transfer.*/
	bool stopAcquisition();

	// DataThread Methods
	void updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
		OwnedArray<EventChannel>* eventChannels,
		OwnedArray<SpikeChannel>* spikeChannels,
		OwnedArray<DataStream>* sourceStreams,
		OwnedArray<DeviceInfo>* devices,
		OwnedArray<ConfigurationObject>* configurationObjects);

	/** Available data sources */
	OwnedArray<OnixDevice> sources;

	/** Pointer to the editor */
	OnixSourceEditor* ed;

	/** The ONI context object */
	oni_ctx ctx;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnixSource);

};





#endif  // __OnixSource_H__
