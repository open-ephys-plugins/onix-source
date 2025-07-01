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
#include "OnixSourceCanvas.h"
#include "Devices/DeviceList.h"

using namespace OnixSourcePlugin;

OnixSource::OnixSource(SourceNode* sn) :
	DataThread(sn),
	devicesFound(false),
	editor(NULL)
{
	try
	{
		context = std::make_shared<Onix1>();
	}
	catch (const std::system_error& e)
	{
		LOGE("Failed to create context. ", e.what());
		CoreServices::sendStatusMessage("Failed to create context." + std::string(e.what()));
		AlertWindow::showMessageBox(
			MessageBoxIconType::WarningIcon,
			"Failed to Create Context",
			"There was an error creating the context. Check the error logs for more details."
		);
		return;
	}
	catch (const error_t& e)
	{
		LOGE("Failed to initialize context. ", e.what());
		CoreServices::sendStatusMessage("Failed to create context. " + std::string(e.what()));
		AlertWindow::showMessageBox(
			MessageBoxIconType::WarningIcon,
			"Failed to Initialize Context",
			"There was an error initializing the context. Check the error logs for more details."
		);
		return;
	}

	portA = std::make_shared<PortController>(PortName::PortA, context);
	portB = std::make_shared<PortController>(PortName::PortB, context);

	if (!context->isInitialized()) { LOGE("Failed to initialize context."); return; }
}

OnixSource::~OnixSource()
{
	if (context != nullptr && context->isInitialized())
	{
		portA->setVoltageOverride(0.0f, false);
		portB->setVoltageOverride(0.0f, false);
	}
}

std::string OnixSource::getLiboniVersion()
{
	if (context != nullptr && context->isInitialized())
		return context->getVersion();
	else
		return "";
}

void OnixSource::registerParameters()
{
	addBooleanParameter(Parameter::PROCESSOR_SCOPE, "passthroughA", "Passthrough A", "Enables passthrough mode for e-variant headstages on Port A", false, true);
	addBooleanParameter(Parameter::PROCESSOR_SCOPE, "passthroughB", "Passthrough B", "Enables passthrough mode for e-variant headstages on Port B", false, true);
}

DataThread* OnixSource::createDataThread(SourceNode* sn)
{
	return new OnixSource(sn);
}

std::unique_ptr<GenericEditor> OnixSource::createEditor(SourceNode* sn)
{
	std::unique_ptr<OnixSourceEditor> e = std::make_unique<OnixSourceEditor>(sn, this);
	editor = e.get();

	return e;
}

bool OnixSource::disconnectDevices(bool updateStreamInfo)
{
	sourceBuffers.clear(true);

	sources.clear();
	hubNames.clear();

	devicesFound = false;

	if (context != nullptr && context->isInitialized())
	{
		if (context->setOption(ONIX_OPT_PASSTHROUGH, 0) != ONI_ESUCCESS)
		{
			LOGE("Unable to set passthrough option when disconnecting devices.");
			return false;
		}
	}

	if (updateStreamInfo) CoreServices::updateSignalChain(editor);

	return true;
}

template <class Device>
bool OnixSource::configureDevice(OnixDeviceVector& sources,
	OnixSourceCanvas* canvas,
	std::string deviceName,
	std::string hubName,
	OnixDeviceType deviceType,
	const oni_dev_idx_t deviceIdx,
	std::shared_ptr<Onix1> ctx)
{
	std::shared_ptr<Device> device = std::static_pointer_cast<Device>(canvas->getDevicePtr(Device::getDeviceType(), deviceIdx));

	if (device != nullptr)
	{
		if (device->getName() != deviceName || device->getHubName() != hubName)
		{
			LOGD("Difference in names found for device at address ", deviceIdx, ". Found ", deviceName, " on ", hubName, ", but was expecting ", device->getName(), " on ", device->getHubName());
		}
	}
	else if (device->getDeviceType() == OnixDeviceType::MEMORYMONITOR)
	{// NB: These are devices with no equivalent settings tab that still need to be created and added to the vector of devices
		LOGD("Creating new device ", deviceName, " on ", hubName);
		device = std::make_shared<Device>(deviceName, hubName, deviceIdx, ctx);
	}

	if (device == nullptr)
	{
		Onix1::showWarningMessageBoxAsync("Device Not Found", "Could not find " + deviceName + ", at address " + std::to_string(deviceIdx) + " on " + hubName);
		return false;
	}

	int res = 1;

	try
	{
		res = device->configureDevice();
	}
	catch (const error_str& e)
	{
		LOGE(e.what());
		AlertWindow::showMessageBox(MessageBoxIconType::WarningIcon, "Configuration Error", e.what());

		return false;
	}

	sources.emplace_back(device);

	return res == ONI_ESUCCESS;
}

