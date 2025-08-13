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

#include "../OnixDevice.h"

namespace OnixSourcePlugin
{
class OnixSourceEditor;
class OnixSourceCanvas;

/**

    A base class for the graphical interface for updating data source settings

*/
class SettingsInterface : public Component
{
public:
    /** Settings interface type*/
    enum class Type
    {
        PROBE_SETTINGS_INTERFACE,
        NEUROPIXELS1E_SETTINGS_INTERFACE,
        NEUROPIXELS1F_SETTINGS_INTERFACE,
        NEUROPIXELS2E_SETTINGS_INTERFACE,
        BNO055_SETTINGS_INTERFACE,
        POLLEDBNO055_SETTINGS_INTERFACE,
        MEMORYMONITOR_SETTINGS_INTERFACE,
        OUTPUTCLOCK_SETTINGS_INTERFACE,
        HEARTBEAT_SETTINGS_INTERFACE,
        HARPSYNCINPUT_SETTINGS_INTERFACE,
        ANALOGIO_SETTINGS_INTERFACE,
        DIGITALIO_SETTINGS_INTERFACE,
        UNKNOWN_SETTINGS_INTERFACE
    };

    enum class VisualizationMode
    {
        ENABLE_VIEW,
        AP_GAIN_VIEW,
        LFP_GAIN_VIEW,
        REFERENCE_VIEW,
    };

    static constexpr int Width = 1000;
    static constexpr int Height = 600;

    /** Constructor */
    SettingsInterface (std::shared_ptr<OnixDevice> device_, OnixSourceEditor* editor_, OnixSourceCanvas* canvas_)
    {
        device = device_;
        editor = editor_;
        canvas = canvas_;

        int width = Width;
        int height = Height;

        setBounds (0, 0, width, height);
    }

    /** Called when acquisition begins */
    virtual void startAcquisition() { setInterfaceEnabledState (false); }

    /** Called when acquisition ends */
    virtual void stopAcquisition() { setInterfaceEnabledState (true); }

    /** Saves settings */
    virtual void saveParameters (XmlElement* xml) = 0;

    /** Loads settings */
    virtual void loadParameters (XmlElement* xml) = 0;

    /** Updates the string with info about the underlying data source*/
    virtual void updateInfoString() = 0;

    /** Updates the UI elements based on the current device settings */
    virtual void updateSettings() = 0;

    std::shared_ptr<OnixDevice> getDevice() { return device; }
    VisualizationMode getMode() const { return mode; }
    Type getType() const { return type; }

    virtual std::string getReferenceText() { return ""; }

    /** Enables or disables all UI elements that should not be changed during acquisition */
    virtual void setInterfaceEnabledState (bool newState) = 0;

protected:
    /** Pointer to the data source*/
    std::shared_ptr<OnixDevice> device = nullptr;

    Type type = Type::UNKNOWN_SETTINGS_INTERFACE;

    OnixSourceEditor* editor;
    OnixSourceCanvas* canvas;

    VisualizationMode mode = VisualizationMode::ENABLE_VIEW;

    const std::string enabledButtonText = "DISABLE";
    const std::string disabledButtonText = "ENABLE";

    static void writeToXmlFile (XmlElement* rootElement, File file)
    {
        if (! rootElement->writeToFile (file, String()))
        {
            Onix1::showWarningMessageBoxAsync (
                "Failed to Write XML File",
                "Unable to write the file " + file.getFullPathName().toStdString());
        }
    }

    static XmlElement* readFromXmlFile (File file)
    {
        XmlDocument xmlDocument (file);

        auto rootElement = xmlDocument.getDocumentElement();

        if (rootElement == nullptr)
        {
            Onix1::showWarningMessageBoxAsync (
                "Failed to Read XML File",
                "Unable to read the file " + file.getFullPathName().toStdString());
            return nullptr;
        }

        return rootElement.release();
    }

    static constexpr int NeuropixelsCalibrationFileRecursiveLevels = 2;

    // Given a File object defining the path to a directory, a filename to search for, and (optionally) the number of recursive levels to
    // iterate through, return a list of all files that match the given filename in the directory (or directories for recursive searches)
    static std::vector<File> searchDirectoryForFile (File directory, std::string filename, int recursiveLevels = 0)
    {
        std::vector<File> matchingFiles;

        if (! directory.isDirectory())
            return matchingFiles;

        if (recursiveLevels > 0)
        {
            for (DirectoryEntry entry : RangedDirectoryIterator (directory, false, "*", File::findDirectories, File::FollowSymlinks::no))
            {
                auto files = searchDirectoryForFile (entry.getFile(), filename, recursiveLevels - 1);

                if (files.size() > 0)
                    matchingFiles.insert (matchingFiles.end(), files.begin(), files.end());
            }
        }

        for (DirectoryEntry entry : RangedDirectoryIterator (directory, false, filename, File::findFiles, File::FollowSymlinks::no))
        {
            matchingFiles.emplace_back (entry.getFile());
            LOGC ("Discovered file: ", entry.getFile().getFullPathName());
        }

        return matchingFiles;
    }
};
} // namespace OnixSourcePlugin
