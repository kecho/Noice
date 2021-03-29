#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "OpenEXR::IlmImf" for configuration "Release"
set_property(TARGET OpenEXR::IlmImf APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(OpenEXR::IlmImf PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/IlmImf-2_5.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/IlmImf-2_5.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS OpenEXR::IlmImf )
list(APPEND _IMPORT_CHECK_FILES_FOR_OpenEXR::IlmImf "${_IMPORT_PREFIX}/lib/IlmImf-2_5.lib" "${_IMPORT_PREFIX}/bin/IlmImf-2_5.dll" )

# Import target "OpenEXR::IlmImfUtil" for configuration "Release"
set_property(TARGET OpenEXR::IlmImfUtil APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(OpenEXR::IlmImfUtil PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/IlmImfUtil-2_5.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/IlmImfUtil-2_5.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS OpenEXR::IlmImfUtil )
list(APPEND _IMPORT_CHECK_FILES_FOR_OpenEXR::IlmImfUtil "${_IMPORT_PREFIX}/lib/IlmImfUtil-2_5.lib" "${_IMPORT_PREFIX}/bin/IlmImfUtil-2_5.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
