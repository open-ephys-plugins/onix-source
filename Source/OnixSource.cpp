/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright (C) 2023 Allen Institute for Brain Science and Open Ephys

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
		LOGE("Failed to create context.");
		return;
	}
	catch (const error_t& e)
	{
		LOGE("Failed to initialize context. ", e.what());
		return;
	}

	// TODO: Add these parameters in the registerParameters() override?
	addBooleanParameter(Parameter::PROCESSOR_SCOPE, "passthroughA", "Passthrough", "Enables passthrough mode for e-variant headstages on Port A", false, true);
	addBooleanParameter(Parameter::PROCESSOR_SCOPE, "passthroughB", "Passthrough", "Enables passthrough mode for e-variant headstages on Port B", false, true);

	addBooleanParameter(Parameter::PROCESSOR_SCOPE, "connected", "Connect", "Connect to Onix hardware", false, true);

	portA = std::make_shared<PortController>(PortName::PortA, context);
	portB = std::make_shared<PortController>(PortName::PortB, context);

	if (!context->isInitialized()) { LOGE("Failed to initialize context."); return; }
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
	headstages.clear();

	devicesFound = false;

	if (updateStreamInfo) CoreServices::updateSignalChain(editor);
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

	// TODO: How to set passthrough for Port A vs. Port B?
	if (getParameter("passthroughA")->getValue() || getParameter("passthroughB")->getValue())
	{
		LOGD("Passthrough mode enabled");
		int val = 1;
		context->setOption(ONIX_OPT_PASSTHROUGH, val);
	}
	else
	{
		int val = 0;
		context->setOption(ONIX_OPT_PASSTHROUGH, val);
	}

	context->issueReset();
	context->updateDeviceTable();

	if (context->getLastResult() != ONI_ESUCCESS) return;

	device_map_t deviceMap = context->getDeviceTable();

	if (deviceMap.size() == 0)
	{
		LOGE("No devices found.");
		if (updateStreamInfo) CoreServices::updateSignalChain(editor);
		return;
	}

	devicesFound = true;

	static const String probeLetters = "ABCDEFGHI";
	const int bufferSizeInSeconds = 10;
	int npxProbeIdx = 0;

	for (const auto& [index, device] : deviceMap)
	{
		if (device.id == ONIX_NEUROPIX1R0)
		{
			auto np1 = std::make_shared<Neuropixels_1>("Probe-" + String::charToString(probeLetters[npxProbeIdx]), index, context);

			int res = np1->configureDevice();

			if (res != 0)
			{
				if (res == -1)
				{
					LOGE("Device Idx: ", index, " Unable to read probe serial number. Device not found.");

					continue;
				}
				else if (res == -2)
				{
					LOGE("Device Idx: ", index, " Error enabling device stream.");

					continue;
				}
				else if (res == -3)
				{
					LOGE("Error enabling device ", index);

					continue;
				}
			}

			sources.push_back(np1);
			headstages.insert({ PortController::getPortFromIndex(index), NEUROPIXELSV1F_HEADSTAGE_NAME });

			npxProbeIdx++;
		}
		else if (device.id == ONIX_BNO055)
		{
			auto bno = std::make_shared<Bno055>("BNO055", index, context);

			int result = bno->configureDevice();

			if (result != 0)
			{
				LOGE("Device Idx: ", index, " Error enabling device stream.");
				continue;
			}

			sources.push_back(bno);
		}
		else if (device.id == ONIX_DS90UB9RAW)
		{
			LOGD("Passthrough device detected");
			//initialize main i2c parameter
			auto serializer = std::make_unique<I2CRegisterContext>(DS90UB9x::SER_ADDR, index, context);
			serializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xSerializerI2CRegister::SCLHIGH, 20);
			serializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xSerializerI2CRegister::SCLLOW, 20);

			auto EEPROM = std::make_unique<HeadStageEEPROM>(index, context);
			uint32_t hsid = EEPROM->GetHeadStageID();
			LOGD("Detected headstage ", hsid);
			if (hsid == 8) //Npix2.0e headstage, constant needs to be added to onix.h
			{
				auto np2 = std::make_shared<Neuropixels2e>("Probe-" + String::charToString(probeLetters[npxProbeIdx]), index, context);
				int res = np2->configureDevice();
				if (res != 0)
				{
					if (res == -1)
					{
						LOGE("Device Idx: ", index, " Unable to read probe serial number. Device not found.");
					}
					//TODO add other errors if needed
					continue;
				}
				npxProbeIdx += np2->getNumProbes();

				sources.push_back(np2);
			}
		}
		else if (devices[dev_idx].id == ONIX_MEMUSAGE)
		{
			auto memoryMonitor = std::make_shared<MemoryMonitor>("Memory Monitor", devices[dev_idx].idx, ctx);

			int result = memoryMonitor->configureDevice();

			if (result != 0)
			{
				LOGE("Device Idx: ", devices[dev_idx].idx, " Error enabling device stream.");
				continue;
			}

			sources.push_back(memoryMonitor);
		}
		else if (devices[dev_idx].id == ONIX_FMCCLKOUT1R3)
		{
			auto outputClock = std::make_shared<OutputClock>("Output Clock", devices[dev_idx].idx, ctx);

			int result = outputClock->configureDevice();

			if (result != 0)
			{
				LOGE("Device Idx: ", devices[dev_idx].idx, " Error enabling device stream.");
				continue;
			}

			sources.push_back(outputClock);
		}
		else if (devices[dev_idx].id == ONIX_HEARTBEAT)
		{
			auto heartbeat = std::make_shared<Heartbeat>("Heartbeat", devices[dev_idx].idx, ctx);

			int result = heartbeat->configureDevice();

			if (result != 0)
			{
				LOGE("Device Idx: ", devices[dev_idx].idx, " Error enabling device stream.");
				continue;
			}

			sources.push_back(heartbeat);
		}
		else if (devices[dev_idx].id == ONIX_HARPSYNCINPUT)
		{
			auto harpSyncInput = std::make_shared<HarpSyncInput>("Harp Sync Input", devices[dev_idx].idx, ctx);

			int result = harpSyncInput->configureDevice();

			if (result != 0)
			{
				LOGE("Device Idx: ", devices[dev_idx].idx, " Error enabling device stream.");
				continue;
			}

			sources.push_back(harpSyncInput);
		}
		else if (devices[dev_idx].id == ONIX_FMCANALOG1R3)
		{
			auto analogIO = std::make_shared<AnalogIO>("Analog IO", devices[dev_idx].idx, ctx);

			int result = analogIO->configureDevice();

			if (result != 0)
			{
				LOGE("Device Idx: ", devices[dev_idx].idx, " Error enabling device stream.");
				continue;
			}

			sources.push_back(analogIO);
		}
	}

	if (portA->configureDevice() != ONI_ESUCCESS) LOGE("Unable to configure Port A.");
	if (portB->configureDevice() != ONI_ESUCCESS) LOGE("Unable to configure Port B.");

	context->issueReset();

	oni_size_t frameSize = context->getOption<oni_size_t>(ONI_OPT_MAXREADFRAMESIZE);
	printf("Max. read frame size: %u bytes\n", frameSize);

	frameSize = context->getOption<oni_size_t>(ONI_OPT_MAXWRITEFRAMESIZE);
	printf("Max. write frame size: %u bytes\n", frameSize);

	context->setOption(ONI_OPT_BLOCKREADSIZE, block_read_size);

	if (updateStreamInfo) CoreServices::updateSignalChain(editor);

	LOGD("All devices initialized.");
}

