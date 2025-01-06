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
#include "OnixSourceEditor.h"

OnixSource::OnixSource(SourceNode* sn) :
	DataThread(sn),
	ctx(NULL),
	devicesFound(false),
	editor(NULL)
{
	addBooleanParameter(Parameter::PROCESSOR_SCOPE, "is_passthrough_A", "Passthrough Mode", "Enables passthrough mode for e-variant headstages", false, true);

	LOGD("ONIX Source creating ONI context.");
	ctx = oni_create_ctx("riffa"); // "riffa" is the PCIe driver name
	if (ctx == NULL) { LOGE("Failed to create context."); return; }

	// Initialize context and discover hardware
	initializeContext();

	if (!contextInitialized) { LOGE("Failed to initialize context."); return; }

	setPortVoltage((oni_dev_idx_t)PortName::PortB, 5.0);
	initializeDevices();

	if (!devicesFound) { return; }

	oni_size_t frame_size = 0;
	size_t frame_size_sz = sizeof(frame_size);
	oni_get_opt(ctx, ONI_OPT_MAXREADFRAMESIZE, &frame_size, &frame_size_sz);
	printf("Max. read frame size: %u bytes\n", frame_size);

	oni_get_opt(ctx, ONI_OPT_MAXWRITEFRAMESIZE, &frame_size, &frame_size_sz);
	printf("Max. write frame size: %u bytes\n", frame_size);

	size_t block_size_sz = sizeof(block_read_size);

	// set block read size
	oni_set_opt(ctx, ONI_OPT_BLOCKREADSIZE, &block_read_size, block_size_sz);
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

void OnixSource::initializeDevices(bool updateStreamInfo)
{
	if (!contextInitialized)
	{
		initializeContext();

		if (!contextInitialized) { LOGD("Unable to initialize context. Cannot initialize devices."); return; }
	}

	if (devicesFound)
	{
		sourceBuffers.clear(true);
		sources.clear(true);
		devicesFound = false;
	}

	oni_size_t num_devs = 0;
	oni_device_t* devices = NULL;

	uint32_t val = 0;

	if (getParameter("is_passthrough_A")->getValue())
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
	if (devices == NULL) { LOGE("No devices found."); return; }
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
			Neuropixels_1* np1 = new Neuropixels_1("Probe-" + String::charToString(probeLetters[npxProbeIdx]), editor->portVoltage, editor->adcCalibrationFile->getText(), editor->gainCalibrationFile->getText(), devices[dev_idx].idx, ctx);

			int res = np1->enableDevice();

			if (res != 0)
			{
				if (res == -1)
				{
					LOGE("Device Idx: ", devices[dev_idx].idx, " Unable to read probe serial number. Device not found.");
				}
				else if (res == -2)
				{
					LOGE("Device Idx: ", devices[dev_idx].idx, " Error enabling device stream.");
				}
				else if (res == -3)
				{
					LOGE("Missing calibration file(s). Ensure that all calibration files exist and the file paths are correct.");
				}

				delete np1;
				continue;
			}

			sources.add(np1);

			for (StreamInfo streamInfo : np1->streams)
			{
				sourceBuffers.add(new DataBuffer(streamInfo.numChannels, (int)streamInfo.sampleRate * bufferSizeInSeconds));

				if (streamInfo.channelPrefix.equalsIgnoreCase("AP"))
					np1->apBuffer = sourceBuffers.getLast();
				else if (streamInfo.channelPrefix.equalsIgnoreCase("LFP"))
					np1->lfpBuffer = sourceBuffers.getLast();
			}

			npxProbeIdx++;
		}
		else if (devices[dev_idx].id == ONIX_BNO055)
		{
			auto bno = std::make_unique<Bno055>("BNO-" + String::charToString(probeLetters[bnoIdx]), devices[dev_idx].idx, ctx);

			int result = bno->enableDevice();

			if (result != 0)
			{
				LOGE("Device Idx: ", devices[dev_idx].idx, " Error enabling device stream.");
				continue;
			}

			for (StreamInfo streamInfo : bno->streams)
			{
				sourceBuffers.add(new DataBuffer(streamInfo.numChannels, (int)streamInfo.sampleRate * bufferSizeInSeconds));

				if (streamInfo.channelPrefix.equalsIgnoreCase("Euler"))
					bno->eulerBuffer = sourceBuffers.getLast();
				else if (streamInfo.channelPrefix.equalsIgnoreCase("Quaternion"))
					bno->quaternionBuffer = sourceBuffers.getLast();
				else if (streamInfo.channelPrefix.equalsIgnoreCase("Acceleration"))
					bno->accelerationBuffer = sourceBuffers.getLast();
				else if (streamInfo.channelPrefix.equalsIgnoreCase("Gravity"))
					bno->gravityBuffer = sourceBuffers.getLast();
				else if (streamInfo.channelPrefix.equalsIgnoreCase("Temperature"))
					bno->temperatureBuffer = sourceBuffers.getLast();
			}

			sources.add(bno.release());

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
				auto np2 = std::make_unique<Neuropixels2e>("Probe-" + String::charToString(probeLetters[npxProbeIdx]), devices[dev_idx].idx, ctx);
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

				int bufferIdx = 0;
				for (StreamInfo streamInfo : np2->streams)
				{
					sourceBuffers.add(new DataBuffer(streamInfo.numChannels, (int)streamInfo.sampleRate * bufferSizeInSeconds));
					np2->apBuffer[bufferIdx++] = sourceBuffers.getLast();
				}
				sources.add(np2.release());
			}
		}
	}

	val = 1;
	oni_set_opt(ctx, ONI_OPT_RESET, &val, sizeof(val));

	if (updateStreamInfo) CoreServices::updateSignalChain(editor);

	LOGD("All devices initialized.");
}

