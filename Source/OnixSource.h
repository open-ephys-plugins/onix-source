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

#include <DataThreadHeaders.h>

#include "Onix1.h"
#include "OnixDevice.h"
#include "OnixSourceEditor.h"
#include "Devices/DeviceList.h"
#include "FrameReader.h"

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
		if (context != nullptr && context->isInitialized())
		{
			portA->setVoltageOverride(0.0f, false);
			portB->setVoltageOverride(0.0f, false);
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
	bool foundInputSource() override;

	/** Returns true if the deviceMap matches the settings tabs that are open */
	bool isDevicesReady();

	/** Initializes data transfer.*/
	bool startAcquisition() override;

	/** Stops data transfer.*/
	bool stopAcquisition() override;

	void updateDiscoveryParameters(PortName port, DiscoveryParameters parameters);

	/** Takes a string from the editor. Can be an empty string to allow for automated discovery */
	bool configurePortVoltage(PortName port, String voltage) const;

	/** Sets the port voltage */
	void setPortVoltage(PortName port, float voltage) const;

	void resetContext() { if (context != nullptr && context->isInitialized()) context->issueReset(); }

	bool isContextInitialized() { return context != nullptr && context->isInitialized(); }

	void initializeDevices(bool updateStreamInfo = false);

	void disconnectDevices(bool updateStreamInfo = false);

	OnixDeviceVector getDataSources() const;

	OnixDeviceVector getDataSourcesFromPort(PortName port) const;

	std::map<int, OnixDeviceType> createDeviceMap(OnixDeviceVector);

	std::map<int, OnixDeviceType> createDeviceMap();

	std::map<PortName, String> getHeadstageMap();

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
	OnixDeviceVector sources;

	/** Available headstages */
	std::map<PortName, String> headstages;

	/** Pointer to the editor */
	OnixSourceEditor* editor;

	/** Thread that reads frames */
	std::unique_ptr<FrameReader> frameReader;

	std::shared_ptr<Onix1> context = nullptr;

	std::shared_ptr<PortController> portA;
	std::shared_ptr<PortController> portB;

	const oni_size_t block_read_size = 2048;

	bool devicesFound = false;

	void addIndividualStreams(Array<StreamInfo>, OwnedArray<DataStream>*, OwnedArray<DeviceInfo>*, OwnedArray<ContinuousChannel>*);

	void addCombinedStreams(DataStream::Settings, Array<StreamInfo>, OwnedArray<DataStream>*, OwnedArray<DeviceInfo>*, OwnedArray<ContinuousChannel>*);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnixSource);
};

#endif  // __OnixSource_H__
