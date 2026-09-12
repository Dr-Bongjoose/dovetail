# Dovetail — a Premiere-native video editor for Linux

> We cut the cord. We keep the muscle memory.

Dovetail is a professional non-linear video editor for Linux, built to make
an Adobe Premiere veteran feel at home within the first ten minutes —
without Adobe, without Windows, without a subscription.

## Why

The user (our founder and only critic that matters) has years of Premiere
muscle memory. Every open-source editor on Linux asks him to unlearn it.
He refuses Windows and refuses Adobe's pricing. Dovetail is both a tool
and a statement: **the workflow belongs to the editor, not the vendor.**

## Principles

1. **Premiere muscle memory is the default**, not a preference buried in
   settings. JKL shuttle, I/O marks, V/A track targeting, razor at
   playhead, ripple/roll/slip/slide, Z-to-fit, `\` zoom-to-sequence.
2. **Model/view separation.** The timeline is a pure C++ data model
   (`TimelineDataModel`) + `QUndoCommand` edits; QML is only a view.
   This makes the model testable headlessly and reusable (batch rendering,
   scripting, future server-side render farm).
3. **Undo is sacred.** Every edit is a command from day one.
4. **Proxy workflow is core**, not an afterthought: background proxy
   generation so 4K H.265 scrubs on mid-range hardware, full-res conform
   at export.
5. **Native Linux citizen.** Qt 6 + FFmpeg. Wayland first. No Electron.

## Branch strategy

| Branch | Owner | Purpose |
|---|---|---|
| `master` | nobody (protected) | reviewed, tested code only |
| `agent/linux-jooselab` | this agent (Linux desktop) | primary development branch |
| `agent/macbook` | MacBook agent | its ideas, its experiments |

Work lands on `master` through PRs with review + green tests, like a real
company. Agents do not push to `master` directly, ever.

See `BRANCH-STRATEGY.md` for the full protocol.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

Requires: Qt 6 (base, declarative, multimedia, tools), FFmpeg, CMake ≥ 3.24.

## Roadmap (slice 1)

- [x] Timeline data model: tracks, clips, razor, ripple, roll
- [x] Undo/redo command stack
- [ ] QML timeline view with JKL scrubbing + playhead
- [ ] Premiere default keymap (swappable JSON keymap profiles)
- [ ] MediaEngine: FFmpeg decode → QVideoFrame, audio meters
- [ ] Proxy generation pipeline
- [ ] Project file (JSON + media relink)
- [ ] Export: FFmpeg encode with Premiere-style presets

## License

GPL-3.0-or-later. Freedom is the feature.