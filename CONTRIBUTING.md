# Dovetail contributing

## The one rule
`main` is protected. Work happens on branches, lands via PR with review + green
tests. Agents do not push to `main` directly, ever.

## Branches
| Branch | Owner | Purpose |
|---|---|---|
| `main` | nobody (protected) | reviewed, tested code only |
| `agent/linux-jooselab` | this agent (Linux desktop) | primary development branch |
| `agent/macbook` | MacBook agent | its ideas, its experiments |

Feature branches off an agent branch use `feat/`, `fix/`, `docs/`, `ci/` prefixes.

## Commits
Conventional Commits: `type(scope): summary` — types: feat, fix, refactor, docs,
test, ci, chore, perf. Wrap at 72 chars.

## PRs
Use the PR template. Every PR states how it was tested — "it compiles" is not
testing. Model changes must include undo/redo round-trip tests.

## The model discipline (non-negotiable)
- Timeline model is pure C++ data + QUndoCommand edits. QML is a view, nothing more.
- Integer frame math only in the model. No floats, ever.
- Every timeline mutation goes through a command. Direct `*Raw` mutation is for
  command internals and tests only.