bool OnixSource::getHubFirmwareVersion(std::shared_ptr<Onix1> ctx, uint32_t hubIndex, uint32_t* firmwareVersion)
{
	if (ctx->readRegister(hubIndex + ONIX_HUB_DEV_IDX, ONIX_HUB_FIRMWAREVER, firmwareVersion) != ONI_ESUCCESS)
	{
		LOGE("Unable to read the hub firmware version at index ", hubIndex);
		return false;
	}

	return true;
}

bool OnixSource::enablePassthroughMode(std::shared_ptr<Onix1> ctx, bool passthroughA, bool passthroughB)
{
	if (ctx == nullptr || !ctx->isInitialized())
	{
		Onix1::showWarningMessageBoxAsync("Invalid Context", "Cannot enable passthrough mode, context is not initialized correctly.");
		return false;
	}

	int val = 0;

	if (passthroughA)
	{
		LOGD("Passthrough mode enabled for Port A");
		val |= 1 << 0;
	}

	if (passthroughB)
	{
		LOGD("Passthrough mode enabled for Port B");
		val |= 1 << 2;
	}

	return ctx->setOption(ONIX_OPT_PASSTHROUGH, val) == ONI_ESUCCESS;
}

bool OnixSource::configurePort(PortName port)
{
	if (context == nullptr || !context->isInitialized())
	{
		Onix1::showWarningMessageBoxAsync("Invalid Context", "Cannot initialize devices, context is not initialized correctly. Please try removing the plugin and adding it again.");
		return false;
	}

	if (port == PortName::PortA)
	{
		if (portA->configureDevice() != ONI_ESUCCESS)
		{
			Onix1::showWarningMessageBoxAsync("Configuration Error", "Unable to configure Port A.");
			return false;
		}
	}
	else if (port == PortName::PortB)
	{
		if (portB->configureDevice() != ONI_ESUCCESS)
		{
			Onix1::showWarningMessageBoxAsync("Configuration Error", "Unable to configure Port B.");
			return false;
		}
	}

	return true;
}

bool OnixSource::checkHubFirmwareCompatibility(std::shared_ptr<Onix1> context, device_map_t deviceTable)
{
	auto hubIds = context->getHubIds(deviceTable);

	if (hubIds.size() == 0)
	{
		LOGE("No hub IDs found.");
		return false;
	}

	for (const auto& [hubIndex, hubId] : hubIds)
	{
		if (hubId == ONIX_HUB_FMCHOST) // NB: Breakout Board
		{
			static constexpr int RequiredMajorVersion = 2;
			uint32_t firmwareVersion = 0;
			if (!getHubFirmwareVersion(context, hubIndex, &firmwareVersion))
			{
				return false;
			}

			auto majorVersion = (firmwareVersion & 0xFF00) >> 8;
			auto minorVersion = firmwareVersion & 0xFF;

			LOGD("Breakout board firmware version: v", majorVersion, ".", minorVersion);

			if (majorVersion != RequiredMajorVersion)
			{
				Onix1::showWarningMessageBoxAsync(
					"Invalid Firmware Version",
					"The breakout board firmware major version is v" + std::to_string(majorVersion) +
					", but this plugin is only compatible with v" + std::to_string(RequiredMajorVersion) +
					". To use this plugin, upgrade the firmware to a version that supports the breakout board v" + std::to_string(majorVersion));
				return false;
			}
		}
	}

	return true;
}

