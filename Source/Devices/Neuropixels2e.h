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

#pragma once

#include "../OnixDevice.h"
#include "../I2CRegisterContext.h"
#include "../NeuropixelsComponents.h"
#include "DS90UB9x.h"

namespace OnixSourcePlugin
{
	enum class NeuropixelsV2Reference : uint32_t
	{
		External,
		Tip1,
		Tip2,
		Tip3,
		Tip4
	};

	enum class NeuropixelsV2Status : uint32_t
	{
		SR_OK = 1 << 7
	};

	/*
		Configures and streams data from a Neuropixels 2.0e device (aka a configured raw deserializer)
	*/
	class Neuropixels2e : public INeuropixel<NeuropixelsV2eValues::numberOfChannels, NeuropixelsV2eValues::numberOfElectrodes>,
		public OnixDevice,
		public I2CRegisterContext
	{
	public:
		Neuropixels2e(std::string name, std::string hubName, const oni_dev_idx_t, std::shared_ptr<Onix1>);

		~Neuropixels2e()
		{
			if (serializer != nullptr)
			{
				selectProbe(NoProbeSelected);
				serializer->WriteByte((uint32_t)DS90UB9x::DS90UB9xSerializerI2CRegister::GPIO10, DefaultGPO10Config);
			}

			if (deviceContext != nullptr && deviceContext->isInitialized())
				deviceContext->setOption(ONIX_OPT_PASSTHROUGH, 0);
		}

		int configureDevice() override;

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

		static const int baseBitsPerChannel = 4;
		static const int configurationBitCount = NeuropixelsV2eValues::numberOfChannels * baseBitsPerChannel / 2;

		static const int referencePixelCount = 4;
		static const int dummyPixelCount = 4;
		static const int registersPerShank = NeuropixelsV2eValues::electrodesPerShank + referencePixelCount + dummyPixelCount;

		using BaseBitsArray = std::array<std::bitset<configurationBitCount>, 2>;
		using ShankBitsArray = std::array<std::bitset<registersPerShank>, 4>;

		BaseBitsArray static makeBaseBits(NeuropixelsV2Reference reference);
		ShankBitsArray static makeShankBits(NeuropixelsV2Reference reference, std::array<ElectrodeMetadata, NeuropixelsV2eValues::numberOfElectrodes> channelMap);

		template<int N>
		void writeShiftRegister(uint32_t srAddress, std::bitset<N> bits);

		void setGainCorrectionFile(int index, std::string filename);

		std::string getGainCorrectionFile(int index);

		// INeuropixel Methods

		std::vector<int> selectElectrodeConfiguration(std::string config) override;

		uint64_t getProbeSerialNumber(int index) override;

		void defineMetadata(ProbeSettings<NeuropixelsV2eValues::numberOfChannels, NeuropixelsV2eValues::numberOfElectrodes>*, int);

		void setSettings(ProbeSettings<numberOfChannels, numberOfElectrodes>* settings_, int index) override;

		static OnixDeviceType getDeviceType();

	private:

		static constexpr int NumberOfProbes = 2;

		DataBuffer* amplifierBuffer[NumberOfProbes];

		std::array<uint64_t, NumberOfProbes> probeSN;
		std::array<float, NumberOfProbes> gainCorrection;
		std::array<std::string, NumberOfProbes> gainCorrectionFilePath;

		void createDataStream(int n);

		uint64_t getProbeSN(uint8_t probeSelect);
		void configureSerDes();
		void setProbeSupply(bool);
		void resetProbes();

		void selectProbe(uint8_t probeSelect);
		void configureProbeStreaming();
		void writeConfiguration(ProbeSettings<numberOfChannels, numberOfElectrodes>*);

		NeuropixelsV2Reference getReference(int);
		static std::string getShankName(uint32_t shiftRegisterAddress);

		int m_numProbes = 0;

		const float sampleRate = 30000.0f;
		static const int numFrames = 10;
		static const int numSamples = numberOfChannels * numFrames;

		std::array<float, numSamples> samples;

		int64 sampleNumbers[numFrames];
		double timestamps[numFrames];
		uint64 eventCodes[numFrames];

		int frameCount = 0;
		int sampleNumber = 0;

		std::unique_ptr<I2CRegisterContext> serializer;
		std::unique_ptr<I2CRegisterContext> deserializer;
		std::unique_ptr<I2CRegisterContext> flex;
		std::unique_ptr<I2CRegisterContext> probeControl;

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

		Array<oni_frame_t*, CriticalSection, 2 * FramesPerSuperFrame> frameArray;

		static inline const std::array<int, AdcsPerProbe> adcIndices = {
			0, 1, 2,
			4, 5, 6,
			8, 9, 10,
			12, 13, 14,
			16, 17, 18,
			20, 21, 22,
			24, 25, 26,
			28, 29, 30
		};

		static inline const std::array<std::array<int, FramesPerSuperFrame>, AdcsPerProbe> rawToChannel = { {
			{ 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30 },                          // Data Index 9, ADC 0
			{ 128, 130, 132, 134, 136, 138, 140, 142, 144, 146, 148, 150, 152, 154, 156, 158 },     // Data Index 10, ADC 8
			{ 256, 258, 260, 262, 264, 266, 268, 270, 272, 274, 276, 278, 280, 282, 284, 286 },     // Data Index 11, ADC 16

			{ 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31 },                          // Data Index 13, ADC 1
			{ 129, 131, 133, 135, 137, 139, 141, 143, 145, 147, 149, 151, 153, 155, 157, 159 },     // Data Index 14, ADC 9
			{ 257, 259, 261, 263, 265, 267, 269, 271, 273, 275, 277, 279, 281, 283, 285, 287 },     // Data Index 15, ADC 17

			{ 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62 },                     // Data Index 17, ADC 2
			{ 160, 162, 164, 166, 168, 170, 172, 174, 176, 178, 180, 182, 184, 186, 188, 190 },     // Data Index 18, ADC 10
			{ 288, 290, 292, 294, 296, 298, 300, 302, 304, 306, 308, 310, 312, 314, 316, 318 },     // Data Index 19, ADC 18

			{ 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63 },                     // Data Index 21, ADC 3
			{ 161, 163, 165, 167, 169, 171, 173, 175, 177, 179, 181, 183, 185, 187, 189, 191 },     // Data Index 22, ADC 11
			{ 289, 291, 293, 295, 297, 299, 301, 303, 305, 307, 309, 311, 313, 315, 317, 319 },     // Data Index 23, ADC 19

			{ 64, 66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94 },                     // Data Index 25, ADC 4
			{ 192, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 214, 216, 218, 220, 222 },     // Data Index 26, ADC 12
			{ 320, 322, 324, 326, 328, 330, 332, 334, 336, 338, 340, 342, 344, 346, 348, 350 },     // Data Index 27, ADC 20

			{ 65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95 },                     // Data Index 29, ADC 5
			{ 193, 195, 197, 199, 201, 203, 205, 207, 209, 211, 213, 215, 217, 219, 221, 223 },     // Data Index 30, ADC 13
			{ 321, 323, 325, 327, 329, 331, 333, 335, 337, 339, 341, 343, 345, 347, 349, 351 },     // Data Index 31, ADC 21

			{ 96, 98, 100, 102, 104, 106, 108, 110, 112, 114, 116, 118, 120, 122, 124, 126 },       // Data Index 33, ADC 6
			{ 224, 226, 228, 230, 232, 234, 236, 238, 240, 242, 244, 246, 248, 250, 252, 254 },     // Data Index 34, ADC 14
			{ 352, 354, 356, 358, 360, 362, 364, 366, 368, 370, 372, 374, 376, 378, 380, 382 },     // Data Index 35, ADC 22

			{ 97, 99, 101, 103, 105, 107, 109, 111, 113, 115, 117, 119, 121, 123, 125, 127 },       // Data Index 37, ADC 7
			{ 225, 227, 229, 231, 233, 235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255 },     // Data Index 38, ADC 15
			{ 353, 355, 357, 359, 361, 363, 365, 367, 369, 371, 373, 375, 377, 379, 381, 383 },     // Data Index 39, ADC 23
		} };

		JUCE_LEAK_DETECTOR(Neuropixels2e);
	};
}
