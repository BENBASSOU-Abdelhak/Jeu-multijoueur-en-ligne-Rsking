#All Boost::Test
# Create test from .cpp
function(create_test test_path)
	get_filename_component(name ${test_path} NAME_WLE)
	add_executable(${name} ${test_path})
	target_include_directories(${name} PUBLIC ${Boost_INCLUDE_DIRS})
	target_link_libraries(${name} PUBLIC risking_lib ${Boost_LIBRARIES})

	message("-- adding test: ${name}")
	add_test(${name} ${name})
endfunction()

# Glob all tests
aux_source_directory(${TES_DIR} TES_LIST)
list(REMOVE_ITEM TES_LIST "${SRC_DIR}/Boost.cmake")

# add all tests
foreach(tname ${TES_LIST})
	create_test(${tname})
endforeach()
