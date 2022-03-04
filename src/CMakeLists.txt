cmake_minimum_required(VERSION 3.14)

target_sources(${TARGET_KALK}
  PUBLIC
  Setup.hpp

  PRIVATE
  ExpressionParserDefaultSetup.cpp
  ExpressionParserChemicalSetup.cpp
  CommandParserSetup.cpp
)
