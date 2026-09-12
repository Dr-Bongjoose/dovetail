#include "timeline_datamodel.h"

#include "commands.h"

#include <algorithm>

namespace dovetail {

TimelineDataModel::TimelineDataModel(QObject* parent) : QObject(parent) {
    // Premiere-style default sequence: V1-V3, A1-A3 (A tracks below V).
    m_tracks.append(defaultTrack(TrackType::Audio, QStringLiteral("A1")));
    m_tracks.append(defaultTrack(TrackType::Audio, QStringLiteral("A2")));
    m_tracks.append(defaultTrack(TrackType::Audio, QStringLiteral("A3")));
    m_tracks.append(defaultTrack(TrackType::Video, QStringLiteral("V1")));
    m_tracks.append(defaultTrack(TrackType::Video, QStringLiteral("V2")));
    m_tracks.append(defaultTrack(TrackType::Video, QStringLiteral("V3")));
}

const Clip* TimelineDataModel::clipAt(int track, qint64 frame) const {
    const auto& clips = m_tracks.at(track).clips;
    for (const Clip& c : clips)
        if (frame >= c.start && frame < c.end())
            return &c;
    return nullptr;
}

const Clip* TimelineDataModel::clipById(qint64 id) const {
    for (const Track& t : m_tracks)
        for (const Clip& c : t.clips)
            if (c.id == id)
                return &c;
    return nullptr;
}

Clip* TimelineDataModel::findClip(qint64 id) {
    for (Track& t : m_tracks)
        for (Clip& c : t.clips)
            if (c.id == id)
                return &c;
    return nullptr;
}

bool TimelineDataModel::rangeFree(int track, qint64 start, qint64 end) const {
    for (const Clip& c : m_tracks.at(track).clips)
        if (clipOverlapsRange(c, start, end))
            return false;
    return true;
}

qint64 TimelineDataModel::sequenceLength() const {
    qint64 len = 0;
    for (const Track& t : m_tracks)
        for (const Clip& c : t.clips)
            len = qMax(len, c.end());
    return len;
}

qint64 TimelineDataModel::insertClipRaw(int track, const Clip& c) {
    Clip copy = c;
    if (copy.id == 0)
        copy.id = m_nextClipId++;
    auto& clips = m_tracks[track].clips;
    // keep sorted by start
    int idx = 0;
    while (idx < clips.size() && clips.at(idx).start < copy.start) ++idx;
    clips.insert(idx, copy);
    emit layoutChanged();
    return copy.id;
}

bool TimelineDataModel::removeClipRaw(int track, qint64 clipId) {
    auto& clips = m_tracks[track].clips;
    for (int i = 0; i < clips.size(); ++i) {
        if (clips.at(i).id == clipId) {
            clips.removeAt(i);
            emit layoutChanged();
            return true;
        }
    }
    return false;
}

void TimelineDataModel::restoreTrackRaw(int track, const QList<Clip>& clips) {
    m_tracks[track].clips = clips;
    emit layoutChanged();
}

qint64 TimelineDataModel::razorRaw(int track, qint64 frame) {
    Clip* c = const_cast<Clip*>(clipAt(track, frame));
    if (!c) return 0;
    if (frame <= c->start || frame >= c->end()) return 0; // not interior
    Clip right = *c;
    right.id = m_nextClipId++;
    qint64 cut = frame - c->start;
    right.sourceIn += cut;
    right.start = frame;
    right.length -= cut;
    c->length = cut;
    insertClipRaw(track, right);
    return right.id;
}

qint64 TimelineDataModel::rippleDeleteRaw(int track, qint64 frame) {
    const Clip* c = clipAt(track, frame);
    if (!c) return 0;
    qint64 id = c->id;
    qint64 start = c->start, len = c->length;
    auto& clips = m_tracks[track].clips;
    clips.removeIf([&](const Clip& x) { return x.id == id; });
    // ripple: shift everything right of the gap left by len
    for (Clip& x : clips)
        if (x.start >= start + len)
            x.start -= len;
    emit layoutChanged();
    return id;
}

bool TimelineDataModel::rollEdgeRaw(int track, qint64 frame, qint64 delta) {
    // Find boundary: end of left clip == start of right clip == frame
    Clip* left = nullptr;
    Clip* right = nullptr;
    for (Clip& c : m_tracks[track].clips) {
        if (c.end() == frame) left = &c;
        if (c.start == frame) right = &c;
    }
    if (!left && !right) return false;
    qint64 newFrame = frame + delta;
    if (left) {
        if (newFrame <= left->start) return false; // would invert clip
        left->length += delta;
    }
    if (right) {
        if (newFrame >= right->end()) return false;
        right->sourceIn += delta;
        right->start += delta;
        right->length -= delta;
    }
    emit layoutChanged();
    return true;
}

void TimelineDataModel::moveClipRaw(qint64 clipId, int toTrack, qint64 newStart) {
    if (toTrack < 0 || toTrack >= m_tracks.size()) return;  // invalid target: no-op
    // Find by value and copy BEFORE removing — no pointers into the list survive removal.
    int fromTrack = -1;
    Clip moved;
    for (int i = 0; i < m_tracks.size(); ++i) {
        auto& clips = m_tracks[i].clips;
        for (int j = 0; j < clips.size(); ++j) {
            if (clips.at(j).id == clipId) {
                fromTrack = i;
                moved = clips.at(j);
                clips.removeAt(j);
                break;
            }
        }
        if (fromTrack >= 0) break;
    }
    if (fromTrack < 0) return;  // unknown clip: no-op
    moved.start = newStart;
    auto& clips = m_tracks[toTrack].clips;
    int idx = 0;
    while (idx < clips.size() && clips.at(idx).start < moved.start) ++idx;
    clips.insert(idx, moved);
    emit layoutChanged();
}

// --- command factories (commands live in commands.cpp) ---

QUndoCommand* TimelineDataModel::commandAddClip(int track, const Clip& c) {
    return new AddClipCommand(this, track, c);
}
QUndoCommand* TimelineDataModel::commandRazor(int track, qint64 frame) {
    return new RazorCommand(this, track, frame);
}
QUndoCommand* TimelineDataModel::commandRippleDelete(int track, qint64 frame) {
    return new RippleDeleteCommand(this, track, frame);
}
QUndoCommand* TimelineDataModel::commandMoveClip(qint64 clipId, int toTrack, qint64 newStart) {
    return new MoveClipCommand(this, clipId, toTrack, newStart);
}

void TimelineDataModel::addTrack(TrackType type, const QString& name) {
    m_tracks.append(defaultTrack(type, name));
    emit layoutChanged();
}

} // namespace dovetail