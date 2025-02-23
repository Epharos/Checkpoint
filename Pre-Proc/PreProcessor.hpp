#pragma once

#define IN_EDITOR // This is a preprocessor definition that can be used to check if the code is being compiled in the editor or in the game

#define NO_COPY(T) T(const T&) = delete; T& operator=(const T&) = delete; // This is a preprocessor definition that can be used to delete the copy constructor and the copy assignment operator of a class
#define NO_MOVE(T) T(T&&) noexcept = delete; T& operator=(T&&) noexcept = delete; // This is a preprocessor definition that can be used to delete the move constructor and the move assignment operator of a class
#define ALLOW_MOVE(T) T(T&&) noexcept = default; T& operator=(T&&) noexcept = default; // This is a preprocessor definition that can be used to allow the move constructor and the move assignment operator of a class