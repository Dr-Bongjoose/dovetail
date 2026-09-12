// Dovetail — QUndoCommand edits. Definitions for the commands declared in commands.h.
#include "commands.h"

namespace dovetail {

void AddClipCommand::redo() {
    Clip c = m_clip;
    c.id = m_insertedId;             // 0 on first run -> model assigns; >0 on redo -> kept
    m_insertedId = m_m->insertClipRaw(m_track, c);
}

void AddClipCommand::undo() {
    m_m->removeClipRaw(m_track, m_insertedId);
}

void RazorCommand::redo() {
    if (!m_rightId) {
        // First run: snapshot the left clip before razorRaw mutates it.
        if (const Clip* c = m_m->clipAt(m_track, m_frame)) {
            m_left = *c;
            m_leftId = c->id;
        }
    }
    m_rightId = m_m->razorRaw(m_track, m_frame);
    if (m_rightId)
        m_right = *m_m->clipById(m_rightId);
}

void RazorCommand::undo() {
    if (!m_rightId) return;
    m_m->removeClipRaw(m_track, m_rightId);
    m_m->removeClipRaw(m_track, m_leftId);
    m_m->insertClipRaw(m_track, m_left);  // id preserved (non-zero): identity restored
}

void RippleDeleteCommand::redo() {
    if (!m_removedId)  // first run only: snapshot pre-delete state for undo
        m_snapshot = m_m->clipsOnTrack(m_track);
    m_removedId = m_m->rippleDeleteRaw(m_track, m_frame);
}

void RippleDeleteCommand::undo() {
    if (!m_removedId) return;
    m_m->restoreTrackRaw(m_track, m_snapshot);
}

MoveClipCommand::MoveClipCommand(TimelineDataModel* m, qint64 clipId, int toTrack, qint64 newStart)
    : m_m(m), m_id(clipId), m_toTrack(toTrack), m_newStart(newStart) {
    setText(QStringLiteral("Move Clip"));
    if (const Clip* c = m_m->clipById(clipId)) {
        m_fromTrack = trackOf(clipId);
        m_oldStart = c->start;
    }
}

void MoveClipCommand::redo() { m_m->moveClipRaw(m_id, m_toTrack, m_newStart); }
void MoveClipCommand::undo() { m_m->moveClipRaw(m_id, m_fromTrack, m_oldStart); }

int MoveClipCommand::trackOf(qint64 id) const {
    // find owning track
    for (int i = 0; i < m_m->trackCount(); ++i)
        for (const Clip& c : m_m->clipsOnTrack(i))
            if (c.id == id) return i;
    return 0;
}

} // namespace dovetail