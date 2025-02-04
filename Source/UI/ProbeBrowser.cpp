/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2021 Allen Institute for Brain Science and Open Ephys

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

#include "ProbeBrowser.h"

#include "../Devices/Neuropixels_1.h"
#include "NeuropixV1Interface.h"

ProbeBrowser::ProbeBrowser(SettingsInterface* parent_) : parent(parent_)
{
	cursorType = MouseCursor::NormalCursor;

	activityToView = ActivityToView::APVIEW;

	Neuropixels_1* device = (Neuropixels_1*)parent->device;
	ProbeType probeType = device->settings.probeType;

	if (probeType == ProbeType::NPX_V2E || probeType == ProbeType::NPX_V2E_BETA)
	{
		maxPeakToPeakAmplitude = 1000.0f;
	}
	else
	{
		maxPeakToPeakAmplitude = 250.0f;
	}

	isOverZoomRegion = false;
	isOverUpperBorder = false;
	isOverLowerBorder = false;
	isSelectionActive = false;
	isOverElectrode = false;

	// PROBE SPECIFIC DRAWING SETTINGS
	minZoomHeight = 40;
	maxZoomHeight = 120;
	pixelHeight = 1;
	zoomOffset = 50;
	int defaultZoomHeight = 100;

	if (device->settings.probeMetadata.columns_per_shank == 8)
	{
		maxZoomHeight = 450;
		minZoomHeight = 300;
		defaultZoomHeight = 400;
		pixelHeight = 10;
		zoomOffset = 0;
	}
	else if (device->settings.probeMetadata.columns_per_shank > 8)
	{
		maxZoomHeight = 520;
		minZoomHeight = 520;
		defaultZoomHeight = 520;
		pixelHeight = 20;
		zoomOffset = 0;
	}

	if (device->settings.probeMetadata.rows_per_shank > 650)
	{
		maxZoomHeight = 60;
		minZoomHeight = 10;
		defaultZoomHeight = 30;
	}

	if (device->settings.probeMetadata.rows_per_shank > 1400)
	{
		maxZoomHeight = 30;
		minZoomHeight = 5;
		defaultZoomHeight = 20;
	}

	if (device->settings.probeMetadata.shank_count == 4)
	{
		defaultZoomHeight = 80;
	}

	// ALSO CONFIGURE CHANNEL JUMP
	if (device->settings.probeMetadata.electrodes_per_shank < 500)
		channelLabelSkip = 50;
	else if (device->settings.probeMetadata.electrodes_per_shank >= 500
		&& device->settings.probeMetadata.electrodes_per_shank < 1500)
		channelLabelSkip = 100;
	else if (device->settings.probeMetadata.electrodes_per_shank >= 1500
		&& device->settings.probeMetadata.electrodes_per_shank < 3000)
		channelLabelSkip = 200;
	else
		channelLabelSkip = 500;

	zoomHeight = defaultZoomHeight; // number of rows
	lowerBound = 530; // bottom of interface

	disconnectedColours[Bank::NONE] = Colour(0, 0, 0);
	disconnectedColours[Bank::A] = Colour(180, 180, 180);
	disconnectedColours[Bank::B] = Colour(160, 160, 160);
	disconnectedColours[Bank::C] = Colour(140, 140, 140);
	disconnectedColours[Bank::D] = Colour(120, 120, 120);
	disconnectedColours[Bank::E] = Colour(180, 180, 180);
	disconnectedColours[Bank::F] = Colour(160, 160, 160);
	disconnectedColours[Bank::G] = Colour(140, 140, 140);
	disconnectedColours[Bank::H] = Colour(120, 120, 120);
	disconnectedColours[Bank::I] = Colour(180, 180, 180);
	disconnectedColours[Bank::J] = Colour(160, 160, 160);
	disconnectedColours[Bank::K] = Colour(140, 140, 140);
	disconnectedColours[Bank::L] = Colour(120, 120, 120);
	disconnectedColours[Bank::M] = Colour(180, 180, 180);

	dragZoneWidth = 10;
}

