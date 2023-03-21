#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_CMAKE =
{
    "add_custom_command add_custom_target add_definitions add_dependencies add_executable add_library "
    "add_subdirectory add_test aux_source_directory build_command build_name cmake_minimum_required "
    "configure_file create_test_sourcelist else elseif enable_language enable_testing endforeach endif "
    "endmacro endwhile exec_program execute_process export_library_dependencies file find_file find_library "
    "find_package find_path find_program fltk_wrap_ui foreach get_cmake_property get_directory_property "
    "get_filename_component get_source_file_property get_target_property get_test_property if include "
    "include_directories include_external_msproject include_regular_expression install install_files "
    "install_programs install_targets link_directories link_libraries list load_cache load_command macro "
    "make_directory mark_as_advanced math message option output_required_files project qt_wrap_cpp qt_wrap_ui "
    "remove remove_definitions separate_arguments set set_directory_properties set_source_files_properties "
    "set_target_properties set_tests_properties site_name source_group string subdir_depends subdirs "
    "target_link_libraries try_compile try_run use_mangled_mesa utility_source variable_requires "
    "vtk_make_instantiator vtk_wrap_java vtk_wrap_python vtk_wrap_tcl while write_file",
    "ABSOLUTE ABSTRACT ADDITIONAL_MAKE_CLEAN_FILES ALL AND APPEND APPLE ARGS ASCII BEFORE BORLAND CACHE "
    "CACHE_VARIABLES CLEAR CMAKE_COMPILER_2005 COMMAND COMMANDS COMMAND_NAME COMMENT COMPARE COMPILE_FLAGS "
    "COPYONLY CYGWIN DEFINED DEFINE_SYMBOL DEPENDS DOC EQUAL ESCAPE_QUOTES EXCLUDE EXCLUDE_FROM_ALL EXISTS "
    "EXPORT_MACRO EXT EXTRA_INCLUDE FATAL_ERROR FILE FILES FORCE FUNCTION GENERATED GLOB GLOB_RECURSE GREATER "
    "GROUP_SIZE HEADER_FILE_ONLY HEADER_LOCATION IMMEDIATE INCLUDES INCLUDE_DIRECTORIES INCLUDE_INTERNALS "
    "INCLUDE_REGULAR_EXPRESSION LESS LINK_DIRECTORIES LINK_FLAGS LOCATION MACOSX_BUNDLE MACROS MAIN_DEPENDENCY "
    "MAKE_DIRECTORY MATCH MATCHALL MATCHES MINGW MODULE MSVC MSVC60 MSVC70 MSVC71 MSVC80 MSVC_IDE MSYS NAME "
    "NAME_WE NOT NOTEQUAL NO_SYSTEM_PATH OBJECT_DEPENDS OFF ON OPTIONAL OR OUTPUT OUTPUT_VARIABLE PATH PATHS "
    "POST_BUILD POST_INSTALL_SCRIPT PREFIX PREORDER PRE_BUILD PRE_INSTALL_SCRIPT PRE_LINK PROGRAM PROGRAM_ARGS "
    "PROPERTIES QUIET RANGE READ REGEX REGULAR_EXPRESSION REPLACE REQUIRED RETURN_VALUE RUNTIME_DIRECTORY "
    "SEND_ERROR SHARED SOURCES STATIC STATUS STREQUAL STRGREATER STRLESS SUFFIX TARGET TOLOWER TOUPPER VAR "
    "VARIABLES VERSION WATCOM WIN32 WRAP_EXCLUDE WRITE",
    NULL,
};


EDITLEXER lexCmake =
{
    SCLEX_CMAKE, "cmake", IDS_LEX_CMAKE, L"Cmake Script", L"cmake; ctest; \\^cmakelists\\.txt$", L"",
    &KeyWords_CMAKE, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_CMAKE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_CMAKE_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {MULTI_STYLE(SCE_CMAKE_STRINGDQ,SCE_CMAKE_STRINGLQ,SCE_CMAKE_STRINGRQ,0)}, IDS_LEX_STR_String, L"String", L"fore:#7F007F; back:#EEEEEE", L"" },
        { {SCE_CMAKE_COMMANDS}, IDS_LEX_STR_63277, L"Function", L"fore:#00007F", L"" },
        { {SCE_CMAKE_PARAMETERS}, IDS_LEX_STR_Param, L"Parameter", L"fore:#7F200F", L"" },
        { {SCE_CMAKE_VARIABLE}, IDS_LEX_STR_Var, L"Variable", L"fore:#CC3300", L"" },
        { {SCE_CMAKE_WHILEDEF}, IDS_LEX_STR_63358, L"While Def", L"fore:#00007F", L"" },
        { {SCE_CMAKE_FOREACHDEF}, IDS_LEX_STR_63357, L"For Each Def", L"fore:#00007F", L"" },
        { {SCE_CMAKE_IFDEFINEDEF}, IDS_LEX_STR_63279, L"If Def", L"fore:#00007F", L"" },
        { {SCE_CMAKE_MACRODEF}, IDS_LEX_STR_63280, L"Macro Def", L"fore:#00007F", L"" },
        { {SCE_CMAKE_STRINGVAR}, IDS_LEX_STR_63267, L"Variable within String", L"fore:#CC3300; back:#EEEEEE", L"" },
        { {SCE_CMAKE_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#008080", L"" },
        //{ {SCE_CMAKE_USERDEFINED}, IDS_LEX_STR_63106, L"User Defined", L"fore:#800020", L"" },
        EDITLEXER_SENTINEL
    }
};
