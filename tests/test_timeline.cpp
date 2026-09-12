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
};

QTEST_MAIN(TestTimeline)
#include "test_timeline.moc"