ProbeBrowser::~ProbeBrowser()
{
}

void ProbeBrowser::mouseMove(const MouseEvent& event)
{
	float y = event.y;
	float x = event.x;

	bool isOverZoomRegionNew = false;
	bool isOverUpperBorderNew = false;
	bool isOverLowerBorderNew = false;

	// check for move into zoom region
	if ((y > lowerBound - zoomOffset - zoomHeight - dragZoneWidth / 2.0f)
		&& (y < lowerBound - zoomOffset + dragZoneWidth / 2.0f)
		&& (x > 9)
		&& (x < 54 + shankOffset))
	{
		isOverZoomRegionNew = true;
	}
	else
	{
		isOverZoomRegionNew = false;
	}

	// check for move over upper border or lower border
	if (isOverZoomRegionNew)
	{
		if (y > lowerBound - zoomHeight - zoomOffset - dragZoneWidth / 2.0f
			&& y < lowerBound - zoomHeight - zoomOffset + dragZoneWidth / 2.0f)
		{
			isOverUpperBorderNew = true;
		}
		else if (y > lowerBound - zoomOffset - dragZoneWidth / 2.0f
			&& y < lowerBound - zoomOffset + dragZoneWidth / 2.0f)
		{
			isOverLowerBorderNew = true;
		}
		else
		{
			isOverUpperBorderNew = false;
			isOverLowerBorderNew = false;
		}
	}

	// update cursor type
	if (isOverZoomRegionNew != isOverZoomRegion || isOverLowerBorderNew != isOverLowerBorder || isOverUpperBorderNew != isOverUpperBorder)
	{
		isOverZoomRegion = isOverZoomRegionNew;
		isOverUpperBorder = isOverUpperBorderNew;
		isOverLowerBorder = isOverLowerBorderNew;

		if (!isOverZoomRegion)
		{
			cursorType = MouseCursor::NormalCursor;
		}
		else
		{
			if (isOverUpperBorder)
				cursorType = MouseCursor::TopEdgeResizeCursor;
			else if (isOverLowerBorder)
				cursorType = MouseCursor::BottomEdgeResizeCursor;
			else
				cursorType = MouseCursor::NormalCursor;
		}

		repaint();
	}

	// check for movement over electrode
	if ((x > leftEdge) // in electrode selection region
		&& (x < rightEdge)
		&& (y < lowerBound)
		&& (y > 18))
	{
		int index = getNearestElectrode(x, y);

		if (index > -1)
		{
			isOverElectrode = true;
			electrodeInfoString = getElectrodeInfoString(index);
		}

		repaint();
	}
	else
	{
		bool isOverChannelNew = false;

		if (isOverChannelNew != isOverElectrode)
		{
			isOverElectrode = isOverChannelNew;
			repaint();
		}
	}
}

int ProbeBrowser::getNearestElectrode(int x, int y) const
{
	int row = (lowerBound - y) / electrodeHeight + zoomAreaMinRow + 1;

	Neuropixels_1* device = (Neuropixels_1*)parent->device;

	int shankWidth = electrodeHeight * device->settings.probeMetadata.columns_per_shank;
	int totalWidth = shankWidth * device->settings.probeMetadata.shank_count + shankWidth * (device->settings.probeMetadata.shank_count - 1);

	int shank = 0;
	int column = -1;

	for (shank = 0; shank < device->settings.probeMetadata.shank_count; shank++)
	{
		int leftEdge = leftEdgeOffset + shankOffset - totalWidth / 2 + shankWidth * 2 * shank;
		int rightEdge = leftEdge + shankWidth;

		if (x >= leftEdge && x <= rightEdge)
		{
			column = (x - leftEdge) / electrodeHeight;
			break;
		}
	}

	Array<ElectrodeMetadata> electrodeMetadata = device->settings.electrodeMetadata;

	for (int i = 0; i < electrodeMetadata.size(); i++)
	{
		if ((electrodeMetadata[i].row_index == row)
			&& (electrodeMetadata[i].column_index == column)
			&& (electrodeMetadata[i].shank == shank))
		{
			return i;
		}
	}

	return -1;
}

