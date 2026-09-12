// Dovetail — media engine stub. Slice-1: type exists, behavior arrives with
// the first media slice (probe/decode) under TDD.
#pragma once

#include <QObject>

namespace dovetail {

class MediaEngine : public QObject {
    Q_OBJECT
public:
    explicit MediaEngine(QObject* parent = nullptr) : QObject(parent) {}
};

} // namespace dovetail