OnixDeviceVector OnixSource::getDataSources() const
{
	OnixDeviceVector devices{};

	for (const auto& source : sources)
	{
		devices.push_back(source);
	}

	return devices;
}

OnixDeviceVector OnixSource::getDataSourcesFromPort(PortName port) const
{
	OnixDeviceVector devices{};

	for (const auto& source : sources)
	{
		if (PortController::getPortFromIndex(source->getDeviceIdx()) == port)
			devices.push_back(source);
	}

	return devices;
}

std::map<int, OnixDeviceType> OnixSource::createDeviceMap(OnixDeviceVector devices)
{
	std::map<int, OnixDeviceType> deviceMap;

	for (const auto& device : devices)
	{
		deviceMap.insert({ device->getDeviceIdx(), device->type });
	}

	return deviceMap;
}

std::map<int, OnixDeviceType> OnixSource::createDeviceMap()
{
	return createDeviceMap(getDataSources());
}

std::map<PortName, String> OnixSource::getHeadstageMap()
{
	return headstages;
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
			return portA->configureVoltage(voltage.getFloatValue());
	case PortName::PortB:
		if (voltage == "" || voltage == "Auto")
			return portB->configureVoltage();
		else
			return portB->configureVoltage(voltage.getFloatValue());
	default:
		return false;
	}
}

