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
	ctx(NULL),
	devicesFound(false),
	editor(NULL)
{
	addBooleanParameter(Parameter::PROCESSOR_SCOPE, "passthroughA", "Passthrough", "Enables passthrough mode for e-variant headstages on Port A", false, true);
	addBooleanParameter(Parameter::PROCESSOR_SCOPE, "passthroughB", "Passthrough", "Enables passthrough mode for e-variant headstages on Port B", false, true);

	addBooleanParameter(Parameter::PROCESSOR_SCOPE, "connected", "Connect", "Connect to Onix hardware", false, true);

	LOGD("ONIX Source creating ONI context.");
	ctx = oni_create_ctx("riffa"); // "riffa" is the PCIe driver name
	if (ctx == NULL) { LOGE("Failed to create context."); return; }

	// Initialize context and discover hardware
	initializeContext();

	if (!contextInitialized) { LOGE("Failed to initialize context."); return; }
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

void OnixSource::initializeContext()
{
	if (contextInitialized) { LOGD("Context is already initialized."); return; }

	if (ctx == NULL) 
	{
		LOGD("ONIX Source creating ONI context.");
		ctx = oni_create_ctx("riffa"); // "riffa" is the PCIe driver name
		if (ctx == NULL) { LOGE("Failed to create context."); return; }
	}

	int errorCode = oni_init_ctx(ctx, 0);

	if (errorCode) { LOGE(oni_error_str(errorCode)); return; }

	contextInitialized = true;
}

void OnixSource::disconnectDevices(bool updateStreamInfo)
{
	sourceBuffers.clear(true);

	for (std::shared_ptr<OnixDevice> source : sources)
	{
		source.reset();
	}

	sources.clear();

	devicesFound = false;

	if (updateStreamInfo) CoreServices::updateSignalChain(editor);
}

void OnixSource::initializeDevices(bool updateStreamInfo)
{
	if (!contextInitialized)
	{
		initializeContext();

		if (!contextInitialized) { LOGD("Unable to initialize context. Cannot initialize devices."); return; }
	}

	if (devicesFound)
	{
		disconnectDevices(false);
	}

	oni_size_t num_devs = 0;
	oni_device_t* devices = NULL;

	uint32_t val = 0;

	if (getParameter("passthroughA")->getValue())
	{
		LOGD("Passthrough mode enabled");
		val = 1;
	}
	oni_set_opt(ctx, ONIX_OPT_PASSTHROUGH, &val, sizeof(val));

	val = 1;
	oni_set_opt(ctx, ONI_OPT_RESET, &val, sizeof(val));

	// Examine device table
	size_t num_devs_sz = sizeof(num_devs);
	oni_get_opt(ctx, ONI_OPT_NUMDEVICES, &num_devs, &num_devs_sz);

	size_t devices_sz = sizeof(oni_device_t) * num_devs;
	devices = (oni_device_t*)realloc(devices, devices_sz);

	if (devices == NULL) 
	{ 
		LOGE("No devices found."); 
		if (updateStreamInfo) CoreServices::updateSignalChain(editor);
		return;
	}

	oni_get_opt(ctx, ONI_OPT_DEVICETABLE, devices, &devices_sz);

	devicesFound = true;

#ifdef DEBUG

	// print device info
	printf("   +--------------------+-------+-------+-------+-------+---------------------\n");
	printf("   |        \t\t|  \t|Firm.\t|Read\t|Wrt. \t|     \n");
	printf("   |Dev. idx\t\t|ID\t|ver. \t|size\t|size \t|Desc.\n");
	printf("   +--------------------+-------+-------+-------+-------+---------------------\n");

	for (size_t dev_idx = 0; dev_idx < num_devs; dev_idx++) {
		if (devices[dev_idx].id != ONIX_NULL) {

			const char* dev_str = onix_device_str(devices[dev_idx].id);

			printf("%02zd |%05zd: 0x%02x.0x%02x\t|%d\t|%d\t|%u\t|%u\t|%s\n",
				dev_idx,
				devices[dev_idx].idx,
				(uint8_t)(devices[dev_idx].idx >> 8),
				(uint8_t)devices[dev_idx].idx,
				devices[dev_idx].id,
				devices[dev_idx].version,
				devices[dev_idx].read_size,
				devices[dev_idx].write_size,
				dev_str);
		}
	}
#endif

	static const String probeLetters = "ABCDEFGHI";
	const int bufferSizeInSeconds = 10;
	int npxProbeIdx = 0;
	int bnoIdx = 0;

	for (size_t dev_idx = 0; dev_idx < num_devs; dev_idx++)
	{
		if (devices[dev_idx].id == ONIX_NEUROPIX1R0)
		{
			auto np1 = std::make_shared<Neuropixels_1>("Probe-" + String::charToString(probeLetters[npxProbeIdx]), devices[dev_idx].idx, ctx);

			int res = np1->enableDevice();

			if (res != 0)
			{
				if (res == -1)
				{
					LOGE("Device Idx: ", devices[dev_idx].idx, " Unable to read probe serial number. Device not found.");

					continue;
				}
				else if (res == -2)
				{
					LOGE("Device Idx: ", devices[dev_idx].idx, " Error enabling device stream.");

					continue;
				}
				else if (res == -3 || res == -4)
				{
					LOGE("Missing or invalid calibration file(s). Ensure that all calibration files exist and the file paths are correct.");
				}
			}

			np1->addSourceBuffers(sourceBuffers);

			sources.push_back(np1);

			npxProbeIdx++;
		}
		else if (devices[dev_idx].id == ONIX_BNO055)
		{
			auto bno = std::make_shared<Bno055>("BNO-" + String::charToString(probeLetters[bnoIdx]), devices[dev_idx].idx, ctx);

			int result = bno->enableDevice();

			if (result != 0)
			{
				LOGE("Device Idx: ", devices[dev_idx].idx, " Error enabling device stream.");
				continue;
			}

			bno->addSourceBuffers(sourceBuffers);

			sources.push_back(bno);

			bnoIdx++;
		}
		else if (devices[dev_idx].id == ONIX_DS90UB9RAW)
		{
			LOGD("Passthrough device detected");
			//initialize main i2c parameter
			auto serializer = std::make_unique<I2CRegisterContext>(DS90UB9x::SER_ADDR, devices[dev_idx].idx, ctx);
			serializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xSerializerI2CRegister::SCLHIGH, 20);
			serializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xSerializerI2CRegister::SCLLOW, 20);
			
			auto EEPROM = std::make_unique<HeadStageEEPROM>(devices[dev_idx].idx, ctx);
			uint32_t hsid = EEPROM->GetHeadStageID();
			LOGD("Detected headstage ", hsid);
			if (hsid == 8) //Npix2.0e headstage, constant needs to be added to onix.h
			{
				auto np2 = std::make_shared<Neuropixels2e>("Probe-" + String::charToString(probeLetters[npxProbeIdx]), devices[dev_idx].idx, ctx);
				int res = np2->enableDevice();
				if (res != 0)
				{
					if (res == -1)
					{
						LOGE("Device Idx: ", devices[dev_idx].idx, " Unable to read probe serial number. Device not found.");
					}
					//TODO add other errors if needed
					continue;
				}
				npxProbeIdx += np2->getNumProbes();

				np2->addSourceBuffers(sourceBuffers);

				sources.push_back(np2);
			}
		}
	}

	val = 1;
	oni_set_opt(ctx, ONI_OPT_RESET, &val, sizeof(val));

	oni_size_t frame_size = 0;
	size_t frame_size_sz = sizeof(frame_size);
	oni_get_opt(ctx, ONI_OPT_MAXREADFRAMESIZE, &frame_size, &frame_size_sz);
	printf("Max. read frame size: %u bytes\n", frame_size);

	oni_get_opt(ctx, ONI_OPT_MAXWRITEFRAMESIZE, &frame_size, &frame_size_sz);
	printf("Max. write frame size: %u bytes\n", frame_size);

	// set block read size
	size_t block_size_sz = sizeof(block_read_size);
	oni_set_opt(ctx, ONI_OPT_BLOCKREADSIZE, &block_read_size, block_size_sz);

	if (updateStreamInfo) CoreServices::updateSignalChain(editor);

	LOGD("All devices initialized.");
}

