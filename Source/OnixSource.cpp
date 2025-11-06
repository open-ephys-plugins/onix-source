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

#include "OnixSource.h"

#include "Devices/DeviceList.h"
#include "OnixSourceCanvas.h"

using namespace OnixSourcePlugin;

OnixSource::OnixSource (SourceNode* sn)
    : DataThread (sn),
      devicesFound (false),
      editor (NULL)
{
    try
    {
        context = std::make_shared<Onix1>();
    }
    catch (const std::system_error& e)
    {
        Onix1::showWarningMessageBoxAsync (
            "Failed to Create Context",
            e.what());
    }
    catch (const error_t& e)
    {
        Onix1::showWarningMessageBoxAsync (
            "Failed to Initialize Context",
            e.what());
    }

    portA = std::make_shared<PortController> (PortName::PortA, context);
    portB = std::make_shared<PortController> (PortName::PortB, context);

    if (context == nullptr || ! context->isInitialized())
    {
        LOGE ("Failed to initialize context.");
        return;
    }
}

OnixSource::~OnixSource()
{
    if (context != nullptr && context->isInitialized())
    {
        portA->setVoltageOverride (0.0f, false);
        portB->setVoltageOverride (0.0f, false);
    }
}

void OnixSource::registerParameters()
{
    addBooleanParameter (Parameter::PROCESSOR_SCOPE, "passthroughA", "Passthrough A", "Enables passthrough mode for e-variant headstages on Port A", false, true);
    addBooleanParameter (Parameter::PROCESSOR_SCOPE, "passthroughB", "Passthrough B", "Enables passthrough mode for e-variant headstages on Port B", false, true);
}

DataThread* OnixSource::createDataThread (SourceNode* sn)
{
    return new OnixSource (sn);
}

std::unique_ptr<GenericEditor> OnixSource::createEditor (SourceNode* sn)
{
    std::unique_ptr<OnixSourceEditor> e = std::make_unique<OnixSourceEditor> (sn, this);
    editor = e.get();

    return e;
}

bool OnixSource::disconnectDevices (bool updateStreamInfo)
{
    sourceBuffers.clear (true);

    sources.clear();
    hubNames.clear();

    devicesFound = false;

    if (context != nullptr && context->isInitialized())
    {
        if (context->setOption (ONIX_OPT_PASSTHROUGH, 0) != ONI_ESUCCESS)
        {
            LOGE ("Unable to set passthrough option when disconnecting devices.");
            return false;
        }
    }

    if (updateStreamInfo)
        CoreServices::updateSignalChain (editor);

    return true;
}

template <class Device>
bool OnixSource::configureDevice (OnixDeviceVector& sources,
                                  OnixSourceEditor* editor,
                                  std::string deviceName,
                                  std::string hubName,
                                  OnixDeviceType deviceType,
                                  const oni_dev_idx_t deviceIdx,
                                  std::shared_ptr<Onix1> ctx)
{
    auto canvas = editor->getCanvas();
    std::shared_ptr<Device> device = std::static_pointer_cast<Device> (canvas->getDevicePtr (Device::getDeviceType(), deviceIdx));

    if (device != nullptr)
    {
        if (device->getName() != deviceName || device->getHubName() != hubName)
        {
            LOGD ("Difference in names found for device at address ", deviceIdx, ". Found ", deviceName, " on ", hubName, ", but was expecting ", device->getName(), " on ", device->getHubName());
        }
    }
    else if (device->getDeviceType() == OnixDeviceType::MEMORYMONITOR)
    { // NB: These are devices with no equivalent settings tab that still need to be created and added to the vector of devices
        LOGD ("Creating new device ", deviceName, " on ", hubName);
        device = std::make_shared<Device> (deviceName, hubName, deviceIdx, ctx);
    }

    if (device == nullptr)
    {
        if (hubName == editor->getHeadstageSelected (OnixDevice::getOffset (deviceIdx)))
        {
            Onix1::showWarningMessageBoxAsync (
                "Device Not Found",
                "Could not find " + deviceName + ", at address " + std::to_string (deviceIdx) + " on " + hubName);
        }
        else
        {
            Onix1::showWarningMessageBoxAsync (
                "Invalid Headstage Selection",
                "Expected to find " + editor->getHeadstageSelected (OnixDevice::getOffset (deviceIdx)) + " on " + OnixDevice::getPortName (deviceIdx) + ", but found " + hubName + " instead. Confirm that the correct headstage is selected, and try to connect again.");
        }

        return false;
    }

    int res = 1;

    try
    {
        res = device->configureDevice();
    }
    catch (const error_str& e)
    {
        Onix1::showWarningMessageBoxAsync ("Configuration Error", e.what());

        return false;
    }

    sources.emplace_back (device);

    return res == ONI_ESUCCESS;
}

bool OnixSource::getHubFirmwareVersion (std::shared_ptr<Onix1> ctx, uint32_t hubIndex, uint32_t* firmwareVersion)
{
    if (ctx->readRegister (hubIndex + ONIX_HUB_DEV_IDX, ONIX_HUB_FIRMWAREVER, firmwareVersion) != ONI_ESUCCESS)
    {
        LOGE ("Unable to read the hub firmware version at index ", hubIndex);
        return false;
    }

    return true;
}

