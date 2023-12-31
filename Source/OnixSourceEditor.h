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

#ifndef __OnixSourceEditor_H__
#define __OnixSourceEditor_H__

#include <EditorHeaders.h>

#include "OnixSource.h"

/** 

	Used to change the number of data streams, and the
	number of channels per stream.

*/
class OnixSourceEditor : public GenericEditor
{
public:

	/** Constructor */
	OnixSourceEditor(GenericProcessor* parentNode, OnixSource* thread);

	/** Destructor */
	virtual ~OnixSourceEditor() { }

    /** Called when settings are changed */
    void updateSettings() override;

	/** Called at start of acquisition */
	void startAcquisition() override;

	/** Called when acquisition finishes */
	void stopAcquisition() override;

private:

	OnixSource* thread;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnixSourceEditor);

};

#endif // __OnixSourceEditor_H__