bool OnixSource::initializeDevices(device_map_t deviceTable, bool updateStreamInfo)
{
	if (context == nullptr || !context->isInitialized())
	{
		Onix1::showWarningMessageBoxAsync("Invalid Context", "Cannot initialize devices, context is not initialized correctly. Please try removing the plugin and adding it again.");
		return false;
	}

	if (deviceTable.size() == 0)
	{
		LOGE("No devices found.");
		if (updateStreamInfo) CoreServices::updateSignalChain(editor);
		return false;
	}

	auto hubIds = context->getHubIds(deviceTable);

	if (hubIds.size() == 0)
	{
		LOGE("No hub IDs found.");
		if (updateStreamInfo) CoreServices::updateSignalChain(editor);
		return false;
	}

	devicesFound = false;

	// NB: Search through all hubs, and initialize devices
	for (const auto& [hubIndex, hubId] : hubIds)
	{
		if (hubId == ONIX_HUB_FMCHOST) // NB: Breakout Board
		{
			hubNames.insert({ hubIndex, BREAKOUT_BOARD_NAME });
			auto canvas = editor->getCanvas();

			devicesFound = configureDevice<OutputClock>(sources, canvas, "Output Clock", BREAKOUT_BOARD_NAME, OutputClock::getDeviceType(), hubIndex + 5, context);
			if (!devicesFound)
			{
				sources.clear();
				return false;
			}

			// NB: Configures AnalogIO and DigitalIO
			devicesFound = configureDevice<AuxiliaryIO>(sources, canvas, "Auxiliary IO", BREAKOUT_BOARD_NAME, AuxiliaryIO::getDeviceType(), hubIndex + 6, context);
			if (!devicesFound)
			{
				sources.clear();
				return false;
			}

			devicesFound = configureDevice<MemoryMonitor>(sources, canvas, "Memory Monitor", BREAKOUT_BOARD_NAME, MemoryMonitor::getDeviceType(), hubIndex + 10, context);
			if (!devicesFound)
			{
				sources.clear();
				return false;
			}

			devicesFound = configureDevice<HarpSyncInput>(sources, canvas, "Harp Sync Input", BREAKOUT_BOARD_NAME, HarpSyncInput::getDeviceType(), hubIndex + 12, context);
			if (!devicesFound)
			{
				sources.clear();
				return false;
			}
		}
		else if (hubId == ONIX_HUB_HSNP)
		{
			hubNames.insert({ hubIndex, NEUROPIXELSV1F_HEADSTAGE_NAME });
			auto canvas = editor->getCanvas();

			for (int i = 0; i < 2; i++)
			{
				devicesFound = configureDevice<Neuropixels1f>(sources, canvas, "Probe" + std::to_string(i), NEUROPIXELSV1F_HEADSTAGE_NAME, Neuropixels1f::getDeviceType(), hubIndex + i, context);
				if (!devicesFound)
				{
					sources.clear();
					return false;
				}
			}

			devicesFound = configureDevice<Bno055>(sources, canvas, "BNO055", NEUROPIXELSV1F_HEADSTAGE_NAME, Bno055::getDeviceType(), hubIndex + 2, context);
			if (!devicesFound)
			{
				sources.clear();
				return false;
			}
		}
	}

	// NB: Search for passthrough devices, and initialize any headstages found in passthrough mode
	for (const auto& [index, device] : deviceTable)
	{
		if (device.id == ONIX_DS90UB9RAW)
		{
			LOGD("Passthrough device detected");

			auto serializer = std::make_unique<I2CRegisterContext>(DS90UB9x::SER_ADDR, index, context);
			serializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xSerializerI2CRegister::SCLHIGH, 20);
			serializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xSerializerI2CRegister::SCLLOW, 20);

			auto EEPROM = std::make_unique<HeadStageEEPROM>(index, context);
			uint32_t hsid = EEPROM->GetHeadStageID();
			LOGD("Detected headstage ", hsid);

			auto canvas = editor->getCanvas();

			if (hsid == ONIX_HUB_HSNP2E)
			{
				auto hubIndex = OnixDevice::getHubIndexFromPassthroughIndex(index);

				devicesFound = configureDevice<Neuropixels2e>(sources, canvas, "", NEUROPIXELSV2E_HEADSTAGE_NAME, Neuropixels2e::getDeviceType(), hubIndex, context);
				if (!devicesFound)
				{
					sources.clear();
					return false;
				}

				devicesFound = configureDevice<PolledBno055>(sources, canvas, "BNO055", NEUROPIXELSV2E_HEADSTAGE_NAME, PolledBno055::getDeviceType(), hubIndex + 1, context);
				if (!devicesFound)
				{
					sources.clear();
					return false;
				}

				if (sources.back()->getDeviceType() != OnixDeviceType::POLLEDBNO)
				{
					LOGE("Unknown device encountered when configuring headstage ", NEUROPIXELSV2E_HEADSTAGE_NAME);
					devicesFound = false;
					sources.clear();
					return false;
				}

				const auto& polledBno = std::static_pointer_cast<PolledBno055>(sources.back());

				polledBno->setBnoAxisMap(PolledBno055::Bno055AxisMap::YZX);
				polledBno->setBnoAxisSign((uint32_t)(PolledBno055::Bno055AxisSign::MirrorX) | (uint32_t)(PolledBno055::Bno055AxisSign::MirrorY));

				hubNames.insert({ PortController::getOffsetFromIndex(polledBno->getDeviceIdx()), NEUROPIXELSV2E_HEADSTAGE_NAME });
			}
			else if (hsid == 0xFFFFFFFF || hsid == ONIX_HUB_HSNP1ET || hsid == ONIX_HUB_HSNP1EH) // TODO: Remove the 0xFFFFFFFF before publishing
			{
				auto hubIndex = OnixDevice::getHubIndexFromPassthroughIndex(index);

				devicesFound = configureDevice<Neuropixels1e>(sources, canvas, "Probe", NEUROPIXELSV1E_HEADSTAGE_NAME, Neuropixels1e::getDeviceType(), hubIndex, context);
				if (!devicesFound)
				{
					sources.clear();
					return false;
				}

				devicesFound = configureDevice<PolledBno055>(sources, canvas, "BNO055", NEUROPIXELSV1E_HEADSTAGE_NAME, PolledBno055::getDeviceType(), hubIndex + 1, context);
				if (!devicesFound)
				{
					sources.clear();
					return false;
				}

				if (sources.back()->getDeviceType() != OnixDeviceType::POLLEDBNO)
				{
					LOGE("Unknown device encountered when setting headstage.");
					devicesFound = false;
					sources.clear();
					return false;
				}

				const auto& polledBno = std::static_pointer_cast<PolledBno055>(sources.back());

				polledBno->setBnoAxisMap(PolledBno055::Bno055AxisMap::YZX);
				polledBno->setBnoAxisSign((uint32_t)(PolledBno055::Bno055AxisSign::MirrorX) | (uint32_t)(PolledBno055::Bno055AxisSign::MirrorZ));

				hubNames.insert({ PortController::getOffsetFromIndex(polledBno->getDeviceIdx()), NEUROPIXELSV1E_HEADSTAGE_NAME });
			}
		}
	}

	context->issueReset();

	if (updateStreamInfo) CoreServices::updateSignalChain(editor);

	LOGD("All devices initialized.");
	return devicesFound;
}