Array<int> ProbeBrowser::getElectrodesWithinBounds(int x, int y, int w, int h) const
{
	int startrow = (lowerBound - y - h) / electrodeHeight + zoomAreaMinRow + 1;
	int endrow = (lowerBound - y) / electrodeHeight + zoomAreaMinRow + 1;

	Neuropixels_1* device = (Neuropixels_1*)parent->device;

	int shankWidth = electrodeHeight * device->settings.probeMetadata.columns_per_shank;
	int totalWidth = shankWidth * device->settings.probeMetadata.shank_count + shankWidth * (device->settings.probeMetadata.shank_count - 1);

	Array<int> selectedColumns;

	for (int i = 0; i < device->settings.probeMetadata.shank_count * device->settings.probeMetadata.columns_per_shank; i++)
	{
		int shank = i / device->settings.probeMetadata.columns_per_shank;
		int column = i % device->settings.probeMetadata.columns_per_shank;

		int l = leftEdge + shankWidth * 2 * shank + electrodeHeight * column;
		int r = l + electrodeHeight / 2;

		if (x < l + electrodeHeight / 2 && x + w > r)
			selectedColumns.add(i);
	}

	Array<int> inds;
	Array<ElectrodeMetadata> electrodeMetadata = device->settings.electrodeMetadata;

	for (int i = 0; i < electrodeMetadata.size(); i++)
	{
		if ((electrodeMetadata[i].row_index >= startrow) && (electrodeMetadata[i].row_index <= endrow))

		{
			int column_id = electrodeMetadata[i].shank * device->settings.probeMetadata.columns_per_shank + electrodeMetadata[i].column_index;

			if (selectedColumns.indexOf(column_id) > -1)
			{
				inds.add(i);
			}
		}
	}

	return inds;
}

String ProbeBrowser::getElectrodeInfoString(int index)
{
	Neuropixels_1* device = (Neuropixels_1*)parent->device;

	Array<ElectrodeMetadata> electrodeMetadata = device->settings.electrodeMetadata;

	String a;
	a += "Electrode ";
	a += String(electrodeMetadata[index].global_index + 1);

	a += "\nBank ";

	switch (electrodeMetadata[index].bank)
	{
	case Bank::A:
		a += "A";
		break;
	case Bank::B:
		a += "B";
		break;
	case Bank::C:
		a += "C";
		break;
	case Bank::D:
		a += "D";
		break;
	case Bank::E:
		a += "E";
		break;
	case Bank::F:
		a += "F";
		break;
	case Bank::G:
		a += "G";
		break;
	case Bank::H:
		a += "H";
		break;
	case Bank::I:
		a += "I";
		break;
	case Bank::J:
		a += "J";
		break;
	case Bank::K:
		a += "K";
		break;
	case Bank::L:
		a += "L";
		break;
	case Bank::M:
		a += "M";
		break;
	default:
		a += " NONE";
	}

	a += ", Channel ";
	a += String(electrodeMetadata[index].channel + 1);

	a += "\nY Position: ";
	a += String(electrodeMetadata[index].ypos);

	a += "\nType: ";

	if (electrodeMetadata[index].type == ElectrodeType::REFERENCE)
	{
		a += "REFERENCE";
	}
	else
	{
		a += "SIGNAL";

		a += "\nEnabled: ";

		if (electrodeMetadata[index].status == ElectrodeStatus::CONNECTED)
			a += "YES";
		else
			a += "NO";
	}

	NeuropixV1Interface* neuropixInterface = (NeuropixV1Interface*)parent;

	if (neuropixInterface->apGainComboBox != nullptr)
	{
		a += "\nAP Gain: ";
		a += String(neuropixInterface->apGainComboBox->getText());
	}

	if (neuropixInterface->lfpGainComboBox != nullptr)
	{
		a += "\nLFP Gain: ";
		a += String(neuropixInterface->lfpGainComboBox->getText());
	}

	if (neuropixInterface->referenceComboBox != nullptr)
	{
		a += "\nReference: ";
		a += String(neuropixInterface->referenceComboBox->getText());
	}

	return a;
}

