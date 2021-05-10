#All Boost::Test

# Paramètres CTest
set(CTEST_OUTPUT_ON_FAILURE ON)

set(ALL_TEST_NAMES)

# Create test from .cpp
function(create_test test_path)
	get_filename_component(name ${test_path} NAME_WLE)
	add_executable(${name} ${test_path})
	target_include_directories(${name} PUBLIC ${INC_DIR} ${Boost_INCLUDE_DIRS})
	target_link_libraries(${name} PUBLIC risking_lib ${Boost_LIBRARIES})
	add_dependencies(${name} risking_lib)

	target_include_directories(${name} PUBLIC ${ODBC_INCLUDE_DIRS})
	target_link_libraries(${name} PUBLIC ODBC::ODBC)

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
		EXCLUDE "tests/*" "include/otlv4"
		DEPENDENCIES risking_lib ${ALL_TEST_NAMES})
	setup_target_for_coverage_gcovr_html(
		NAME coverage_html
		EXECUTABLE ctest
		BASE_DIR ${PROJECT_SOURCE_DIR}
		EXCLUDE "tests/*"  "include/otlv4"
		DEPENDENCIES risking_lib ${ALL_TEST_NAMES})
endif()

add_custom_target(copy_test_certs
	COMMAND cp "${TES_DIR}/*.cert" "${TES_DIR}/*.key" ./)
add_dependencies(network_listener copy_test_certs)

add_custom_target(copy_test_maps
	COMMAND cp -r "${TES_DIR}/testmaps/" ./)
add_dependencies(map_test copy_test_maps)
