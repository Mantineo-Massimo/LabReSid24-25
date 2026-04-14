VENV = venv
PYTHON = $(VENV)/bin/python
PIP = $(VENV)/bin/pip

all: run report

venv:
	python3 -m venv $(VENV)
	$(PIP) install ray

run:
	$(PYTHON) mapreduce_lotr.py

report:
	mkdir -p LaTex
	# Assuming Report_HO18.tex will be created
	# pdflatex -output-directory=LaTex LaTex/Report_HO18.tex

clean:
	rm -rf $(VENV)
	rm -f lotr_report.txt
	rm -rf LaTex/*.aux LaTex/*.log LaTex/*.out LaTex/*.pdf

.PHONY: all venv run report clean
