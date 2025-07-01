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
#include "SettingsInterface.h"

namespace OnixSourcePlugin
{
	template<int ch, int e>
	class ProbeBrowser : public Component,
		public Timer
	{
	public:
		ProbeBrowser(SettingsInterface* parent_, int probeIndex_)
		{
			parent = parent_;
			probeIndex = probeIndex_;

			cursorType = MouseCursor::NormalCursor;

			activityToView = ActivityToView::APVIEW;

			isOverZoomRegion = false;
			isOverUpperBorder = false;
			isOverLowerBorder = false;
			isSelectionActive = false;
			isOverElectrode = false;

			setDrawingSettings();
		}

		void setDrawingSettings()
		{
			auto settings = getSettings();

			if (settings == nullptr) return;

			// PROBE SPECIFIC DRAWING SETTINGS
			minZoomHeight = 40;
			maxZoomHeight = 120;
			pixelHeight = 1;
			zoomOffset = 50;
			int defaultZoomHeight = 100;

			ProbeMetadata probeMetadata = settings->probeMetadata;

			if (probeMetadata.columns_per_shank == 8)
			{
				maxZoomHeight = 450;
				minZoomHeight = 300;
				defaultZoomHeight = 400;
				pixelHeight = 10;
				zoomOffset = 0;
			}
			else if (probeMetadata.columns_per_shank > 8)
			{
				maxZoomHeight = 520;
				minZoomHeight = 520;
				defaultZoomHeight = 520;
				pixelHeight = 20;
				zoomOffset = 0;
			}

			if (probeMetadata.rows_per_shank > 650)
			{
				maxZoomHeight = 60;
				minZoomHeight = 10;
				defaultZoomHeight = 30;
			}

			if (probeMetadata.rows_per_shank > 1400)
			{
				maxZoomHeight = 30;
				minZoomHeight = 5;
				defaultZoomHeight = 20;
			}

			if (probeMetadata.shank_count == 4)
			{
				defaultZoomHeight = 80;
			}

			// ALSO CONFIGURE CHANNEL JUMP
			if (probeMetadata.electrodes_per_shank < 500)
				channelLabelSkip = 50;
			else if (probeMetadata.electrodes_per_shank >= 500
				&& probeMetadata.electrodes_per_shank < 1500)
				channelLabelSkip = 100;
			else if (probeMetadata.electrodes_per_shank >= 1500
				&& probeMetadata.electrodes_per_shank < 3000)
				channelLabelSkip = 200;
			else
				channelLabelSkip = 500;

			zoomHeight = defaultZoomHeight; // number of rows
			lowerBound = 530; // bottom of interface

			disconnectedColors[Bank::NONE] = Colour(0, 0, 0);
			disconnectedColors[Bank::A] = Colour(180, 180, 180);
			disconnectedColors[Bank::B] = Colour(160, 160, 160);
			disconnectedColors[Bank::C] = Colour(140, 140, 140);
			disconnectedColors[Bank::D] = Colour(120, 120, 120);
			disconnectedColors[Bank::E] = Colour(180, 180, 180);
			disconnectedColors[Bank::F] = Colour(160, 160, 160);
			disconnectedColors[Bank::G] = Colour(140, 140, 140);
			disconnectedColors[Bank::H] = Colour(120, 120, 120);
			disconnectedColors[Bank::I] = Colour(180, 180, 180);
			disconnectedColors[Bank::J] = Colour(160, 160, 160);
			disconnectedColors[Bank::K] = Colour(140, 140, 140);
			disconnectedColors[Bank::L] = Colour(120, 120, 120);
			disconnectedColors[Bank::M] = Colour(180, 180, 180);

			dragZoneWidth = 10;
		}

		void mouseMove(const MouseEvent& event)
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

		void mouseDown(const MouseEvent& event)
		{
			auto settings = getSettings();

			if (settings == nullptr) return;

			initialOffset = zoomOffset;
			initialHeight = zoomHeight;

			if (event.x > 150 && event.x < 400)
			{
				if (!event.mods.isShiftDown())
				{
					for (int i = 0; i < settings->electrodeMetadata.size(); i++)
						settings->electrodeMetadata[i].isSelected = false;
				}

				if (event.x > leftEdge && event.x < rightEdge)
				{
					int chan = getNearestElectrode(event.x, event.y);

					if (chan >= 0 && chan < settings->electrodeMetadata.size())
					{
						settings->electrodeMetadata[chan].isSelected = !settings->electrodeMetadata[chan].isSelected;
					}
				}
				repaint();
			}
		}

		void mouseDrag(const MouseEvent& event)
		{
			auto settings = getSettings();

			if (settings == nullptr) return;

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
					for (int i = 0; i < settings->electrodeMetadata.size(); i++)
					{
						if (inBounds.indexOf(i) > -1)
						{
							settings->electrodeMetadata[i].isSelected = true;
						}
						else
						{
							if (!event.mods.isShiftDown())
								settings->electrodeMetadata[i].isSelected = false;
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

		void mouseUp(const MouseEvent& event)
		{
			if (isSelectionActive)
			{
				isSelectionActive = false;
				repaint();
			}
		}

		void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel)
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

		MouseCursor getMouseCursor()
		{
			MouseCursor c = MouseCursor(cursorType);

			return c;
		}

		void timerCallback() {};

		void paint(Graphics& g)
		{
			auto settings = getSettings();

			if (settings == nullptr) return;

			int LEFT_BORDER = 30;
			int TOP_BORDER = 33;
			int SHANK_HEIGHT = 480;
			int INTERSHANK_DISTANCE = 30;

			// draw zoomed-out channels

			int channelSpan = SHANK_HEIGHT;
			int pixelGap = 2;

			if (settings->probeMetadata.columns_per_shank > 8)
			{
				pixelGap = 1;
			}

			for (int i = 0; i < settings->electrodeMetadata.size(); i++)
			{
				g.setColour(getElectrodeColour(i).withAlpha(0.5f));

				for (int px = 0; px < pixelHeight; px++)
					g.fillRect(
						LEFT_BORDER
						+ settings->electrodeMetadata[i].column_index
						* pixelGap
						+ settings->electrodeMetadata[i].shank
						* INTERSHANK_DISTANCE,
						TOP_BORDER
						+ channelSpan
						- int(float(settings->electrodeMetadata[i].row_index)
							* float(channelSpan)
							/ float(settings->probeMetadata.rows_per_shank))
						- px,
						1,
						1);
			}

			// draw channel numbers
			g.setColour(Colours::grey);
			g.setFont(12);

			int ch = 0;

			int ch_interval = SHANK_HEIGHT * channelLabelSkip / settings->probeMetadata.electrodes_per_shank;

			// draw mark for every N channels

			shankOffset = INTERSHANK_DISTANCE * (settings->probeMetadata.shank_count - 1);
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
			g.drawText(String(settings->probeMetadata.electrodes_per_shank),
				59 + shankOffset,
				TOP_BORDER - 6,
				100,
				12,
				Justification::left,
				false);

			// draw shank outline
			g.setColour(Colours::lightgrey);

			for (int i = 0; i < settings->probeMetadata.shank_count; i++)
			{
				Path shankPath = convertVectorToPath(settings->probeMetadata.shankOutline);
				shankPath.applyTransform(AffineTransform::translation(INTERSHANK_DISTANCE * i, 0.0f));
				g.strokePath(shankPath, PathStrokeType(1.0));
			}

			// draw zoomed channels
			float miniRowHeight = float(channelSpan) / float(settings->probeMetadata.rows_per_shank); // pixels per row

			float lowestRow = (zoomOffset - 17) / miniRowHeight;
			float highestRow = lowestRow + (zoomHeight / miniRowHeight);

			zoomAreaMinRow = int(lowestRow);

			if (settings->probeMetadata.columns_per_shank > 8)
			{
				electrodeHeight = jmin(lowerBound / (highestRow - lowestRow), 12.0f);
			}
			else
			{
				electrodeHeight = lowerBound / (highestRow - lowestRow);
			}

			for (int i = 0; i < settings->electrodeMetadata.size(); i++)
			{
				if (settings->electrodeMetadata[i].row_index >= int(lowestRow)
					&& settings->electrodeMetadata[i].row_index < int(highestRow))
				{
					float xLoc = leftEdgeOffset + shankOffset - electrodeHeight * settings->probeMetadata.columns_per_shank / 2
						+ electrodeHeight * settings->electrodeMetadata[i].column_index + settings->electrodeMetadata[i].shank * electrodeHeight * 4
						- (floor(settings->probeMetadata.shank_count / 2.0f) * electrodeHeight * 3);
					float yLoc = lowerBound - ((settings->electrodeMetadata[i].row_index - int(lowestRow)) * electrodeHeight);

					if (settings->electrodeMetadata[i].isSelected)
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
			int shankWidth = electrodeHeight * settings->probeMetadata.columns_per_shank;
			int totalWidth = shankWidth * settings->probeMetadata.shank_count + shankWidth * (settings->probeMetadata.shank_count - 1);

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

		int getZoomHeight()
		{
			return zoomHeight;
		}

		int getZoomOffset()
		{
			return zoomOffset;
		}

		void setZoomHeightAndOffset(int newHeight, int newOffset)
		{
			if (newHeight >= minZoomHeight && newHeight <= maxZoomHeight)
			{
				zoomHeight = newHeight;
				zoomOffset = newOffset;
			}
		}

		ActivityToView activityToView;
		float maxPeakToPeakAmplitude;

	protected:

		SettingsInterface* parent;

		int probeIndex;

		virtual ProbeSettings<ch, e>* getSettings() { return nullptr; }

	private:
		const int leftEdgeOffset = 220;

		std::map<Bank, Colour> disconnectedColors;

		// display variables
		int zoomHeight;
		int zoomOffset;

		bool isOverZoomRegion;
		bool isOverUpperBorder;
		bool isOverLowerBorder;
		bool isOverElectrode;
		bool isSelectionActive;

		int initialOffset;
		int initialHeight;
		int lowerBound;
		int dragZoneWidth;
		int zoomAreaMinRow;
		int minZoomHeight;
		int maxZoomHeight;
		int shankOffset;
		int leftEdge;
		int rightEdge;
		int channelLabelSkip;
		int pixelHeight;

		int lowestElectrode;
		int highestElectrode;

		float electrodeHeight;

		MouseCursor::StandardCursorType cursorType;

		std::string electrodeInfoString;

		Colour getElectrodeColour(int i)
		{
			auto mode = parent->getMode();

			auto settings = getSettings();
			if (settings == nullptr) return Colours::grey;

			if (settings->electrodeMetadata[i].status == ElectrodeStatus::DISCONNECTED) // not available
			{
				return disconnectedColors[settings->electrodeMetadata[i].bank];
			}
			else if (settings->electrodeMetadata[i].type == ElectrodeType::REFERENCE)
				return Colours::black;
			else
			{
				if (mode == SettingsInterface::VisualizationMode::ENABLE_VIEW) // ENABLED STATE
				{
					return Colours::yellow;
				}
				else if (mode == SettingsInterface::VisualizationMode::AP_GAIN_VIEW) // AP GAIN
				{
					return Colour(25 * settings->apGainIndex, 25 * settings->apGainIndex, 50);
				}
				else if (mode == SettingsInterface::VisualizationMode::LFP_GAIN_VIEW) // LFP GAIN
				{
					return Colour(66, 25 * settings->lfpGainIndex, 35 * settings->lfpGainIndex);
				}
				else if (mode == SettingsInterface::VisualizationMode::REFERENCE_VIEW)
				{
					auto ref = parent->getReferenceText();

					if (ref == "Ext")
						return Colours::darksalmon;
					else if (ref == "Tip")
						return Colours::orange;
					else
						return Colours::black;
				}
				else if (mode == SettingsInterface::VisualizationMode::ACTIVITY_VIEW)
				{
					if (settings->electrodeMetadata[i].status == ElectrodeStatus::CONNECTED)
					{
						return settings->electrodeMetadata[i].colour;
					}
					else
					{
						return Colours::grey;
					}
				}
			}
		}

		int getNearestElectrode(int x, int y)
		{
			auto settings = getSettings();
			if (settings == nullptr) return -1;

			int row = (lowerBound - y) / electrodeHeight + zoomAreaMinRow + 1;

			int shankWidth = electrodeHeight * settings->probeMetadata.columns_per_shank;
			int totalWidth = shankWidth * settings->probeMetadata.shank_count + shankWidth * (settings->probeMetadata.shank_count - 1);

			int shank = 0;
			int column = -1;

			for (shank = 0; shank < settings->probeMetadata.shank_count; shank++)
			{
				int leftEdge = leftEdgeOffset + shankOffset - totalWidth / 2 + shankWidth * 2 * shank;
				int rightEdge = leftEdge + shankWidth;

				if (x >= leftEdge && x <= rightEdge)
				{
					column = (x - leftEdge) / electrodeHeight;
					break;
				}
			}

			for (int i = 0; i < settings->electrodeMetadata.size(); i++)
			{
				if ((settings->electrodeMetadata[i].row_index == row)
					&& (settings->electrodeMetadata[i].column_index == column)
					&& (settings->electrodeMetadata[i].shank == shank))
				{
					return i;
				}
			}

			return -1;
		}

		Array<int> getElectrodesWithinBounds(int x, int y, int w, int h)
		{
			Array<int> inds;

			auto settings = getSettings();
			if (settings == nullptr) return inds;

			int startRow = (lowerBound - y - h) / electrodeHeight + zoomAreaMinRow + 1;
			int endRow = (lowerBound - y) / electrodeHeight + zoomAreaMinRow + 1;

			int shankWidth = electrodeHeight * settings->probeMetadata.columns_per_shank;
			int totalWidth = shankWidth * settings->probeMetadata.shank_count + shankWidth * (settings->probeMetadata.shank_count - 1);

			Array<int> selectedColumns;

			for (int i = 0; i < settings->probeMetadata.shank_count * settings->probeMetadata.columns_per_shank; i++)
			{
				int shank = i / settings->probeMetadata.columns_per_shank;
				int column = i % settings->probeMetadata.columns_per_shank;

				int l = leftEdge + shankWidth * 2 * shank + electrodeHeight * column;
				int r = l + electrodeHeight / 2;

				if (x < l + electrodeHeight / 2 && x + w > r)
					selectedColumns.add(i);
			}

			for (int i = 0; i < settings->electrodeMetadata.size(); i++)
			{
				if ((settings->electrodeMetadata[i].row_index >= startRow) && (settings->electrodeMetadata[i].row_index <= endRow))
				{
					int column_id = settings->electrodeMetadata[i].shank * settings->probeMetadata.columns_per_shank + settings->electrodeMetadata[i].column_index;

					if (selectedColumns.indexOf(column_id) > -1)
					{
						inds.add(i);
					}
				}
			}

			return inds;
		}

		std::string getElectrodeInfoString(int index)
		{
			auto settings = getSettings();
			if (settings == nullptr) return "";

			String a;
			a += "Electrode ";
			a += String(settings->electrodeMetadata[index].global_index + 1);

			a += "\nBank ";

			switch (settings->electrodeMetadata[index].bank)
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
			a += String(settings->electrodeMetadata[index].channel + 1);

			a += "\nY Position: ";
			a += String(settings->electrodeMetadata[index].ypos);

			a += "\nType: ";

			if (settings->electrodeMetadata[index].type == ElectrodeType::REFERENCE)
			{
				a += "REFERENCE";
			}
			else
			{
				a += "SIGNAL";

				a += "\nEnabled: ";

				if (settings->electrodeMetadata[index].status == ElectrodeStatus::CONNECTED)
					a += "YES";
				else
					a += "NO";
			}

			return a.toStdString();
		}

		Path convertVectorToPath(std::vector<std::array<float, 2>> vectorPath)
		{
			Path path;
			path.startNewSubPath(Point(vectorPath[0][0], vectorPath[0][1]));

			for (int i = 1; i < vectorPath.size(); i++)
			{
				path.lineTo(Point(vectorPath[i][0], vectorPath[i][1]));
			}

			return path;
		}

		JUCE_LEAK_DETECTOR(ProbeBrowser);
	};
}
