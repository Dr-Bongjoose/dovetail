// Dovetail — QUndoCommand edits. Every timeline mutation is one of these.
// Declarations live here; definitions in commands.cpp.
#pragma once

#include <QUndoCommand>

#include "timeline_datamodel.h"

namespace dovetail {

class AddClipCommand : public QUndoCommand {
public:
    AddClipCommand(TimelineDataModel* m, int track, Clip c)
        : m_m(m), m_track(track), m_clip(c), m_insertedId(0) {
        setText(QStringLiteral("Add Clip"));
    }
    void redo() override;
    void undo() override;
private:
    TimelineDataModel* m_m;
    int m_track;
    Clip m_clip;
    qint64 m_insertedId;
};

class RazorCommand : public QUndoCommand {
public:
    RazorCommand(TimelineDataModel* m, int track, qint64 frame)
        : m_m(m), m_track(track), m_frame(frame), m_rightId(0) {
        setText(QStringLiteral("Razor"));
    }
    void redo() override;
    void undo() override;
private:
    void mergeBack();
    TimelineDataModel* m_m;
    int m_track;
    qint64 m_frame;
    qint64 m_rightId;
};

class RippleDeleteCommand : public QUndoCommand {
public:
    RippleDeleteCommand(TimelineDataModel* m, int track, qint64 frame)
        : m_m(m), m_track(track), m_frame(frame), m_removedId(0) {
        setText(QStringLiteral("Ripple Delete"));
    }
    void redo() override;
    void undo() override;
    // called by model before rippleDeleteRaw mutates (see model hook)
    void captureSnapshot(const QList<Clip>& snap) { m_snapshot = snap; }
private:
    TimelineDataModel* m_m;
    int m_track;
    qint64 m_frame;
    qint64 m_removedId;
    QList<Clip> m_snapshot;
};

class MoveClipCommand : public QUndoCommand {
public:
    MoveClipCommand(TimelineDataModel* m, qint64 clipId, int toTrack, qint64 newStart);
    void redo() override;
    void undo() override;
private:
    int trackOf(qint64 id) const;
    TimelineDataModel* m_m;
    qint64 m_id;
    int m_toTrack;
    qint64 m_newStart;
    int m_fromTrack = 0;
    qint64 m_oldStart = 0;
};

} // namespace dovetail