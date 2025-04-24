# This file is inside the 'src' directory (src/CMakeLists.txt)

# Define the name of our final program executable and list its C++ source files
add_executable(DigiviceSim # You can change the executable name if you like
    main.cpp
    # Add the C++ file for our PC display implementation
    platform/pc/pc_display.cpp
    # We'll add other .cpp files here later (like pc_input.cpp)
)

# Tell the compiler where to look for header files (#include directives)
target_include_directories(DigiviceSim PRIVATE
    # Look inside the 'hal' folder within 'src'
    ${CMAKE_CURRENT_SOURCE_DIR}/hal
    # Look inside the 'platform/pc' folder within 'src'
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/pc
    # Look inside the top-level 'assets' folder
    ${CMAKE_SOURCE_DIR}/assets
)

# Link the necessary libraries to our executable
# SDL2::SDL2 is the special name CMake uses for the SDL2 library we found earlier
# Link SDL2main first, then the main SDL2 library that provides its functions
target_link_libraries(DigiviceSim PRIVATE SDL2::SDL2main SDL2::SDL2)
# We might add SDL2::SDL2_image here later if we need it