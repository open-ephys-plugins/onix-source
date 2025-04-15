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

#include <VisualizerEditorHeaders.h>
#include "CustomTabButton.h"

/**
	Adds a callback when tab is changed
*/
class CustomTabComponent : public TabbedComponent
{
public:
	CustomTabComponent(bool isTopLevel_) :
		TabbedComponent(TabbedButtonBar::TabsAtTop),
		isTopLevel(isTopLevel_)
	{
		setTabBarDepth(26);
		setOutline(0);
		setIndent(0);
	}

	TabBarButton* createTabButton(const juce::String& name, int index) override
	{
		return new CustomTabButton(name, this, isTopLevel);
	}

private:
	bool isTopLevel;
};
