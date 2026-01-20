.PHONY: venv install test lint format clean

VENV := .venv
PYTHON := $(VENV)/bin/python
PIP := $(VENV)/bin/pip
UV := ~/.local/bin/uv

venv:
	$(UV) venv

install: venv
	$(UV) pip install -e ".[dev]"
	$(UV) pip install ruff isort black

test:
	$(VENV)/bin/pytest -v

lint:
	$(VENV)/bin/ruff check mogrix/ tests/

format:
	$(VENV)/bin/isort mogrix/ tests/
	$(VENV)/bin/black mogrix/ tests/
	$(VENV)/bin/ruff check --fix mogrix/ tests/

clean:
	rm -rf $(VENV)
	rm -rf __pycache__ */__pycache__ */*/__pycache__
	rm -rf .pytest_cache
	rm -rf *.egg-info
	rm -rf build/ dist/
	find . -name "*.pyc" -delete
	find . -name "*.pyo" -delete
