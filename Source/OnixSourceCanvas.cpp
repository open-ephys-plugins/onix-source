/*
    ------------------------------------------------------------------

    Copyright(C) Open Ephys

    ------------------------------------------------------------------

    This program is free software : you can redistribute it and /or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see < http://www.gnu.org/licenses/>.

*/

#include "OnixSourceCanvas.h"

#include "OnixSource.h"
#include "OnixSourceEditor.h"
#include "UI/SettingsInterface.h"

using namespace OnixSourcePlugin;

OnixSourceCanvas::OnixSourceCanvas (GenericProcessor* processor_, OnixSourceEditor* editor_, OnixSource* onixSource_)
    : Visualizer (processor_),
      editor (editor_),
      source (onixSource_)
{
    topLevelTabComponent = std::make_unique<CustomTabComponent> (true);
    addAndMakeVisible (topLevelTabComponent.get());

    addHub (BREAKOUT_BOARD_NAME, 0);
}

CustomTabComponent* OnixSourceCanvas::addTopLevelTab (std::string tabName, int index)
{
    CustomTabComponent* tab = new CustomTabComponent (false);

    topLevelTabComponent->addTab (tabName, Colours::grey, tab, true, index);
    tab->setName (tabName);

    hubTabs.add (tab);

    return tab;
}

void OnixSourceCanvas::addHub (std::string hubName, int offset)
{
    CustomTabComponent* tab = nullptr;
    OnixDeviceVector devices;
    PortName port = PortController::getPortFromIndex (offset);
    auto context = source->getContext();

    if (context == nullptr || ! context->isInitialized())
    {
        LOGE ("Unable to find an initialized context when adding " + hubName + " to the canvas.");
        return;
    }

    if (hubName == NEUROPIXELSV1E_HEADSTAGE_NAME)
    {
        tab = addTopLevelTab (getTopLevelTabName (port, hubName), (int) port);

        const int passthroughIndex = OnixDevice::getPassthroughIndexFromHubIndex (offset);

        devices.emplace_back (std::make_shared<Neuropixels1e> (ProbeString, hubName, passthroughIndex, context));
        devices.emplace_back (std::make_shared<PolledBno055> (OnixDevice::TypeString.at (PolledBno055::getDeviceType()), hubName, passthroughIndex, context));
    }
    else if (hubName == NEUROPIXELSV1F_HEADSTAGE_NAME)
    {
        tab = addTopLevelTab (getTopLevelTabName (port, hubName), (int) port);

        static constexpr int Probe0Offset = 0, Probe1Offset = 1, BnoOffset = 2;

        devices.emplace_back (std::make_shared<Neuropixels1f> (ProbeString + "0", hubName, offset + Probe0Offset, context));
        devices.emplace_back (std::make_shared<Neuropixels1f> (ProbeString + "1", hubName, offset + Probe1Offset, context));
        devices.emplace_back (std::make_shared<Bno055> (OnixDevice::TypeString.at (Bno055::getDeviceType()), hubName, offset + BnoOffset, context));
    }
    else if (hubName == BREAKOUT_BOARD_NAME)
    {
        tab = addTopLevelTab (hubName, 0);

        static constexpr int OutputClockOffset = 5, AnalogIOOffset = 6, DigitalIOOffset = 7, HarpSyncInputOffset = 12;

        devices.emplace_back (std::make_shared<DigitalIO> (OnixDevice::TypeString.at (DigitalIO::getDeviceType()), hubName, DigitalIOOffset, context));
        devices.emplace_back (std::make_shared<AnalogIO> (OnixDevice::TypeString.at (AnalogIO::getDeviceType()), hubName, AnalogIOOffset, context));
        devices.emplace_back (std::make_shared<HarpSyncInput> (OnixDevice::TypeString.at (HarpSyncInput::getDeviceType()), hubName, HarpSyncInputOffset, context));
        devices.emplace_back (std::make_shared<OutputClock> (OnixDevice::TypeString.at (OutputClock::getDeviceType()), hubName, OutputClockOffset, context));
    }
    else if (hubName == NEUROPIXELSV2E_HEADSTAGE_NAME)
    {
        const int passthroughIndex = OnixDevice::getPassthroughIndexFromHubIndex (offset);

        tab = addTopLevelTab (getTopLevelTabName (port, hubName), (int) port);

        devices.emplace_back (std::make_shared<Neuropixels2e> (ProbeString, hubName, passthroughIndex, context));
        devices.emplace_back (std::make_shared<PolledBno055> (OnixDevice::TypeString.at (OnixDeviceType::POLLEDBNO), hubName, passthroughIndex, context));
    }

    if (tab != nullptr && devices.size() > 0)
    {
        populateSourceTabs (tab, devices);
    }
}