bool OnixSource::configureBlockReadSize(std::shared_ptr<Onix1> context, uint32_t blockReadSize)
{
	if (context == nullptr || !context->isInitialized())
	{
		Onix1::showWarningMessageBoxAsync("Invalid Context", "Cannot set block read size, context is not initialized correctly. Please try removing the plugin and adding it again.");
		return false;
	}

	oni_size_t readFrameSize;
	int rc = context->getOption<oni_size_t>(ONI_OPT_MAXREADFRAMESIZE, &readFrameSize);
	if (rc == ONI_ESUCCESS)
	{
		LOGD("Max read frame size: ", readFrameSize, " bytes");
	}
	else
	{
		LOGE("Unable to get read frame size.");
		return false;
	}

	oni_size_t writeFrameSize;
	rc = context->getOption<oni_size_t>(ONI_OPT_MAXWRITEFRAMESIZE, &writeFrameSize);
	if (rc == ONI_ESUCCESS)
	{
		LOGD("Max write frame size: ", writeFrameSize, " bytes");
	}
	else
	{
		LOGE("Unable to get write frame size.");
		return false;
	}

	if (!writeBlockReadSize(context, blockReadSize, readFrameSize))
	{
		return false;
	}

	return true;
}

uint32_t OnixSource::getBlockReadSize() const
{
	return blockReadSize;
}