bool OnixSource::enablePassthroughMode (std::shared_ptr<Onix1> ctx, bool passthroughA, bool passthroughB)
{
    if (ctx == nullptr || ! ctx->isInitialized())
    {
        Onix1::showWarningMessageBoxAsync ("Invalid Context", "Cannot enable passthrough mode, context is not initialized correctly.");
        return false;
    }

    int val = 0;

    if (passthroughA)
    {
        LOGD ("Passthrough mode enabled for Port A");
        val |= 1 << 0;
    }

    if (passthroughB)
    {
        LOGD ("Passthrough mode enabled for Port B");
        val |= 1 << 2;
    }

    return ctx->setOption (ONIX_OPT_PASSTHROUGH, val) == ONI_ESUCCESS;
}

bool OnixSource::configurePort (PortName port)
{
    if (context == nullptr || ! context->isInitialized())
    {
        Onix1::showWarningMessageBoxAsync ("Invalid Context", "Cannot initialize devices, context is not initialized correctly. Please try removing the plugin and adding it again.");
        return false;
    }

    if (port == PortName::PortA)
    {
        if (portA->configureDevice() != ONI_ESUCCESS)
        {
            Onix1::showWarningMessageBoxAsync ("Configuration Error", "Unable to configure Port A.");
            return false;
        }
    }
    else if (port == PortName::PortB)
    {
        if (portB->configureDevice() != ONI_ESUCCESS)
        {
            Onix1::showWarningMessageBoxAsync ("Configuration Error", "Unable to configure Port B.");
            return false;
        }
    }

    return true;
}

bool OnixSource::resetPortLinkFlags()
{
    if (context == nullptr || ! context->isInitialized())
        return false;

    return portA->resetLinkFlags() != ONI_ESUCCESS || portB->resetLinkFlags() != ONI_ESUCCESS;
}

bool OnixSource::resetPortLinkFlags (PortName port)
{
    if (context == nullptr || ! context->isInitialized())
        return false;

    if (port == PortName::PortA)
    {
        if (portA->resetLinkFlags() != ONI_ESUCCESS)
            return false;
    }
    else if (port == PortName::PortB)
    {
        if (portB->resetLinkFlags() != ONI_ESUCCESS)
            return false;
    }

    return true;
}

bool OnixSource::checkHubFirmwareCompatibility (std::shared_ptr<Onix1> context, device_map_t deviceTable)
{
    auto hubIds = context->getHubIds (deviceTable);

    if (hubIds.size() == 0)
    {
        LOGE ("No hub IDs found.");
        return false;
    }

    for (const auto& [hubIndex, hubId] : hubIds)
    {
        if (hubId == ONIX_HUB_FMCHOST)
        {
            static constexpr int RequiredMajorVersion = 2;
            uint32_t firmwareVersion = 0;
            if (! getHubFirmwareVersion (context, hubIndex, &firmwareVersion))
            {
                return false;
            }

            auto majorVersion = (firmwareVersion & 0xFF00) >> 8;
            auto minorVersion = firmwareVersion & 0xFF;

            LOGC ("PCIe Host firmware version: v", majorVersion, ".", minorVersion);

            if (majorVersion != RequiredMajorVersion)
            {
                Onix1::showWarningMessageBoxAsync (
                    "Invalid Firmware Version",
                    "The PCIe Host firmware major version is v"
                        + std::to_string (majorVersion) + ", but this plugin is only compatible with v"
                        + std::to_string (RequiredMajorVersion) + ". To use this plugin, modify the firmware"
                        + " to version v" + std::to_string (majorVersion));
                return false;
            }
        }
    }

    return true;
}