void OnixSource::setPortVoltage(PortName port, float voltage) const
{
	if (!context->isInitialized()) return;

	switch (port)
	{
	case PortName::PortA:
		portA->setVoltageOverride(voltage);
		return;
	case PortName::PortB:
		portB->setVoltageOverride(voltage);
		return;
	default:
		return;
	}
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

			// create device info object
			if (source->type == OnixDeviceType::NEUROPIXELS_1)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(),
					"Neuropixels 1.0 Probe",
					"neuropixels1.probe",
					"0000000",
					"imec"
				};

				deviceInfos->add(new DeviceInfo(deviceSettings));
			}
			else if (source->type == OnixDeviceType::BNO)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(),
					"Bno055 9-axis IMU",
					"bno055",
					"0000000",
					"Bosch"
				};

				deviceInfos->add(new DeviceInfo(deviceSettings));
			}
			else if (source->type == OnixDeviceType::NEUROPIXELS_2)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(),
					"Neuropixels 2.0 Probe",
					"neuropixels2.probe",
					"0000000",
					"imec"
				};

				deviceInfos->add(new DeviceInfo(deviceSettings));
			}
			else if (source->type == OnixDeviceType::MEMORYMONITOR)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(),
					"Memory Monitor",
					"memorymonitor",
					"0000000",
					""
				};

				deviceInfos->add(new DeviceInfo(deviceSettings));
			}
			else if (source->type == OnixDeviceType::ANALOGIO)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(),
					"Analog IO",
					"analogio",
					"0000000",
					""
				};

				deviceInfos->add(new DeviceInfo(deviceSettings));
			}

			// add data streams and channels
			for (const auto& streamInfo : source->streams)
			{
				DataStream::Settings streamSettings
				{
					streamInfo.name,
					streamInfo.description,
					streamInfo.identifier,
					streamInfo.sampleRate
				};

				DataStream* stream = new DataStream(streamSettings);
				dataStreams->add(stream);
				stream->device = deviceInfos->getLast();

				// Add continuous channels
				for (int chan = 0; chan < streamInfo.numChannels; chan++)
				{
					ContinuousChannel::Settings channelSettings{
						streamInfo.channelType,
						streamInfo.channelPrefix + String(chan + 1),
						streamInfo.description,
						streamInfo.identifier,
						streamInfo.bitVolts,
						stream
					};
					continuousChannels->add(new ContinuousChannel(channelSettings));
				}
			}
		}
	}
}

bool OnixSource::isDevicesReady()
{
	auto tabMap = editor->createTabMapFromCanvas();
	auto sourceMap = createDeviceMap(sources);

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
		CoreServices::sendStatusMessage("Selected headstages do not match hardware found.");
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
	context->setOption(ONI_OPT_RESETACQCOUNTER, val);
	if (context->getLastResult() != ONI_ESUCCESS) return false;

	return true;
}

bool OnixSource::startAcquisition()
{
	frameReader.reset();

	OnixDeviceVector devices;

	for (const auto& source : sources)
	{
		devices.push_back(source);
	}

	devices.push_back(portA);
	devices.push_back(portB);

	for (const auto& source : devices)
	{
		if (!source->isEnabled()) continue;

		source->startAcquisition();
	}

	frameReader = std::make_unique<FrameReader>(devices, context);
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

	if (devicesFound)
	{
		oni_size_t reg = 0;
		context->setOption(ONI_OPT_RUNNING, reg);
		if (context->getLastResult() != ONI_ESUCCESS) return false;

		uint32_t val = 1;
		context->setOption(ONI_OPT_RESET, val);
	}

	for (const auto& source : sources)
	{
		if (!source->isEnabled()) continue;

		source->stopAcquisition();
	}

	portA->stopAcquisition();
	portB->stopAcquisition();

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
	for (const auto& source : sources)
	{
		if (!source->isEnabled()) continue;

		source->processFrames();
	}

	portA->processFrames();
	portB->processFrames();

	return !portA->getErrorFlag() && !portB->getErrorFlag();
}