void ProbeBrowser::mouseUp(const MouseEvent& event)
{
	if (isSelectionActive)
	{
		isSelectionActive = false;
		repaint();
	}
}

void ProbeBrowser::mouseDown(const MouseEvent& event)
{
	initialOffset = zoomOffset;
	initialHeight = zoomHeight;

	Neuropixels_1* device = (Neuropixels_1*)parent->device;

	if (!event.mods.isRightButtonDown())
	{
		if (event.x > 150 && event.x < 400)
		{
			if (!event.mods.isShiftDown())
			{
				for (int i = 0; i < device->settings.electrodeMetadata.size(); i++)
					device->settings.electrodeMetadata.getReference(i).isSelected = false;
			}

			if (event.x > leftEdge && event.x < rightEdge)
			{
				int chan = getNearestElectrode(event.x, event.y);

				if (chan >= 0 && chan < device->settings.electrodeMetadata.size())
				{
					device->settings.electrodeMetadata.getReference(chan).isSelected = !device->settings.electrodeMetadata.getReference(chan).isSelected;
				}
			}
			repaint();
		}
	}
	else
	{
		if (event.x > 225 + 10 && event.x < 225 + 150)
		{
			int currentAnnotationNum = 0;

			NeuropixV1Interface* neuropixInterface = (NeuropixV1Interface*)parent;

			for (int i = 0; i < neuropixInterface->annotations.size(); i++)
			{
				Annotation& a = neuropixInterface->annotations.getReference(i);
				float yLoc = a.currentYLoc;

				if (float(event.y) < yLoc && float(event.y) > yLoc - 12)
				{
					currentAnnotationNum = i;
					break;
				}
				else
				{
					currentAnnotationNum = -1;
				}
			}

			if (currentAnnotationNum > -1)
			{
				PopupMenu annotationMenu;

				annotationMenu.addItem(1, "Delete annotation", true);

				const int result = annotationMenu.show();

				switch (result)
				{
				case 0:
					break;
				case 1:
					neuropixInterface->annotations.removeRange(currentAnnotationNum, 1);
					repaint();
					break;
				default:

					break;
				}
			}
		}
	}
}

void ProbeBrowser::mouseDrag(const MouseEvent& event)
{
	if (isOverZoomRegion)
	{
		if (isOverUpperBorder)
		{
			zoomHeight = initialHeight - event.getDistanceFromDragStartY();

			if (zoomHeight > lowerBound - zoomOffset - 15)
				zoomHeight = lowerBound - zoomOffset - 15;
		}
		else if (isOverLowerBorder)
		{
			zoomOffset = initialOffset - event.getDistanceFromDragStartY();

			if (zoomOffset < 0)
			{
				zoomOffset = 0;
			}
			else
			{
				zoomHeight = initialHeight + event.getDistanceFromDragStartY();
			}
		}
		else
		{
			zoomOffset = initialOffset - event.getDistanceFromDragStartY();

			if (zoomOffset < 0)
			{
				zoomOffset = 0;
			}
		}
	}
	else if (event.x > 150 && event.x < 450)
	{
		int w = event.getDistanceFromDragStartX();
		int h = event.getDistanceFromDragStartY();
		int x = event.getMouseDownX();
		int y = event.getMouseDownY();

		if (w < 0)
		{
			x = x + w;
			w = -w;
		}

		if (h < 0)
		{
			y = y + h;
			h = -h;
		}

		isSelectionActive = true;

		Array<int> inBounds = getElectrodesWithinBounds(x, y, w, h);

		if (x < rightEdge)
		{
			Neuropixels_1* device = (Neuropixels_1*)parent->device;

			for (int i = 0; i < device->settings.electrodeMetadata.size(); i++)
			{
				if (inBounds.indexOf(i) > -1)
				{
					device->settings.electrodeMetadata.getReference(i).isSelected = true;
				}
				else
				{
					if (!event.mods.isShiftDown())
						device->settings.electrodeMetadata.getReference(i).isSelected = false;
				}
			}
		}

		repaint();
	}

	if (zoomHeight < minZoomHeight)
		zoomHeight = minZoomHeight;
	if (zoomHeight > maxZoomHeight)
		zoomHeight = maxZoomHeight;

	if (zoomOffset > lowerBound - zoomHeight - 15)
		zoomOffset = lowerBound - zoomHeight - 15;
	else if (zoomOffset < 0)
		zoomOffset = 0;

	repaint();
}

