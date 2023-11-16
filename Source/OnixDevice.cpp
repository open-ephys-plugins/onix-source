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

#include "OnixDevice.h"

OnixDevice::OnixDevice(String name, int channels_, float sampleRate_, OnixDeviceType type_)
	: Thread(name), numChannels(channels_), sampleRate(sampleRate_), type(type_)
{

}


void OnixDevice::run()
{

	int64 sampleNumber = 0;
	int64 uSecPerBuffer = samplesPerBuffer / sampleRate * 1e6;
	eventCode = 0;

	int64 start = Time::getHighResolutionTicks();
	int64 bufferCount = 0;

	while (!threadShouldExit())
	{

		bufferCount++;

		// grab available data and add to buffer

		/*for (int sample_num = 0; sample_num < samplesPerBuffer; sample_num++)
		{

			for (int i = 0; i < numChannels; i++)
			{
				samples[i + sample_num * numChannels] = (*data)[sampleNumber % availableSamples];
			}

			sampleNumbers[sample_num] = sampleNumber++;
			timestamps[sample_num] = -1.0;
			
			if (sampleNumber % int(sampleRate) == 0)
			{
				if (eventCode == 0)
					eventCode = 1;
				else
					eventCode = 0;
			}

			event_codes[sample_num] = eventCode;
		}

		buffer->addToBuffer(samples, sampleNumbers, timestamps, event_codes, samplesPerBuffer);

		int64 uSecElapsed = int64(Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks() - start) * 1e6);

		if (uSecElapsed < (uSecPerBuffer * bufferCount))
		{
			std::this_thread::sleep_for(std::chrono::microseconds((uSecPerBuffer * bufferCount) - uSecElapsed));
		}*/
	}
}