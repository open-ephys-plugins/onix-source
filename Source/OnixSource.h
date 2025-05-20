/*
	------------------------------------------------------------------

	Copyright (C) Open Ephys

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

#pragma once

#include <DataThreadHeaders.h>

#include "Onix1.h"
#include "OnixDevice.h"
#include "OnixSourceEditor.h"
#include "FrameReader.h"
#include "Devices/PortController.h"

namespace OnixSourcePlugin
{
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

		void registerParameters() override;

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
		void setPortVoltage(PortName port, double voltage) const;

		double getLastVoltageSet(PortName port);

		void resetContext();

		bool isContextInitialized();

		std::shared_ptr<Onix1> getContext();

		void initializeDevices(bool updateStreamInfo = false);

		void disconnectDevices(bool updateStreamInfo = false);

		OnixDeviceVector getDataSources();

		OnixDeviceVector getEnabledDataSources();

		OnixDeviceVector getDataSourcesFromPort(PortName port);

		OnixDeviceVector getDataSourcesFromOffset(int offset);

		std::shared_ptr<OnixDevice> getDevice(OnixDeviceType type);

		static std::map<int, OnixDeviceType> createDeviceMap(OnixDeviceVector, bool filterDevices = false);

		std::map<int, OnixDeviceType> createDeviceMap(bool filterDevices = false);

		std::map<int, std::string> getHubNames();

		String getLiboniVersion() { if (context != nullptr && context->isInitialized()) return context->getVersion(); else return ""; }

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
		OnixDeviceVector enabledSources;

		/** Available headstages, indexed by their offset value */
		std::map<int, std::string> hubNames;

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

		String createContinuousChannelIdentifier(StreamInfo streamInfo, int channelNumber);

		/** Template method to initialize an OnixDevice and add it to the currently active OnixDeviceVector variable */
		template <class Device>
		static bool configureDevice(OnixDeviceVector&, OnixSourceCanvas*, std::string, std::string, OnixDeviceType, const oni_dev_idx_t, std::shared_ptr<Onix1>);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnixSource);
	};
}