void OnixSource::setBlockReadSize(uint32_t newReadSize)
{
	blockReadSize = newReadSize;
}

bool OnixSource::writeBlockReadSize(std::shared_ptr<Onix1> context, uint32_t blockReadSize, uint32_t readFrameSize)
{
	if (context == nullptr || !context->isInitialized())
	{
		Onix1::showWarningMessageBoxAsync("Invalid Context", "Cannot initialize devices, context is not initialized correctly. Please try removing the plugin and adding it again.");
		return false;
	}

	if (blockReadSize < readFrameSize)
	{
		Onix1::showWarningMessageBoxAsync("Invalid Block Read Size", "The block read size is too small. The max read frame size is " + std::to_string(readFrameSize) + ", but the block read size is " + std::to_string(blockReadSize) + ".\n\nTo continue, increase the block read size to be greater than " + std::to_string(readFrameSize) + " and reconnect.");
		return false;
	}

	int rc = context->setOption(ONI_OPT_BLOCKREADSIZE, blockReadSize);

	if (rc != ONI_ESUCCESS)
	{
		LOGE("Unknown error found when setting block read size to ", blockReadSize);
		return false;
	}

	LOGD("Block read size: ", blockReadSize, " bytes");

	return true;
}

OnixDeviceVector OnixSource::getDataSources()
{
	OnixDeviceVector devices{};

	for (const auto& source : sources)
	{
		devices.emplace_back(source);
	}

	return devices;
}

OnixDeviceVector OnixSource::getEnabledDataSources()
{
	OnixDeviceVector devices{};

	for (const auto& source : sources)
	{
		if (source->isEnabled())
			devices.emplace_back(source);
	}

	return devices;
}

OnixDeviceVector OnixSource::getDataSourcesFromOffset(int offset)
{
	OnixDeviceVector devices{};
	offset = PortController::getOffsetFromIndex(offset);

	for (const auto& source : sources)
	{
		if (PortController::getOffsetFromIndex(source->getDeviceIdx()) == offset)
			devices.emplace_back(source);
	}

	return devices;
}

std::shared_ptr<OnixDevice> OnixSource::getDevice(OnixDeviceType type, int offset)
{
	for (const auto& device : sources)
	{
		if (device->getDeviceType() == type) return device;
	}

	return nullptr;
}

OnixDeviceVector OnixSource::getDevices(OnixDeviceType type)
{
	OnixDeviceVector foundDevices{};

	for (const auto& device : sources)
	{
		if (device->getDeviceType() == type)
			foundDevices.emplace_back(device);
	}

	return foundDevices;
}

std::map<int, OnixDeviceType> OnixSource::createDeviceMap(OnixDeviceVector devices, bool filterDevices)
{
	std::map<int, OnixDeviceType> deviceMap;

	for (const auto& device : devices)
	{
		if (filterDevices && (device->getDeviceType() == OnixDeviceType::MEMORYMONITOR)) continue;

		deviceMap.insert({ device->getDeviceIdx(), device->getDeviceType() });
	}

	return deviceMap;
}

std::map<int, OnixDeviceType> OnixSource::createDeviceMap(bool filterDevices)
{
	return createDeviceMap(getEnabledDataSources(), filterDevices);
}

std::map<int, std::string> OnixSource::getHubNames()
{
	return hubNames;
}

void OnixSource::updateSourceBuffers()
{
	sourceBuffers.clear(true);

	for (const auto& source : sources)
	{
		if (source->isEnabled())
		{
			source->addSourceBuffers(sourceBuffers);
		}
	}
}

