# Claude Code Instructions for Mogrix

## Python Development

- Use `uv` for package management (not pip directly)
  - Install packages: `uv pip install <package>`
  - Create venv: `uv venv`
  - Sync dependencies: `uv pip sync requirements.txt`

- Follow Test-Driven Development (TDD) methodology:
  1. Write a failing test first
  2. Write the minimum code to pass the test
  3. Refactor as needed
  4. Repeat

## Git Usage

- Use git to track changes
- Never commit everything at once - use specific, targeted commits
- Commit messages must be 5 words or fewer
- Never use emoji in commits or anywhere else
