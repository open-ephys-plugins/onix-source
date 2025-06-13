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

#include "Neuropixels1.h"

namespace OnixSourcePlugin
{
	/**
		Configures and streams data from a Neuropixels 1.0e device
	*/
	class Neuropixels1e : public Neuropixels1
	{
	public:
		friend class NeuropixelsV1eBackgroundUpdater;

		Neuropixels1e(std::string, std::string, const oni_dev_idx_t, std::shared_ptr<Onix1>);

		int configureDevice() override;
		bool updateSettings() override;
		void startAcquisition() override;
		void stopAcquisition() override;
		void addSourceBuffers(OwnedArray<DataBuffer>& sourceBuffers) override;
		void addFrame(oni_frame_t*) override;
		void processFrames() override;

		void configureSerDes();

		static OnixDeviceType getDeviceType();

	private:

		static constexpr char* STREAM_NAME_AP = "AP";
		static constexpr char* STREAM_NAME_LFP = "LFP";

		static constexpr int FlexEepromI2CAddress = 0x50;

		static constexpr uint32_t OFFSET_ID = 0;
		static constexpr uint32_t OFFSET_VERSION = 10;
		static constexpr uint32_t OFFSET_REVISION = 11;
		static constexpr uint32_t OFFSET_FLEXPN = 20;
		static constexpr uint32_t OFFSET_PROBEPN = 40;

		static constexpr uint8_t DefaultGPO10Config = 0b00010001; // GPIO0 Low, NP in MUX reset
		static constexpr uint8_t DefaultGPO32Config = 0b10010001; // LED off, GPIO1 Low
		static constexpr uint32_t Gpo10ResetMask = 1 << 3; // Used to issue mux reset command to probe
		static constexpr uint32_t Gpo32LedMask = 1 << 7; // Used to turn on and off LED

		std::unique_ptr<I2CRegisterContext> deserializer;
		std::unique_ptr<I2CRegisterContext> serializer;
		std::unique_ptr<I2CRegisterContext> flex;

		void resetProbe();
		void writeShiftRegisters();

		bool ledEnabled = true;

		// ADC to frame index
		// Input: ADC index
		// Output: index of ADC's data within a frame
		static constexpr std::array<size_t, NeuropixelsV1Values::AdcCount> adcToFrameIndex = {
			1, 9 , 17, 25, 33,
			2, 10, 18, 26, 34,
			3, 11, 19, 27, 35,
			4, 12, 20, 28, 36,
			5, 13, 21, 29, 37,
			6, 14, 22, 30, 38,
			7, 15
		};

		// ADC to channel
		// First dimension: ADC index
		// Second dimension: frame index within super frame
		// Output: channel number
		static constexpr std::array<std::array<size_t, superFramesPerUltraFrame>, NeuropixelsV1Values::AdcCount> rawToChannel = { {
			{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22 },
			{1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23 },
			{24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46 },
			{25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47 },
			{48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70 },
			{49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71 },
			{72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94 },
			{73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95 },
			{96, 98, 100, 102, 104, 106, 108, 110, 112, 114, 116, 118 },
			{97, 99, 101, 103, 105, 107, 109, 111, 113, 115, 117, 119 },
			{120, 122, 124, 126, 128, 130, 132, 134, 136, 138, 140, 142 },
			{121, 123, 125, 127, 129, 131, 133, 135, 137, 139, 141, 143 },
			{144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 166 },
			{145, 147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167 },
			{168, 170, 172, 174, 176, 178, 180, 182, 184, 186, 188, 190 },
			{169, 171, 173, 175, 177, 179, 181, 183, 185, 187, 189, 191 },
			{192, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 214 },
			{193, 195, 197, 199, 201, 203, 205, 207, 209, 211, 213, 215 },
			{216, 218, 220, 222, 224, 226, 228, 230, 232, 234, 236, 238 },
			{217, 219, 221, 223, 225, 227, 229, 231, 233, 235, 237, 239 },
			{240, 242, 244, 246, 248, 250, 252, 254, 256, 258, 260, 262 },
			{241, 243, 245, 247, 249, 251, 253, 255, 257, 259, 261, 263 },
			{264, 266, 268, 270, 272, 274, 276, 278, 280, 282, 284, 286 },
			{265, 267, 269, 271, 273, 275, 277, 279, 281, 283, 285, 287 },
			{288, 290, 292, 294, 296, 298, 300, 302, 304, 306, 308, 310 },
			{289, 291, 293, 295, 297, 299, 301, 303, 305, 307, 309, 311 },
			{312, 314, 316, 318, 320, 322, 324, 326, 328, 330, 332, 334 },
			{313, 315, 317, 319, 321, 323, 325, 327, 329, 331, 333, 335 },
			{336, 338, 340, 342, 344, 346, 348, 350, 352, 354, 356, 358 },
			{337, 339, 341, 343, 345, 347, 349, 351, 353, 355, 357, 359 },
			{360, 362, 364, 366, 368, 370, 372, 374, 376, 378, 380, 382 },
			{361, 363, 365, 367, 369, 371, 373, 375, 377, 379, 381, 383 }
		} };

		JUCE_LEAK_DETECTOR(Neuropixels1e);
	};

	/*

		A thread that updates probe settings in the background and shows a progress bar

	*/
	class NeuropixelsV1eBackgroundUpdater : public NeuropixelsV1BackgroundUpdater
	{
	public:
		NeuropixelsV1eBackgroundUpdater(Neuropixels1e* d);

		void run() override;

	private:

		JUCE_LEAK_DETECTOR(NeuropixelsV1eBackgroundUpdater);
	};
}
