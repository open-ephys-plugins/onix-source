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

#include "FrameReader.h"

FrameReader::FrameReader(OnixDeviceVector sources_, std::shared_ptr<Onix1> ctx_)
	: Thread("FrameReader")
{
	sources = sources_;
	context = ctx_;
}

void FrameReader::run()
{
	while (!threadShouldExit())
	{
		oni_frame_t* frame = context->readFrame();

		if (context->getLastResult() < ONI_ESUCCESS)
		{
			if (threadShouldExit()) return;

			CoreServices::sendStatusMessage("Unable to read data frames. Stopping acquisition...");
			CoreServices::setAcquisitionStatus(false);
			return;
		}

		bool destroyFrame = true;

		for (const auto& source : sources)
		{
			if (frame->dev_idx == source->getDeviceIdx())
			{
				source->addFrame(frame);
				destroyFrame = false;
				break;
			}
		}

		if (destroyFrame)
		{
			oni_destroy_frame(frame);
		}
	}
}
