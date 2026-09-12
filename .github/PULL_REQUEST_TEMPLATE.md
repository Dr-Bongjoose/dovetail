# Dovetail Pull Request

## What & why
<!-- One paragraph: what this PR does and why it exists. Link the issue if there is one. -->

## How it was tested
<!-- Concrete: which tests ran, what was verified by hand. "It compiles" is not testing. -->

## Premiere-parity notes
<!-- If this changes editor behavior: what does Premiere do, what do we do, and why (same / deliberately different)? -->

## Checklist
- [ ] Builds clean (`cmake --build build --target all`)
- [ ] Tests pass (`ctest --test-dir build --output-on-failure`)
- [ ] Model changes are undo/redo round-trip tested
- [ ] No floats in timeline model math (frame integers only)
- [ ] Docs updated (README / docs/) if behavior or architecture changed