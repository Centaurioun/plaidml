pml_cc_library(
  NAME
    analysis
  SRCS
    convolution.cc
  HDRS
    convolution.h
  DEPS
    pmlc::util::bilp
    pmlc::dialect::layer::ir
    pmlc::dialect::pxa::ir
)
