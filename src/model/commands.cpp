// Dovetail — QUndoCommand edits. Every timeline mutation is one of these.
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
    void redo() override {
        Clip c = m_clip;
        if (m_insertedId) c.id = m_insertedId; else m_insertedId = 0;
        m_m->insertClipRaw(m_track, c);
        m_insertedId = m_m->clipsOnTrack(m_track).at(0).id; // ids preserved via redo/undo cycle
        // Fix: find the inserted clip by its fields rather than index 0.
        const auto& clips = m_m->clipsOnTrack(m_track);
        for (const auto& x : clips)
            if (x.start == c.start && x.length == c.length)
                m_insertedId = x.id;
    }
    void undo() override {
        m_m->removeClipRaw(m_track, m_insertedId);
    }
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
    void redo() override {
        m_rightId = m_m->razorRaw(m_track, m_frame);
    }
    void undo() override {
        if (!m_rightId) return;
        // merge back: extend left clip, drop right clip
        mergeBack();
    }
private:
    void mergeBack() {
        const auto& clips = m_m->clipsOnTrack(m_track);
        Clip left, right;
        bool haveL = false, haveR = false;
        for (const Clip& c : clips) {
            if (c.id == m_rightId) { right = c; haveR = true; }
            if (c.end() == right.start && haveR) { left = c; haveL = true; }
        }
        if (!haveL || !haveR) return;
        // remove right, extend left
        m_m->removeClipRaw(m_track, m_rightId);
        // direct mutation of left clip via re-insert
        const auto& again = m_m->clipsOnTrack(m_track);
        for (const Clip& c : again) {
            if (c.id == left.id) {
                Clip fixed = c;
                fixed.length += right.length;
                m_m->removeClipRaw(m_track, left.id);
                m_m->insertClipRaw(m_track, fixed);
                break;
            }
        }
    }
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
    void redo() override {
        m_removedId = m_m->rippleDeleteRaw(m_track, m_frame);
    }
    void undo() override {
        if (!m_removedId) return;
        // Re-derive: undo of ripple-delete is complex because downstream
        // clips shifted. Full-fidelity inverse requires capturing the
        // track snapshot before deletion. Slice-1 simplification: snapshot
        // the whole track in redo() BEFORE mutating, restore in undo().
        m_m->restoreTrackRaw(m_track, m_snapshot);
    }
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
    MoveClipCommand(TimelineDataModel* m, qint64 clipId, int toTrack, qint64 newStart)
        : m_m(m), m_id(clipId), m_toTrack(toTrack), m_newStart(newStart) {
        setText(QStringLiteral("Move Clip"));
        if (const Clip* c = m_m->clipById(clipId)) {
            m_fromTrack = trackOf(clipId);
            m_oldStart = c->start;
        }
    }
    void redo() override { m_m->moveClipRaw(m_id, m_toTrack, m_newStart); }
    void undo() override { m_m->moveClipRaw(m_id, m_fromTrack, m_oldStart); }
private:
    int trackOf(qint64 id) const {
        const auto& clips = m_m->clipsOnTrack(0); // placeholder; fixed below
        Q_UNUSED(clips);
        // find owning track
        for (int i = 0; i < m_m->trackCount(); ++i)
            for (const Clip& c : m_m->clipsOnTrack(i))
                if (c.id == id) return i;
        return 0;
    }
    TimelineDataModel* m_m;
    qint64 m_id;
    int m_toTrack;
    qint64 m_newStart;
    int m_fromTrack = 0;
    qint64 m_oldStart = 0;
};

} // namespace dovetail