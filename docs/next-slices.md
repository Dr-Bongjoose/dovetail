# Next slices — picked and scoped (post-PR-#2)

Start order after PR #2 merges. Each is one PR, TDD, CI green.

## 1. Test media generator (`tools/gen_media.py`) — unblocks all of Phase 1
Generates deterministic tiny fixtures (2s color-bars 24fps H.264, tone WAV)
via ffmpeg; tests reference fixture *generation*, not committed binaries.
Acceptance: `ctest` gains a fixture-exists step; repo stays binary-free.

## 2. `MediaProbe` (slice 1.1)
`SourceMedia { path, fps (rational), durationFrames, width, height, codec }`
via ffprobe JSON. Model gets `sequenceFps` + frame-timebase conversions at
the edge. Acceptance: probe both fixtures; 29.97 drop-frame naming decided
and documented in `docs/timecode.md` (NDF first, DF as an issue).

## 3. Sequence/list-model adapter (first Phase 2 stone)
`SequenceListModel : QAbstractListModel` exposing track 3 (V1) clips to QML:
roles id/start/length/sourceIn/trackType. Acceptance: a QQmlEngine test
instantiates the adapter, mutates the model via commands, asserts the roles
update (QAbstractItemModelTester in CI).

## 4. TimelineCanvas static render
Ruler with timecode, playhead line, clip rectangles from the adapter.
No interaction yet — screenshot-verified against a golden render in CI
(offscreen). Acceptance: window renders, tests green.

## 5. First interaction: drag-to-move
Mouse down/move/up on a clip → `MoveClipCommand` (one command per gesture,
coalesced). Acceptance: Qt Quick drag test in QML; undo restores layout
exactly (model test already proves the command — this proves the wiring).

## Known tech-debt tickets to file as issues
- `rollEdgeRaw` still validates-after-first-mutation; command layer papers
  over it with snapshot-restore. Fold validation into the raw op when
  SlipCommand lands (same mutation pattern).
- `findClip`/`clipById` are O(total clips); fine until ~5k clips — add an
  id→(track,index) index when SequenceListModel lands.
- AddClipCommand rejection is silent; add an out-param/enum so the view can
  surface "target occupied" like Premiere's red overlap indicator.
- QTEST_MAIN pulls in GUI platform plugin; consider QTEST_GUILESS_MAIN once
  MediaProbe lands (no QML in those tests).