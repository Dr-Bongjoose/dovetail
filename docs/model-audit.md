# Dovetail — Timeline Model Audit (agent/linux-jooselab, 2026-09-11)

## Summary
The seed timeline model (src/model/) has the right shape — integer frame math,
QUndoCommand-per-edit, half-open intervals — but it does not compile against its
own CMakeLists, and three commands have correctness bugs that would corrupt the
timeline on undo. All findings below verified against the seed sources.

## Blocking: files referenced by CMakeLists.txt do not exist
CMakeLists.txt lists: src/main.cpp, src/media/media_engine.cpp, src/qml/Main.qml,
tests/test_timeline.cpp. None exist in the seed. Project cannot configure/build.

## Bug 1 — restoreTrackRaw() is called but never declared/defined
- commands.cpp:96 calls m_m->restoreTrackRaw(m_track, m_snapshot)
- timeline_datamodel.h declares no such member. RippleDeleteCommand cannot compile.

## Bug 2 — AddClipCommand::redo() id recovery is unreliable (and useless work)
- commands.cpp:16-25: after insertClipRaw, it scans the track for a clip matching
  (start, length) to recover the id. Two clips with identical start+length on the
  same track (legal after a razor with zero-length segments elsewhere, or duplicate
  paste) match ambiguously; wrong id gets stored and undo() deletes the wrong clip.
- Root cause: insertClipRaw() does not return the id of what it inserted, and the
  Clip passed by value means the command never learns the assigned id.
- Fix direction: have insertClipRaw assign id only if c.id == 0 AND report the id
  back (out-param or return struct). Command stores the id it was given, not a scan.

## Bug 3 — RazorCommand::undo() mergeBack() is lossy and order-dependent
- commands.cpp:52-74: it re-finds "left" by scanning for c.end() == right.start
  AFTER removal ordering assumptions; if the left clip's end was modified between
  redo and undo (e.g., user rolls the edge, then undoes razor), the scan finds no
  match or the wrong clip, and the razor becomes un-undoable.
- Fix direction: capture BOTH clip snapshots (left + right, full Clip structs) at
  redo time, before mutating. undo() restores both — no scanning, no inference.

## Bug 4 — MoveClipCommand::trackOf() is a placeholder that can return wrong track
- commands.cpp:121-128: iterates all tracks and returns the first track containing
  the clip — correct only if called before any move. But the constructor calls it
  AFTER the command may have been constructed on a moved clip; also, redo() of a
  redone (second) move uses m_fromTrack captured from constructor time.
  Multi-undo/redo sequences can teleport clips to stale tracks.
- Fix direction: capture fromTrack + oldStart in the constructor BEFORE any redo
  runs (it currently does), but never re-derive in redo; store m_fromTrack once.

## Bug 5 — rollEdgeRaw() silently leaves the model inconsistent on partial failure
- timeline_datamodel.cpp:108-130: if left succeeds but right's newFrame >= end()
  check fails, left has ALREADY been extended (line 121) — model is now corrupted:
  left got longer, right didn't shrink, they overlap.
- Fix direction: validate both sides first, mutate only if both checks pass.

## Bug 6 — rippleDeleteRaw() shifts clips but not gaps correctly
- timeline_datamodel.cpp:93-106: it shifts clips whose start >= start+len. But a
  clip that OVERLAPS the deleted clip's range (shouldn't exist on same track given
  rangeFree, but raw mutators are also called from tests) would be shifted by the
  full length even if it started mid-way inside the deleted clip's span, causing
  negative-gap overlaps. Minor, but the fix is to shift by (deleted.end() - x.start)
  for overlapping clips rather than the blanket len.
- Fix direction: for each clip with start >= deleted.start: shift = min(len,
  max(0, deleted.end() - x.start)).

## Bug 7 — RazorCommand::mergeBack() reconstructs left clip with wrong sourceIn
- commands.cpp:63-73: the merge-back does fixed.length += right.length but does not
  restore left.sourceIn — it was never saved. After undo of a razor, the left clip's
  sourceIn is correct only because razorRaw never touched it — but if a future edit
  changes sourceIn between redo and undo, the merge-back bakes in a stale value.
- Fix direction: same as Bug 3 — snapshot both clips, restore verbatim.

## Non-blocking gaps (v0 roadmap, not bugs)
- No track targeting (V1/A1 source patching) — Premiere's heart.
- No snapping, no playhead, no selection set, no clipboard.
- sequenceLength() is O(n) full scan; fine for v0, index later.
- Clip has no speed/ramp, no effects, no opacity/keyframes — by design for v0.
- QML view: Main.qml does not exist yet; the "view" half of the project is 0%.

## Recommended fix order for the first PR
1. Create missing files (main.cpp, Main.qml, media_engine stub, test_timeline.cpp)
2. Fix Bug 1 (declare restoreTrackRaw OR rework RippleDeleteCommand to snapshot)
3. Fix Bug 2 (id plumbing through insertClipRaw)
4. Fix Bugs 3+7 together (snapshot-based razor undo)
5. Fix Bug 4 (MoveClipCommand track capture)
6. Fix Bug 5 (roll validation-before-mutation)
7. Fix Bug 6 (ripple shift accounting)
8. Add tests covering every command's undo/redo round-trip
