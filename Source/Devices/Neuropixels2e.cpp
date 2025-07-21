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

#include "Neuropixels2e.h"

using namespace OnixSourcePlugin;

Neuropixels2e::Neuropixels2e(std::string name, std::string hubName, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx_) :
	OnixDevice(name, hubName, Neuropixels2e::getDeviceType(), deviceIdx_, ctx_, true),
	I2CRegisterContext(ProbeI2CAddress, deviceIdx_, ctx_),
	INeuropixel(NeuropixelsV2eValues::numberOfSettings, NeuropixelsV2eValues::numberOfShanks)
{
	probeSN.fill(0);
  frameCount.fill(0);
  sampleNumber.fill(0);

	for (int i = 0; i < NeuropixelsV2eValues::numberOfSettings; i++)
	{
		defineMetadata(settings[i].get(), NeuropixelsV2eValues::numberOfShanks);
	}

	for (int i = 0; i < NumberOfProbes; i++)
		eventCodes[i].fill(0);
}

Neuropixels2e::~Neuropixels2e()
{
	if (serializer != nullptr)
	{
		selectProbe(NoProbeSelected);
		serializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xSerializerI2CRegister::GPIO10, DefaultGPO10Config);
	}

	if (deviceContext != nullptr && deviceContext->isInitialized())
		deviceContext->setOption(ONIX_OPT_PASSTHROUGH, 0);
}

void Neuropixels2e::createDataStream(int n)
{
	StreamInfo apStream = StreamInfo(
		OnixDevice::createStreamName({ getPortName(getDeviceIdx()), getHubName(), "Probe" + std::to_string(n) }),
		"Neuropixels 2.0 data stream",
		getStreamIdentifier(),
		numberOfChannels,
		sampleRate,
		"CH",
		ContinuousChannel::Type::ELECTRODE,
		0.195f,
		"uV",
		{},
		"ap"
	);
	streamInfos.add(apStream);
}

int Neuropixels2e::getNumProbes() const
{
	return m_numProbes;
}

void Neuropixels2e::selectElectrodesInRange(std::vector<int>& selection, int startIndex, int numberOfElectrodes)
{
	for (int i = startIndex; i < startIndex + numberOfElectrodes; i++)
		selection.emplace_back(i);
}

void Neuropixels2e::selectElectrodesAcrossShanks(std::vector<int>& selection, int startIndex, int numberOfElectrodes)
{
	for (int shank = 0; shank < 4; shank++)
	{
		auto shankOffset = NeuropixelsV2eValues::electrodesPerShank * shank;
		for (int i = startIndex + shankOffset; i < startIndex + numberOfElectrodes + shankOffset; i++)
		{
			selection.emplace_back(i);
		}
	}
}

