# Makefile para compilar Rocket y generar documentación

# Variables
PYTHON      = python3
SETUP       = setup.py
DOXYFILE    = Doxyfile
DOCS_DIR    = docs

# Objetivo por defecto
all: build docs

# Compilar la librería con setuptools/pybind11
build:
	@echo "Compilando librería Rocket..."
	$(PYTHON) $(SETUP) build_ext --inplace
	@echo "¡Compilación completada!"
	mv Rocket.cpython-311-x86_64-linux-gnu.so lib

# Generar documentación con Doxygen
docs:
	@echo "Generando documentación con Doxygen..."
	@doxygen $(DOXYFILE)
	@echo "Documentación generada en $(DOCS_DIR)/html"

# Limpiar archivos de compilación y librería
clean:
	@echo "Limpiando archivos compilados y documentación..."
	rm -rf build *.so *.pyd *.o $(DOCS_DIR)
	@echo "¡Limpieza completada!"

# Ejecutar todo desde cero
rebuild: clean all

.PHONY: all build docs clean rebuild
