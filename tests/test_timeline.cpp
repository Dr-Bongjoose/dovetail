// Dovetail — timeline model tests. Slice-1: model-construction smoke test.
// Every command behavior gets its own failing test before its fix (TDD).
#include <QtTest>

#include "timeline_datamodel.h"

using namespace dovetail;

class TestTimeline : public QObject {
    Q_OBJECT

private slots:
    // --- construction (slice-1 smoke) ---
    void constructsDefaultTracks() {
        TimelineDataModel m;
        QCOMPARE(m.trackCount(), 6);
        // Premiere layout: A tracks 0-2, V tracks 3-5
        QCOMPARE(m.track(0).type, TrackType::Audio);
        QCOMPARE(m.track(0).name, QStringLiteral("A1"));
        QCOMPARE(m.track(3).type, TrackType::Video);
        QCOMPARE(m.track(3).name, QStringLiteral("V1"));
        QCOMPARE(m.track(5).name, QStringLiteral("V3"));
        QCOMPARE(m.sequenceLength(), qint64(0));
    }

    // --- ripple delete: undo must restore the track exactly ---
    void rippleDeleteUndoRestoresTrack() {
        TimelineDataModel m;
        auto push = [&](QUndoCommand* cmd) { m.undoStack()->push(cmd); };
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/a.mp4"), .start = 0, .length = 30}));
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/b.mp4"), .start = 30, .length = 30}));
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/c.mp4"), .start = 60, .length = 30}));
        QCOMPARE(m.clipsOnTrack(3).size(), 3);

        push(m.commandRippleDelete(3, 40)); // deletes B, C ripples 60 -> 30
        QCOMPARE(m.clipsOnTrack(3).size(), 2);
        QCOMPARE(m.sequenceLength(), qint64(60));

        m.undoStack()->undo();
        QCOMPARE(m.clipsOnTrack(3).size(), 3);       // RED: track was wiped (empty snapshot restore)
        QCOMPARE(m.sequenceLength(), qint64(90));
        QCOMPARE(m.clipById(3)->start, qint64(60));  // C back at 60
    }

    // --- razor: undo must restore the exact pre-split clip ---
    void razorUndoRestoresClip() {
        TimelineDataModel m;
        auto push = [&](QUndoCommand* cmd) { m.undoStack()->push(cmd); };
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/a.mp4"), .sourceIn = 100, .start = 0, .length = 90}));
        const qint64 origId = m.clipsOnTrack(3).at(0).id;

        push(m.commandRazor(3, 40));
        QCOMPARE(m.clipsOnTrack(3).size(), 2);
        QCOMPARE(m.clipsOnTrack(3).at(1).sourceIn, qint64(140)); // right continues source

        m.undoStack()->undo();
        QCOMPARE(m.clipsOnTrack(3).size(), 1);           // RED: mergeBack scan never finds left
        QCOMPARE(m.clipsOnTrack(3).at(0).id, origId);    // identity preserved
        QCOMPARE(m.clipsOnTrack(3).at(0).length, qint64(90));
        QCOMPARE(m.clipsOnTrack(3).at(0).sourceIn, qint64(100));

        m.undoStack()->redo();                            // undo/redo round-trip
        QCOMPARE(m.clipsOnTrack(3).size(), 2);
        m.undoStack()->undo();
        QCOMPARE(m.clipsOnTrack(3).size(), 1);
        QCOMPARE(m.clipsOnTrack(3).at(0).length, qint64(90));
    }

    // --- move: cross-track move + undo, invalid target is a no-op ---
    void moveClipCrossTrackAndBack() {
        TimelineDataModel m;
        auto push = [&](QUndoCommand* cmd) { m.undoStack()->push(cmd); };
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/a.mp4"), .start = 0, .length = 30}));
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/b.mp4"), .start = 30, .length = 30}));
        const qint64 id1 = m.clipsOnTrack(3).at(0).id;

        push(m.commandMoveClip(id1, 4, 10)); // V1 -> V2 at frame 10
        QCOMPARE(m.clipsOnTrack(3).size(), 1);
        QCOMPARE(m.clipsOnTrack(4).size(), 1);
        QCOMPARE(m.clipById(id1)->start, qint64(10));

        m.undoStack()->undo();
        QCOMPARE(m.clipsOnTrack(3).size(), 2);
        QCOMPARE(m.clipsOnTrack(4).size(), 0);
        QCOMPARE(m.clipById(id1)->start, qint64(0));

        // invalid target track: no crash, no change (RED: currently UB/crash)
        push(m.commandMoveClip(id1, 99, 5));
        QCOMPARE(m.trackCount(), 6);
        QCOMPARE(m.clipsOnTrack(3).size(), 2);
        QCOMPARE(m.clipById(id1)->start, qint64(0));
    }

    // --- roll: rejected edit mutates nothing; undo restores pre-roll state ---
    void rollEdgeValidateThenMutate() {
        TimelineDataModel m;
        auto push = [&](QUndoCommand* cmd) { m.undoStack()->push(cmd); };
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/a.mp4"), .sourceIn = 0, .start = 0, .length = 40}));
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/b.mp4"), .sourceIn = 30, .start = 40, .length = 50}));
        const qint64 idA = m.clipsOnTrack(3).at(0).id;
        const qint64 idB = m.clipsOnTrack(3).at(1).id;

        // rejected: rolling boundary 40 by +60 runs B past its own end
        push(m.commandRollEdge(3, 40, 60));
        QCOMPARE(m.clipById(idA)->length, qint64(40));  // RED: left was extended before right rejected
        QCOMPARE(m.clipById(idB)->start, qint64(40));
        QCOMPARE(m.clipById(idB)->length, qint64(50));
        QCOMPARE(m.clipById(idB)->sourceIn, qint64(30));

        // accepted roll, then undo
        push(m.commandRollEdge(3, 40, 10));
        QCOMPARE(m.clipById(idA)->length, qint64(50));
        QCOMPARE(m.clipById(idB)->start, qint64(50));
        QCOMPARE(m.clipById(idB)->sourceIn, qint64(40)); // source follows the edge

        m.undoStack()->undo();
        QCOMPARE(m.clipById(idA)->length, qint64(40));
        QCOMPARE(m.clipById(idB)->start, qint64(40));
        QCOMPARE(m.clipById(idB)->length, qint64(50));
        QCOMPARE(m.clipById(idB)->sourceIn, qint64(30));
    }

