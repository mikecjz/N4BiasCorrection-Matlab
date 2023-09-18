cmake_minimum_required(VERSION 3.16.2)



project(N4BiasCorrection)

set(SimpleITK_DIR "/scratch1/jc_350/SimpleITK/SimpleITK-build/SimpleITK-build")
set(ITK_DIR "/scratch1/jc_350/SimpleITK/SimpleITK-build/ITK-build")

# Set compiler flags for debugging
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

find_package(Matlab)
find_package(SimpleITK)
find_package(ITK)

#message(STATUS "SimpleITK_LIBRARIES: ${SimpleITK_LIBRARIES}")

matlab_add_mex (
		NAME N4BiasCorrection 
		SRC N4BiasCorrection.cpp
		LINK_TO ${SimpleITK_LIBRARIES}
		)

# target_link_libraries ( N4BiasCorrection ${SimpleITK_LIBRARIES} )