void ProbeBrowser::mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel)
{
	if (event.x > 100 && event.x < 450)
	{
		if (wheel.deltaY > 0)
			zoomOffset += 2;
		else
			zoomOffset -= 2;

		if (zoomOffset > lowerBound - zoomHeight - 15)
			zoomOffset = lowerBound - zoomHeight - 15;
		else if (zoomOffset < 0)
			zoomOffset = 0;

		repaint();
	}
}

MouseCursor ProbeBrowser::getMouseCursor()
{
	MouseCursor c = MouseCursor(cursorType);

	return c;
}

void ProbeBrowser::paint(Graphics& g)
{
	int LEFT_BORDER = 30;
	int TOP_BORDER = 33;
	int SHANK_HEIGHT = 480;
	int INTERSHANK_DISTANCE = 30;

	// draw zoomed-out channels

	int channelSpan = SHANK_HEIGHT;
	int pixelGap = 2;

	Neuropixels_1* device = (Neuropixels_1*)parent->device;

	Array<ElectrodeMetadata> electrodeMetadata = device->settings.electrodeMetadata;

	if (device->settings.probeMetadata.columns_per_shank > 8)
	{
		pixelGap = 1;
	}

	for (int i = 0; i < electrodeMetadata.size(); i++)
	{
		g.setColour(getElectrodeColour(i).withAlpha(0.5f));

		for (int px = 0; px < pixelHeight; px++)
			g.fillRect(
				LEFT_BORDER
				+ electrodeMetadata[i].column_index
				* pixelGap
				+ electrodeMetadata[i].shank
				* INTERSHANK_DISTANCE,
				TOP_BORDER
				+ channelSpan
				- int(float(electrodeMetadata[i].row_index)
					* float(channelSpan)
					/ float(device->settings.probeMetadata.rows_per_shank))
				- px,
				1,
				1);
	}

	// draw channel numbers
	g.setColour(Colours::grey);
	g.setFont(12);

	int ch = 0;

	int ch_interval = SHANK_HEIGHT * channelLabelSkip / device->settings.probeMetadata.electrodes_per_shank;

	// draw mark for every N channels

	shankOffset = INTERSHANK_DISTANCE * (device->settings.probeMetadata.shank_count - 1);
	for (int i = TOP_BORDER + channelSpan; i > TOP_BORDER; i -= ch_interval)
	{
		g.drawLine(6, i, 18, i);
		g.drawLine(44 + shankOffset, i, 54 + shankOffset, i);
		g.drawText(String(ch), 59 + shankOffset, int(i) - 6, 100, 12, Justification::left, false);
		ch += channelLabelSkip;
	}

	// draw top channel
	g.drawLine(6, TOP_BORDER, 18, TOP_BORDER);
	g.drawLine(44 + shankOffset, TOP_BORDER, 54 + shankOffset, TOP_BORDER);
	g.drawText(String(device->settings.probeMetadata.electrodes_per_shank),
		59 + shankOffset,
		TOP_BORDER - 6,
		100,
		12,
		Justification::left,
		false);

	// draw shank outline
	g.setColour(Colours::lightgrey);

	for (int i = 0; i < device->settings.probeMetadata.shank_count; i++)
	{
		Path shankPath = Path();
		shankPath.restoreFromString(device->settings.probeMetadata.shankOutline.toString());
		shankPath.applyTransform(AffineTransform::translation(INTERSHANK_DISTANCE * i, 0.0f));
		g.strokePath(shankPath, PathStrokeType(1.0));
	}

	// draw zoomed channels
	float miniRowHeight = float(channelSpan) / float(device->settings.probeMetadata.rows_per_shank); // pixels per row

	float lowestRow = (zoomOffset - 17) / miniRowHeight;
	float highestRow = lowestRow + (zoomHeight / miniRowHeight);

	zoomAreaMinRow = int(lowestRow);

	if (device->settings.probeMetadata.columns_per_shank > 8)
	{
		electrodeHeight = jmin(lowerBound / (highestRow - lowestRow), 12.0f);
	}
	else
	{
		electrodeHeight = lowerBound / (highestRow - lowestRow);
	}

	for (int i = 0; i < electrodeMetadata.size(); i++)
	{
		if (electrodeMetadata[i].row_index >= int(lowestRow)
			&& electrodeMetadata[i].row_index < int(highestRow))
		{
			float xLoc = leftEdgeOffset + shankOffset - electrodeHeight * device->settings.probeMetadata.columns_per_shank / 2
				+ electrodeHeight * electrodeMetadata[i].column_index + electrodeMetadata[i].shank * electrodeHeight * 4
				- (floor(device->settings.probeMetadata.shank_count / 2.0f) * electrodeHeight * 3);
			float yLoc = lowerBound - ((electrodeMetadata[i].row_index - int(lowestRow)) * electrodeHeight);

			if (electrodeMetadata[i].isSelected)
			{
				g.setColour(Colours::white);
				g.fillRect(xLoc, yLoc, electrodeHeight, electrodeHeight);
			}

			g.setColour(getElectrodeColour(i));

			g.fillRect(xLoc + 1, yLoc + 1, electrodeHeight - 2, electrodeHeight - 2);
		}
	}

	if (isOverZoomRegion)
		g.setColour(Colour(25, 25, 25));
	else
		g.setColour(Colour(55, 55, 55));

	Path upperBorder;
	upperBorder.startNewSubPath(5, lowerBound - zoomOffset - zoomHeight);
	upperBorder.lineTo(54 + shankOffset, lowerBound - zoomOffset - zoomHeight);
	upperBorder.lineTo(100 + shankOffset, 16);
	upperBorder.lineTo(330 + shankOffset, 16);

	Path lowerBorder;
	lowerBorder.startNewSubPath(5, lowerBound - zoomOffset);
	lowerBorder.lineTo(54 + shankOffset, lowerBound - zoomOffset);
	lowerBorder.lineTo(100 + shankOffset, lowerBound + 16);
	lowerBorder.lineTo(330 + shankOffset, lowerBound + 16);

	g.strokePath(upperBorder, PathStrokeType(2.0));
	g.strokePath(lowerBorder, PathStrokeType(2.0));

	// draw selection zone
	int shankWidth = electrodeHeight * device->settings.probeMetadata.columns_per_shank;
	int totalWidth = shankWidth * device->settings.probeMetadata.shank_count + shankWidth * (device->settings.probeMetadata.shank_count - 1);

	leftEdge = leftEdgeOffset + shankOffset - totalWidth / 2;
	rightEdge = leftEdgeOffset + shankOffset + totalWidth / 2;

	if (isSelectionActive)
	{
		g.setColour(Colours::white.withAlpha(0.5f));
	}

	if (isOverElectrode)
	{
		g.setColour(Colour(55, 55, 55));
		g.setFont(15);
		g.drawMultiLineText(electrodeInfoString,
			250 + shankOffset + 45,
			330,
			250);
	}
}

