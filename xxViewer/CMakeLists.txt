cmake_minimum_required(VERSION 3.0)
project(trailsViewer)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall") # Turn on warnings

set(SOURCE_CORE
	main.cpp
	)

set(SOURCE_UTILS
	Falcom.cpp
	utils/ArrayView.cpp
	utils/File.cpp
	utils/StringView.cpp
	)

add_executable(trailsTester main-test.cpp Falcom.cpp ${SOURCE_UTILS})

add_executable(trailsExtract main-extract-archive.cpp Falcom.cpp ${SOURCE_UTILS})

add_executable(trailsCompress main-compress-archive.cpp Falcom.cpp ${SOURCE_UTILS})

add_executable(trailsViewer ${SOURCE_CORE} ${SOURCE_UTILS})


# Set-up libraries

target_link_libraries(trailsViewer -lopengl32 -lglfw3 -lglew32 -lpng)

# Enable C++14 features
set_property(TARGET trailsViewer PROPERTY CXX_STANDARD 17)
set_property(TARGET trailsViewer PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET trailsExtract PROPERTY CXX_STANDARD 17)
set_property(TARGET trailsExtract PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET trailsCompress PROPERTY CXX_STANDARD 17)
set_property(TARGET trailsCompress PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET trailsTester PROPERTY CXX_STANDARD 17)
set_property(TARGET trailsTester PROPERTY CXX_STANDARD_REQUIRED ON)
