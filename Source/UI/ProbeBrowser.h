/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2021 Allen Institute for Brain Science and Open Ephys

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

#ifndef __PROBEBROWSER_H__
#define __PROBEBROWSER_H__

#include <VisualizerEditorHeaders.h>

#include "SettingsInterface.h"
#include "../NeuropixComponents.h"

class ProbeBrowser : public Component,
                     public Timer
{
public:
    ProbeBrowser (SettingsInterface*);

    void mouseMove (const MouseEvent& event);
    void mouseDown (const MouseEvent& event);
    void mouseDrag (const MouseEvent& event);
    void mouseUp (const MouseEvent& event);
    void mouseWheelMove (const MouseEvent& event, const MouseWheelDetails& wheel);

    MouseCursor getMouseCursor();

    void timerCallback();

    void paint (Graphics& g);

    int getZoomHeight() const;
    int getZoomOffset() const;

    void setZoomHeightAndOffset (int, int);

    ActivityToView activityToView;
    float maxPeakToPeakAmplitude;

private:
    const int leftEdgeOffset = 220;

    std::map<Bank, Colour> disconnectedColours;

    // display variables
    int zoomHeight;
    int zoomOffset;

    bool isOverZoomRegion;
    bool isOverUpperBorder;
    bool isOverLowerBorder;
    bool isOverElectrode;
    bool isSelectionActive;

    int initialOffset;
    int initialHeight;
    int lowerBound;
    int dragZoneWidth;
    int zoomAreaMinRow;
    int minZoomHeight;
    int maxZoomHeight;
    int shankOffset;
    int leftEdge;
    int rightEdge;
    int channelLabelSkip;
    int pixelHeight;

    int lowestElectrode;
    int highestElectrode;

    float electrodeHeight;

    MouseCursor::StandardCursorType cursorType;

    String electrodeInfoString;

    Colour getElectrodeColour (int index);
    int getNearestElectrode (int x, int y) const;
    Array<int> getElectrodesWithinBounds (int x, int y, int w, int h) const;
    String getElectrodeInfoString (int index);

    SettingsInterface* parent;

    JUCE_LEAK_DETECTOR(ProbeBrowser);
};

#endif  // __PROBEBROWSER_H__
