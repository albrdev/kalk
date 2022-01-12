cmake_minimum_required(VERSION 3.14)

target_sources(${TARGET_MAIN}
  PUBLIC
    ExpressionParserSetup.hpp

  PRIVATE
    ExpressionParserDefaultSetup.cpp
    ExpressionParserChemicalSetup.cpp
)
