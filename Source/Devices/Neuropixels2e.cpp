#include "Neuropixels2e.h"
#include "DS90UB9x.h"

Neuropixels2e::Neuropixels2e(String name, const oni_dev_idx_t deviceIdx_, std::shared_ptr<Onix1> ctx_)
	: OnixDevice(name, OnixDeviceType::NEUROPIXELS_2, deviceIdx_, ctx_)
{}

void Neuropixels2e::createDataStream(int n)
{
	StreamInfo apStream = StreamInfo(
		getName() + "-" + String(n),
		"Neuropixels 2.0 data stream",
		"onix-neuropixels2.data",
		384,
		30000.0f,
		"CH",
		ContinuousChannel::Type::ELECTRODE,
		0.195f,
		CharPointer_UTF8("\xc2\xb5V"),
		{});
	streams.add(apStream);
}

Neuropixels2e::~Neuropixels2e()
{
}

int Neuropixels2e::getNumProbes() const
{
	return m_numProbes;
}

int Neuropixels2e::configureDevice()
{
	configureSerDes();
	setProbeSupply(true);
	probeSNA = getProbeSN(ProbeASelected);
	probeSNB = getProbeSN(ProbeBSelected);
	setProbeSupply(false);
	LOGD("Probe A SN: ", probeSNA);
	LOGD("Probe B SN: ", probeSNB);

	if (probeSNA == -1 && probeSNB == -1)
	{
		m_numProbes = 0;
		return -1;
	}
	else if (probeSNA != -1 && probeSNB != -1)
	{
		m_numProbes = 2;
	}
	else
	{
		m_numProbes = 1;
	}

	for (int i = 0; i < m_numProbes; i++)
		createDataStream(i);
	
	//TODO: Gain correction loading would be here

	return 0;
}

bool Neuropixels2e::updateSettings()
{
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
	deviceContext->writeRegister(deviceIdx,DS90UB9x::TRIGGEROFF, 0);
	deviceContext->writeRegister(deviceIdx,DS90UB9x::TRIGGER, (uint32_t)(DS90UB9x::DS90UB9xTriggerMode::Continuous));
	deviceContext->writeRegister(deviceIdx,DS90UB9x::SYNCBITS, 0);
	deviceContext->writeRegister(deviceIdx,DS90UB9x::DATAGATE, (uint32_t)(DS90UB9x::DS90UB9xDataGate::Disabled));
	deviceContext->writeRegister(deviceIdx,DS90UB9x::MARK, (uint32_t)(DS90UB9x::DS90UB9xMarkMode::Disabled));

	// configure two 4-bit magic word-triggered streams, one for each probe
	deviceContext->writeRegister(deviceIdx,DS90UB9x::READSZ, 0x00100009); // 16 frames/superframe, 8x 12-bit words + magic bits
	deviceContext->writeRegister(deviceIdx,DS90UB9x::MAGIC_MASK, 0xC000003F); // Enable inverse, wait for non-inverse, 14-bit magic word
	deviceContext->writeRegister(deviceIdx,DS90UB9x::MAGIC, 0b0000000000101110); // Super-frame sync word
	deviceContext->writeRegister(deviceIdx,DS90UB9x::MAGIC_WAIT, 0);
	deviceContext->writeRegister(deviceIdx,DS90UB9x::DATAMODE, 0b00100000000000000000001010110101);
	deviceContext->writeRegister(deviceIdx,DS90UB9x::DATALINES0, 0xFFFFF8A6); // NP A
	deviceContext->writeRegister(deviceIdx,DS90UB9x::DATALINES1, 0xFFFFF97B); // NP B

	deserializer = std::make_unique<I2CRegisterContext>(DS90UB9x::DES_ADDR, deviceIdx, deviceContext);
	int coaxMode = 0x4 + (uint32_t)(DS90UB9x::DS90UB9xMode::Raw12BitHighFrequency); // 0x4 maintains coax mode
	deserializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xDeserializerI2CRegister::PortMode), coaxMode);

	int alias = ProbeI2CAddress << 1;
	deserializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID1), alias);
	deserializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias1), alias);

	alias = FlexAddress << 1;
	deserializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveID2), alias);
	deserializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xDeserializerI2CRegister::SlaveAlias2), alias);

	serializer = std::make_unique<I2CRegisterContext>(DS90UB9x::SER_ADDR, deviceIdx, deviceContext);
	flex = std::make_unique<I2CRegisterContext>(FlexAddress, deviceIdx, deviceContext);
	probeControl = std::make_unique<I2CRegisterContext>(ProbeI2CAddress, deviceIdx, deviceContext); //TODO: this probably be a derived class that includes all the connfiguration methods
}

void Neuropixels2e::setProbeSupply(bool en)
{
	auto gpo10Config = en ? DefaultGPO10Config | GPO10SupplyMask : DefaultGPO10Config;

	selectProbe(NoProbeSelected);
	serializer->WriteByte((uint32_t)(DS90UB9x::DS90UB9xSerializerI2CRegister::GPIO10), gpo10Config);
	Thread::sleep(10);
}

void Neuropixels2e::selectProbe(uint8_t probeSelect)
{
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
	selectProbe(probeSelect);
	uint64_t probeSN = 0;
	int errorCode = 0;
	for (unsigned int i = 0; i < sizeof(probeSN); i++)
	{
		oni_reg_addr_t reg_addr = OFFSET_PROBE_SN + i;
		oni_reg_val_t val;

		flex->ReadByte(reg_addr, &val);

		if (flex->getLastResult() != ONI_ESUCCESS) return -1;

		if (val <= 0xFF)
		{
			probeSN |= (((uint64_t)val) << (i * 8));
		}
	}
	return probeSN;
}

void Neuropixels2e::startAcquisition()
{
	setProbeSupply(true);
	resetProbes();
	if (probeSNA != -1)
	{
		selectProbe(ProbeASelected);
		//TODO: Here would be proper probe configuration theough a proper probe control class 
		configureProbeStreaming();
	}
	if (probeSNB != -1)
	{
		selectProbe(ProbeBSelected);
		//TODO: Here would be proper probe configuration theough a proper probe control class 
		configureProbeStreaming();
	}

	// disconnect i2c bus from both probes to prevent digital interference during acquisition
	selectProbe(NoProbeSelected); //IMPORTANT! BNO polling thread must be started after this
}

void Neuropixels2e::stopAcquisition()
{
	setProbeSupply(false);

	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_destroy_frame(frameArray.removeAndReturn(0));
	}
}

void Neuropixels2e::addFrame(oni_frame_t* frame)
{
	const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
	frameArray.add(frame);
}

void Neuropixels2e::addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers)
{
	int bufferIdx = 0;
	for (const auto& streamInfo : streams)
	{
		sourceBuffers.add(new DataBuffer(streamInfo.getNumChannels(), (int)streamInfo.getSampleRate() * bufferSizeInSeconds));
		apBuffer[bufferIdx++] = sourceBuffers.getLast();
	}
}

void Neuropixels2e::processFrames()
{
	while (!frameArray.isEmpty())
	{
		const GenericScopedLock<CriticalSection> frameLock(frameArray.getLock());
		oni_frame_t* frame = frameArray.removeAndReturn(0);

		uint16_t* dataPtr;
		dataPtr = (uint16_t*)frame->data;
		uint16_t probeIndex = *(dataPtr + 4);
		uint16_t* amplifierData = dataPtr + 6;

		if (m_numProbes == 1)
		{
			//Fill always in buffer[0], we can ignore the probeIndex
		}
		else
		{
			//Fill buffer[probeindex]
		}

		oni_destroy_frame(frame);
	}
}
