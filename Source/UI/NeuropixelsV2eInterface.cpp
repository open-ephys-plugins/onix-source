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

#include "NeuropixelsV2eInterface.h"

#include "../OnixSourceEditor.h"
#include "../OnixSourceCanvas.h"

NeuropixelsV2eInterface::NeuropixelsV2eInterface(std::shared_ptr<Neuropixels2e> d, OnixSourceEditor* e, OnixSourceCanvas* c) :
	SettingsInterface(d, e, c)
{
	topLevelTabComponent = std::make_unique<CustomTabComponent>(true);
	addAndMakeVisible(topLevelTabComponent.get());

	probeInterfaces[0] = std::make_unique<NeuropixelsV2eProbeInterface>(d, 0, e, c);
	topLevelTabComponent->addTab("Probe A", Colours::grey, CustomViewport::createCustomViewport(probeInterfaces[0].get()), true, 0);

	probeInterfaces[1] = std::make_unique<NeuropixelsV2eProbeInterface>(d, 1, e, c);
	topLevelTabComponent->addTab("Probe B", Colours::grey, CustomViewport::createCustomViewport(probeInterfaces[1].get()), true, 1);

	deviceComponent = std::make_unique<Component>();
	deviceComponent->setBounds(0, 0, 600, 600);

	probeEnableButton = std::make_unique<UtilityButton>("ENABLED");
	probeEnableButton->setFont(FontOptions("Fira Code", "Regular", 12.0f));
	probeEnableButton->setRadius(3.0f);
	probeEnableButton->setBounds(50, 40, 100, 22);
	probeEnableButton->setClickingTogglesState(true);
	probeEnableButton->setToggleState(device->isEnabled(), dontSendNotification);
	probeEnableButton->setTooltip("If disabled, probe will not stream data during acquisition");
	probeEnableButton->addListener(this);
	deviceComponent->addAndMakeVisible(probeEnableButton.get());

	topLevelTabComponent->addTab("Device", Colours::grey, deviceComponent.get(), true);
}

void NeuropixelsV2eInterface::updateDevice(std::shared_ptr<Neuropixels2e> d)
{
	device = d;

	for (const auto& probeInterface : probeInterfaces)
	{
		probeInterface->device = device;
	}
}

void NeuropixelsV2eInterface::resized()
{
	topLevelTabComponent->setBounds(0, 0, canvas->getWidth(), canvas->getHeight());
}

void NeuropixelsV2eInterface::updateInfoString()
{
	for (const auto& probeInterface : probeInterfaces)
	{
		probeInterface->updateInfoString();
	}
}

void NeuropixelsV2eInterface::buttonClicked(Button* b)
{
	if (b == probeEnableButton.get())
	{
		device->setEnabled(probeEnableButton->getToggleState());

		if (canvas->foundInputSource())
		{
			device->configureDevice();
			canvas->resetContext();
		}

		if (device->isEnabled())
		{
			probeEnableButton->setLabel("ENABLED");
		}
		else
		{
			probeEnableButton->setLabel("DISABLED");
		}

		CoreServices::updateSignalChain(editor);
	}
}
