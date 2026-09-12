// Dovetail — timeline data model.
// Pure C++ project model: tracks of clips, edit operations, undo stack.
// No Qt Quick types here — the QML timeline is a VIEW over this model.
#pragma once

#include <QObject>
#include <QList>
#include <QUndoStack>
#include <QUuid>
#include <QtGlobal>

#include "clip.h"

namespace dovetail {

enum class TrackType { Video, Audio };

struct Track {
    TrackType type = TrackType::Video;
    QString name;
    bool locked = false;
    // Video: "eyeball" visibility. Audio: mute.
    bool visible = true;
    QList<Clip> clips;
};

class TimelineDataModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(int trackCount READ trackCount NOTIFY layoutChanged)
    Q_PROPERTY(qint64 sequenceLength READ sequenceLength NOTIFY layoutChanged)
public:
    explicit TimelineDataModel(QObject* parent = nullptr);

    // --- structure ---
    int trackCount() const { return int(m_tracks.size()); }
    const Track& track(int i) const { return m_tracks.at(i); }
    void addTrack(TrackType type, const QString& name);

    // --- queries (const, O(n) per track) ---
    qint64 sequenceLength() const;                 // last clip end across all tracks
    const Clip* clipAt(int track, qint64 frame) const;
    const Clip* clipById(qint64 id) const;
    bool rangeFree(int track, qint64 start, qint64 end) const;
    QList<Clip> clipsOnTrack(int track) const { return m_tracks.at(track).clips; }

    // --- edit operations. Each returns a QUndoCommand the caller pushes.
    // The model mutates only through these commands (Premiere-grade undo).
    QUndoCommand* commandAddClip(int track, const Clip& c);
    QUndoCommand* commandRazor(int track, qint64 frame);
    QUndoCommand* commandRippleDelete(int track, qint64 frame);
    QUndoCommand* commandMoveClip(qint64 clipId, int toTrack, qint64 newStart);
    QUndoCommand* commandRollEdge(int track, qint64 frame, qint64 delta);

    QUndoStack* undoStack() { return &m_undo; }

    // Direct mutation — ONLY for command internals and tests. Not an edit API.
    // Returns index of inserted clip on the track.
    int insertClipRaw(int track, const Clip& c);
    bool removeClipRaw(int track, qint64 clipId);
    // Replace the entire contents of a track with the given clips
    // (sorted-by-start snapshot). Command undo support.
    void restoreTrackRaw(int track, const QList<Clip>& clips);
    // Split the clip under (track, frame) into two. Returns new right clip id, 0 if none.
    qint64 razorRaw(int track, qint64 frame);
    // Delete clip at frame and shift everything to its right left by its length.
    qint64 rippleDeleteRaw(int track, qint64 frame);
    // Move clip boundary at frame by delta frames (roll edit: both neighbors follow).
    bool rollEdgeRaw(int track, qint64 frame, qint64 delta);
    void moveClipRaw(qint64 clipId, int toTrack, qint64 newStart);

    static Track defaultTrack(TrackType t, const QString& name) {
        Track tr; tr.type = t; tr.name = name; return tr;
    }

signals:
    void layoutChanged();   // clips added/removed/moved — full re-render
    void clipChanged(qint64 clipId);

private:
    Clip* findClip(qint64 id);
    QList<Track> m_tracks;
    QUndoStack m_undo;
    qint64 m_nextClipId = 1;
};

} // namespace dovetail