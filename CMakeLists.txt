cmake_minimum_required(VERSION 3.5.0)

if (NOT DEFINED GUI_BASE_DIR)
	if (DEFINED ENV{GUI_BASE_DIR})
		set(GUI_BASE_DIR $ENV{GUI_BASE_DIR})
	else()
		set(GUI_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../plugin-GUI)
	endif()
endif()

get_filename_component(PROJECT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
get_filename_component(PLUGIN_NAME ${PROJECT_FOLDER} NAME)

project(OE_PLUGIN_${PLUGIN_NAME})
set(CMAKE_SHARED_LIBRARY_PREFIX "")

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
	OEPLUGIN
	"$<$<PLATFORM_ID:Windows>:JUCE_API=__declspec(dllimport)>"
	$<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
	$<$<PLATFORM_ID:Linux>:JUCE_DISABLE_NATIVE_FILECHOOSERS=1>
	$<$<CONFIG:Debug>:DEBUG=1>
	$<$<CONFIG:Debug>:_DEBUG=1>
	$<$<CONFIG:Release>:NDEBUG=1>
	)

set(SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Source)
file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false "${SOURCE_PATH}/*.cpp" "${SOURCE_PATH}/*.h")
set(GUI_COMMONLIB_DIR ${GUI_BASE_DIR}/installed_libs)

set(CONFIGURATION_FOLDER $<$<CONFIG:Debug>:Debug>$<$<NOT:$<CONFIG:Debug>>:Release>)

list(APPEND CMAKE_PREFIX_PATH ${GUI_COMMONLIB_DIR} ${GUI_COMMONLIB_DIR}/${CONFIGURATION_FOLDER})

add_library(${PLUGIN_NAME} SHARED ${SRC_FILES})

target_compile_features(${PLUGIN_NAME} PUBLIC cxx_auto_type cxx_generalized_initializers)
target_include_directories(${PLUGIN_NAME} PUBLIC ${GUI_BASE_DIR}/JuceLibraryCode ${GUI_BASE_DIR}/JuceLibraryCode/modules ${GUI_BASE_DIR}/Plugins/Headers ${GUI_COMMONLIB_DIR}/include)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

set(GUI_BIN_DIR ${GUI_BASE_DIR}/Build/${CONFIGURATION_FOLDER})

if (NOT CMAKE_LIBRARY_ARCHITECTURE)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(CMAKE_LIBRARY_ARCHITECTURE "x64")
	else()
		set(CMAKE_LIBRARY_ARCHITECTURE "x86")
	endif()
endif()

#Libraries and compiler options
target_link_libraries(${PLUGIN_NAME} ${GUI_BIN_DIR}/open-ephys.lib)
target_compile_options(${PLUGIN_NAME} PRIVATE /sdl- /W0 /MP)

install(TARGETS ${PLUGIN_NAME} RUNTIME DESTINATION ${GUI_BIN_DIR}/plugins  CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})
install(FILES $<TARGET_PDB_FILE:${PLUGIN_NAME}> DESTINATION  ${GUI_BIN_DIR}/plugins OPTIONAL)

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../libs)

#create filters for vs and xcode

foreach( src_file IN ITEMS ${SRC_FILES})
	get_filename_component(src_path "${src_file}" PATH)
	file(RELATIVE_PATH src_path_rel "${SOURCE_PATH}" "${src_path}")
	string(REPLACE "/" "\\" group_name "${src_path_rel}")
	source_group("${group_name}" FILES "${src_file}")
endforeach()

function(vs_set_all_configurations target_name target_config_name)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config} config_upper)
        set_property(
            TARGET ${target_name}
            APPEND PROPERTY MAP_IMPORTED_CONFIG_${config_upper} "${target_config_name}"
        )
    endforeach()
endfunction()

# add liboni project

set(LIBONI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/liboni/api/liboni")
set(LIBONI_NAME liboni)
set(LIBONI_PNAME "submodule_${LIBONI_NAME}")

include_external_msproject (
    ${LIBONI_PNAME}
    "${LIBONI_DIR}/liboni.vcxproj"
    PLATFORM  ${CMAKE_LIBRARY_ARCHITECTURE}
)

vs_set_all_configurations(${LIBONI_PNAME} "ReleaseStatic")

add_library("${LIBONI_NAME}" STATIC IMPORTED)
set_target_properties("${LIBONI_NAME}" PROPERTIES
  IMPORTED_LOCATION "${LIBONI_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}/ReleaseStatic/liboni.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${LIBONI_DIR}")

add_dependencies(${LIBONI_NAME} ${LIBONI_PNAME})
target_link_libraries(${PLUGIN_NAME} "${LIBONI_NAME}")

# add riffa project

set(RIFFA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/liboni/drivers/riffa/windows/lib")
set(RIFFA_NAME riffa)
set(RIFFA_PNAME "submodule_${RIFFA_NAME}")
set(RELEASE_CONFIGURATION "Release")

include_external_msproject (
    ${RIFFA_PNAME}
    "${RIFFA_DIR}/riffa.vcxproj"
    PLATFORM  ${CMAKE_LIBRARY_ARCHITECTURE}
)

vs_set_all_configurations(${RIFFA_PNAME} "${RELEASE_CONFIGURATION}")

# add onidriver_riffa project

set(ONIDRIVER_RIFFA_DIR "${LIBONI_DIR}/drivers/riffa")
set(ONIDRIVER_RIFFA_NAME libonidriver_riffa)
set(ONIDRIVER_RIFFA_PNAME "submodule_${ONIDRIVER_RIFFA_NAME}")

include_external_msproject (
    ${ONIDRIVER_RIFFA_PNAME}
    "${ONIDRIVER_RIFFA_DIR}/onidriver_riffa.vcxproj"
    PLATFORM  ${CMAKE_LIBRARY_ARCHITECTURE}
)

vs_set_all_configurations(${ONIDRIVER_RIFFA_PNAME} "${RELEASE_CONFIGURATION}")

add_dependencies(${ONIDRIVER_RIFFA_PNAME} ${RIFFA_PNAME}) 

# Ensure needed DLLs are installed to the shared folder

install(FILES "${CMAKE_BINARY_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}/${RELEASE_CONFIGURATION}/riffa.dll" "${CMAKE_BINARY_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}/${RELEASE_CONFIGURATION}/${ONIDRIVER_RIFFA_NAME}.dll" DESTINATION ${GUI_BIN_DIR}/shared)