bool OnixSource::initializeDevices (device_map_t deviceTable, bool updateStreamInfo)
{
    if (context == nullptr || ! context->isInitialized())
    {
        Onix1::showWarningMessageBoxAsync ("Invalid Context", "Cannot initialize devices, context is not initialized correctly. Please try removing the plugin and adding it again.");
        return false;
    }

    if (deviceTable.size() == 0)
    {
        Onix1::showWarningMessageBoxAsync ("No Devices Found", "Could not find any devices from the connected hardware.");
        if (updateStreamInfo)
            CoreServices::updateSignalChain (editor);
        return false;
    }

    auto hubIds = context->getHubIds (deviceTable);

    if (hubIds.size() == 0)
    {
        Onix1::showWarningMessageBoxAsync ("No Hubs Found", "Could not find any hubs from the connected hardware.");
        if (updateStreamInfo)
            CoreServices::updateSignalChain (editor);
        return false;
    }

    devicesFound = false;

    // NB: Search through all hubs, and initialize devices
    for (const auto& [hubIndex, hubId] : hubIds)
    {
        if (hubId == ONIX_HUB_FMCHOST)
        {
            hubNames.insert ({ hubIndex, BREAKOUT_BOARD_NAME });

            static constexpr int OutputClockOffset = 5, AnalogIOOffset = 6, DigitalIOOffset = 7, MemoryMonitorOffset = 10, HarpSyncInputOffset = 12;

            devicesFound = configureDevice<OutputClock> (sources, editor, OnixDevice::TypeString.at (OutputClock::getDeviceType()), BREAKOUT_BOARD_NAME, OutputClock::getDeviceType(), hubIndex + OutputClockOffset, context);
            if (! devicesFound)
            {
                sources.clear();
                return false;
            }

            devicesFound = configureDevice<DigitalIO> (sources, editor, OnixDevice::TypeString.at (DigitalIO::getDeviceType()), BREAKOUT_BOARD_NAME, DigitalIO::getDeviceType(), hubIndex + DigitalIOOffset, context);
            if (! devicesFound)
            {
                sources.clear();
                return false;
            }

            devicesFound = configureDevice<AnalogIO> (sources, editor, OnixDevice::TypeString.at (AnalogIO::getDeviceType()), BREAKOUT_BOARD_NAME, AnalogIO::getDeviceType(), hubIndex + AnalogIOOffset, context);
            if (! devicesFound)
            {
                sources.clear();
                return false;
            }

            devicesFound = configureDevice<MemoryMonitor> (sources, editor, OnixDevice::TypeString.at (MemoryMonitor::getDeviceType()), BREAKOUT_BOARD_NAME, MemoryMonitor::getDeviceType(), hubIndex + MemoryMonitorOffset, context);
            if (! devicesFound)
            {
                sources.clear();
                return false;
            }

            devicesFound = configureDevice<HarpSyncInput> (sources, editor, OnixDevice::TypeString.at (HarpSyncInput::getDeviceType()), BREAKOUT_BOARD_NAME, HarpSyncInput::getDeviceType(), hubIndex + HarpSyncInputOffset, context);
            if (! devicesFound)
            {
                sources.clear();
                return false;
            }
        }
        else if (hubId == ONIX_HUB_HSNP)
        {
            hubNames.insert ({ hubIndex, NEUROPIXELSV1F_HEADSTAGE_NAME });

            for (int i = 0; i < 2; i++)
            {
                devicesFound = configureDevice<Neuropixels1f> (sources, editor, ProbeString + std::to_string (i), NEUROPIXELSV1F_HEADSTAGE_NAME, Neuropixels1f::getDeviceType(), hubIndex + i, context);
                if (! devicesFound)
                {
                    sources.clear();
                    return false;
                }
            }

            devicesFound = configureDevice<Bno055> (sources, editor, OnixDevice::TypeString.at (Bno055::getDeviceType()), NEUROPIXELSV1F_HEADSTAGE_NAME, Bno055::getDeviceType(), hubIndex + 2, context);
            if (! devicesFound)
            {
                sources.clear();
                return false;
            }
        }
        else
        {
            Onix1::showWarningMessageBoxAsync (
                "Unknown Hub ID",
                "Discovered hub ID " + std::to_string (hubId) + " (" + onix_hub_str (hubId) + ") on " + OnixDevice::getPortName (hubIndex) + " which does not match any currently implemented hubs.");
            devicesFound = false;
            sources.clear();
            return false;
        }
    }

    // NB: Search for passthrough devices, and initialize any headstages found in passthrough mode
    for (const auto& [index, device] : deviceTable)
    {
        if (device.id == ONIX_DS90UB9RAW)
        {
            LOGD ("Passthrough device detected");

            auto serializer = std::make_unique<I2CRegisterContext> (DS90UB9x::SER_ADDR, index, context);
            serializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xSerializerI2CRegister::SCLHIGH, 20);
            serializer->WriteByte ((uint32_t) DS90UB9x::DS90UB9xSerializerI2CRegister::SCLLOW, 20);

            auto EEPROM = std::make_unique<HeadStageEEPROM> (index, context);
            uint32_t hsid = EEPROM->GetHeadStageID();
            LOGC ("Detected headstage ", onix_hub_str (hsid));

            if (hsid == ONIX_HUB_HSNP2E)
            {
                auto hubIndex = OnixDevice::getHubIndexFromPassthroughIndex (index);

                devicesFound = configureDevice<Neuropixels2e> (sources, editor, ProbeString, NEUROPIXELSV2E_HEADSTAGE_NAME, Neuropixels2e::getDeviceType(), hubIndex, context);
                if (! devicesFound)
                {
                    sources.clear();
                    return false;
                }

                devicesFound = configureDevice<PolledBno055> (sources, editor, OnixDevice::TypeString.at (PolledBno055::getDeviceType()), NEUROPIXELSV2E_HEADSTAGE_NAME, PolledBno055::getDeviceType(), hubIndex + 1, context);
                if (! devicesFound)
                {
                    sources.clear();
                    return false;
                }

                if (sources.back()->getDeviceType() != OnixDeviceType::POLLEDBNO)
                {
                    LOGE ("Unknown device encountered when configuring headstage ", NEUROPIXELSV2E_HEADSTAGE_NAME);
                    devicesFound = false;
                    sources.clear();
                    return false;
                }

                const auto& polledBno = std::static_pointer_cast<PolledBno055> (sources.back());

                polledBno->setBnoAxisMap (PolledBno055::Bno055AxisMap::YZX);
                polledBno->setBnoAxisSign ((uint32_t) (PolledBno055::Bno055AxisSign::MirrorX) | (uint32_t) (PolledBno055::Bno055AxisSign::MirrorY));

                hubNames.insert ({ OnixDevice::getOffset (polledBno->getDeviceIdx()), NEUROPIXELSV2E_HEADSTAGE_NAME });
            }
            else if (hsid == ONIX_HUB_HSNP1ET || hsid == ONIX_HUB_HSNP1EH)
            {
                auto hubIndex = OnixDevice::getHubIndexFromPassthroughIndex (index);

                devicesFound = configureDevice<Neuropixels1e> (sources, editor, ProbeString, NEUROPIXELSV1E_HEADSTAGE_NAME, Neuropixels1e::getDeviceType(), hubIndex, context);
                if (! devicesFound)
                {
                    sources.clear();
                    return false;
                }

                devicesFound = configureDevice<PolledBno055> (sources, editor, OnixDevice::TypeString.at (PolledBno055::getDeviceType()), NEUROPIXELSV1E_HEADSTAGE_NAME, PolledBno055::getDeviceType(), hubIndex + 1, context);
                if (! devicesFound)
                {
                    sources.clear();
                    return false;
                }

                if (sources.back()->getDeviceType() != OnixDeviceType::POLLEDBNO)
                {
                    LOGE ("Unknown device encountered when setting headstage.");
                    devicesFound = false;
                    sources.clear();
                    return false;
                }

                const auto& polledBno = std::static_pointer_cast<PolledBno055> (sources.back());

                polledBno->setBnoAxisMap (PolledBno055::Bno055AxisMap::YZX);
                polledBno->setBnoAxisSign ((uint32_t) (PolledBno055::Bno055AxisSign::MirrorX) | (uint32_t) (PolledBno055::Bno055AxisSign::MirrorZ));

                hubNames.insert ({ OnixDevice::getOffset (polledBno->getDeviceIdx()), NEUROPIXELSV1E_HEADSTAGE_NAME });
            }
            else
            {
                Onix1::showWarningMessageBoxAsync (
                    "Unknown Hub ID",
                    "Discovered hub ID " + std::to_string (hsid) + " (" + onix_hub_str (hsid) + ") on " + OnixDevice::getPortName (OnixDevice::getHubIndexFromPassthroughIndex (index)) + " which does not match any currently implemented hubs.");
                devicesFound = false;
                sources.clear();
                return false;
            }
        }
    }

    context->issueReset();

    if (updateStreamInfo)
        CoreServices::updateSignalChain (editor);

    LOGD ("All devices initialized.");
    return devicesFound;
}