std::vector<int> Neuropixels2e::selectElectrodeConfiguration(int electrodeConfigurationIndex)
{
	static int numberOfElectrodesAcrossShanks = 96;

	std::vector<int> selection;

	if (numberOfShanks == 1)
	{
		if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationSingleShank::BankA)
		{
			selectElectrodesInRange(selection, 0, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationSingleShank::BankB)
		{
			selectElectrodesInRange(selection, numberOfChannels, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationSingleShank::BankC)
		{
			selectElectrodesInRange(selection, numberOfChannels * 2, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationSingleShank::BankD)
		{
			static int32_t bankDOffset = 896;
			selectElectrodesInRange(selection, bankDOffset, numberOfChannels);
		}
	}
	else if (numberOfShanks == 4)
	{
		if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::Shank1BankA)
		{
			selectElectrodesInRange(selection, 0, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::Shank1BankB)
		{
			selectElectrodesInRange(selection, numberOfChannels, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::Shank1BankC)
		{
			selectElectrodesInRange(selection, numberOfChannels * 2, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::Shank2BankA)
		{
			selectElectrodesInRange(selection, NeuropixelsV2eValues::electrodesPerShank, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::Shank2BankB)
		{
			selectElectrodesInRange(selection, NeuropixelsV2eValues::electrodesPerShank + numberOfChannels, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::Shank2BankC)
		{
			selectElectrodesInRange(selection, NeuropixelsV2eValues::electrodesPerShank + numberOfChannels * 2, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::Shank3BankA)
		{
			selectElectrodesInRange(selection, NeuropixelsV2eValues::electrodesPerShank * 2, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::Shank3BankB)
		{
			selectElectrodesInRange(selection, NeuropixelsV2eValues::electrodesPerShank * 2 + numberOfChannels, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::Shank3BankC)
		{
			selectElectrodesInRange(selection, NeuropixelsV2eValues::electrodesPerShank * 2 + numberOfChannels * 2, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::Shank4BankA)
		{
			selectElectrodesInRange(selection, NeuropixelsV2eValues::electrodesPerShank * 3, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::Shank4BankB)
		{
			selectElectrodesInRange(selection, NeuropixelsV2eValues::electrodesPerShank * 3 + numberOfChannels, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::Shank4BankC)
		{
			selectElectrodesInRange(selection, NeuropixelsV2eValues::electrodesPerShank * 3 + numberOfChannels * 2, numberOfChannels);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks1To96)
		{
			selectElectrodesAcrossShanks(selection, 0, numberOfElectrodesAcrossShanks);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks97To192)
		{
			selectElectrodesAcrossShanks(selection, numberOfElectrodesAcrossShanks, numberOfElectrodesAcrossShanks);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks193To288)
		{
			selectElectrodesAcrossShanks(selection, numberOfElectrodesAcrossShanks * 2, numberOfElectrodesAcrossShanks);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks289To384)
		{
			selectElectrodesAcrossShanks(selection, numberOfElectrodesAcrossShanks * 3, numberOfElectrodesAcrossShanks);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks385To480)
		{
			selectElectrodesAcrossShanks(selection, numberOfElectrodesAcrossShanks * 4, numberOfElectrodesAcrossShanks);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks481To576)
		{
			selectElectrodesAcrossShanks(selection, numberOfElectrodesAcrossShanks * 5, numberOfElectrodesAcrossShanks);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks577To672)
		{
			selectElectrodesAcrossShanks(selection, numberOfElectrodesAcrossShanks * 6, numberOfElectrodesAcrossShanks);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks673To768)
		{
			selectElectrodesAcrossShanks(selection, numberOfElectrodesAcrossShanks * 7, numberOfElectrodesAcrossShanks);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks769To864)
		{
			selectElectrodesAcrossShanks(selection, numberOfElectrodesAcrossShanks * 8, numberOfElectrodesAcrossShanks);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks865To960)
		{
			selectElectrodesAcrossShanks(selection, numberOfElectrodesAcrossShanks * 9, numberOfElectrodesAcrossShanks);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks961To1056)
		{
			selectElectrodesAcrossShanks(selection, numberOfElectrodesAcrossShanks * 10, numberOfElectrodesAcrossShanks);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks1057To1152)
		{
			selectElectrodesAcrossShanks(selection, numberOfElectrodesAcrossShanks * 11, numberOfElectrodesAcrossShanks);
		}
		else if (electrodeConfigurationIndex == (int32_t)ElectrodeConfigurationQuadShank::AllShanks1153To1248)
		{
			selectElectrodesAcrossShanks(selection, numberOfElectrodesAcrossShanks * 12, numberOfElectrodesAcrossShanks);
		}
	}

	return selection;
}

uint64_t Neuropixels2e::getProbeSerialNumber(int index)
{
	try {
		return probeSN.at(index);
	}
	catch (const std::out_of_range& ex) // filter for out of range
	{
		LOGE("Invalid index given requesting probe serial number.");
	}

	return 0ull;
}

OnixDeviceType Neuropixels2e::getDeviceType()
{
	return OnixDeviceType::NEUROPIXELSV2E;
}

int Neuropixels2e::configureDevice()
{
	if (deviceContext == nullptr || !deviceContext->isInitialized())
		throw error_str("Device context is not initialized properly for " + getName());

	int rc = deviceContext->writeRegister(deviceIdx, DS90UB9x::ENABLE, isEnabled() ? 1 : 0);
	if (rc != ONI_ESUCCESS)
		throw error_str("Unable to enable " + getName());

	configureSerDes();
	setProbeSupply(true);
	rc = serializer->set933I2cRate(400e3);
	if (rc != ONI_ESUCCESS)
		throw error_str("Unable to set I2C rate for " + getName());
	probeSN[0] = getProbeSN(ProbeASelected);
	probeSN[1] = getProbeSN(ProbeBSelected);
	setProbeSupply(false);
	LOGD("Probe A SN: ", probeSN[0]);
	LOGD("Probe B SN: ", probeSN[1]);

	if (probeSN[0] == 0 && probeSN[1] == 0)
	{
		m_numProbes = 0;
		throw error_str("No probes were found connected at address " + std::to_string(getDeviceIdx()));
	}
	else if (probeSN[0] != 0 && probeSN[1] != 0)
	{
		m_numProbes = 2;
	}
	else
	{
		m_numProbes = 1;
	}

	streamInfos.clear();

	for (int i = 0; i < m_numProbes; i++)
	{
		createDataStream(i);
	}

	return ONI_ESUCCESS;
}

bool Neuropixels2e::updateSettings()
{
	for (int i = 0; i < 2; i++)
	{
		if (probeSN[i] != 0)
		{
			if (gainCorrectionFilePath[i] == "None" || gainCorrectionFilePath[i] == "")
			{
				Onix1::showWarningMessageBoxAsync("Missing File", "Missing gain correction file for probe " + std::to_string(probeSN[i]));
				return false;
			}

			File gainCorrectionFile = File(gainCorrectionFilePath[i]);

			if (!gainCorrectionFile.existsAsFile())
			{
				Onix1::showWarningMessageBoxAsync("Missing File", "The gain correction file \"" + gainCorrectionFilePath[i] + "\" for probe " + std::to_string(probeSN[i]) + " does not exist.");
				return false;
			}

			StringArray fileLines;
			gainCorrectionFile.readLines(fileLines);

			fileLines.removeEmptyStrings(true);

			auto gainSN = std::stoull(fileLines[0].toStdString());

			if (gainSN != probeSN[i])
			{
				Onix1::showWarningMessageBoxAsync("Invalid Serial Number", "Gain correction serial number (" + std::to_string(gainSN) + ") does not match probe serial number (" + std::to_string(probeSN[i]) + ").");
				return false;
			}

			if (fileLines.size() != numberOfChannels + 1)
			{
				Onix1::showWarningMessageBoxAsync("File Format Invalid", "Found the wrong number of lines in the calibration file. Expected " + std::to_string(numberOfChannels + 1) + ", found " + std::to_string(fileLines.size()));
				return false;
			}

			StringRef breakCharacters = ",";
			StringRef noQuote = "";
			StringArray firstLine = StringArray::fromTokens(fileLines[1], breakCharacters, noQuote);
			auto correctionValue = std::stod(firstLine[1].toStdString());

			for (int j = 0; j < numberOfChannels; j++)
			{
				StringArray calibrationValues = StringArray::fromTokens(fileLines[j + 1], breakCharacters, noQuote);

				if (std::stoi(calibrationValues[0].toStdString()) != j || std::stod(calibrationValues[1].toStdString()) != correctionValue)
				{
					Onix1::showWarningMessageBoxAsync("File Format Invalid", "Calibration file is incorrectly formatted for probe " + std::to_string(probeSN[i]));
					return false;
				}
			}

			gainCorrection[i] = correctionValue * -1.0f;
		}
		else
			gainCorrection[i] = 0;
	}

	setProbeSupply(true);
	resetProbes();

	for (int i = 0; i < NumberOfProbes; i++)
	{
		if (probeSN[i] != 0)
		{
			selectProbe(i == 0 ? ProbeASelected : ProbeBSelected);
			writeConfiguration(settings[i].get());
			configureProbeStreaming();
		}
	}

	selectProbe(NoProbeSelected);
	//IMPORTANT! BNO polling thread must be started after this

	return true;
}

void Neuropixels2e::configureProbeStreaming()
{
	// Write super sync bits into ASIC
	probeControl->WriteByte(SUPERSYNC11, 0b00011000);
	probeControl->WriteByte(SUPERSYNC10, 0b01100001);
	probeControl->WriteByte(SUPERSYNC9, 0b10000110);
	probeControl->WriteByte(SUPERSYNC8, 0b00011000);
	probeControl->WriteByte(SUPERSYNC7, 0b01100001);
	probeControl->WriteByte(SUPERSYNC6, 0b10000110);
	probeControl->WriteByte(SUPERSYNC5, 0b00011000);
	probeControl->WriteByte(SUPERSYNC4, 0b01100001);
	probeControl->WriteByte(SUPERSYNC3, 0b10000110);
	probeControl->WriteByte(SUPERSYNC2, 0b00011000);
	probeControl->WriteByte(SUPERSYNC1, 0b01100001);
	probeControl->WriteByte(SUPERSYNC0, 0b10111001);

	// Activate recording mode on NP
	probeControl->WriteByte(OP_MODE, 0b01000000);

	// Set global ADC settings
	// TODO: Undocumented
	probeControl->WriteByte(ADC_CONFIG, 0b00001000);
}

void Neuropixels2e::configureSerDes()
{
	deviceContext->writeRegister(deviceIdx, DS90UB9x::ENABLE, 1);

	// configure deserializer trigger mode
	deviceContext->writeRegister(deviceIdx, DS90UB9x::TRIGGEROFF, 0);
	deviceContext->writeRegister(deviceIdx, DS90UB9x::TRIGGER, (uint32_t)(DS90UB9x::DS90UB9xTriggerMode::Continuous));
	deviceContext->writeRegister(deviceIdx, DS90UB9x::SYNCBITS, 0);
	deviceContext->writeRegister(deviceIdx, DS90UB9x::DATAGATE, (uint32_t)(DS90UB9x::DS90UB9xDataGate::Disabled));
	deviceContext->writeRegister(deviceIdx, DS90UB9x::MARK, (uint32_t)(DS90UB9x::DS90UB9xMarkMode::Disabled));

	// configure two 4-bit magic word-triggered streams, one for each probe
	deviceContext->writeRegister(deviceIdx, DS90UB9x::READSZ, 0x00100009); // 16 frames/superframe, 8x 12-bit words + magic bits
	deviceContext->writeRegister(deviceIdx, DS90UB9x::MAGIC_MASK, 0xC000003F); // Enable inverse, wait for non-inverse, 14-bit magic word
	deviceContext->writeRegister(deviceIdx, DS90UB9x::MAGIC, 0b0000000000101110); // Super-frame sync word
	deviceContext->writeRegister(deviceIdx, DS90UB9x::MAGIC_WAIT, 0);
	deviceContext->writeRegister(deviceIdx, DS90UB9x::DATAMODE, 0b00100000000000000000001010110101);
	deviceContext->writeRegister(deviceIdx, DS90UB9x::DATALINES0, 0xFFFFF8A6); // NP A
	deviceContext->writeRegister(deviceIdx, DS90UB9x::DATALINES1, 0xFFFFF97B); // NP B

	deserializer = std::make_unique<I2CRegisterContext>(DS90UB9x::DES_ADDR, deviceIdx, deviceContext);
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::PortSel, 0x01);
	int coaxMode = 0x4 + (uint32_t)(DS90UB9x::DS90UB9xMode::Raw12BitHighFrequency); // 0x4 maintains coax mode
	deserializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xDeserializerI2CRegister::PortMode), coaxMode);
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::I2CConfig, 0b01011000);
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::SerAlias, DS90UB9x::SER_ADDR << 1);

	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::GpioCtrl0, 0x10);
	deserializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xDeserializerI2CRegister::GpioCtrl1, 0x32);

	int alias = ProbeI2CAddress << 1;
	deserializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID1), alias);
	deserializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias1), alias);

	alias = FlexAddress << 1;
	deserializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID2), alias);
	deserializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias2), alias);

	serializer = std::make_unique<I2CRegisterContext>(DS90UB9x::SER_ADDR, deviceIdx, deviceContext);
	flex = std::make_unique<I2CRegisterContext>(FlexAddress, deviceIdx, deviceContext);
	probeControl = std::make_unique<I2CRegisterContext>(ProbeI2CAddress, deviceIdx, deviceContext);
}