void OnixSource::updateDiscoveryParameters(PortName port, DiscoveryParameters parameters)
{
	switch (port)
	{
	case PortName::PortA:
		portA->updateDiscoveryParameters(parameters);
		break;
	case PortName::PortB:
		portB->updateDiscoveryParameters(parameters);
		break;
	default:
		break;
	}
}

bool OnixSource::configurePortVoltage(PortName port, std::string voltage) const
{
	if (!context->isInitialized()) return false;

	switch (port)
	{
	case PortName::PortA:
		if (voltage == "" || voltage == "Auto")
			return portA->configureVoltage();
		else
			return portA->configureVoltage(std::stod(voltage));
	case PortName::PortB:
		if (voltage == "" || voltage == "Auto")
			return portB->configureVoltage();
		else
			return portB->configureVoltage(std::stod(voltage));
	default:
		return false;
	}
}

void OnixSource::setPortVoltage(PortName port, double voltage) const
{
	if (!context->isInitialized()) return;

	bool waitToSettle = voltage > 0;

	switch (port)
	{
	case PortName::PortA:
		portA->setVoltageOverride(voltage, waitToSettle);
		return;
	case PortName::PortB:
		portB->setVoltageOverride(voltage, waitToSettle);
		return;
	default:
		return;
	}
}

double OnixSource::getLastVoltageSet(PortName port)
{
	if (!context->isInitialized()) return 0.0;

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

bool OnixSource::getDeviceTable(device_map_t* deviceTable)
{
	return context->getDeviceTable(deviceTable) == ONI_ESUCCESS;
}

void OnixSource::updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
	OwnedArray<EventChannel>* eventChannels,
	OwnedArray<SpikeChannel>* spikeChannels,
	OwnedArray<DataStream>* dataStreams,
	OwnedArray<DeviceInfo>* deviceInfos,
	OwnedArray<ConfigurationObject>* configurationObjects)
{
	LOGD("ONIX Source initializing data streams.");

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
			if (!source->isEnabled()) continue;

			if (source->getDeviceType() == OnixDeviceType::NEUROPIXELSV1F)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(),
					"Neuropixels 1.0 Probe",
					"neuropixels1.probe",
					"0000000",
					"imec"
				};

				deviceInfos->add(new DeviceInfo(deviceSettings));

				addIndividualStreams(source->streamInfos, dataStreams, deviceInfos, continuousChannels);
			}
			else if (source->getDeviceType() == OnixDeviceType::BNO || source->getDeviceType() == OnixDeviceType::POLLEDBNO)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(),
					"Bno055 9-axis IMU",
					"bno055",
					"0000000",
					"Bosch"
				};

				deviceInfos->add(new DeviceInfo(deviceSettings));

				DataStream::Settings dataStreamSettings{
					OnixDevice::createStreamName({OnixDevice::getPortNameFromIndex(source->getDeviceIdx()), source->getHubName(), source->getName()}),
					"Continuous data from a Bno055 9-axis IMU",
					source->getStreamIdentifier(),
					source->streamInfos[0].getSampleRate(),
					true
				};

				addCombinedStreams(dataStreamSettings, source->streamInfos, dataStreams, deviceInfos, continuousChannels);
			}
			else if (source->getDeviceType() == OnixDeviceType::NEUROPIXELSV2E)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(),
					"Neuropixels 2.0 Probe",
					"neuropixels2.probe",
					"0000000",
					"imec"
				};

				deviceInfos->add(new DeviceInfo(deviceSettings));

				addIndividualStreams(source->streamInfos, dataStreams, deviceInfos, continuousChannels);
			}
			else if (source->getDeviceType() == OnixDeviceType::MEMORYMONITOR)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(),
					"Memory Monitor",
					"memorymonitor",
					"0000000",
					""
				};

				deviceInfos->add(new DeviceInfo(deviceSettings));

				addIndividualStreams(source->streamInfos, dataStreams, deviceInfos, continuousChannels);
			}
			else if (source->getDeviceType() == OnixDeviceType::HARPSYNCINPUT)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(),
					"Harp Sync Input",
					"harpsyncinput",
					"0000000",
					""
				};

				deviceInfos->add(new DeviceInfo(deviceSettings));

				addIndividualStreams(source->streamInfos, dataStreams, deviceInfos, continuousChannels);
			}
			else if (source->getDeviceType() == OnixDeviceType::NEUROPIXELSV1E)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(),
					"Neuropixels 1.0 Probe",
					"neuropixels1.probe",
					"0000000",
					"imec"
				};

				deviceInfos->add(new DeviceInfo(deviceSettings));

				addIndividualStreams(source->streamInfos, dataStreams, deviceInfos, continuousChannels);
			}
			else if (source->getDeviceType() == OnixDeviceType::HEARTBEAT ||
				source->getDeviceType() == OnixDeviceType::PERSISTENTHEARTBEAT || 
				source->getDeviceType() == OnixDeviceType::OUTPUTCLOCK)
			{
				continue;
			}
			else if (source->getDeviceType() == OnixDeviceType::COMPOSITE)
			{
				auto compositeDevice = std::static_pointer_cast<CompositeDevice>(source);

				if (compositeDevice->getCompositeDeviceType() == CompositeDeviceType::AUXILIARYIO)
				{
					DeviceInfo::Settings deviceSettings{
						source->getName(),
						"Auxiliary device containing analog and digital IO data",
						"auxiliaryio",
						"0000000",
						""
					};

					deviceInfos->add(new DeviceInfo(deviceSettings));

					auto auxiliaryIO = std::static_pointer_cast<AuxiliaryIO>(compositeDevice);

					addIndividualStreams(auxiliaryIO->getAnalogIO()->streamInfos, dataStreams, deviceInfos, continuousChannels);

					auto eventChannelSettings = auxiliaryIO->getDigitalIO()->getEventChannelSettings(dataStreams->getLast());
					eventChannels->add(new EventChannel(eventChannelSettings));
				}
				else
				{
					Onix1::showWarningMessageBoxAsync(
						"Unknown Composite Source", 
						"Found an unknown composite source (" + source->getName() + ") on hub " + source->getHubName() +
						" at address " + std::to_string(source->getDeviceIdx()));
				}
			}
			else
			{
				Onix1::showWarningMessageBoxAsync(
					"Unknown Source", 
					"Found an unknown source (" + source->getName() + ") on hub " + source->getHubName() +
					" at address " + std::to_string(source->getDeviceIdx()));
			}
		}
	}
}