std::shared_ptr<OnixDevice> OnixSourceCanvas::getDevicePtr (OnixDeviceType deviceType, int deviceIndex)
{
    for (const auto& settingsInterface : settingsInterfaces)
    {
        auto device = settingsInterface->getDevice();

        if (device->getDeviceType() == deviceType && device->getDeviceIdx() == deviceIndex)
        {
            return device;
        }
    }

    return nullptr;
}

void OnixSourceCanvas::populateSourceTabs (CustomTabComponent* tab, OnixDeviceVector devices)
{
    int portTabNumber = 0;

    for (const auto& device : devices)
    {
        auto type = device->getDeviceType();

        if (type == OnixDeviceType::NEUROPIXELSV1F)
        {
            auto neuropixInterface = std::make_shared<NeuropixelsV1Interface> (std::static_pointer_cast<Neuropixels1> (device), editor, this);
            addInterfaceToTab (device->getName(), tab, neuropixInterface);
        }
        else if (type == OnixDeviceType::NEUROPIXELSV1E)
        {
            auto neuropixInterface = std::make_shared<NeuropixelsV1Interface> (std::static_pointer_cast<Neuropixels1> (device), editor, this);
            addInterfaceToTab (device->getName(), tab, neuropixInterface);
        }
        else if (type == OnixDeviceType::BNO)
        {
            auto bno055Interface = std::make_shared<Bno055Interface> (std::static_pointer_cast<Bno055> (device), editor, this);
            addInterfaceToTab (device->getName(), tab, bno055Interface);
        }
        else if (type == OnixDeviceType::OUTPUTCLOCK)
        {
            auto outputClockInterface = std::make_shared<OutputClockInterface> (std::static_pointer_cast<OutputClock> (device), editor, this);
            addInterfaceToTab (device->getName(), tab, outputClockInterface);
        }
        else if (type == OnixDeviceType::HARPSYNCINPUT)
        {
            auto harpSyncInputInterface = std::make_shared<HarpSyncInputInterface> (std::static_pointer_cast<HarpSyncInput> (device), editor, this);
            addInterfaceToTab (device->getName(), tab, harpSyncInputInterface);
        }
        else if (type == OnixDeviceType::ANALOGIO)
        {
            auto analogIOInterface = std::make_shared<AnalogIOInterface> (std::static_pointer_cast<AnalogIO> (device), editor, this);
            addInterfaceToTab (device->getName(), tab, analogIOInterface);
        }
        else if (type == OnixDeviceType::DIGITALIO)
        {
            auto digitalIOInterface = std::make_shared<DigitalIOInterface> (std::static_pointer_cast<DigitalIO> (device), editor, this);
            addInterfaceToTab (device->getName(), tab, digitalIOInterface);
        }
        else if (type == OnixDeviceType::NEUROPIXELSV2E)
        {
            auto npxv2eInterface = std::make_shared<NeuropixelsV2eInterface> (std::static_pointer_cast<Neuropixels2e> (device), editor, this);
            std::string substring = " Headstage";
            std::string hubName = device->getHubName();
            addInterfaceToTab (hubName.erase (hubName.find (substring), substring.size()), tab, npxv2eInterface);
        }
        else if (type == OnixDeviceType::POLLEDBNO)
        {
            auto polledBnoInterface = std::make_shared<PolledBno055Interface> (std::static_pointer_cast<PolledBno055> (device), editor, this);
            addInterfaceToTab (device->getName(), tab, polledBnoInterface);
        }
        else
        {
            Onix1::showWarningMessageBoxAsync ("Device Type Not Found", "The device type " + OnixDevice::TypeString.at (type) + " is not known.");
            return;
        }
    }
}

void OnixSourceCanvas::addInterfaceToTab (std::string tabName, CustomTabComponent* tab, std::shared_ptr<SettingsInterface> interface_)
{
    settingsInterfaces.emplace_back (interface_);
    tab->addTab (tabName, Colours::darkgrey, CustomViewport::createCustomViewport (interface_.get()), true);
}

std::string OnixSourceCanvas::getTopLevelTabName (PortName port, std::string headstage)
{
    return OnixDevice::getPortName (port) + ": " + headstage;
}

Array<CustomTabComponent*> OnixSourceCanvas::getHubTabs()
{
    Array<CustomTabComponent*> tabs;

    for (const auto hub : hubTabs)
    {
        tabs.add (hub);
    }

    return tabs;
}

void OnixSourceCanvas::refresh()
{
    repaint();
}

void OnixSourceCanvas::refreshState()
{
    resized();
}