bool OnixSource::configureBlockReadSize (std::shared_ptr<Onix1> context, uint32_t blockReadSize)
{
    if (context == nullptr || ! context->isInitialized())
    {
        Onix1::showWarningMessageBoxAsync ("Invalid Context", "Cannot set block read size, context is not initialized correctly. Please try removing the plugin and adding it again.");
        return false;
    }

    oni_size_t readFrameSize;
    int rc = context->getOption<oni_size_t> (ONI_OPT_MAXREADFRAMESIZE, &readFrameSize);
    if (rc == ONI_ESUCCESS)
    {
        LOGD ("Max read frame size: ", readFrameSize, " bytes");
    }
    else
    {
        LOGE ("Unable to get read frame size.");
        return false;
    }

    oni_size_t writeFrameSize;
    rc = context->getOption<oni_size_t> (ONI_OPT_MAXWRITEFRAMESIZE, &writeFrameSize);
    if (rc == ONI_ESUCCESS)
    {
        LOGD ("Max write frame size: ", writeFrameSize, " bytes");
    }
    else
    {
        LOGE ("Unable to get write frame size.");
        return false;
    }

    if (! writeBlockReadSize (context, blockReadSize, readFrameSize))
    {
        return false;
    }

    return true;
}

uint32_t OnixSource::getBlockReadSize() const
{
    return blockReadSize;
}

void OnixSource::setBlockReadSize (uint32_t newReadSize)
{
    blockReadSize = newReadSize;
}

bool OnixSource::writeBlockReadSize (std::shared_ptr<Onix1> context, uint32_t blockReadSize, uint32_t readFrameSize)
{
    if (context == nullptr || ! context->isInitialized())
    {
        Onix1::showWarningMessageBoxAsync ("Invalid Context", "Cannot initialize devices, context is not initialized correctly. Please try removing the plugin and adding it again.");
        return false;
    }

    if (blockReadSize < readFrameSize)
    {
        Onix1::showWarningMessageBoxAsync ("Invalid Block Read Size", "The block read size is too small. The max read frame size is " + std::to_string (readFrameSize) + ", but the block read size is " + std::to_string (blockReadSize) + ".\n\nTo continue, increase the block read size to be greater than " + std::to_string (readFrameSize) + " and reconnect.");
        return false;
    }

    int rc = context->setOption (ONI_OPT_BLOCKREADSIZE, blockReadSize);

    if (rc != ONI_ESUCCESS)
    {
        LOGE ("Unknown error found when setting block read size to ", blockReadSize);
        return false;
    }

    LOGD ("Block read size: ", blockReadSize, " bytes");

    return true;
}

OnixDeviceVector OnixSource::getDataSources()
{
    OnixDeviceVector devices {};

    for (const auto& source : sources)
    {
        devices.emplace_back (source);
    }

    return devices;
}

OnixDeviceVector OnixSource::getEnabledDataSources()
{
    OnixDeviceVector devices {};

    for (const auto& source : sources)
    {
        if (source->isEnabled())
            devices.emplace_back (source);
    }

    return devices;
}