std::vector<std::shared_ptr<OnixDevice>> OnixSource::getDataSources()
{
	std::vector<std::shared_ptr<OnixDevice>> devices;

	for (std::shared_ptr<OnixDevice> source : sources)
	{
		devices.push_back(source);
	}

	return devices;
}

void OnixSource::updateSourceBuffers()
{
	sourceBuffers.clear(true);

	for (std::shared_ptr<OnixDevice> source : sources)
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
		portA.updateDiscoveryParameters(parameters);
		break;
	case PortName::PortB:
		portB.updateDiscoveryParameters(parameters);
		break;
	default:
		break;
	}
}

bool OnixSource::configurePortVoltage(PortName port, String voltage) const
{
	if (!contextInitialized) return false;

	switch (port)
	{
	case PortName::PortA:
		if (voltage == "") return portA.configureVoltage(ctx);
		else			   return portA.configureVoltage(ctx, voltage.getFloatValue());
	case PortName::PortB:
		if (voltage == "") return portB.configureVoltage(ctx);
		else			   return portB.configureVoltage(ctx, voltage.getFloatValue());
	default:
		return false;
	}
}

bool OnixSource::setPortVoltage(PortName port, float voltage) const
{
	if (!contextInitialized) return false;

	switch (port)
	{
	case PortName::PortA:
		return portA.setVoltage(ctx, voltage);
	case PortName::PortB:
		return portB.setVoltage(ctx, voltage);
	default:
		return false;
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
		for (std::shared_ptr<OnixDevice> source : sources)
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
					"neuropixels1.probe",
					"0000000",
					"imec"
				};

				deviceInfos->add(new DeviceInfo(deviceSettings));
			}

			// add data streams and channels
			for (StreamInfo streamInfo : source->streams)
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