void OnixSource::addCombinedStreams(DataStream::Settings dataStreamSettings,
	Array<StreamInfo> streamInfos,
	OwnedArray<DataStream>* dataStreams,
	OwnedArray<DeviceInfo>* deviceInfos,
	OwnedArray<ContinuousChannel>* continuousChannels)
{
	DataStream* stream = new DataStream(dataStreamSettings);
	dataStreams->add(stream);
	stream->device = deviceInfos->getLast();

	for (const auto& streamInfo : streamInfos)
	{
		auto suffixes = streamInfo.getChannelNameSuffixes();

		for (int chan = 0; chan < streamInfo.getNumChannels(); chan++)
		{
			auto prefix = streamInfo.getChannelPrefix();

			if (suffixes[chan] != "")
				prefix += "-" + suffixes[chan];

			ContinuousChannel::Settings channelSettings{
				streamInfo.getChannelType(),
				prefix,
				streamInfo.getDescription(),
				createContinuousChannelIdentifier(streamInfo, chan),
				streamInfo.getBitVolts(),
				stream
			};
			continuousChannels->add(new ContinuousChannel(channelSettings));
			continuousChannels->getLast()->setUnits(streamInfo.getUnits());
		}
	}
}

void OnixSource::addIndividualStreams(Array<StreamInfo> streamInfos,
	OwnedArray<DataStream>* dataStreams,
	OwnedArray<DeviceInfo>* deviceInfos,
	OwnedArray<ContinuousChannel>* continuousChannels)
{
	for (StreamInfo streamInfo : streamInfos)
	{
		DataStream::Settings streamSettings
		{
			streamInfo.getName(),
			streamInfo.getDescription(),
			streamInfo.getStreamIdentifier(),
			streamInfo.getSampleRate(),
			true
		};

		DataStream* stream = new DataStream(streamSettings);
		dataStreams->add(stream);
		stream->device = deviceInfos->getLast();

		auto suffixes = streamInfo.getChannelNameSuffixes();

		// Add continuous channels
		for (int chan = 0; chan < streamInfo.getNumChannels(); chan++)
		{
			ContinuousChannel::Settings channelSettings{
				streamInfo.getChannelType(),
				streamInfo.getChannelPrefix() + suffixes[chan],
				streamInfo.getDescription(),
				createContinuousChannelIdentifier(streamInfo, chan),
				streamInfo.getBitVolts(),
				stream
			};
			continuousChannels->add(new ContinuousChannel(channelSettings));
			continuousChannels->getLast()->setUnits(streamInfo.getUnits());
		}
	}
}