OnixDeviceVector OnixSource::getDataSourcesFromOffset (int offset)
{
    OnixDeviceVector devices {};
    offset = OnixDevice::getOffset (offset);

    for (const auto& source : sources)
    {
        if (OnixDevice::getOffset (source->getDeviceIdx()) == offset)
            devices.emplace_back (source);
    }

    return devices;
}

std::shared_ptr<OnixDevice> OnixSource::getDevice (OnixDeviceType type, int offset)
{
    for (const auto& device : sources)
    {
        if (device->getDeviceType() == type)
            return device;
    }

    return nullptr;
}

OnixDeviceVector OnixSource::getDevices (OnixDeviceType type)
{
    OnixDeviceVector foundDevices {};

    for (const auto& device : sources)
    {
        if (device->getDeviceType() == type)
            foundDevices.emplace_back (device);
    }

    return foundDevices;
}

OnixDeviceMap OnixSource::getConnectedDevices (OnixDeviceVector devices, bool filterDevices)
{
    OnixDeviceMap deviceMap;

    for (const auto& device : devices)
    {
        if (filterDevices && (device->getDeviceType() == OnixDeviceType::MEMORYMONITOR))
            continue;

        deviceMap.insert ({ device->getDeviceIdx(), device->getDeviceType() });
    }

    return deviceMap;
}

OnixDeviceMap OnixSource::getConnectedDevices (bool filterDevices)
{
    return getConnectedDevices (getEnabledDataSources(), filterDevices);
}

std::map<int, std::string> OnixSource::getHubNames()
{
    return hubNames;
}

void OnixSource::updateSourceBuffers()
{
    sourceBuffers.clear (true);

    for (const auto& source : sources)
    {
        if (source->isEnabled())
        {
            source->addSourceBuffers (sourceBuffers);
        }
    }
}

void OnixSource::updateDiscoveryParameters (PortName port, DiscoveryParameters parameters)
{
    switch (port)
    {
        case PortName::PortA:
            portA->updateDiscoveryParameters (parameters);
            break;
        case PortName::PortB:
            portB->updateDiscoveryParameters (parameters);
            break;
        default:
            break;
    }
}

bool OnixSource::configurePortVoltage (PortName port, std::string voltage) const
{
    if (! context->isInitialized())
        return false;

    switch (port)
    {
        case PortName::PortA:
            if (voltage == "" || voltage == "Auto")
                return portA->configureVoltage();
            else
                return portA->configureVoltage (std::stod (voltage));
        case PortName::PortB:
            if (voltage == "" || voltage == "Auto")
                return portB->configureVoltage();
            else
                return portB->configureVoltage (std::stod (voltage));
        default:
            return false;
    }
}

void OnixSource::setPortVoltage (PortName port, double voltage) const
{
    if (! context->isInitialized())
        return;

    bool waitToSettle = voltage > 0;

    switch (port)
    {
        case PortName::PortA:
            portA->setVoltageOverride (voltage, waitToSettle);
            return;
        case PortName::PortB:
            portB->setVoltageOverride (voltage, waitToSettle);
            return;
        default:
            return;
    }
}

double OnixSource::getLastVoltageSet (PortName port)
{
    if (! context->isInitialized())
        return 0.0;

    switch (port)
    {
        case PortName::PortA:
            return portA->getLastVoltageSet();
        case PortName::PortB:
            return portB->getLastVoltageSet();
        default:
            return 0.0;
    }
}

void OnixSource::resetContext()
{
    if (context != nullptr && context->isInitialized())
        context->issueReset();
}

bool OnixSource::isContextInitialized()
{
    return context != nullptr && context->isInitialized();
}

std::shared_ptr<Onix1> OnixSource::getContext()
{
    return context;
}

bool OnixSource::getDeviceTable (device_map_t* deviceTable)
{
    return context->getDeviceTable (deviceTable) == ONI_ESUCCESS;
}

