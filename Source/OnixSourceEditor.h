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

#ifndef __OnixSourceEditor_H__
#define __OnixSourceEditor_H__

#include <VisualizerEditorHeaders.h>

#include "OnixSourceCanvas.h"
#include "NeuropixComponents.h"

class OnixSource;

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

    /** Constructor */
    OnixSourceEditor(GenericProcessor* parentNode, OnixSource* thread);

    /** Destructor */
    virtual ~OnixSourceEditor() { }

    /** Listener methods */
    void labelTextChanged(Label* l) override;

    void buttonClicked(Button* b) override;

    void comboBoxChanged(ComboBox* cb) override;

    /** Called when settings are changed */
    void updateSettings() override;

    /** Called at start of acquisition */
    void startAcquisition() override;

    /** Called when acquisition finishes */
    void stopAcquisition() override;

    /** Creates the Onix Source settings interface*/
    Visualizer* createNewCanvas(void) override;

    void checkCanvas() { checkForCanvas(); };

    void resetCanvas();

    float getPortVoltage(PortName port);

    OnixSourceCanvas* canvas;

private:

    OnixSource* thread;

    std::unique_ptr<Label> portLabelA;
    std::unique_ptr<Label> portLabelB;

    std::unique_ptr<ComboBox> headstageComboBoxA;
    std::unique_ptr<ComboBox> headstageComboBoxB;

    std::unique_ptr<Label> portVoltageValueA;
    std::unique_ptr<Label> portVoltageValueB;

    std::unique_ptr<UtilityButton> connectButton;

    std::unique_ptr<ToggleParameterEditor> passthroughEditorA;
    std::unique_ptr<ToggleParameterEditor> passthroughEditorB;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnixSourceEditor);
};

#endif // __OnixSourceEditor_H__