void Neuropixels2e::setProbeSupply(bool en)
{
	auto gpo10Config = en ? DefaultGPO10Config | GPO10SupplyMask : DefaultGPO10Config;

	selectProbe(NoProbeSelected);
	serializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xSerializerI2CRegister::GPIO10), gpo10Config);
	Thread::sleep(20);
}

void Neuropixels2e::selectProbe(uint8_t probeSelect)
{
	if (serializer == nullptr)
	{
		LOGE("Serializer is not initialized for Neuropixels 2.0");
		return;
	}

	serializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xSerializerI2CRegister::GPIO32), probeSelect);
	Thread::sleep(20);
}

void Neuropixels2e::resetProbes()
{
	auto gpo10Config = DefaultGPO10Config | GPO10SupplyMask;
	gpo10Config &= ~GPO10ResetMask;
	serializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xSerializerI2CRegister::GPIO10), gpo10Config);
	gpo10Config |= GPO10ResetMask;
	serializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xSerializerI2CRegister::GPIO10), gpo10Config);
}

uint64_t Neuropixels2e::getProbeSN(uint8_t probeSelect)
{
	if (flex == nullptr)
	{
		LOGE("Flex is not initialized for Neuropixels 2.0");
		return 0ull;
	}

	selectProbe(probeSelect);
	uint64_t probeSN = 0ull;
	int errorCode = 0, rc;
	for (unsigned int i = 0; i < sizeof(probeSN); i++)
	{
		oni_reg_addr_t reg_addr = OFFSET_PROBE_SN + i;
		oni_reg_val_t val;

		rc = flex->ReadByte(reg_addr, &val);

		if (rc != ONI_ESUCCESS) return 0ull;

		if (val <= 0xFF)
		{
			probeSN |= (((uint64_t)val) << (i * 8));
		}
	}
	return probeSN;
}

