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

#include <VisualizerEditorHeaders.h>
#include "OnixDevice.h"

namespace OnixSourcePlugin
{
	class OnixSource;
	class OnixSourceCanvas;
	class MemoryMonitorUsage;

	/**

		Used to change the number of data streams, and the
		number of channels per stream.

	*/
	class OnixSourceEditor : public VisualizerEditor,
		public Label::Listener,
		public Button::Listener,
		public TextEditor::Listener,
		public ComboBox::Listener
	{
	public:

		OnixSourceEditor(GenericProcessor* parentNode, OnixSource* source_);

		void labelTextChanged(Label* l) override;

		void buttonClicked(Button* b) override;

		void comboBoxChanged(ComboBox* cb) override;

		void updateComboBox(ComboBox* cb);

		void updateSettings() override;

		void setInterfaceEnabledState(bool newState);

		void startAcquisition() override;

		void stopAcquisition() override;

		Visualizer* createNewCanvas(void) override;

		void saveVisualizerEditorParameters(XmlElement* xml) override;

		void loadVisualizerEditorParameters(XmlElement* xml) override;

		void checkCanvas() { checkForCanvas(); };

		void resetCanvas();

		OnixSourceCanvas* getCanvas();

		bool isHeadstageSelected(PortName port);

		std::string getHeadstageSelected(int offset);

		std::string getHeadstageSelected(PortName port);

		void refreshComboBoxSelection();

		OnixDeviceMap createTabMapFromCanvas();

		void setConnectedStatus(bool);

	private:

		OnixSourceCanvas* canvas;
		OnixSource* source;

		const FillType fillDisconnected = FillType(Colours::transparentBlack);
		const FillType fillSearching = FillType(Colour::fromFloatRGBA(0.0f, 1.0f, 87.0f / 255, 1.0f));
		const FillType fillConnected = FillType(Colour::fromFloatRGBA(184.0f / 255, 0.0f, 252.0f / 255, 1.0f));

		const Colour statusIndicatorStrokeColor = Colours::black;

		const float statusIndicatorStrokeThickness = 1.0f;

		std::unique_ptr<DrawableRectangle> portStatusA;
		std::unique_ptr<DrawableRectangle> portStatusB;

		std::unique_ptr<Label> portLabelA;
		std::unique_ptr<Label> portLabelB;

		std::unique_ptr<ComboBox> headstageComboBoxA;
		std::unique_ptr<ComboBox> headstageComboBoxB;

		std::unique_ptr<Label> portVoltageOverrideLabelA;
		std::unique_ptr<Label> portVoltageOverrideLabelB;

		std::unique_ptr<Label> portVoltageValueA;
		std::unique_ptr<Label> portVoltageValueB;

		std::unique_ptr<Label> lastVoltageSetA;
		std::unique_ptr<Label> lastVoltageSetB;

		std::unique_ptr<UtilityButton> connectButton;

		std::unique_ptr<Label> liboniVersionLabel;

		std::unique_ptr<Label> blankEditor;

		void setComboBoxSelection(ComboBox* comboBox, std::string headstage);
		void addHeadstageComboBoxOptions(ComboBox* comboBox);
		void enableEditorElements(bool);
		bool configureAllDevices();
		bool configurePortVoltage(PortName, Label*, Label*, DrawableRectangle*);
		void setPortStatusAndVoltageValue(PortName, double, FillType, Label*, DrawableRectangle*);

		std::unique_ptr<MemoryMonitorUsage> memoryUsage;

		std::unique_ptr<Label> blockReadSizeValue;

		static constexpr int DefaultBlockReadSize = 4096;

		static constexpr char* missingCanvasErrorMessage = "The canvas for this plugin could not be found. Some functionality may not work as expected, and you may not be able to acquire or record data. Try removing and replacing the plugin.";

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnixSourceEditor);
	};
}
