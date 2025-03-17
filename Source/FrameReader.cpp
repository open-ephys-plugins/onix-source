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

FrameReader::FrameReader(Array<OnixDevice*> sources_, oni_ctx ctx_)
	: Thread("FrameReader"),
	sources(sources_),
	ctx(ctx_)
{
}

FrameReader::~FrameReader()
{
}

void FrameReader::run()
{
	if (ctx == NULL)
	{
		LOGE("Context is not initialized, no data will be read from the hardware.");
		return;
	}

	while (!threadShouldExit())
	{
		oni_frame_t* frame;

		int res = oni_read_frame(ctx, &frame);

		if (res < ONI_ESUCCESS)
		{
			LOGE("Error reading ONI frame: ", oni_error_str(res), " code ", res);
			return;
		}

		bool destroyFrame = true;

		for (auto source : sources)
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
