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

#ifndef __OnixDevice_H__
#define __OnixDevice_H__

#include <DataThreadHeaders.h>

#include <ctime>
#include <ratio>
#include <chrono>
#include <thread>

#include <oni.h>

#include "I2CRegisterContext.h"

using namespace std::chrono;

namespace Onix
{
	enum PortName
	{
		PortA = 1,
		PortB = 2
	};

	enum OnixDeviceType {
		HS64,
		BNO,
		NEUROPIXELS_1,
		NEUROPIXELS_2,
		ADC
	};

	struct StreamInfo {
		String name;
		String description;
		String identifier;
		int numChannels;
		float sampleRate;
		String channelPrefix;
		ContinuousChannel::Type channelType;
		float bitVolts;
	};

	/**

		Streams data from an ONIX device

	*/
	class OnixDevice : public Thread
	{
	public:

		/** Constructor */
		OnixDevice(String name, OnixDeviceType type, const oni_dev_idx_t, const oni_ctx);

		/** Destructor */
		~OnixDevice() { }

		virtual void addFrame(oni_frame_t*) = 0;

		const String getName() { return name; }

		virtual int enableDevice() = 0;

		virtual void startAcquisition() = 0;

		virtual void stopAcquisition() = 0;

		const oni_dev_idx_t getDeviceIdx() const { return deviceIdx; }

		OnixDeviceType type;

		/** Holds incoming data */
		DataBuffer* deviceBuffer;

		Array<StreamInfo> streams;

		int checkLinkState(oni_dev_idx_t port);

	protected:

		const oni_dev_idx_t deviceIdx;
		const oni_ctx ctx;

	private:

		std::vector<float>* data;
		int availableSamples;
		int samplesPerBuffer;

		int64 numSamples;
		uint64 eventCode;

		String name;
	};
}

#endif