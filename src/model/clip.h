// Dovetail — one clip instance on a timeline track.
// Times are integer frames in project timebase. Frame math only:
// no floats in the model, ever (Premiere-style frame accuracy).
#pragma once

#include <QString>
#include <QtGlobal>

namespace dovetail {

struct Clip {
    qint64 id = 0;          // unique per project, monotonically increasing
    QString sourcePath;     // absolute path to source media
    qint64 sourceIn = 0;    // source frame at clip head
    qint64 start = 0;       // timeline position, frames
    qint64 length = 0;      // duration on timeline, frames
    bool enabled = true;

    qint64 end() const { return start + length; }
};

// Half-open interval overlap: [start, end)
inline bool rangeOverlaps(qint64 aStart, qint64 aEnd, qint64 bStart, qint64 bEnd) {
    return aStart < bEnd && bStart < aEnd;
}

inline bool clipOverlapsRange(const Clip& c, qint64 start, qint64 end) {
    return rangeOverlaps(c.start, c.end(), start, end);
}

} // namespace dovetail