std::string OnixSource::createContinuousChannelIdentifier(StreamInfo streamInfo, int channelNumber)
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
	auto sourceMap = createDeviceMap(true);

	return tabMap == sourceMap;
}

bool OnixSource::foundInputSource()
{
	return devicesFound;
}

bool OnixSource::isReady()
{
	if (context == nullptr || !devicesFound)
		return false;

	if (!isDevicesReady())
	{
		CoreServices::sendStatusMessage("Devices not initialized properly. Check error logs for details.");
		return false;
	}

	if (editor->isHeadstageSelected(PortName::PortA) && !portA->checkLinkState()) return false;
	if (editor->isHeadstageSelected(PortName::PortB) && !portB->checkLinkState()) return false;

	for (const auto& source : sources)
	{
		if (!source->isEnabled()) continue;

		if (!source->updateSettings())
			return false;
	}

	uint32_t val = 2;
	int rc = context->setOption(ONI_OPT_RESETACQCOUNTER, val);
	if (rc != ONI_ESUCCESS) return false;

	return true;
}

bool OnixSource::startAcquisition()
{
	frameReader.reset();

	enabledSources = getEnabledDataSources();

	enabledSources.emplace_back(portA);
	enabledSources.emplace_back(portB);

	for (const auto& source : enabledSources)
	{
		source->startAcquisition();
	}

	frameReader = std::make_unique<FrameReader>(enabledSources, context);
	frameReader->startThread();

	startThread();

	return true;
}

bool OnixSource::stopAcquisition()
{
	if (isThreadRunning())
		signalThreadShouldExit();

	if (frameReader->isThreadRunning())
		frameReader->signalThreadShouldExit();

	if (!portA->getErrorFlag() && !portB->getErrorFlag())
		waitForThreadToExit(2000);

	for (const auto& source : enabledSources)
	{
		source->stopAcquisition();
	}

	oni_size_t reg = 0;
	context->setOption(ONI_OPT_RUNNING, reg);

	for (auto buffers : sourceBuffers)
		buffers->clear();

	if (portA->getErrorFlag() || portB->getErrorFlag())
	{
		if (portA->getErrorFlag())
		{
			LOGE("Port A lost communication lock. Reconnect hardware to continue.");
			CoreServices::sendStatusMessage("Port A lost communication lock");
		}

		if (portB->getErrorFlag())
		{
			LOGE("Port B lost communication lock. Reconnect hardware to continue.");
			CoreServices::sendStatusMessage("Port B lost communication lock");
		}

		devicesFound = false;

		Onix1::showWarningMessageBoxAsync("Port Communication Lock Lost", "The port communication lock was lost during acquisition. Inspect hardware connections and port switch. \n\nTo continue, press disconnect in the GUI, then press connect.");
	}

	return true;
}

bool OnixSource::updateBuffer()
{
	for (const auto& source : enabledSources)
	{
		source->processFrames();

		if (threadShouldExit()) return true;
	}

	portA->processFrames();
	portB->processFrames();

	return !portA->getErrorFlag() && !portB->getErrorFlag();
}
