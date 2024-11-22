/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#include <stdio.h>
#include "ProbeNameConfig.h"
#include "../NeuropixComponents.h"

ProbeNameEditor::ProbeNameEditor (ProbeNameConfig* p, int port_, int dock_)
{
    port = port_;
    dock = dock_;

    config = p;

    autoName = "<>";
    autoNumber = "<>";
    customPort = "<>";
    customProbe = "<>";

    setJustificationType (Justification::centred);
    addListener (this);
    setEditable (false);
}

void ProbeNameEditor::labelTextChanged (Label* label)
{
    String desiredText = label->getText();

    String charactersRemoved = desiredText.removeCharacters (" ./_");

    String uniqueName = config->checkUnique (charactersRemoved, this);

    setText (uniqueName, dontSendNotification);
}

String ProbeNameConfig::checkUnique (String input, ProbeNameEditor* originalLabel)
{
    String output = String (input);

    bool foundMatch = true;
    int index = 1;

    while (foundMatch)
    {
        for (auto&& label : probeNames)
        {
            if (label.get() != originalLabel)
            {
                if (label->getText().equalsIgnoreCase (output))
                {
                    foundMatch = true;
                    output = String (input) + "-" + String (index++);
                    break;
                }
                else
                {
                    foundMatch = false;
                }
            }
            else
            {
                foundMatch = false;
            }
        }
    }

    return output;
}

void SelectionButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    int x = getScreenX();
    int y = getScreenY();
    int width = getWidth();
    int height = getHeight();

    int padding = 0.3 * height;
    Path triangle;

    /* Draw triangle in the correct direction */
    if (isPrev)
        triangle.addTriangle (padding, height / 2, width / 2, padding, width / 2, height - padding);
    else
        triangle.addTriangle (width / 2, padding, width / 2, height - padding, width - padding, height / 2);

    g.setColour (Colour (255, 255, 255));
    g.fillPath (triangle);
}

void SelectionButton::mouseUp (const MouseEvent& event)
{
    if (isPrev)
        p->showPrevScheme();
    else
        p->showNextScheme();
}

ProbeNameConfig::ProbeNameConfig()
{
    namingScheme = NamingScheme::AUTO_NAMING;

    int width = 340;
    int height = 300;

    setSize (width, height);

    titleLabel = std::make_unique<Label> ("Probe Naming Scheme", "Probe Naming Scheme");
    titleLabel->setColour (juce::Label::ColourIds::textColourId, Colours::white);
    titleLabel->setJustificationType (Justification::centred);
    titleLabel->setBounds (0, 0, width, 40);
    titleLabel->setFont (FontOptions ("Fira Code", "Semi Bold", 20.0f));
    addAndMakeVisible (titleLabel.get());

    prevButton = std::make_unique<SelectionButton> (this, true);
    prevButton->setBounds (0, 42, 40, 40);
    addAndMakeVisible (prevButton.get());

    nextButton = std::make_unique<SelectionButton> (this, false);
    nextButton->setBounds (width - 40, 42, 40, 40);
    addAndMakeVisible (nextButton.get());

    schemeLabel = std::make_unique<Label> ("Active Scheme", schemes[(int) namingScheme]);
    schemeLabel->setJustificationType (Justification::centred);
    schemeLabel->setBounds (40, 42, width - 80, 40);
    schemeLabel->setFont (FontOptions ("Fira Code", "Medium", 20.0f));
    addAndMakeVisible (schemeLabel.get());

    description = std::make_unique<Label> ("Scheme description", descriptions[(int) namingScheme]);
    description->setJustificationType (Justification::centredTop);
    description->setBounds (0, 82, width + 2, 150);
    description->setFont (FontOptions ("Fira Code", "Regular", 12.0f));
    addAndMakeVisible (description.get());

    int padding = 9;
    width = (width - 3 * padding) / 2;
    height = height / 8 - int (5.0f * padding / 4);

    int x, y;

    for (int port = 4; port > 0; port--)
    {
        for (int dock = 1; dock <= 2; dock++)
        {
            x = padding + (dock - 1) * (padding + width);
            y = getHeight() - (port) * (padding + height);

            probeNames.push_back (std::move (std::make_unique<ProbeNameEditor> (this, port, dock)));
            probeNames.back()->setBounds (x, y, width, height);
            probeNames.back()->setText ("<EMPTY>", dontSendNotification);
            addAndMakeVisible (probeNames.back().get());
        }
    }

    dock1Label = std::make_unique<Label> ("dock1Label", "Dock 1");
    dock1Label->setJustificationType (Justification::centred);
    dock1Label->setColour (juce::Label::ColourIds::textColourId, Colours::white);
    dock1Label->setBounds (x - (padding + width), getHeight() - 5 * (padding + height), width, 1.5 * height);
    dock1Label->setFont (FontOptions ("Fira Code", "Medium", 14.0f));
    addAndMakeVisible (dock1Label.get());

    dock2Label = std::make_unique<Label> ("dock2Label", "Dock 2");
    dock2Label->setJustificationType (Justification::centred);
    dock2Label->setColour (juce::Label::ColourIds::textColourId, Colours::white);
    dock2Label->setBounds (x, getHeight() - 5 * (padding + height), width, 1.5 * height);
    dock2Label->setFont (FontOptions ("Fira Code", "Medium", 14.0f));
    addAndMakeVisible (dock2Label.get());

    update();
}

void ProbeNameConfig::update()
{
    LOGD("Naming scheme: ", namingScheme);

    schemeLabel->setText (schemes[(int) namingScheme], juce::NotificationType::sendNotificationAsync);
    schemeLabel->setColour (juce::Label::ColourIds::textColourId, Colours::white);
    description->setText (descriptions[(int) namingScheme], juce::NotificationType::sendNotificationAsync);
    description->setColour (juce::Label::ColourIds::textColourId, Colours::white);

    for (auto&& label : probeNames)
    {

        label->setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);
        

        if (namingScheme == PORT_SPECIFIC_NAMING)
        {
            label->setEditable (true);
            label->setColour (juce::Label::ColourIds::backgroundColourId, juce::Colour (130, 130, 130));
            label->setColour (juce::Label::ColourIds::outlineWhenEditingColourId, Colours::orange);
        }
        else
        {
            label->setEditable (false);
            label->setColour (juce::Label::ColourIds::backgroundColourId, juce::Colour (70, 70, 70));
            label->setText ("", dontSendNotification);
        }
    }
}

void ProbeNameConfig::showPrevScheme()
{
    int currentIndex = (int) namingScheme;
    currentIndex--;
    if (currentIndex < 0)
        currentIndex = 3;

    namingScheme = (NamingScheme) currentIndex;
    update();
}

void ProbeNameConfig::showNextScheme()
{
    int currentIndex = (int) namingScheme;
    currentIndex++;
    if (currentIndex > 3)
        currentIndex = 0;

    namingScheme = (NamingScheme) currentIndex;
    update();
}
