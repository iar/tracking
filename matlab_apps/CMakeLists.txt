SET(MATLAB_UTIL_SRC
  ${PROJECT_SOURCE_DIR}/cxx/matlab_utils.cpp
  ${PROJECT_SOURCE_DIR}/cxx/generic_wrapper_matlab.cpp
  ${PROJECT_SOURCE_DIR}/cxx/generic_parameters_matlab.cpp
  ${PROJECT_SOURCE_DIR}/cxx/cell_matlab.cpp 
  ${PROJECT_SOURCE_DIR}/cxx/wrapper_i_matlab.cpp
  ${PROJECT_SOURCE_DIR}/cxx/wrapper_o_matlab.cpp
  )

SET(CMAKE_CXX_COMPILER "/usr/local/bin/mex") 
include_directories(${MATLAB_INCLUDE_DIR})

# message( ${PROJECT_BINARY_DIR})
# add_executable (tracking_cmd ${COMMON_TRACK_SRC} test.cpp ${HDF_SRC})
# target_link_libraries(tracking_cmd ${HDF5_LIBRARIES})
# set_target_properties(tracking_cmd PROPERTIES compile_flags "-D_BSD_SOURCE -D_POSIX_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE" 
#   link_flags "-D_BSD_SOURCE -D_POSIX_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE")


 
add_library(test SHARED mex_test.cpp ${COMMON_TRACK_SRC} ${MATLAB_UTIL_SRC})
target_link_libraries(test ${MATLAB_LIBRARIES})
set_target_properties(test PROPERTIES link_flags "-Wl,--version-script=/usr/local/matlab/extern/lib/glnxa64/mexFunction.map")
set_target_properties(test PROPERTIES compile_flags  "-DMATLAB_MEX_FILE")
set_target_properties(test PROPERTIES
    PREFIX ""
    SUFFIX  ".${MATLAB_MEXFILE_EXT}"
)





add_library(test3 SHARED mex_test3.cpp ${COMMON_TRACK_SRC} ${MATLAB_UTIL_SRC}
  ${UTIL_SRC} ${MATLAB_WRAPPER_SRC} wrapper_o_hdf.cpp)
target_link_libraries(test3 ${MATLAB_LIBRARIES})
target_link_libraries(test3 ${HDF5_LIBRARIES})
set_target_properties(test3 PROPERTIES link_flags "-Wl,--version-script=/usr/local/matlab/extern/lib/glnxa64/mexFunction.map")
set_target_properties(test3 PROPERTIES compile_flags  "-DMATLAB_MEX_FILE")
set_target_properties(test3 PROPERTIES
    PREFIX ""
    SUFFIX  ".${MATLAB_MEXFILE_EXT}"
)
message( ${HDF_INCLUDE_DIR})

add_library(test2 SHARED mex_test2.cpp ${COMMON_TRACK_SRC} ${MATLAB_UTIL_SRC})
target_link_libraries(test2 ${MATLAB_LIBRARIES})
set_target_properties(test2 PROPERTIES
    PREFIX ""
    SUFFIX  ".${MATLAB_MEXFILE_EXT}"
)

add_library(quick_test SHARED quick_mex_test.cpp ${COMMON_TRACK_SRC} ${MATLAB_UTIL_SRC})
target_link_libraries(quick_test ${MATLAB_LIBRARIES})
set_target_properties(quick_test PROPERTIES
    PREFIX ""
    SUFFIX  ".${MATLAB_MEXFILE_EXT}"
)

SET(APPS basic_static basic_dynamic nn_vectors nnn_vectors corr_test phi6)

foreach(app ${APPS})
  add_library(${app} SHARED apps/${app}.cpp ${COMMON_TRACK_SRC} ${MATLAB_UTIL_SRC})
  target_link_libraries(${app} ${MATLAB_LIBRARIES})
  set_target_properties(${app} PROPERTIES
    PREFIX ""
    SUFFIX  ".${MATLAB_MEXFILE_EXT}"
    )
endforeach(app)

include_directories(/home/tcaswell/other_source/gnuplot-cpp-read-only)

add_subdirectory(iden)
add_subdirectory(apps)