bool OnixSource::foundInputSource()
{
	return devicesFound; // TODO: Check here if the settings tabs match the hardware; only return true if the hardware and tabs match
}

bool OnixSource::isReady()
{
	if (!devicesFound)
		return false;

	if (editor->isHeadstageSelected(PortName::PortA) && !portA.checkLinkState(ctx)) return false;
	if (editor->isHeadstageSelected(PortName::PortB) && !portB.checkLinkState(ctx)) return false;

	for (std::shared_ptr<OnixDevice> source : sources)
	{
		if (!source->isEnabled()) continue;

		int result = source->updateSettings();

		if (result != 0)
			return false;
	}

	oni_reg_val_t reg = 2;
	int res = oni_set_opt(ctx, ONI_OPT_RESETACQCOUNTER, &reg, sizeof(oni_size_t));
	if (res < ONI_ESUCCESS)
	{
		LOGE("Error starting acquisition: ", oni_error_str(res), " code ", res);

		return false;
	}

	return true;
}

bool OnixSource::startAcquisition()
{
	startThread();

	frameReader.reset();

	frameReader = std::make_unique<FrameReader>(sources, ctx);
	frameReader->startThread();

	for (std::shared_ptr<OnixDevice> source : sources)
	{
		if (!source->isEnabled()) continue;

		source->startAcquisition();
	}

	return true;
}

bool OnixSource::stopAcquisition()
{
	if (isThreadRunning())
		signalThreadShouldExit();

	if (frameReader->isThreadRunning())
		frameReader->signalThreadShouldExit();

	waitForThreadToExit(2000);

	if (devicesFound)
	{
		oni_size_t reg = 0;
		int res = oni_set_opt(ctx, ONI_OPT_RUNNING, &reg, sizeof(reg));
		if (res < ONI_ESUCCESS)
		{
			LOGE("Error stopping acquisition: ", oni_error_str(res), " code ", res);
			return false;
		}

		uint32_t val = 1;
		oni_set_opt(ctx, ONI_OPT_RESET, &val, sizeof(val));
		oni_set_opt(ctx, ONI_OPT_BLOCKREADSIZE, &block_read_size, sizeof(block_read_size));
	}

	for (std::shared_ptr<OnixDevice> source : sources)
	{
		if (!source->isEnabled()) continue;

		source->stopAcquisition();
	}

	for (auto buffers : sourceBuffers)
		buffers->clear();

	return true;
}

bool OnixSource::updateBuffer()
{
	for (std::shared_ptr<OnixDevice> source : sources)
	{
		if (!source->isEnabled()) continue;

		source->processFrames();
	}

	return true;
}
