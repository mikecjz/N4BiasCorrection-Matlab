cmake_minimum_required(VERSION 3.16.3)



project(N4BiasCorrection)

set(SimpleITK_DIR "/home/jc_350/Toolboxes/SimpleITK/SimpleITK-build/SimpleITK-build")
set(ITK_DIR "/home/jc_350/Toolboxes/SimpleITK/SimpleITK-build/ITK-build")

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
