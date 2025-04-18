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

#include "Devices/MemoryMonitor.h"

namespace OnixSourcePlugin
{
	class OnixSource;
	class OnixSourceCanvas;

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

		bool isHeadstageSelected(PortName port);

		String getHeadstageSelected(int offset);

		String getHeadstageSelected(PortName port);

		void refreshComboBoxSelection();

		std::map<int, OnixDeviceType> createTabMapFromCanvas();

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

		void setComboBoxSelection(ComboBox* comboBox, String headstage);
		void addHeadstageComboBoxOptions(ComboBox* comboBox);

		std::unique_ptr<MemoryMonitorUsage> memoryUsage;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnixSourceEditor);
	};
}
