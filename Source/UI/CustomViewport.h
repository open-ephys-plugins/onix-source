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

#include "SettingsInterface.h"

namespace OnixSourcePlugin
{
/**

    A viewport with a pointer to the settings interface it holds

*/
class CustomViewport : public Component
{
public:
    CustomViewport (SettingsInterface* settingsInterface_, int width_, int height_)
        : settingsInterface (settingsInterface_)
    {
        width = width_;
        height = height_;

        viewport = std::make_unique<Viewport>();
        viewport->setViewedComponent ((Component*) settingsInterface, false);
        viewport->setScrollBarsShown (true, true);
        viewport->setScrollBarThickness (12);

        addAndMakeVisible (viewport.get());
    }

    static CustomViewport* createCustomViewport (SettingsInterface* settingsInterface)
    {
        Rectangle bounds = settingsInterface->getBounds();

        return new CustomViewport (settingsInterface, bounds.getWidth(), bounds.getHeight());
    }

    void resized() override
    {
        viewport->setBounds (getLocalBounds());

        int contentWidth = width;

        if (getWidth() > width + 12)
            contentWidth = getWidth() - 12;

        viewport->getViewedComponent()->setSize (contentWidth, height);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::grey);
    }

    SettingsInterface* settingsInterface;

private:
    std::unique_ptr<Viewport> viewport;

    int width;
    int height;

    JUCE_LEAK_DETECTOR (CustomViewport);
};
} // namespace OnixSourcePlugin
