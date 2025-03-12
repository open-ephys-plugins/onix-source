#pragma once
#include "../OnixDevice.h"
#include "../I2CRegisterContext.h"

class Neuropixels2e : public OnixDevice
{
public:
	Neuropixels2e(String name, const oni_dev_idx_t, std::shared_ptr<Onix1>);

	/** Destructor */
	~Neuropixels2e();

	int enableDevice() override;

	/** Update the settings of the device */
	bool updateSettings() override;

	/** Starts probe data streaming */
	void startAcquisition() override;

	/** Stops probe data streaming*/
	void stopAcquisition() override;

	void addFrame(oni_frame_t*) override;

	void processFrames() override;

	void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override;

	int getNumProbes() const;

private:
	DataBuffer* apBuffer[2];

	void createDataStream(int n);

	uint64_t getProbeSN(uint8_t probeSelect);
	void configureSerDes();
	void setProbeSupply(bool);
	void resetProbes();

	void selectProbe(uint8_t probeSelect);
	void configureProbeStreaming();

	int m_numProbes = 0;

	std::unique_ptr<I2CRegisterContext> serializer;
	std::unique_ptr<I2CRegisterContext> deserializer;
	std::unique_ptr<I2CRegisterContext> flex;
	std::unique_ptr<I2CRegisterContext> probeControl; //TODO: this probably be a derived class that includes all the connfiguration methods

	static const int ProbeI2CAddress = 0x10;
	static const int FlexAddress = 0x50;

	static const int ProbeAddress = 0x10;
	static const int FlexEEPROMAddress = 0x50;

	static const uint32_t GPO10SupplyMask = 1 << 3; // Used to turn on VDDA analog supply
	static const uint32_t GPO10ResetMask = 1 << 7; // Used to issue full reset commands to probes
	static const uint8_t DefaultGPO10Config = 0b00010001; // NPs in reset, VDDA not enabled
	static const uint8_t NoProbeSelected = 0b00010001; // No probes selected
	static const uint8_t ProbeASelected = 0b00011001; // TODO: Changes in Rev. B of headstage
	static const uint8_t ProbeBSelected = 0b10011001;

	static const int FramesPerSuperFrame = 16;
	static const int AdcsPerProbe = 24;
	static const int ChannelCount = 384;
	static const int FrameWords = 36; // TRASH TRASH TRASH 0 ADC0 ADC8 ADC16 0 ADC1 ADC9 ADC17 0 ... ADC7 ADC15 ADC23 0

	// unmanaged register map
	static const uint32_t OP_MODE = 0x00;
	static const uint32_t REC_MODE = 0x01;
	static const uint32_t CAL_MODE = 0x02;
	static const uint32_t ADC_CONFIG = 0x03;
	static const uint32_t TEST_CONFIG1 = 0x04;
	static const uint32_t TEST_CONFIG2 = 0x05;
	static const uint32_t TEST_CONFIG3 = 0x06;
	static const uint32_t TEST_CONFIG4 = 0x07;
	static const uint32_t TEST_CONFIG5 = 0x08;
	static const uint32_t STATUS = 0x09;
	static const uint32_t SUPERSYNC0 = 0x0A;
	static const uint32_t SUPERSYNC1 = 0x0B;
	static const uint32_t SUPERSYNC2 = 0x0C;
	static const uint32_t SUPERSYNC3 = 0x0D;
	static const uint32_t SUPERSYNC4 = 0x0E;
	static const uint32_t SUPERSYNC5 = 0x0F;
	static const uint32_t SUPERSYNC6 = 0x10;
	static const uint32_t SUPERSYNC7 = 0x11;
	static const uint32_t SUPERSYNC8 = 0x12;
	static const uint32_t SUPERSYNC9 = 0x13;
	static const uint32_t SUPERSYNC10 = 0x14;
	static const uint32_t SUPERSYNC11 = 0x15;
	static const uint32_t SR_CHAIN6 = 0x16; // Odd channel base config
	static const uint32_t SR_CHAIN5 = 0x17; // Even channel base config
	static const uint32_t SR_CHAIN4 = 0x18; // Shank 4
	static const uint32_t SR_CHAIN3 = 0x19; // Shank 3
	static const uint32_t SR_CHAIN2 = 0x1A; // Shank 2
	static const uint32_t SR_CHAIN1 = 0x1B; // Shank 1
	static const uint32_t SR_LENGTH2 = 0x1C;
	static const uint32_t SR_LENGTH1 = 0x1D;
	static const uint32_t PROBE_ID = 0x1E;
	static const uint32_t SOFT_RESET = 0x1F;

	const uint32_t OFFSET_PROBE_SN = 0x00;
	const uint32_t OFFSET_FLEX_VERSION = 0x10;
	const uint32_t OFFSET_FLEX_REVISION = 0x11;
	const uint32_t OFFSET_FLEX_PN = 0x20;
	const uint32_t OFFSET_PROBE_PN = 0x40;

	uint64_t probeSNA;
	uint64_t probeSNB;

	Array<oni_frame_t*, CriticalSection, 2 * FramesPerSuperFrame> frameArray;

	JUCE_LEAK_DETECTOR(Neuropixels2e);
};
