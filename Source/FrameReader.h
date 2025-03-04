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

#ifndef __FRAMEREADER_H__
#define __FRAMEREADER_H__

#include <DataThreadHeaders.h>

#include "OnixDevice.h"
#include "oni.h"

class FrameReader : public Thread
{
public:
	FrameReader(std::vector<std::shared_ptr<OnixDevice>> sources_, oni_ctx& ctx_);

	~FrameReader();

	void run() override;

private:

	std::vector<std::shared_ptr<OnixDevice>> sources;
	oni_ctx& ctx;

	JUCE_LEAK_DETECTOR(FrameReader);
};

#endif // __FRAMEREADER_H__
