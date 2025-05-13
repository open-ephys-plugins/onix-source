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

#include "UI/InterfaceList.h"
#include "UI/CustomTabComponent.h"

namespace OnixSourcePlugin
{
	class OnixSource;
	class OnixSourceEditor;

	/**
		Holds the visualizer for additional probe settings
	*/
	class OnixSourceCanvas : public Visualizer
	{
	public:
		/** Constructor */
		OnixSourceCanvas(GenericProcessor*, OnixSourceEditor*, OnixSource*);

		/** Renders the Visualizer on each animation callback cycle */
		void refresh() override;

		/** Starts animation (not needed for this component) */
		void beginAnimation() override {}

		/** Stops animation (not needed for this component) */
		void endAnimation() override {}

		/** Called when the Visualizer's tab becomes visible after being hidden */
		void refreshState() override;

		/** Removes tabs from the canvas at the specified port */
		void removeTabs(PortName port);

		/** Removes all tabs from the canvas */
		void removeAllTabs();

		/** Called when the hardware is connected, to ensure the right tabs are present */
		void refreshTabs();

		/** Called when the Visualizer is first created, and optionally when
			the parameters of the underlying processor are changed */
		void update() const;

		/** Starts animation of sub-interfaces */
		void startAcquisition();

		/** Stops animation of sub-interfaces */
		void stopAcquisition();

		/** Add the hub and all of its devices to the canvas */
		void addHub(std::string, int);

		/** Called when the basestation is created or refreshed */
		void populateSourceTabs(CustomTabComponent*, OnixDeviceVector);

		/** Saves custom UI settings */
		void saveCustomParametersToXml(XmlElement* xml) override;

		/** Loads custom UI settings*/
		void loadCustomParametersFromXml(XmlElement* xml) override;

		/** Sets bounds of sub-components*/
		void resized();

		Array<CustomTabComponent*> getHubTabs();

		std::map<int, OnixDeviceType> createSelectedMap(std::vector<std::shared_ptr<SettingsInterface>>);

		std::vector<std::shared_ptr<SettingsInterface>> settingsInterfaces;

		void resetContext();

		bool foundInputSource();

		std::shared_ptr<OnixDevice> getDevicePtr(OnixDeviceType, int);

	private:

		OnixSourceEditor* editor;

		OnixSource* source;

		std::unique_ptr<CustomTabComponent> topLevelTabComponent;
		OwnedArray<CustomTabComponent> hubTabs;

		CustomTabComponent* addTopLevelTab(String tabName, int index = -1);

		void addInterfaceToTab(String tabName, CustomTabComponent* tab, std::shared_ptr<SettingsInterface> interface_);

		std::string getTopLevelTabName(PortName port, std::string headstage);

		/**
			Create an alert window that asks whether to keep the selected headstage on the given port,
			or to remove it since the hardware was not found
		*/
		void askKeepRemove(int offset);

		/**
			Create an alert window that asks whether to keep the selected headstage on the given port,
			or to update to the headstage that was found
		*/
		void askKeepUpdate(int offset, std::string foundHeadstage, OnixDeviceVector devices);

		JUCE_LEAK_DETECTOR(OnixSourceCanvas);
	};
}
