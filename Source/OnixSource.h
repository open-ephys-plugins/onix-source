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
#include <onix.h>

#include <DataThreadHeaders.h>

#include "OnixDevice.h"
#include "Devices/Neuropixels_1.h"
#include "Devices/Bno055.h"
#include "Devices/HeadStageEEPROM.h"

namespace Onix
{
	class OnixSourceEditor;
	class Neuropixels_1;
	class Bno055;

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
			if (ctx != NULL && contextInitialized)
			{
				oni_destroy_ctx(ctx);
			}
		}

		/** Static method to create DataThread */
		static DataThread* createDataThread(SourceNode* sn);

		/** Creates the custom editor */
		std::unique_ptr<GenericEditor> createEditor(SourceNode* sn);

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

		bool setPortVoltage(oni_dev_idx_t port, int voltage);

		void initializeContext();

		void initializeDevices(bool updateStreamInfo = false);

		// DataThread Methods
		void updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
			OwnedArray<EventChannel>* eventChannels,
			OwnedArray<SpikeChannel>* spikeChannels,
			OwnedArray<DataStream>* sourceStreams,
			OwnedArray<DeviceInfo>* devices,
			OwnedArray<ConfigurationObject>* configurationObjects);

	private:

		/** Available data sources */
		OwnedArray<OnixDevice> sources;

		/** Pointer to the editor */
		OnixSourceEditor* ed;

		/** The ONI context object */
		oni_ctx ctx;

		static const oni_dev_idx_t DEVICE_NPX1_1 = 0x0100;
		static const oni_dev_idx_t DEVICE_NPX1_2 = 0x0101;

		const oni_size_t block_read_size = 2048;

		bool contextInitialized = false;
		bool devicesFound = false;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnixSource);
	};
}

#endif  // __OnixSource_H__