void OnixSourceCanvas::removeTabs (PortName port)
{
    bool tabExists = false;

    for (int i = hubTabs.size() - 1; i >= 0; i -= 1)
    {
        if (hubTabs[i]->getName().contains (OnixDevice::getPortName (port)))
        {
            hubTabs.remove (i, true);
            tabExists = true;
            break;
        }
    }

    int offset = PortController::getPortOffset (port);

    for (int i = settingsInterfaces.size() - 1; i >= 0; i -= 1)
    {
        auto selectedDevice = settingsInterfaces[i]->getDevice();

        if ((selectedDevice->getDeviceIdx() & offset) > 0)
        {
            settingsInterfaces.erase (settingsInterfaces.begin() + i);
            tabExists = true;
        }
    }

    if (tabExists)
    {
        if (port == PortName::PortB && hubTabs.size() == 1 && hubTabs[0]->getName().contains (BREAKOUT_BOARD_NAME))
            topLevelTabComponent->removeTab ((int) port - 1); // NB: If only one headstage is selected in the editor, the index needs to be corrected here.
        else
            topLevelTabComponent->removeTab ((int) port);
    }
}

void OnixSourceCanvas::removeAllTabs()
{
    hubTabs.clear (true);
    settingsInterfaces.clear();

    topLevelTabComponent->clearTabs();
}

OnixDeviceMap OnixSourceCanvas::getSelectedDevices (std::vector<std::shared_ptr<SettingsInterface>> interfaces)
{
    OnixDeviceMap tabMap;

    for (const auto& settings : interfaces)
    {
        auto device = settings->getDevice();

        if (! device->isEnabled())
            continue;

        tabMap.insert ({ device->getDeviceIdx(), device->getDeviceType() });
    }

    return tabMap;
}

bool OnixSourceCanvas::verifyHeadstageSelection()
{
    auto selectedDevices = getSelectedDevices (settingsInterfaces);
    auto connectedDevices = source->getConnectedDevices (true);

    if (selectedDevices != connectedDevices)
    {
        auto connectedOffsets = OnixDevice::getUniqueOffsets (connectedDevices);
        auto selectedOffsets = OnixDevice::getUniqueOffsets (selectedDevices);

        auto hubNames = source->getHubNames();

        std::string title = "Invalid Headstage Selection", msg;

        msg = "There is a mismatch between the headstages that are selected, and the headstages that are connected.\n\n";
        msg += "Selected headstage";
        msg += (selectedOffsets.size() > 1) ? "s" : "";
        msg += ":\n\n";

        if (selectedOffsets.size() == 0)
        {
            msg += "None\n";
        }
        else
        {
            for (const auto offset : selectedOffsets)
            {
                msg += OnixDevice::getPortName (offset) + ": " + editor->getHeadstageSelected (offset) + "\n";
            }
        }

        msg += "\nConnected headstage";
        msg += (connectedOffsets.size() > 1) ? "s" : "";
        msg += ":\n\n";

        if (connectedOffsets.size() == 0)
        {
            msg += "None\n";
        }
        else
        {
            for (const auto offset : connectedOffsets)
            {
                msg += OnixDevice::getPortName (offset) + ": " + hubNames[offset] + "\n";
            }
        }

        msg += "\nVerify that the correct headstage(s) are selected, and that all hardware connections are correct before trying to connect again.";

        Onix1::showWarningMessageBoxAsync (title, msg);

        return false;
    }

    return true;
}

void OnixSourceCanvas::update() const
{
    for (int i = 0; i < settingsInterfaces.size(); i++)
        settingsInterfaces[i]->updateInfoString();
}

void OnixSourceCanvas::resized()
{
    topLevelTabComponent->setBounds (0, -3, getWidth(), getHeight() + 3);
}

void OnixSourceCanvas::resetContext()
{
    source->resetContext();
}

bool OnixSourceCanvas::foundInputSource()
{
    return source->foundInputSource();
}

void OnixSourceCanvas::startAcquisition()
{
    for (const auto& settingsInterface : settingsInterfaces)
    {
        settingsInterface->startAcquisition();
    }
}

void OnixSourceCanvas::stopAcquisition()
{
    for (const auto& settingsInterface : settingsInterfaces)
    {
        settingsInterface->stopAcquisition();
    }
}

void OnixSourceCanvas::saveCustomParametersToXml (XmlElement* xml)
{
    for (int i = 0; i < settingsInterfaces.size(); i++)
        settingsInterfaces[i]->saveParameters (xml);
}

void OnixSourceCanvas::loadCustomParametersFromXml (XmlElement* xml)
{
    for (int i = 0; i < settingsInterfaces.size(); i++)
        settingsInterfaces[i]->loadParameters (xml);
}