void OnixSource::updateSettings (OwnedArray<ContinuousChannel>* continuousChannels,
                                 OwnedArray<EventChannel>* eventChannels,
                                 OwnedArray<SpikeChannel>* spikeChannels,
                                 OwnedArray<DataStream>* dataStreams,
                                 OwnedArray<DeviceInfo>* deviceInfos,
                                 OwnedArray<ConfigurationObject>* configurationObjects)
{
    LOGD ("ONIX Source initializing data streams.");

    continuousChannels->clear();
    eventChannels->clear();
    spikeChannels->clear();
    dataStreams->clear();
    deviceInfos->clear();
    configurationObjects->clear();

    updateSourceBuffers();

    if (devicesFound)
    {
        for (const auto& source : sources)
        {
            if (! source->isEnabled())
                continue;

            auto type = source->getDeviceType();

            if (type == OnixDeviceType::NEUROPIXELSV1F)
            {
                DeviceInfo::Settings deviceSettings {
                    source->getName(),
                    "Neuropixels 1.0 Probe",
                    "neuropixels1.probe",
                    "0000000",
                    "imec"
                };

                auto device = new DeviceInfo (deviceSettings);

                MetadataDescriptor descriptor (MetadataDescriptor::MetadataType::UINT16,
                                               1,
                                               "num_adcs",
                                               "Number of analog-to-digital converter for this probe",
                                               "neuropixels.adcs");

                MetadataValue value (MetadataDescriptor::MetadataType::UINT16, 1);
                value.setValue ((uint16) std::static_pointer_cast<Neuropixels1f> (source)->settings[0]->probeMetadata.num_adcs);

                device->addMetadata (descriptor, value);

                deviceInfos->add (device);

                addIndividualStreams (source->streamInfos, dataStreams, deviceInfos, continuousChannels);

                NeuropixelsHelpers::setChannelMetadata (continuousChannels, std::static_pointer_cast<Neuropixels1f> (source)->settings);
            }
            else if (type == OnixDeviceType::BNO || type == OnixDeviceType::POLLEDBNO)
            {
                DeviceInfo::Settings deviceSettings {
                    source->getName(),
                    "Bno055 9-axis IMU",
                    "bno055",
                    "0000000",
                    "Bosch"
                };

                deviceInfos->add (new DeviceInfo (deviceSettings));

                DataStream::Settings dataStreamSettings {
                    source->createStreamName(),
                    "Continuous data from a Bno055 9-axis IMU",
                    source->getStreamIdentifier(),
                    source->streamInfos[0].getSampleRate(),
                    true
                };

                addCombinedStreams (dataStreamSettings, source->streamInfos, dataStreams, deviceInfos, continuousChannels);
            }
            else if (type == OnixDeviceType::NEUROPIXELSV2E)
            {
                DeviceInfo::Settings deviceSettings {
                    source->getName(),
                    "Neuropixels 2.0 Probe",
                    "neuropixels2.probe",
                    "0000000",
                    "imec"
                };

                auto device = new DeviceInfo (deviceSettings);

                MetadataDescriptor descriptor (MetadataDescriptor::MetadataType::UINT16,
                                               1,
                                               "num_adcs",
                                               "Number of analog-to-digital converter for this probe",
                                               "neuropixels.adcs");

                MetadataValue value (MetadataDescriptor::MetadataType::UINT16, 1);
                value.setValue ((uint16) std::static_pointer_cast<Neuropixels2e> (source)->settings[0]->probeMetadata.num_adcs);

                device->addMetadata (descriptor, value);

                deviceInfos->add (device);

                addIndividualStreams (source->streamInfos, dataStreams, deviceInfos, continuousChannels);

                NeuropixelsHelpers::setChannelMetadata (continuousChannels, std::static_pointer_cast<Neuropixels2e> (source)->settings);
            }
            else if (type == OnixDeviceType::MEMORYMONITOR)
            {
                DeviceInfo::Settings deviceSettings {
                    source->getName(),
                    "Memory Monitor",
                    "memorymonitor",
                    "0000000",
                    ""
                };

                deviceInfos->add (new DeviceInfo (deviceSettings));

                addIndividualStreams (source->streamInfos, dataStreams, deviceInfos, continuousChannels);
            }
            else if (type == OnixDeviceType::HARPSYNCINPUT)
            {
                DeviceInfo::Settings deviceSettings {
                    source->getName(),
                    "Harp Sync Input",
                    "harpsyncinput",
                    "0000000",
                    ""
                };

                deviceInfos->add (new DeviceInfo (deviceSettings));

                addIndividualStreams (source->streamInfos, dataStreams, deviceInfos, continuousChannels);
            }
            else if (type == OnixDeviceType::DIGITALIO)
            {
                DeviceInfo::Settings digitalIODeviceSettings {
                    source->getName(),
                    "DigitalIO",
                    "digitalio",
                    "0000000",
                    ""
                };

                deviceInfos->add (new DeviceInfo (digitalIODeviceSettings));

                DataStream::Settings dataStreamSettings {
                    source->createStreamName ("", false),
                    "Digital inputs and buttons",
                    source->getStreamIdentifier(),
                    source->streamInfos[0].getSampleRate(),
                    true
                };

                addCombinedStreams (dataStreamSettings, source->streamInfos, dataStreams, deviceInfos, continuousChannels);

                auto eventChannelSettings = std::static_pointer_cast<DigitalIO> (source)->getEventChannelSettings (dataStreams->getLast());
                eventChannels->add (new EventChannel (eventChannelSettings));
            }
            else if (type == OnixDeviceType::ANALOGIO)
            {
                DeviceInfo::Settings analogIODeviceSettings {
                    source->getName(),
                    "AnalogIO",
                    "analogio",
                    "0000000",
                    ""
                };

                deviceInfos->add (new DeviceInfo (analogIODeviceSettings));

                addIndividualStreams (source->streamInfos, dataStreams, deviceInfos, continuousChannels);
            }
            else if (type == OnixDeviceType::NEUROPIXELSV1E)
            {
                DeviceInfo::Settings deviceSettings {
                    source->getName(),
                    "Neuropixels 1.0 Probe",
                    "neuropixels1.probe",
                    "0000000",
                    "imec"
                };

                auto device = new DeviceInfo (deviceSettings);

                MetadataDescriptor descriptor (MetadataDescriptor::MetadataType::UINT16,
                                               1,
                                               "num_adcs",
                                               "Number of analog-to-digital converter for this probe",
                                               "neuropixels.adcs");

                MetadataValue value (MetadataDescriptor::MetadataType::UINT16, 1);
                value.setValue ((uint16) std::static_pointer_cast<Neuropixels1e> (source)->settings[0]->probeMetadata.num_adcs);

                device->addMetadata (descriptor, value);

                deviceInfos->add (device);

                addIndividualStreams (source->streamInfos, dataStreams, deviceInfos, continuousChannels);

                NeuropixelsHelpers::setChannelMetadata (continuousChannels, std::static_pointer_cast<Neuropixels1e> (source)->settings);
            }
            else if (type == OnixDeviceType::OUTPUTCLOCK)
            {
                continue;
            }
            else
            {
                Onix1::showWarningMessageBoxAsync (
                    "Unknown Source",
                    "Found an unknown source (" + source->getName() + ") on hub " + source->getHubName() + " at address " + std::to_string (source->getDeviceIdx()));
            }
        }
    }
}

