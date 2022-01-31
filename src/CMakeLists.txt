cmake_minimum_required(VERSION 3.14)

target_sources(${TARGET_KALK}
  PUBLIC
    KalkSetup.hpp

  PRIVATE
    ExpressionParserDefaultSetup.cpp
    ExpressionParserChemicalSetup.cpp
    CommandParserSetup.cpp
)
