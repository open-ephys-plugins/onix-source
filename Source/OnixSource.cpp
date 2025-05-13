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
		CoreServices::sendStatusMessage("Failed to create context." + String(e.what()));
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
		CoreServices::sendStatusMessage("Failed to create context. " + String(e.what()));
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

void OnixSource::disconnectDevices(bool updateStreamInfo)
{
	sourceBuffers.clear(true);

	sources.clear();
	hubNames.clear();

	devicesFound = false;

	if (context != nullptr && context->isInitialized())
		context->setOption(ONIX_OPT_PASSTHROUGH, 0);

	if (updateStreamInfo) CoreServices::updateSignalChain(editor);
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
	else
	{
		// NB: Create a new device if a tab does not exist, but the device was found while connecting
		LOGD("Creating new device ", deviceName, " on ", hubName);
		device = std::make_shared<Device>(deviceName, hubName, deviceIdx, ctx);
	}

	if (device == nullptr)
	{
		LOGE("Could not create or find ", deviceName, " on ", hubName);
		return false;
	}

	int res = -1;

	try
	{
		res = device->configureDevice();
	}
	catch (const error_str& e)
	{
		LOGE(e.what());
		AlertWindow::showMessageBox(MessageBoxIconType::WarningIcon, "Error", e.what());

		return false;
	}

	sources.emplace_back(device);

	return res == ONI_ESUCCESS;
}

void OnixSource::initializeDevices(bool updateStreamInfo)
{
	if (context == nullptr || !context->isInitialized())
	{
		LOGE("Cannot initialize devices, context is not initialized correctly. Please try removing the plugin and adding it again.");
		return;
	}

	if (devicesFound)
	{
		disconnectDevices(false);
	}

	int val = 0;

	if (getParameter("passthroughA")->getValue())
	{
		LOGD("Passthrough mode enabled for Port A");
		val |= 1 << 0;
	}

	if (getParameter("passthroughB")->getValue())
	{
		LOGD("Passthrough mode enabled for Port B");
		val |= 1 << 2;
	}

	context->setOption(ONIX_OPT_PASSTHROUGH, val);

	context->issueReset();
	int rc = context->updateDeviceTable();

	if (rc != ONI_ESUCCESS) return;

	if (portA->configureDevice() != ONI_ESUCCESS) LOGE("Unable to configure Port A.");
	if (portB->configureDevice() != ONI_ESUCCESS) LOGE("Unable to configure Port B.");

	device_map_t deviceMap = context->getDeviceTable();

	if (deviceMap.size() == 0)
	{
		LOGE("No devices found.");
		if (updateStreamInfo) CoreServices::updateSignalChain(editor);
		return;
	}

	auto hubIds = context->getHubIds();

	if (hubIds.size() == 0)
	{
		LOGE("No hub IDs found.");
		if (updateStreamInfo) CoreServices::updateSignalChain(editor);
		return;
	}

	devicesFound = true;

	const int bufferSizeInSeconds = 10;

	// NB: Search through all hubs, and initialize devices
	for (const auto& [hubIndex, hubId] : hubIds)
	{
		if (hubId == ONIX_HUB_FMCHOST) // NB: Breakout Board
		{
			hubNames.insert({ hubIndex, BREAKOUT_BOARD_NAME });
			bool result = false;
			auto canvas = editor->getCanvas();

			result = configureDevice<Heartbeat>(sources, canvas, "Heartbeat", BREAKOUT_BOARD_NAME, Heartbeat::getDeviceType(), hubIndex, context);
			if (!result) devicesFound = false;

			result = configureDevice<OutputClock>(sources, canvas, "Output Clock", BREAKOUT_BOARD_NAME, OutputClock::getDeviceType(), hubIndex + 5, context);
			if (!result) devicesFound = false;

			result = configureDevice<AnalogIO>(sources, canvas, "Analog IO", BREAKOUT_BOARD_NAME, AnalogIO::getDeviceType(), hubIndex + 6, context);
			if (!result) devicesFound = false;

			result = configureDevice<DigitalIO>(sources, canvas, "Digital IO", BREAKOUT_BOARD_NAME, DigitalIO::getDeviceType(), hubIndex + 7, context);
			if (!result) devicesFound = false;

			result = configureDevice<MemoryMonitor>(sources, canvas, "Memory Monitor", BREAKOUT_BOARD_NAME, MemoryMonitor::getDeviceType(), hubIndex + 10, context);
			if (!result) devicesFound = false;

			result = configureDevice<HarpSyncInput>(sources, canvas, "Harp Sync Input", BREAKOUT_BOARD_NAME, HarpSyncInput::getDeviceType(), hubIndex + 12, context);
			if (!result) devicesFound = false;
		}
		else if (hubId == ONIX_HUB_HSNP)
		{
			hubNames.insert({ hubIndex, NEUROPIXELSV1F_HEADSTAGE_NAME });
			bool result = false;
			auto canvas = editor->getCanvas();

			for (int i = 0; i < 2; i++)
			{
				result = configureDevice<Neuropixels1f>(sources, canvas, "Probe" + std::to_string(i), NEUROPIXELSV1F_HEADSTAGE_NAME, Neuropixels1f::getDeviceType(), hubIndex + i, context);
				if (!result) devicesFound = false;
			}

			result = configureDevice<Bno055>(sources, canvas, "BNO055", NEUROPIXELSV1F_HEADSTAGE_NAME, Bno055::getDeviceType(), hubIndex + 2, context);
			if (!result) devicesFound = false;
		}
	}

	// NB: Search for passthrough devices, and initialize any headstages found in passthrough mode
	for (const auto& [index, device] : deviceMap)
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

			bool result = false;
			auto canvas = editor->getCanvas();

			if (hsid == ONIX_HUB_HSNP2E)
			{
				auto hubIndex = OnixDevice::getHubIndexFromPassthroughIndex(index);

				result = configureDevice<Neuropixels2e>(sources, canvas, "", NEUROPIXELSV2E_HEADSTAGE_NAME, Neuropixels2e::getDeviceType(), hubIndex, context);
				if (!result) devicesFound = false;

				result = configureDevice<PolledBno055>(sources, canvas, "BNO055", NEUROPIXELSV2E_HEADSTAGE_NAME, PolledBno055::getDeviceType(), hubIndex + 1, context);
				if (!result) devicesFound = false;

				if (sources.back()->getDeviceType() != OnixDeviceType::POLLEDBNO)
					LOGE("Unknown device encountered when setting headstage.");

				const auto& polledBno = std::static_pointer_cast<PolledBno055>(sources.back());

				polledBno->setBnoAxisMap(PolledBno055::Bno055AxisMap::YZX);
				polledBno->setBnoAxisSign(PolledBno055::Bno055AxisSign::MirrorXAndY);

				hubNames.insert({ PortController::getOffsetFromIndex(polledBno->getDeviceIdx()), NEUROPIXELSV2E_HEADSTAGE_NAME });
			}
		}
	}

	context->issueReset();

	oni_size_t frameSize;
	rc = context->getOption<oni_size_t>(ONI_OPT_MAXREADFRAMESIZE, &frameSize);
	printf("Max. read frame size: %u bytes\n", frameSize);

	rc = context->getOption<oni_size_t>(ONI_OPT_MAXWRITEFRAMESIZE, &frameSize);
	printf("Max. write frame size: %u bytes\n", frameSize);

	context->setOption(ONI_OPT_BLOCKREADSIZE, block_read_size);

	if (updateStreamInfo) CoreServices::updateSignalChain(editor);

	LOGD("All devices initialized.");
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