void Neuropixels2e::startAcquisition()
{
  frameCount.fill(0);
  sampleNumber.fill(0);
}

void Neuropixels2e::stopAcquisition()
{
	setProbeSupply(false);

	OnixDevice::stopAcquisition();
}

void Neuropixels2e::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
	if (m_numProbes == 1)
	{
		sourceBuffers.add(new DataBuffer(streamInfos.getFirst().getNumChannels(), (int)streamInfos.getFirst().getSampleRate() * bufferSizeInSeconds));
		auto bufferIndex = probeSN[0] != 0 ? 0 : 1;
		amplifierBuffer[bufferIndex] = sourceBuffers.getLast();
	}
	else
	{
		int bufferIdx = 0;
		for (const auto& streamInfo : streamInfos)
		{
			sourceBuffers.add(new DataBuffer(streamInfo.getNumChannels(), (int)streamInfo.getSampleRate() * bufferSizeInSeconds));
			amplifierBuffer[bufferIdx++] = sourceBuffers.getLast();
		}
	}
}

void Neuropixels2e::processFrames()
{
	oni_frame_t* frame;
	while (frameQueue.try_dequeue(frame))
	{
		uint16_t* dataPtr = (uint16_t*)frame->data;

		uint16_t probeIndex = *(dataPtr + 4);
		uint16_t* amplifierData = dataPtr + 9;

		sampleNumbers[probeIndex][frameCount[probeIndex]] = sampleNumber[probeIndex]++;
		timestamps[probeIndex][frameCount[probeIndex]] = deviceContext->convertTimestampToSeconds(frame->time);

		for (int i = 0; i < FramesPerSuperFrame; i++)
		{
			auto adcDataOffset = i * FrameWords;

			for (int j = 0; j < AdcsPerProbe; j++)
			{
				const size_t channelIndex = rawToChannel[j][i];

				samples[probeIndex][channelIndex * numFrames + frameCount[probeIndex]] =
					(float)(*(amplifierData + adcIndices[j] + adcDataOffset)) * gainCorrection[probeIndex] + DataMidpoint;
			}
		}

		frameCount[probeIndex]++;

		if (frameCount[probeIndex] >= numFrames)
		{
			amplifierBuffer[probeIndex]->addToBuffer(samples[probeIndex].data(), sampleNumbers[probeIndex].data(), timestamps[probeIndex].data(), eventCodes[probeIndex].data(), numFrames);
			frameCount[probeIndex] = 0;
		}

		oni_destroy_frame(frame);
	}
}