    // --- raw ops are total: out-of-range track / unknown clip are safe no-ops ---
    void rawOpsRejectInvalidInputs() {
        TimelineDataModel m;
        auto push = [&](QUndoCommand* cmd) { m.undoStack()->push(cmd); };
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/a.mp4"), .start = 0, .length = 30}));
        const qint64 idA = m.clipsOnTrack(3).at(0).id;

        QCOMPARE(m.razorRaw(99, 5), qint64(0));        // RED: at() throws
        QCOMPARE(m.rippleDeleteRaw(99, 5), qint64(0)); // RED: at() throws
        QCOMPARE(m.rollEdgeRaw(99, 5, 1), false);      // RED: operator[] UB
        QCOMPARE(m.clipAt(99, 5), nullptr);
        QCOMPARE(m.rangeFree(99, 0, 10), false);

        // valid inputs still work after the noise
        QCOMPARE(m.razorRaw(3, 15) != 0, true);
        QCOMPARE(m.clipsOnTrack(3).size(), 2);
        Q_UNUSED(idA);
    }

    // --- integration: a full edit sequence unwinds to the pristine state ---
    void editSequenceRoundTrips() {
        TimelineDataModel m;
        auto push = [&](QUndoCommand* cmd) { m.undoStack()->push(cmd); };
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/a.mp4"), .sourceIn = 200, .start = 0, .length = 90}));
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/b.mp4"), .sourceIn = 10, .start = 90, .length = 40}));
        const qint64 idA = m.clipsOnTrack(3).at(0).id;
        const Clip pristineA = m.clipsOnTrack(3).at(0);

        push(m.commandRazor(3, 40));
        push(m.commandRollEdge(3, 40, 10));
        push(m.commandMoveClip(m.clipsOnTrack(3).at(1).id, 4, 500));
        push(m.commandRippleDelete(3, 100));
        // B deleted; A2 now lives at [500,540) on track 4 -> sequence end 540
        QCOMPARE(m.sequenceLength(), qint64(540));

        for (int i = 0; i < 4; ++i) m.undoStack()->undo();
        QCOMPARE(m.clipsOnTrack(3).size(), 2);
        QCOMPARE(m.clipsOnTrack(4).size(), 0);
        const Clip* a = m.clipById(idA);
        QVERIFY(a != nullptr);
        QCOMPARE(a->start, pristineA.start);
        QCOMPARE(a->length, pristineA.length);
        QCOMPARE(a->sourceIn, pristineA.sourceIn);
        QCOMPARE(m.sequenceLength(), qint64(130));
    }
    void editsRejectOccupiedRanges() {
        TimelineDataModel m;
        auto push = [&](QUndoCommand* cmd) { m.undoStack()->push(cmd); };
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/a.mp4"), .start = 0, .length = 30}));
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/b.mp4"), .start = 30, .length = 30}));
        const qint64 idA = m.clipsOnTrack(3).at(0).id;
        const qint64 idB = m.clipsOnTrack(3).at(1).id;

        // add overlapping A/B: rejected
        push(m.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/c.mp4"), .start = 20, .length = 30}));
        QCOMPARE(m.clipsOnTrack(3).size(), 2);      // RED: silently stacked -> 3
        QCOMPARE(m.sequenceLength(), qint64(60));

        // move A onto occupied range (B at [30,60)): rejected, A unchanged
        push(m.commandMoveClip(idA, 3, 20)); // [20,50) overlaps B
        QCOMPARE(m.clipsOnTrack(3).size(), 2);
        QCOMPARE(m.clipById(idA)->start, qint64(0)); // RED: move corrupted the layout
        QCOMPARE(m.clipById(idA)->length, qint64(30));

        // move A to empty track at a free position: allowed
        push(m.commandMoveClip(idA, 4, 100));
        QCOMPARE(m.clipsOnTrack(3).size(), 1);
        QCOMPARE(m.clipById(idA)->start, qint64(100));
        QCOMPARE(m.clipById(idA)->length, qint64(30));

        // undo of a REJECTED add must not remove anything (inert command)
        TimelineDataModel m2;
        m2.undoStack()->push(m2.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/a.mp4"), .start = 0, .length = 30}));
        m2.undoStack()->push(m2.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/b.mp4"), .start = 20, .length = 30})); // rejected
        m2.undoStack()->push(m2.commandAddClip(3, Clip{.sourcePath = QStringLiteral("/z.mp4"), .start = 40, .length = 0}));  // degenerate
        m2.undoStack()->undo(); // undoes the inert (degenerate) command
        QCOMPARE(m2.clipsOnTrack(3).size(), 1);     // A must still be there
        m2.undoStack()->undo(); // undoes the inert (rejected) command
        QCOMPARE(m2.clipsOnTrack(3).size(), 1);     // A must STILL be there
        m2.undoStack()->undo(); // undoes the real add
        QCOMPARE(m2.clipsOnTrack(3).size(), 0);
    }
};

QTEST_MAIN(TestTimeline)
#include "test_timeline.moc"