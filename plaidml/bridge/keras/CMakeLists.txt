pml_py_library(
  NAME py
  DEPS
    plaidml::py
  SRCS
    __init__.py
)

pml_py_test(
  NAME backend_test
  SRC backend_test.py
  DEPS
    ::py
  CHECKS
    keras
    test
)

pml_py_test(
  NAME trivial_model_test
  SRC trivial_model_test.py
  DEPS
    ::py
  CHECKS
    core
    keras
    smoke
    test
)

pml_py_wheel(
  NAME wheel
  PKG_NAME "plaidml_keras"
  VERSION ${PLAIDML_VERSION}
  PY_DEPS ::py
)

pml_py_library(
  NAME mnist_mlp
  SRCS
    mnist_mlp.py
  DEPS
    ::py
)