void Neuropixels2e::writeConfiguration(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings)
{
	auto baseBits = makeBaseBits(getReference(settings->referenceIndex));
	writeShiftRegister(SR_CHAIN5, baseBits[0]);
	writeShiftRegister(SR_CHAIN6, baseBits[1]);

	auto shankBits = makeShankBits(getReference(settings->referenceIndex), settings->electrodeMetadata);
	writeShiftRegister(SR_CHAIN1, shankBits[0]);
	writeShiftRegister(SR_CHAIN2, shankBits[1]);
	writeShiftRegister(SR_CHAIN3, shankBits[2]);
	writeShiftRegister(SR_CHAIN4, shankBits[3]);
}

template<int N>
void Neuropixels2e::writeShiftRegister(uint32_t srAddress, std::bitset<N> bits)
{
	std::vector<unsigned char> bytes = toBitReversedBytes<N>(bits);

	for (int i = 2; i > 0; i -= 1)
	{
		WriteByte(SOFT_RESET, 0xFF);
		WriteByte(SOFT_RESET, 0x00);

		WriteByte(SR_LENGTH1, (uint32_t)(bytes.size() % 0x100));
		WriteByte(SR_LENGTH2, (uint32_t)(bytes.size() / 0x100));

		for (auto b : bytes)
		{
			WriteByte(srAddress, b);
		}
	}

	uint32_t status;
	ReadByte(STATUS, &status);
	if (status != (uint32_t)NeuropixelsV2Status::SR_OK)
	{
		LOGE("Warning: Shift register ", srAddress, " status check failed. ", getShankName(srAddress), " may be damaged.");
	}
}

std::string Neuropixels2e::getShankName(uint32_t shiftRegisterAddress)
{
	switch (shiftRegisterAddress)
	{
	case SR_CHAIN1:
		return "Shank 1";
	case SR_CHAIN2:
		return "Shank 2";
	case SR_CHAIN3:
		return "Shank 3";
	case SR_CHAIN4:
		return "Shank 4";
	default:
		return "";
	}
}

void Neuropixels2e::setGainCorrectionFile(int index, std::string filename)
{
	if (index < gainCorrectionFilePath.size())
	{
		gainCorrectionFilePath[index] = filename;
	}
}

std::string Neuropixels2e::getGainCorrectionFile(int index)
{
	if (index < gainCorrectionFilePath.size())
	{
		return gainCorrectionFilePath[index];
	}
	else
		return "";
}

