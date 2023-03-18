# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

# Declares and names the project.

project("pacman")
message(STATUS "ANDROID_ABI val : ${ANDROID_ABI}")
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
# Sets lib_src_DIR to the path of the target CMake project.
set(lib_src_DIR ../cpp)
include_directories(${lib_src_DIR})


add_library( # Sets the name of the library.
        pacman

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        pacmanlib.cpp

        graphics/IRenderable.cpp
        graphics/ui/controls/Control.cpp
        graphics/ui/controls/RectButton.cpp
        graphics/ui/controls/Label.cpp
        graphics/ui/controls/CheckBox.cpp
        graphics/ui/menu/Menu.cpp
        graphics/ui/menu/MainMenu.cpp
        graphics/ui/menu/GameMenu.cpp
        graphics/ui/menu/SwipeGameMenu.cpp
        graphics/ui/menu/GameOverMenu.cpp
        graphics/ui/menu/PauseMenu.cpp
        graphics/ui/menu/WinMenu.cpp
        graphics/animation/Animation.cpp
        graphics/animation/Plume.cpp
        graphics/animation/Pulsation.cpp
        managers/Art.cpp
        managers/Audio.cpp
        managers/Statistics.cpp
        managers/Store.cpp
        managers/ShadersManager.cpp
        logic/Game.cpp
        logic/Engine.cpp
        logic/StateMachine.cpp
        logic/actors/Actor.cpp
        logic/actors/Pacman.cpp
        logic/actors/monsters/Monster.cpp
        logic/actors/monsters/StupidMonster.cpp
        logic/actors/monsters/CleverMonster.cpp
        logic/actors/bonuses/Bonus.cpp
        logic/actors/bonuses/LifeBonus.cpp
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        pacman

        #lib to link
        android

        # GL ES 2, used for the sample renderer
        GLESv2

        OpenSLES
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})