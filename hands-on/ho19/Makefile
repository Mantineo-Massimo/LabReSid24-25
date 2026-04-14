VENV = venv
PYTHON = $(VENV)/bin/python
PIP = $(VENV)/bin/pip

all: run report

venv:
	python3 -m venv $(VENV)
	$(PIP) install ray torch torchvision --index-url https://download.pytorch.org/whl/cpu

run:
	$(PYTHON) federated_learning.py

report:
	mkdir -p LaTex
	# Compilation of Report_HO19.tex will be done after its creation

clean:
	rm -rf $(VENV)
	rm -rf data
	rm -rf LaTex/*.aux LaTex/*.log LaTex/*.out LaTex/*.pdf

.PHONY: all venv run report clean