NeuropixelsV2Reference Neuropixels2e::getReference(int index)
{
	switch (index)
	{
	case 0:
		return NeuropixelsV2Reference::External;
	case 1:
		return NeuropixelsV2Reference::Tip1;
	case 2:
		return NeuropixelsV2Reference::Tip2;
	case 3:
		return NeuropixelsV2Reference::Tip3;
	case 4:
		return NeuropixelsV2Reference::Tip4;
	default:
		break;
	}

	return NeuropixelsV2Reference::External;
}

Neuropixels2e::BaseBitsArray Neuropixels2e::makeBaseBits(NeuropixelsV2Reference reference)
{
	BaseBitsArray baseBits;

	int referenceBit;

	if (reference == NeuropixelsV2Reference::External)
		referenceBit = 1;
	else
		referenceBit = 2;

	for (size_t i = 0; i < numberOfChannels; i++)
	{
		auto configIndex = i % 2;
		auto bitOffset = (382 - i + configIndex) / 2 * baseBitsPerChannel;
		baseBits[configIndex][bitOffset + 0] = false;
		baseBits[configIndex][bitOffset + referenceBit] = true;
	}

	return baseBits;
}

Neuropixels2e::ShankBitsArray Neuropixels2e::makeShankBits(NeuropixelsV2Reference reference, std::array<ElectrodeMetadata, numberOfElectrodes> channelMap)
{
	ShankBitsArray shankBits;

	if (reference != NeuropixelsV2Reference::External)
	{
		shankBits[(size_t)reference - 1][643] = true;
		shankBits[(size_t)reference - 1][644] = true;
	}
	else
	{
		shankBits[0][2] = true;
		shankBits[0][1285] = true;
		shankBits[1][2] = true;
		shankBits[1][1285] = true;
		shankBits[2][2] = true;
		shankBits[2][1285] = true;
		shankBits[3][2] = true;
		shankBits[3][1285] = true;
	}

	const int pixelOffset = (NeuropixelsV2eValues::electrodesPerShank - 1) / 2;
	const int referencePixelOffset = 3;

	int count = 0;

	for (const auto& e : channelMap)
	{
		if (e.status == ElectrodeStatus::CONNECTED)
		{
			auto baseIndex = e.shank_local_index % 2;
			auto pixelIndex = e.shank_local_index / 2;
			pixelIndex = baseIndex == 0
				? pixelIndex + pixelOffset + 2 * referencePixelOffset
				: pixelOffset - pixelIndex + referencePixelOffset;

			shankBits[e.shank][pixelIndex] = true;

			count++;
		}
	}

	if (count != numberOfChannels)
	{
		LOGE("Invalid number of channels connected for Neuropixels 2.0e, configuration might be invalid.");
	}

	return shankBits;
}

void Neuropixels2e::setSettings(ProbeSettings<NeuropixelsV2eValues::numberOfChannels, NeuropixelsV2eValues::numberOfElectrodes>* settings_, int index)
{
	if (index >= settings.size())
	{
		LOGE("Invalid index given when trying to update settings.");
		return;
	}

	settings[index]->updateProbeSettings(settings_);
}

