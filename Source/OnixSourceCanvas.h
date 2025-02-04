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

# ifndef __ONIXSOURCECANVAS_H__
# define __ONIXSOURCECANVAS_H__

#include <VisualizerEditorHeaders.h>

#include "UI/InterfaceList.h"

#include "OnixSourceEditor.h"
#include "OnixSource.h"

/**

	TabBarButton with custom appearance

*/
class CustomTabButton : public TabBarButton
{
public:
	/** Constructor */
	CustomTabButton(const String& name, TabbedComponent* parent, bool isTopLevel_);

	/** Paints the button */
	void paintButton(Graphics& g, bool isMouseOver, bool isMouseDown) override;

private:
	bool isTopLevel;
};

/**

	Adds a callback when tab is changed

*/

class CustomTabComponent : public TabbedComponent
{
public:
	/** Constructor */
	CustomTabComponent(OnixSourceEditor* editor_, bool isTopLevel_);

	/** Create tab buttons*/
	TabBarButton* createTabButton(const juce::String& name, int index) override
	{
		return new CustomTabButton(name, this, isTopLevel);
	}

private:
	OnixSourceEditor* editor;
	bool isTopLevel;

	JUCE_LEAK_DETECTOR(CustomTabComponent);
};

/**

	Holds the visualizer for additional probe settings

*/
class OnixSourceCanvas : public Visualizer
{
public:
	/** Constructor */
	OnixSourceCanvas(GenericProcessor*, OnixSourceEditor*, OnixSource*);

	/** Destructor */
	~OnixSourceCanvas();

	/** Fills background */
	void paint(Graphics& g);

	/** Renders the Visualizer on each animation callback cycle */
	void refresh() override;

	/** Starts animation (not needed for this component) */
	void beginAnimation() override {}

	/** Stops animation (not needed for this component) */
	void endAnimation() override {}

	/** Called when the Visualizer's tab becomes visible after being hidden */
	void refreshState() override;

	/** Called when tabs need to be removed */
	void removeTabs();

	/** Called when the number of tabs might have changed, so they are refreshed */
	void refreshTabs();

	/** Called when the Visualizer is first created, and optionally when
		the parameters of the underlying processor are changed */
	void update() const;

	/** Starts animation of sub-interfaces */
	void startAcquisition();

	/** Stops animation of sub-interfaces */
	void stopAcquisition();

	/** Called when the basestation is created or refreshed */
	void populateSourceTabs(CustomTabComponent* basestationTab);

	/** Saves custom UI settings */
	void saveCustomParametersToXml(XmlElement* xml) override;

	/** Loads custom UI settings*/
	void loadCustomParametersFromXml(XmlElement* xml) override;

	/** Sets bounds of sub-components*/
	void resized();

	/** Get the given parameter from the source node */
	Parameter* getSourceParameter(String name);

	/** Creates a custom viewport for the given interface */
	CustomViewport* createCustomViewport(SettingsInterface* settingsInterface);

	OwnedArray<SettingsInterface> settingsInterfaces;

private:

	OnixSourceEditor* editor;

	OnixSource* onixSource;

	std::unique_ptr<CustomTabComponent> topLevelTabComponent;
	OwnedArray<CustomTabComponent> portTabs;

	Array<int> portTabIndex;

	JUCE_LEAK_DETECTOR(OnixSourceCanvas);
};

# endif // __ONIXSOURCECANVAS_H__
