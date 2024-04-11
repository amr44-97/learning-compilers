#pragma once

struct Color {
  static constexpr const char *Black = "\x1b[30m";
  static constexpr const char *Red = "\x1b[31m";
  static constexpr const char *Green = "\x1b[32m";
  static constexpr const char *Yellow = "\x1b[33m";
  static constexpr const char *Blue = "\x1b[34m";
  static constexpr const char *Magenta = "\x1b[35m";
  static constexpr const char *Cyan = "\x1b[36m";
  static constexpr const char *White = "\x1b[37m";
  static constexpr const char *Bright_black = "\x1b[90m";
  static constexpr const char *Bright_red = "\x1b[91m";
  static constexpr const char *Bright_green = "\x1b[92m";
  static constexpr const char *Bright_yellow = "\x1b[93m";
  static constexpr const char *Bright_blue = "\x1b[94m";
  static constexpr const char *Bright_magenta = "\x1b[95m";
  static constexpr const char *Bright_cyan = "\x1b[96m";
  static constexpr const char *Bright_white = "\x1b[97m";
  static constexpr const char *Bold = "\x1b[1m";
  static constexpr const char *Dim = "\x1b[2m";
  static constexpr const char *Reset = "\x1b[0m";
};