void ProbeBrowser::drawAnnotations(Graphics& g)
{
	NeuropixV1Interface* neuropixInterface = (NeuropixV1Interface*)parent;

	for (int i = 0; i < neuropixInterface->annotations.size(); i++)
	{
		bool shouldAppear = false;

		Annotation& a = neuropixInterface->annotations.getReference(i);

		for (int j = 0; j < a.electrodes.size(); j++)
		{
			if (j > lowestElectrode || j < highestElectrode)
			{
				shouldAppear = true;
				break;
			}
		}

		if (shouldAppear)
		{
			float xLoc = 225 + 30;
			int ch = a.electrodes[0];

			float midpoint = lowerBound / 2.0f + 8;

			float yLoc = lowerBound - ((ch - lowestElectrode - (ch % 2)) / 2.0f * electrodeHeight) + 10;

			yLoc = (midpoint + 3 * yLoc) / 4;
			a.currentYLoc = yLoc;

			float alpha;

			if (yLoc > lowerBound - 250)
				alpha = (lowerBound - yLoc) / (250.f);
			else if (yLoc < 250)
				alpha = 1.0f - (250.f - yLoc) / 200.f;
			else
				alpha = 1.0f;

			if (alpha < 0)
				alpha = -alpha;

			if (alpha < 0)
				alpha = 0;

			if (alpha > 1.0f)
				alpha = 1.0f;

			g.setColour(a.colour.withAlpha(alpha));

			g.drawMultiLineText(a.text, xLoc + 2, yLoc, 150);

			float xLoc2 = 225 - electrodeHeight * (1 - (ch % 2)) + electrodeHeight / 2;
			float yLoc2 = lowerBound - ((ch - lowestElectrode - (ch % 2)) / 2.0f * electrodeHeight) + electrodeHeight / 2;

			g.drawLine(xLoc - 5, yLoc - 3, xLoc2, yLoc2);
			g.drawLine(xLoc - 5, yLoc - 3, xLoc, yLoc - 3);
		}
	}
}

