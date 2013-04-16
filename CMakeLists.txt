cmake_minimum_required(VERSION 2.6)
 
project(LiverSegmentationMRI2)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
add_executable(LiverSegmentationMRI2 LiverSegmentationMRI2.cxx)
target_link_libraries(LiverSegmentationMRI2 ${ITK_LIBRARIES})
