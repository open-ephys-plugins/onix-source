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

# add liboni project

set(LIBONI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/liboni/api/liboni")
set(LIBONI_NAME liboni)

add_custom_command(
  TARGET "${PLUGIN_NAME}"
  PRE_BUILD
  COMMAND ${CMAKE_MAKE_PROGRAM} "${LIBONI_DIR}/liboni.vcxproj" /p:Configuration=ReleaseStatic /p:Platform=${CMAKE_LIBRARY_ARCHITECTURE}
  WORKING_DIRECTORY "${LIBONI_DIR}"
  COMMENT "Building liboni"
  VERBATIM
)

add_library("${LIBONI_NAME}" STATIC IMPORTED)
set_target_properties("${LIBONI_NAME}" PROPERTIES
  IMPORTED_LOCATION "${LIBONI_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}/ReleaseStatic/liboni.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${LIBONI_DIR}")

target_link_libraries(${PLUGIN_NAME} "${LIBONI_NAME}")

# add riffa project

set(RIFFA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/liboni/drivers/riffa/windows/lib")
set(RIFFA_NAME riffa)

add_custom_command(
  TARGET "${PLUGIN_NAME}"
  PRE_BUILD
  COMMAND ${CMAKE_MAKE_PROGRAM} "${RIFFA_DIR}/riffa.vcxproj" /p:Configuration=Release /p:Platform=${CMAKE_LIBRARY_ARCHITECTURE}
  WORKING_DIRECTORY "${RIFFA_DIR}"
  COMMENT "Building riffa"
  VERBATIM
)

add_library("${RIFFA_NAME}" STATIC IMPORTED)
set_target_properties("${RIFFA_NAME}" PROPERTIES
  IMPORTED_LOCATION "${RIFFA_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}/Release/riffa.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${RIFFA_DIR}")

target_link_libraries(${PLUGIN_NAME} "${RIFFA_NAME}")

# add onidriver_riffa project

set(ONIDRIVER_RIFFA_DIR "${LIBONI_DIR}/drivers/riffa")
set(ONIDRIVER_RIFFA_NAME libonidriver_riffa)

add_custom_command(
  TARGET "${PLUGIN_NAME}"
  PRE_BUILD
  COMMAND ${CMAKE_MAKE_PROGRAM} "${ONIDRIVER_RIFFA_DIR}/onidriver_riffa.vcxproj" /p:Configuration=Release /p:Platform=${CMAKE_LIBRARY_ARCHITECTURE}
  WORKING_DIRECTORY "${ONIDRIVER_RIFFA_DIR}"
  COMMENT "Building onidriver_riffa"
  VERBATIM
)

add_library("${ONIDRIVER_RIFFA_NAME}" STATIC IMPORTED)
set_target_properties("${ONIDRIVER_RIFFA_NAME}" PROPERTIES
  IMPORTED_LOCATION "${ONIDRIVER_RIFFA_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}/Release/onidriver_riffa.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${ONIDRIVER_RIFFA_DIR}")

target_link_libraries(${PLUGIN_NAME} "${ONIDRIVER_RIFFA_NAME}")

# # Ensure needed DLLs are installed to the shared folder

install(FILES "${RIFFA_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}/Release/riffa.dll" "${ONIDRIVER_RIFFA_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}/Release/${ONIDRIVER_RIFFA_NAME}.dll" DESTINATION ${GUI_BIN_DIR}/shared)
