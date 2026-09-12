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
};

QTEST_MAIN(TestTimeline)
#include "test_timeline.moc"