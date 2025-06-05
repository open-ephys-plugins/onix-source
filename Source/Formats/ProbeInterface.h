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

#include "../NeuropixelsComponents.h"

namespace OnixSourcePlugin
{
	class ProbeInterfaceJson
	{
	public:
		template <int numChannels, int numElectrodes>
		static bool writeProbeSettingsToJson(File& file, ProbeSettings<numChannels, numElectrodes>* settings)
		{
			DynamicObject output;

			output.setProperty(Identifier("specification"),
				var("probeinterface"));
			output.setProperty(Identifier("version"),
				var("0.2.23"));

			Array<var> contact_positions;
			Array<var> shank_ids;
			Array<var> device_channel_indices;
			Array<var> contact_plane_axes;
			Array<var> contact_shapes;
			Array<var> contact_shape_params;

			Array<var> ax1 = { 1.0f, 0.0f };
			Array<var> ax2 = { 0.0f, 1.0f };
			Array<var> contact_plane_axis = { ax1, ax2 };

			for (int elec = 0; elec < settings->electrodeMetadata.size(); elec++)
			{
				ElectrodeMetadata& em = settings->electrodeMetadata[elec];

				Array<var> contact_position;
				contact_position.add(em.xpos);
				contact_position.add(em.ypos);

				DynamicObject::Ptr contact_shape_param = new DynamicObject;
				contact_shape_param->setProperty(Identifier("width"), em.site_width);

				contact_positions.add(contact_position);
				shank_ids.add(String(em.shank));
				device_channel_indices.add(em.status == ElectrodeStatus::CONNECTED ? em.channel : -1);
				contact_plane_axes.add(contact_plane_axis);
				contact_shapes.add("square");
				contact_shape_params.add(contact_shape_param.get());
			}

			Array<var> probe_planar_contours;
			auto& probeContour = settings->probeMetadata.probeContour;

			for (int i = 0; i < probeContour.size(); i++)
			{
				probe_planar_contours.add(Array<var> { probeContour[i][0], probeContour[i][1] });
			}

			DynamicObject::Ptr probe = new DynamicObject();
			DynamicObject::Ptr annotations = new DynamicObject();
			annotations->setProperty(Identifier("manufacturer"), "imec");
			auto probeName = getProbeName(settings->probeType);
			annotations->setProperty(Identifier("name"), String(probeName));

			probe->setProperty(Identifier("ndim"), 2);
			probe->setProperty(Identifier("si_units"), "um");
			probe->setProperty(Identifier("annotations"), var(annotations));
			probe->setProperty(Identifier("contact_positions"), contact_positions);
			probe->setProperty(Identifier("contact_plane_axes"), contact_plane_axes);
			probe->setProperty(Identifier("contact_shapes"), contact_shapes);
			probe->setProperty(Identifier("contact_shape_params"), contact_shape_params);
			probe->setProperty(Identifier("probe_planar_contour"), probe_planar_contours);
			probe->setProperty(Identifier("device_channel_indices"), device_channel_indices);
			probe->setProperty(Identifier("shank_ids"), shank_ids);

			Array<var> probes;
			probes.add(probe.get());

			output.setProperty(Identifier("probes"), probes);

			if (file.exists())
				file.deleteFile();

			FileOutputStream f(file);

			JSON::FormatOptions options;

			options = options.withMaxDecimalPlaces(4);
			options = options.withSpacing(JSON::Spacing::multiLine);

			output.writeAsJSON(f, options);

			return true;
		}

		template <int numChannels, int numElectrodes>
		static bool readProbeSettingsFromJson(File& file, ProbeSettings<numChannels, numElectrodes>* settings)
		{
			auto json = JSON::parse(file);

			if (json == var())
				return false;

			DynamicObject::Ptr obj = json.getDynamicObject();

			if (obj->hasProperty(Identifier("specification")))
			{
				std::string specification = obj->getProperty(Identifier("specification")).toString().toStdString();

				if (specification.compare("probeinterface") != 0)
					return false;
			}
			else
				return false;

			if (obj->hasProperty(Identifier("probes")))
			{
				Array<var>* probes = obj->getProperty(Identifier("probes")).getArray();

				if (probes->size() != 1)
					return false;

				DynamicObject::Ptr probe = probes->getReference(0).getDynamicObject();

				if (probe->hasProperty(Identifier("ndim")))
				{
					if (!probe->getProperty(Identifier("ndim")).equalsWithSameType(2))
					{
						return false;
					}
				}
				else
					return false;

				if (probe->hasProperty(Identifier("si_units")))
				{
					if (!probe->getProperty(Identifier("si_units")).equalsWithSameType("um"))
					{
						return false;
					}
				}
				else
					return false;

				Array<var>* contact_positions = nullptr;

				if (probe->hasProperty(Identifier("contact_positions")))
				{
					contact_positions = probe->getProperty(Identifier("contact_positions")).getArray();

					if (contact_positions->size() != settings->electrodeMetadata.size())
						return false;
				}
				else
					return false;

				Array<var>* probe_planar_contour = nullptr;

				if (probe->hasProperty(Identifier("probe_planar_contour")))
				{
					probe_planar_contour = probe->getProperty(Identifier("probe_planar_contour")).getArray();
				}
				else
					return false;

				Array<var>* device_channel_indices = nullptr;

				if (probe->hasProperty(Identifier("device_channel_indices")))
				{
					device_channel_indices = probe->getProperty(Identifier("device_channel_indices")).getArray();

					if (device_channel_indices->size() != settings->electrodeMetadata.size())
						return false;
				}
				else
					return false;

				Array<var>* shank_ids = nullptr;

				if (probe->hasProperty(Identifier("shank_ids")))
				{
					shank_ids = probe->getProperty(Identifier("shank_ids")).getArray();

					if (shank_ids->size() != settings->electrodeMetadata.size())
						return false;
				}
				else
					return false;

				for (int ch = 0; ch < contact_positions->size(); ch++)
				{
					Array<var>* contact_position = contact_positions->getReference(ch).getArray();

					settings->electrodeMetadata[ch].xpos = int(contact_position->getReference(0));
					settings->electrodeMetadata[ch].ypos = int(contact_position->getReference(1));
				}

				settings->probeMetadata.probeContour.clear();

				for (int i = 0; i < probe_planar_contour->size(); i++)
				{
					Array<var>* point = probe_planar_contour->getReference(i).getArray();

					settings->probeMetadata.probeContour.emplace_back(std::array<float, 2>{float(point->getReference(0)), float(point->getReference(1))});
				}

				for (int ch = 0; ch < shank_ids->size(); ch++)
				{
					settings->electrodeMetadata[ch].shank = int(shank_ids->getReference(ch));
				}

				settings->clearElectrodeSelection();
				std::vector<int> selectedChannels{};

				for (int ch = 0; ch < device_channel_indices->size(); ch++)
				{
					if (int(device_channel_indices->getReference(ch)) >= 0)
					{
						selectedChannels.emplace_back(ch);
					}
				}

				settings->selectElectrodes(selectedChannels);
			}
			else
				return false;

			return true;
		}

		static std::string getProbeName(ProbeType type)
		{
			switch (type)
			{
			case OnixSourcePlugin::ProbeType::NONE:
				return "";
			case OnixSourcePlugin::ProbeType::NPX_V1:
				return "Neuropixels 1.0";
			case OnixSourcePlugin::ProbeType::NPX_V2:
				return "Neuropixels 2.0";
			default:
				return "";
			}
		}
	};
}
