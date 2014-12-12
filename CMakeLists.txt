cmake_minimum_required(VERSION 2.8)

SET(COMPILE_DEFINITIONS -Werror)

include_directories(/opt/vc/include)
include_directories(/opt/vc/include/interface/vcos/pthreads)
include_directories(/opt/vc/include/interface/vmcs_host)
include_directories(/opt/vc/include/interface/vmcs_host/linux)
include_directories(/opt/vc/include/interface/mmal)
include_directories(/opt/vc/userland-master)
include_directories(/opt/vc/src/hello_pi/libs/vgfont)

link_directories(/opt/vc/lib)
link_directories(/opt/vc/src/hello_pi/libs/vgfont)

find_package( OpenCV REQUIRED )

set (TEST_SOURCES
   	m_components.c)

set (MOVE_SOURCES
   	m_options.c 
   	m_components.c
   	m_file.c
   	m_move.c)

set (MMAL_LIBS 
	mmal_core 
	mmal_util 
	mmal_vc_client)

add_executable(test_preview  ${TEST_SOURCES}   test_preview.c)
add_executable(test_preview_no_tunneling  ${TEST_SOURCES} test_preview_no_tunneling.c)
add_executable(test_encode   ${TEST_SOURCES}   test_encode.c)
add_executable(test_encode_no_tunneling   ${TEST_SOURCES}   test_encode_no_tunneling.c)
add_executable(test_movement_detection   ${MOVE_SOURCES}   test_movement_detection.c)

target_link_libraries(test_preview ${MMAL_LIBS} vcos bcm_host)
target_link_libraries(test_preview_no_tunneling ${MMAL_LIBS} vcos bcm_host)
target_link_libraries(test_encode ${MMAL_LIBS} vcos bcm_host)
target_link_libraries(test_encode_no_tunneling ${MMAL_LIBS} vcos bcm_host)
target_link_libraries(test_movement_detection ${MMAL_LIBS} vcos bcm_host)
