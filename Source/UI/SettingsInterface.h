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

#ifndef __SETTINGSINTERFACE_H__
#define __SETTINGSINTERFACE_H__

#include <VisualizerEditorHeaders.h>

#include "../NeuropixComponents.h"
#include "../OnixDevice.h"

class OnixSourceEditor;
class OnixSourceCanvas;
class SettingsInterface;

/** 

    A viewport with a pointer to the settings interface it holds

*/
class CustomViewport : public Component
{
public:
    CustomViewport (SettingsInterface* settingsInterface_) : settingsInterface (settingsInterface_)
    {
        viewport = std::make_unique<Viewport>();
        viewport->setViewedComponent ((Component*) settingsInterface, false);
        viewport->setScrollBarsShown (true, true);
        viewport->setScrollBarThickness (12);

        addAndMakeVisible (viewport.get());
    }

    ~CustomViewport()
    {
    }

    void resized() override
    {
        viewport->setBounds (getLocalBounds());

        int contentWidth = 1000;

        if (getWidth() > 1012)
            contentWidth = getWidth() - 12;

        viewport->getViewedComponent()->setSize (contentWidth, 820);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::grey);
    }

    SettingsInterface* settingsInterface;

private:
    std::unique_ptr<Viewport> viewport;
};

/** 

	A base class for the graphical interface for updating data source settings

*/
class SettingsInterface : public Component
{
public:
    /** Settings interface type*/
    enum Type
    {
        PROBE_SETTINGS_INTERFACE,
        BNO055_SETTINGS_INTERFACE,
        UNKNOWN_SETTINGS_INTERFACE
    };

    /** Constructor */
    SettingsInterface (OnixDevice* dataSource_, OnixSourceEditor* editor_, OnixSourceCanvas* canvas_)
    {
        dataSource = dataSource_;
        editor = editor_;
        canvas = canvas_;

        setBounds (0, 0, 1000, 820);

        viewport = std::make_unique<CustomViewport> (this);
    }

    /** Destructor */
    ~SettingsInterface() {}

    /** Called when acquisition begins */
    virtual void startAcquisition() = 0;

    /** Called when acquisition ends */
    virtual void stopAcquisition() = 0;

    /** Saves settings */
    virtual void saveParameters (XmlElement* xml) = 0;

    /** Loads settings */
    virtual void loadParameters (XmlElement* xml) = 0;

    /** Updates the string with info about the underlying data source*/
    virtual void updateInfoString() = 0;

    /** Default type */
    Type type = UNKNOWN_SETTINGS_INTERFACE;

    /** Viewport for scrolling around this interface */
    std::unique_ptr<CustomViewport> viewport;

    /** Pointer to the data source*/
    OnixDevice* dataSource;

protected:
    OnixSourceEditor* editor;
    OnixSourceCanvas* canvas;
};

#endif //__SETTINGSINTERFACE_H__