void Neuropixels2e::defineMetadata(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings, int shankCount)
{
	settings->probeType = ProbeType::NPX_V2;
	settings->probeMetadata.name = "Neuropixels 2.0e" + (shankCount == 1) ? " - Single Shank" : " - Quad Shank";

	constexpr float shankTipY = 0.0f;
	constexpr float shankBaseY = 155.0f;
	constexpr float shankLengthY = 10000.0f;
	constexpr float probeLengthY = 10155.0f;
	constexpr float shankOffsetX = 200.0f;
	constexpr float shankWidthX = 70.0f;
	constexpr float shankPitchX = 250.0f;

	std::vector<std::array<float, 2>> probeContour{
		{0, probeLengthY},
		{0, shankLengthY},
	};

	for (int i = 0; i < shankCount; i++)
	{
		probeContour.emplace_back(std::array<float, 2>{ shankOffsetX + (shankWidthX + shankPitchX) * i, shankLengthY });
		probeContour.emplace_back(std::array<float, 2>{ shankOffsetX + (shankWidthX + shankPitchX) * i, shankBaseY });
		probeContour.emplace_back(std::array<float, 2>{ shankOffsetX + (shankWidthX + shankPitchX) * i + shankWidthX / 2, shankTipY });
		probeContour.emplace_back(std::array<float, 2>{ shankOffsetX + (shankWidthX + shankPitchX) * i + shankWidthX, shankBaseY });
		probeContour.emplace_back(std::array<float, 2>{ shankOffsetX + (shankWidthX + shankPitchX) * i + shankWidthX, shankLengthY });
	}

	probeContour.emplace_back(std::array<float, 2>{shankOffsetX * 2 + (shankWidthX + shankPitchX) * (numberOfShanks - 1) + shankWidthX, shankLengthY});
	probeContour.emplace_back(std::array<float, 2>{shankOffsetX * 2 + (shankWidthX + shankPitchX) * (numberOfShanks - 1) + shankWidthX, probeLengthY});
	probeContour.emplace_back(std::array<float, 2>{0.0f, probeLengthY});

	std::vector<std::array<float, 2>> shankOutline{
		{27, 31},
		{27, 514},
		{27 + 5, 522},
		{27 + 10, 514},
		{27 + 10, 31}
	};

	settings->probeMetadata.shank_count = shankCount;
	settings->probeMetadata.electrodes_per_shank = NeuropixelsV2eValues::electrodesPerShank;
	settings->probeMetadata.rows_per_shank = NeuropixelsV2eValues::electrodesPerShank / 2;
	settings->probeMetadata.columns_per_shank = 2;
	settings->probeMetadata.shankOutline = shankOutline;
	settings->probeMetadata.probeContour = probeContour;
	settings->probeMetadata.num_adcs = 24;
	settings->probeMetadata.adc_bits = 12;

	settings->availableBanks = {
		Bank::A,
		Bank::B,
		Bank::C,
		Bank::D,
		Bank::NONE //disconnected
	};

	for (int i = 0; i < settings->probeMetadata.electrodes_per_shank * settings->probeMetadata.shank_count; i++)
	{
		ElectrodeMetadata metadata;

		metadata.global_index = i;

		metadata.shank = i / settings->probeMetadata.electrodes_per_shank;
		metadata.shank_local_index = i % settings->probeMetadata.electrodes_per_shank;

		auto offset = shankOffsetX + (shankWidthX + shankPitchX) * metadata.shank + 11.0f;
		metadata.xpos = offset + (i % 2) * 32.0f + 8.0f;
		metadata.ypos = std::floor((i % settings->probeMetadata.electrodes_per_shank) / 2.0f) * 15 + 170;
		metadata.site_width = 12;

		metadata.column_index = i % 2;
		metadata.row_index = metadata.shank_local_index / 2;

		metadata.isSelected = false;

		metadata.colour = Colours::lightgrey;

		if (shankCount == 1)
		{
			if (i < 384)
			{
				metadata.bank = Bank::A;

				int bank_index = metadata.shank_local_index % 384;
				int block = bank_index / 32;
				int row = (bank_index % 32) / 2;

				if (i % 2 == 0)
				{
					metadata.channel = row * 2 + block * 32;
				}
				else
				{
					metadata.channel = row * 2 + block * 32 + 1;
				}

				metadata.status = ElectrodeStatus::CONNECTED;
			}
			else if (i >= 384 && i < 768)
			{
				metadata.bank = Bank::B;

				int bank_index = metadata.shank_local_index % 384;
				int block = bank_index / 32;
				int row = (bank_index % 32) / 2;

				if (i % 2 == 0)
				{
					metadata.channel = ((row * 7) % 16) * 2 + block * 32;
				}
				else
				{
					metadata.channel = ((row * 7 + 4) % 16) * 2 + block * 32 + 1;
				}

				metadata.status = ElectrodeStatus::DISCONNECTED;
			}
			else if (i >= 768 && i < 1152)
			{
				metadata.bank = Bank::C;

				int bank_index = metadata.shank_local_index % 384;
				int block = bank_index / 32;
				int row = (bank_index % 32) / 2;

				if (i % 2 == 0)
				{
					metadata.channel = ((row * 5) % 16) * 2 + block * 32;
				}
				else
				{
					metadata.channel = ((row * 5 + 8) % 16) * 2 + block * 32 + 1;
				}

				metadata.status = ElectrodeStatus::DISCONNECTED;
			}
			else
			{
				metadata.bank = Bank::D;

				int bank_index = metadata.shank_local_index % 384;
				int block = bank_index / 32;
				int row = (bank_index % 32) / 2;

				if (i % 2 == 0)
				{
					metadata.channel = ((row * 3) % 16) * 2 + block * 32;
				}
				else
				{
					metadata.channel = ((row * 3 + 12) % 16) * 2 + block * 32 + 1;
				}

				metadata.status = ElectrodeStatus::DISCONNECTED;
			}
		}
		else if (shankCount == 4)
		{
			if (i < 384)
			{
				metadata.status = ElectrodeStatus::CONNECTED;
			}
			else
			{
				metadata.status = ElectrodeStatus::DISCONNECTED;
			}

			if (metadata.shank_local_index < 384)
				metadata.bank = Bank::A;
			else if (metadata.shank_local_index >= 384 && metadata.shank_local_index < 768)
				metadata.bank = Bank::B;
			else if (metadata.shank_local_index >= 768 && metadata.shank_local_index < 1152)
				metadata.bank = Bank::C;
			else
				metadata.bank = Bank::D;

			int block = metadata.shank_local_index % 384 / 48 + 1;
			int block_index = metadata.shank_local_index % 48;

			if (metadata.shank == 0)
			{
				switch (block)
				{
				case 1:
					metadata.channel = block_index + 48 * 0; // 1-48 (Bank 0-3)
					break;
				case 2:
					metadata.channel = block_index + 48 * 2; // 96-144 (Bank 0-3)
					break;
				case 3:
					metadata.channel = block_index + 48 * 4; // 192-223 (Bank 0-3)
					break;
				case 4:
					metadata.channel = block_index + 48 * 6; // 288-336 (Bank 0-2)
					break;
				case 5:
					metadata.channel = block_index + 48 * 5; // 240-288 (Bank 0-2)
					break;
				case 6:
					metadata.channel = block_index + 48 * 7; // 336-384 (Bank 0-2)
					break;
				case 7:
					metadata.channel = block_index + 48 * 1; // 48-96 (Bank 0-2)
					break;
				case 8:
					metadata.channel = block_index + 48 * 3; // 144-192 (Bank 0-2)
					break;
				default:
					metadata.channel = -1;
				}
			}
			else if (metadata.shank == 1)
			{
				switch (block)
				{
				case 1:
					metadata.channel = block_index + 48 * 1;
					break;
				case 2:
					metadata.channel = block_index + 48 * 3;
					break;
				case 3:
					metadata.channel = block_index + 48 * 5;
					break;
				case 4:
					metadata.channel = block_index + 48 * 7;
					break;
				case 5:
					metadata.channel = block_index + 48 * 4;
					break;
				case 6:
					metadata.channel = block_index + 48 * 6;
					break;
				case 7:
					metadata.channel = block_index + 48 * 0;
					break;
				case 8:
					metadata.channel = block_index + 48 * 2;
					break;
				default:
					metadata.channel = -1;
				}
			}
			else if (metadata.shank == 2)
			{
				switch (block)
				{
				case 1:
					metadata.channel = block_index + 48 * 4;
					break;
				case 2:
					metadata.channel = block_index + 48 * 6;
					break;
				case 3:
					metadata.channel = block_index + 48 * 0;
					break;
				case 4:
					metadata.channel = block_index + 48 * 2;
					break;
				case 5:
					metadata.channel = block_index + 48 * 1;
					break;
				case 6:
					metadata.channel = block_index + 48 * 3;
					break;
				case 7:
					metadata.channel = block_index + 48 * 5;
					break;
				case 8:
					metadata.channel = block_index + 48 * 7;
					break;
				default:
					metadata.channel = -1;
				}
			}
			else if (metadata.shank == 3)
			{
				switch (block)
				{
				case 1:
					metadata.channel = block_index + 48 * 5;
					break;
				case 2:
					metadata.channel = block_index + 48 * 7;
					break;
				case 3:
					metadata.channel = block_index + 48 * 1;
					break;
				case 4:
					metadata.channel = block_index + 48 * 3;
					break;
				case 5:
					metadata.channel = block_index + 48 * 0;
					break;
				case 6:
					metadata.channel = block_index + 48 * 2;
					break;
				case 7:
					metadata.channel = block_index + 48 * 4;
					break;
				case 8:
					metadata.channel = block_index + 48 * 6;
					break;
				default:
					metadata.channel = -1;
				}
			}

			metadata.type = ElectrodeType::ELECTRODE;
		}

		settings->electrodeMetadata[i] = metadata;
	}

	settings->apGainIndex = -1;
	settings->lfpGainIndex = -1;
	settings->referenceIndex = 0;
	settings->apFilterState = false;

	settings->electrodeConfigurationIndex = (int32_t)ElectrodeConfigurationSingleShank::BankA;
	auto selection = selectElectrodeConfiguration(settings->electrodeConfigurationIndex);
	settings->selectElectrodes(selection);

	settings->availableReferences.add("Ext");
	settings->availableReferences.add("Tip1");

	if (shankCount == 4)
	{
		settings->availableReferences.add("Tip2");
		settings->availableReferences.add("Tip3");
		settings->availableReferences.add("Tip4");
	}

	if (shankCount == 1)
	{
		for (const auto& [_, config] : electrodeConfigurationSingleShank)
		{
			settings->availableElectrodeConfigurations.add(config);
		}
	}
	else if (shankCount == 4)
	{
		for (const auto& [_, config] : electrodeConfigurationQuadShank)
		{
			settings->availableElectrodeConfigurations.add(config);
		}
	}
}
