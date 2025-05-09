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

namespace OnixSourcePlugin
{
	/**
		TabBarButton with custom appearance
	*/
	class CustomTabButton : public TabBarButton
	{
	public:
		/** Constructor */
		CustomTabButton(const String& name, TabbedComponent* parent, bool isTopLevel_) :
			TabBarButton(name, parent->getTabbedButtonBar()),
			isTopLevel(isTopLevel_)
		{
		}

		/** Paints the button */
		void paintButton(Graphics& g, bool isMouseOver, bool isMouseDown) override
		{
			getTabbedButtonBar().setTabBackgroundColour(getIndex(), Colours::grey);
			getLookAndFeel().drawTabButton(*this, g, isMouseOver, isMouseDown);
		}

	private:
		bool isTopLevel;
	};
}