OnixDeviceVector OnixSource::getDataSourcesFromPort(PortName port)
{
	OnixDeviceVector devices{};

	for (const auto& source : sources)
	{
		if (PortController::getPortFromIndex(source->getDeviceIdx()) == port)
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

std::shared_ptr<OnixDevice> OnixSource::getDevice(OnixDeviceType type)
{
	for (const auto& device : sources)
	{
		if (device->getDeviceType() == type) return device;
	}

	return nullptr;
}

std::map<int, OnixDeviceType> OnixSource::createDeviceMap(OnixDeviceVector devices, bool filterDevices)
{
	std::map<int, OnixDeviceType> deviceMap;

	for (const auto& device : devices)
	{
		if (filterDevices && (device->getDeviceType() == OnixDeviceType::HEARTBEAT || device->getDeviceType() == OnixDeviceType::MEMORYMONITOR)) continue;

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

bool OnixSource::configurePortVoltage(PortName port, String voltage) const
{
	if (!context->isInitialized()) return false;

	switch (port)
	{
	case PortName::PortA:
		if (voltage == "" || voltage == "Auto")
			return portA->configureVoltage();
		else
			return portA->configureVoltage(voltage.getDoubleValue());
	case PortName::PortB:
		if (voltage == "" || voltage == "Auto")
			return portB->configureVoltage();
		else
			return portB->configureVoltage(voltage.getDoubleValue());
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

	std::shared_ptr<DigitalIO> digitalIO = std::static_pointer_cast<DigitalIO>(getDevice(OnixDeviceType::DIGITALIO));

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
					source->streamInfos[0].getSampleRate()
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

				if (digitalIO != nullptr && digitalIO->isEnabled())
				{
					auto ttlChannelSettings = digitalIO->getEventChannelSettings();
					ttlChannelSettings.stream = dataStreams->getLast();
					eventChannels->add(new EventChannel(ttlChannelSettings));

					std::static_pointer_cast<MemoryMonitor>(source)->setDigitalIO(digitalIO);
				}
			}
			else if (source->getDeviceType() == OnixDeviceType::ANALOGIO)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(),
					"Analog IO",
					"analogio",
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
			streamInfo.getSampleRate()
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

String OnixSource::createContinuousChannelIdentifier(StreamInfo streamInfo, int channelNumber)
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

	auto polledBno055 = getDevice(OnixDeviceType::POLLEDBNO);

	if (polledBno055 != nullptr && polledBno055->isEnabled())
		polledBno055->stopAcquisition(); // NB: Polled BNO must be stopped before other devices to ensure there are no stream clashes

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

		MessageManager::callAsync([] { AlertWindow::showMessageBoxAsync(MessageBoxIconType::WarningIcon, "Port Communication Lock Lost",
			"The port communication lock was lost during acquisition, inspect hardware connections and port switch." +
			String("\n\nTo continue, press disconnect in the GUI, then press connect."), "Okay"); });
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