Array<OnixDevice*> OnixSource::getDataSources()
{
	Array<OnixDevice*> devices;

	for (auto source : sources)
	{
		devices.add(source);
	}

	return devices;
}

bool OnixSource::setPortVoltage(oni_dev_idx_t port, int voltage) const
{
	if (!contextInitialized) return false;

	const oni_reg_addr_t voltageRegister = 3;

	auto result = oni_write_reg(ctx, port, voltageRegister, 0);

	sleep(500);

	result = oni_write_reg(ctx, port, voltageRegister, voltage);

	if (result != 0) { LOGE(oni_error_str(result)); return -1; }

	sleep(200);

	return result;
}

void OnixSource::updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
	OwnedArray<EventChannel>* eventChannels,
	OwnedArray<SpikeChannel>* spikeChannels,
	OwnedArray<DataStream>* dataStreams,
	OwnedArray<DeviceInfo>* deviceInfos,
	OwnedArray<ConfigurationObject>* configurationObjects)
{

	LOGD("ONIX Source initializing data streams.");

	dataStreams->clear();
	eventChannels->clear();
	continuousChannels->clear();
	spikeChannels->clear();
	deviceInfos->clear();
	configurationObjects->clear();

	if (devicesFound)
	{
		DataStream* currentStream;

		for (auto source : sources)
		{
			// create device info object
			if (source->type == OnixDeviceType::NEUROPIXELS_1)
			{
				DeviceInfo::Settings deviceSettings{
					source->getName(), // device name
					"Neuropixels 1.0 Probe",
					"neuropixels1.probe",
					"0000000",
					"imec"
				};

				DeviceInfo* device = new DeviceInfo(deviceSettings);
				deviceInfos->add(device); // unique device object owned by SourceNode
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
					source->getName(), // device name
					"Neuropixels 2.0 Probe",
					"neuropixels1.probe",
					"0000000",
					"imec"
				};
			}

			// add data streams and channels
			for (StreamInfo streamInfo : source->streams)
			{
				DataStream::Settings apStreamSettings
				{
					streamInfo.name, // stream name
					streamInfo.description, // stream description
					streamInfo.identifier, // stream identifier
					streamInfo.sampleRate // sample rate

				};

				DataStream* stream = new DataStream(apStreamSettings);
				dataStreams->add(stream);
				currentStream = stream;
				currentStream->device = deviceInfos->getLast();

				// Add continuous channels
				for (int chan = 0; chan < streamInfo.numChannels; chan++)
				{
					ContinuousChannel::Settings channelSettings{
						streamInfo.channelType,
						streamInfo.channelPrefix + String(chan + 1),
						streamInfo.description,
						streamInfo.identifier,
						streamInfo.bitVolts, // bitVolts
						currentStream
					};
					continuousChannels->add(new ContinuousChannel(channelSettings));
				}
			}
		}
	}
}

bool OnixSource::foundInputSource()
{
	return devicesFound;
}

bool OnixSource::isReady()
{
	if (!devicesFound)
		return false;

	File adcFile = File(editor->adcCalibrationFile->getText());
	File gainFile = File(editor->gainCalibrationFile->getText());

	if (!adcFile.existsAsFile() || !gainFile.existsAsFile())
	{
		LOGE("Missing calibration file(s). Ensure that all calibration files exist and the file paths are correct.");
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

	for (auto source : sources)
	{
		source->startAcquisition();
	}

	return true;
}

bool OnixSource::stopAcquisition()
{
	if (isThreadRunning())
		signalThreadShouldExit();

	waitForThreadToExit(2000);

	for (auto source : sources)
	{
		source->stopAcquisition();
	}

	for (auto buffers : sourceBuffers)
		buffers->clear();

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

	return true;
}

bool OnixSource::updateBuffer()
{
	const int nSamps = 120;
	oni_frame_t* frame;

	for (int samp = 0; samp < nSamps; samp++)
	{
		int res = oni_read_frame(ctx, &frame);

		if (res < ONI_ESUCCESS)
		{
			LOGE("Error reading ONI frame: ", oni_error_str(res), " code ", res);
			return false;
		}

		bool destroyFrame = true;

		for (auto source : sources)
		{
			if (frame->dev_idx == source->getDeviceIdx())
			{
				source->addFrame(frame);
				destroyFrame = false;
			}
		}

		if (destroyFrame)
		{
			oni_destroy_frame(frame);
		}
	}

	return true;
}