void OnixSource::addCombinedStreams (DataStream::Settings dataStreamSettings,
                                     Array<StreamInfo> streamInfos,
                                     OwnedArray<DataStream>* dataStreams,
                                     OwnedArray<DeviceInfo>* deviceInfos,
                                     OwnedArray<ContinuousChannel>* continuousChannels)
{
    DataStream* stream = new DataStream (dataStreamSettings);
    dataStreams->add (stream);
    stream->device = deviceInfos->getLast();

    for (const auto& streamInfo : streamInfos)
    {
        auto suffixes = streamInfo.getChannelNameSuffixes();

        for (int chan = 0; chan < streamInfo.getNumChannels(); chan++)
        {
            auto prefix = streamInfo.getChannelPrefix();

            if (suffixes[chan] != "")
            {
                if (prefix != "")
                    prefix += "-" + suffixes[chan];
                else
                    prefix = suffixes[chan];
            }

            ContinuousChannel::Settings channelSettings {
                streamInfo.getChannelType(),
                prefix,
                streamInfo.getDescription(),
                createContinuousChannelIdentifier (streamInfo, chan),
                streamInfo.getBitVolts(),
                stream
            };
            continuousChannels->add (new ContinuousChannel (channelSettings));
            continuousChannels->getLast()->setUnits (streamInfo.getUnits());
        }
    }
}

void OnixSource::addIndividualStreams (Array<StreamInfo> streamInfos,
                                       OwnedArray<DataStream>* dataStreams,
                                       OwnedArray<DeviceInfo>* deviceInfos,
                                       OwnedArray<ContinuousChannel>* continuousChannels)
{
    for (StreamInfo streamInfo : streamInfos)
    {
        DataStream::Settings streamSettings {
            streamInfo.getName(),
            streamInfo.getDescription(),
            streamInfo.getStreamIdentifier(),
            streamInfo.getSampleRate(),
            true
        };

        DataStream* stream = new DataStream (streamSettings);
        dataStreams->add (stream);
        stream->device = deviceInfos->getLast();

        auto suffixes = streamInfo.getChannelNameSuffixes();

        // Add continuous channels
        for (int chan = 0; chan < streamInfo.getNumChannels(); chan++)
        {
            ContinuousChannel::Settings channelSettings {
                streamInfo.getChannelType(),
                streamInfo.getChannelPrefix() + suffixes[chan],
                streamInfo.getDescription(),
                createContinuousChannelIdentifier (streamInfo, chan),
                streamInfo.getBitVolts(),
                stream
            };
            continuousChannels->add (new ContinuousChannel (channelSettings));
            continuousChannels->getLast()->setUnits (streamInfo.getUnits());
        }
    }
}

std::string OnixSource::createContinuousChannelIdentifier (StreamInfo streamInfo, int channelNumber)
{
    auto dataType = streamInfo.getChannelIdentifierDataType();
    auto subTypes = streamInfo.getChannelIdentifierSubTypes();

    auto channelIdentifier = streamInfo.getStreamIdentifier() + ".continuous." + dataType;

    if (subTypes.size() == streamInfo.getNumChannels())
        channelIdentifier += "." + subTypes[channelNumber];
    else if (subTypes.size() == 1)
        channelIdentifier += "." + subTypes[0];

    return channelIdentifier;
}

bool OnixSource::isDevicesReady()
{
    auto tabMap = editor->createTabMapFromCanvas();
    auto sourceMap = getConnectedDevices (true);

    return tabMap == sourceMap;
}

bool OnixSource::foundInputSource()
{
    return devicesFound;
}

bool OnixSource::checkPortControllerStatus (OnixSourceEditor* editor, std::shared_ptr<PortController> port)
{
    if (editor->isHeadstageSelected (port->getPort()))
    {
        if (! port->checkLinkState())
        {
            Onix1::showWarningMessageBoxAsync ("Port Controller Error", port->getName() + " is not currently connected.");
            return false;
        }
        else if (port->getLinkFlags() != 0)
        {
            Onix1::showWarningMessageBoxAsync ("Port Controller Error", port->getName() + " was disconnected, and must be reconnected.");
            return false;
        }
    }

    return true;
}

bool OnixSource::isReady()
{
    if (context == nullptr || ! devicesFound)
        return false;

    if (! isDevicesReady())
    {
        CoreServices::sendStatusMessage ("Devices not initialized properly. Check error logs for details.");
        return false;
    }

    if (! checkPortControllerStatus (editor, portA) || ! checkPortControllerStatus (editor, portB))
    {
        editor->setConnectedStatus (false); // NB: If either port controller lost lock, disconnect all devices
        return false;
    }

    for (const auto& source : sources)
    {
        if (! source->isEnabled())
            continue;

        if (! source->updateSettings())
            return false;
    }

    uint32_t val = 2;
    int rc = context->setOption (ONI_OPT_RESETACQCOUNTER, val);
    if (rc != ONI_ESUCCESS)
        return false;

    return true;
}