Colour ProbeBrowser::getElectrodeColour(int i)
{
	Neuropixels_1* device = (Neuropixels_1*)parent->device;

	Array<ElectrodeMetadata> electrodeMetadata = device->settings.electrodeMetadata;

	if (electrodeMetadata[i].status == ElectrodeStatus::DISCONNECTED) // not available
	{
		return disconnectedColours[electrodeMetadata[i].bank];
	}
	else if (electrodeMetadata[i].type == ElectrodeType::REFERENCE)
		return Colours::black;
	else
	{
		NeuropixV1Interface* neuropixInterface = (NeuropixV1Interface*)parent;
		if (neuropixInterface->mode == VisualizationMode::ENABLE_VIEW) // ENABLED STATE
		{
			return Colours::yellow;
		}
		else if (neuropixInterface->mode == VisualizationMode::AP_GAIN_VIEW) // AP GAIN
		{
		    return Colour (25 * device->settings.apGainIndex, 25 * device->settings.apGainIndex, 50);
		}
		else if (neuropixInterface->mode == VisualizationMode::LFP_GAIN_VIEW) // LFP GAIN
		{
		    return Colour (66, 25 * device->settings.lfpGainIndex, 35 * device->settings.lfpGainIndex);
		}
		else if (neuropixInterface->mode == VisualizationMode::REFERENCE_VIEW)
		{
			if (neuropixInterface->referenceComboBox != nullptr)
			{
				String referenceDescription = neuropixInterface->referenceComboBox->getText();

				if (referenceDescription.contains("Ext"))
					return Colours::darksalmon;
				else if (referenceDescription.contains("Tip"))
					return Colours::orange;
				else
					return Colours::purple;
			}
			else
			{
				return Colours::black;
			}
		}
		else if (neuropixInterface->mode == VisualizationMode::ACTIVITY_VIEW)
		{
			if (electrodeMetadata[i].status == ElectrodeStatus::CONNECTED)
			{
				return electrodeMetadata.getReference(i).colour;
			}
			else
			{
				return Colours::grey;
			}
		}
	}
}

void ProbeBrowser::timerCallback()
{
}

int ProbeBrowser::getZoomHeight() const
{
	return zoomHeight;
}

int ProbeBrowser::getZoomOffset() const
{
	return zoomOffset;
}

void ProbeBrowser::setZoomHeightAndOffset(int newHeight, int newOffset)
{
	if (newHeight >= minZoomHeight && newHeight <= maxZoomHeight)
	{
		zoomHeight = newHeight;
		zoomOffset = 0;
	}
}
