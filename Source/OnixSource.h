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

#include <oni.h>
#include <onix.h>

#include <DataThreadHeaders.h>

#include "OnixDevice.h"
#include "OnixSourceEditor.h"
#include "Devices/DeviceList.h"
#include "FrameReader.h"
#include "PortController.h"

class Onix1
{
public:
	Onix1()
	{
		LOGD("ONIX Source creating ONI context.");
		ctx = oni_create_ctx("riffa");
		if (ctx == NULL) { LOGE("Failed to create context."); return; }

		int errorCode = oni_init_ctx(ctx, 0);

		if (errorCode) 
		{ 
			LOGE(oni_error_str(errorCode));
			ctx = NULL;
			return; 
		}
	};

	~Onix1()
	{ 
		oni_destroy_ctx(ctx);
	};

	bool isInitialized() const { return ctx != NULL; }

	oni_ctx get() const { return ctx; }

private:

	/** The ONI context object */
	oni_ctx ctx;
};

/**

	@see DataThread, SourceNode

*/

class OnixSource : public DataThread
{
public:

	/** Constructor */
	OnixSource(SourceNode* sn);

	/** Destructor */
	~OnixSource()
	{
		if (context.isInitialized())
		{
			portA.setVoltage(context.get(), 0.0f);
			portB.setVoltage(context.get(), 0.0f);
		}
	}

	/** Static method to create DataThread */
	static DataThread* createDataThread(SourceNode* sn);

	/** Creates the custom editor */
	std::unique_ptr<GenericEditor> createEditor(SourceNode* sn) override;

	/** Reads frames and adds them to the appropriate device */
	bool updateBuffer() override;

	/** Returns true if the processor is ready to stream data, including all hardware initialization. Returns false if not ready. */
	bool isReady() override;

	/** Returns true if the hardware is connected, false otherwise.*/
	bool foundInputSource();

	/** Initializes data transfer.*/
	bool startAcquisition();

	/** Stops data transfer.*/
	bool stopAcquisition();

	void updateDiscoveryParameters(PortName port, DiscoveryParameters parameters);

	/** Takes a string from the editor. Can be an empty string to allow for automated discovery */
	bool configurePortVoltage(PortName port, String voltage) const;

	/** Sets the port voltage */
	bool setPortVoltage(PortName port, float voltage) const;

	void initializeDevices(bool updateStreamInfo = false);

	void disconnectDevices(bool updateStreamInfo = false);

	std::vector<std::shared_ptr<OnixDevice>> getDataSources();

	void updateSourceBuffers();

	// DataThread Methods
	void updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
		OwnedArray<EventChannel>* eventChannels,
		OwnedArray<SpikeChannel>* spikeChannels,
		OwnedArray<DataStream>* sourceStreams,
		OwnedArray<DeviceInfo>* devices,
		OwnedArray<ConfigurationObject>* configurationObjects);

private:

	/** Available data sources */
	std::vector<std::shared_ptr<OnixDevice>> sources;

	/** Pointer to the editor */
	OnixSourceEditor* editor;

	/** Thread that reads frames */
	std::unique_ptr<FrameReader> frameReader;

	Onix1 context;

	PortController portA = PortController(PortName::PortA);
	PortController portB = PortController(PortName::PortB);

	const oni_size_t block_read_size = 2048;

	bool devicesFound = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnixSource);
};

#endif  // __OnixSource_H__