bool OnixSource::startAcquisition()
{
    frameReader.reset();

    enabledSources = getEnabledDataSources();

    enabledSources.emplace_back (portA);
    enabledSources.emplace_back (portB);

    for (const auto& source : enabledSources)
    {
        source->startAcquisition();
    }

    frameReader = std::make_unique<FrameReader> (enabledSources, context);
    frameReader->startThread();

    startThread();

    return true;
}

void OnixSource::disconnectDevicesAfterAcquisition (OnixSourceEditor* editor)
{
    while (CoreServices::getAcquisitionStatus())
        std::this_thread::sleep_for (50ms);

    if (editor != nullptr)
    {
        const MessageManagerLock mmLock;
        editor->setConnectedStatus (false);
    }
}

bool OnixSource::stopAcquisition()
{
    if (isThreadRunning())
        signalThreadShouldExit();

    if (frameReader->isThreadRunning())
        frameReader->signalThreadShouldExit();

    if (! portA->getErrorFlag() && ! portB->getErrorFlag())
        waitForThreadToExit (2000);

    oni_size_t reg = 0;
    context->setOption (ONI_OPT_RUNNING, reg);

    for (const auto& source : enabledSources)
    {
        source->stopAcquisition();
    }

    for (auto buffers : sourceBuffers)
        buffers->clear();

    if (portA->getErrorFlag() || portB->getErrorFlag())
    {
        std::string msg = "";

        if (portA->getErrorFlag())
        {
            msg += "Port A";
        }

        if (portA->getErrorFlag() && portB->getErrorFlag())
        {
            msg += " and ";
        }

        if (portB->getErrorFlag())
        {
            msg += "Port B";
        }

        msg += " lost communication lock during acquisition. Inspect hardware connections and port switch before reconnecting.";

        std::thread t (disconnectDevicesAfterAcquisition, editor);
        t.detach(); // NB: Detach to allow the current thread to finish, stopping acquisition and allowing the called thread to complete

        Onix1::showWarningMessageBoxAsync (
            "Port Communication Lock Lost",
            msg);

        return false;
    }

    return true;
}

bool OnixSource::dataStreamExists(std::string streamName, Array<const DataStream*> dataStreams)
{
	for (const auto& stream : dataStreams)
	{
		if (stream->getName().contains(streamName))
		{
			return true;
		}
	}

	return false;
}

void OnixSource::startRecording()
{
	OnixDeviceVector devicesWithProbeInterface;

	for (const auto& device : getEnabledDataSources())
	{
		if (device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1E
			|| device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1F
			|| device->getDeviceType() == OnixDeviceType::NEUROPIXELSV2E)
		{
			devicesWithProbeInterface.emplace_back(device);
		}
	}

	if (devicesWithProbeInterface.empty())
		return;

	File recPath = CoreServices::getRecordingParentDirectory();

	int recordNodeId = CoreServices::getAvailableRecordNodeIds().getFirst();
	int experimentNumber = CoreServices::RecordNode::getExperimentNumber(recordNodeId);

	auto dir = File(
		recPath.getFullPathName() + File::getSeparatorString() +
		CoreServices::getRecordingDirectoryName() + File::getSeparatorString() +
		PLUGIN_NAME + " " + String(sn->getNodeId()) + File::getSeparatorString() +
		"experiment" + String(experimentNumber));

	if (!dir.exists())
	{
		auto result = dir.createDirectory();

		if (result.failed())
		{
			Onix1::showWarningMessageBoxAsync("Unable to Create ONIX Source Folder",
				"The plugin was unable to create a recording directory at '" + dir.getFullPathName().toStdString() + "'. No Probe Interface files will be saved for this recording, please stop recording and determine why the directory could not be created.");
			return;
		}
	}

	for (const auto& device : devicesWithProbeInterface)
	{
		if (device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1E || device->getDeviceType() == OnixDeviceType::NEUROPIXELSV1F)
		{
			auto npx = std::static_pointer_cast<Neuropixels1>(device);
			auto streamName = npx->createStreamName();

			auto streamExists = dataStreamExists(streamName, sn->getDataStreams());

			if (!streamExists)
				return;

			if (!npx->saveProbeInterfaceFile(dir, streamName))
				return;
		}
		else if (device->getDeviceType() == OnixDeviceType::NEUROPIXELSV2E)
		{
			auto npx = std::static_pointer_cast<Neuropixels2e>(device);

			for (int i = 0; i < npx->getNumProbes(); i++)
			{
				auto streamName = npx->createStreamName(i);

				auto streamExists = dataStreamExists(streamName, sn->getDataStreams());

				if (!streamExists)
					return;

				if (!npx->saveProbeInterfaceFile(dir, streamName, i))
					return;
			}
		}
	}
}

bool OnixSource::updateBuffer()
{
    for (const auto& source : enabledSources)
    {
        source->processFrames();

        if (threadShouldExit())
            return true;
    }

    return ! portA->getErrorFlag() && ! portB->getErrorFlag();
}
