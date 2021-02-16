#All Boost::Test

set(ALL_TEST_NAMES)

# Create test from .cpp
function(create_test test_path)
	get_filename_component(name ${test_path} NAME_WLE)
	add_executable(${name} ${test_path})
	target_include_directories(${name} PUBLIC ${INC_DIR} ${Boost_INCLUDE_DIRS})
	target_link_libraries(${name} PUBLIC risking_lib ${Boost_LIBRARIES})

	message("-- adding test: ${name}")
	add_test(${name} ${name})
	list(APPEND ALL_TEST_NAMES ${tname})
endfunction()

# Glob all tests
aux_source_directory(${TES_DIR} TES_LIST)
list(REMOVE_ITEM TES_LIST "${SRC_DIR}/Boost.cmake")

# coverage
if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND COVERAGE)
	append_coverage_compiler_flags("--coverage")
endif()

# add all tests
foreach(tname ${TES_LIST})
	create_test(${tname})
endforeach()

# coverage
if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND COVERAGE)
	#set(LCOV_ARGS "${LCOV_ARGS} --no-external")
	setup_target_for_coverage_gcovr_xml(
		NAME coverage
		EXECUTABLE ctest
		BASE_DIR ${PROJECT_SOURCE_DIR}
		EXCLUDE "tests/*"
		DEPENDENCIES risking_lib ${ALL_TEST_NAMES})
	setup_target_for_coverage_gcovr_html(
		NAME coverage_html
		EXECUTABLE ctest
		BASE_DIR ${PROJECT_SOURCE_DIR}
		EXCLUDE "tests/*"
		DEPENDENCIES risking_lib ${ALL_TEST_NAMES})
endif()
