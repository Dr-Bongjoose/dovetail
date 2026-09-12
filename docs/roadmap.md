# Dovetail Roadmap

> The spec is the user's Premiere muscle memory. Every phase ships slices that
> remove one more reason to open Adobe. Model work is TDD-enforced; view work
> is demoed against real muscle-memory tasks ("razor a clip and ripple in
> under 3 seconds without thinking").

## Phase 0 — Model correctness ✅ (PR #2)
Buildable project, undo-true timeline model, 10-test regression net, CI.
Every defect class from `model-audit.md` closed plus duplicate-id and
inert-command semantics. **This is the floor everything else stands on.**

## Phase 1 — Media reality (model gets teeth)
| Slice | Deliverable | Test anchor |
|---|---|---|
| 1.1 | `MediaProbe`: ffprobe-backed source metadata — fps, duration, resolution, codec, audio layout | generated tiny fixtures (script-checked in, not binaries) |
| 1.2 | `MediaEngine`: frame-accurate seek+decode → QImage for (source, frame); proxy-generation decision point | decode frame N of fixture equals expected hash/fingerprint |
| 1.3 | Playback clock: sequence-fps timer, A/V sync, JKL shuttle (play/pause/reverse, 1x/2x/4x), spacebar | clock model unit tests + manual latency budget check |

## Phase 2 — The view (muscle memory starts)
| Slice | Deliverable | Premiere anchor |
|---|---|---|
| 2.1 | TimelineCanvas: tracks V3→A1 visual order, ruler + timecode, playhead, zoom (−/+, `\` fit) | Z-to-fit, `\` zoom-to-sequence |
| 2.2 | Clip interactions: drag-move, edge-trim (roll), razor tool `C` / razor-at-playhead, all wired through model commands | one-command drag = one undo entry |
| 2.3 | Source + Program monitors, I/O marks, insert `,` / overwrite `.` — three-point editing | the core Premiere loop |
| 2.4 | Selections (V/A tools), marquee, ripple/roll/slip/slide toolset (B/N) | tool palette parity |

Architecture rule: QML never touches `TimelineDataModel` directly. A
`QAbstractListModel` adapter per track is the only binding surface; commands
are invoked from QML via a thin controller. Keeps the model headless-testable
and ready for a future render farm.

## Phase 3 — Editor ergonomics
- Keymap file with `premiere-defaults` as the shipped default (charter principle 1)
- Track targeting (V1/A1 source patching) — Premiere's heart, arrives with insert/overwrite
- Snapping (clips/playhead/markers), markers, sequence settings, project save/load (JSON first)

## Phase 4 — Pipeline maturity
- Proxy workflow end-to-end (charter principle 4): auto-generate on import,
  auto-attach, full-res conform at export
- Audio: waveforms on clips, audio scrub, mixer tracks
- Color: Rec.709 pipeline first; GPU preview via Qt RHI when profiling demands

## Phase 5 — The statement ships
- Flatpak distribution; VAAPI/NVENC hardware encode; export presets (YouTube,
  ProRes master); FCPXML interchange — the anti-lock-in answer to Adobe

## Working agreements
- One PR per slice, TDD on all model code, CI green before review.
- Agents claim slices through issues (`agent/linux-jooselab` vs `agent/macbook`)
  to avoid collision; the shared test suite is the contract between agents.
- No floats in the model, ever. Times are integer frames in the sequence
  timebase; source times in the media's own timebase, converted at the edges.