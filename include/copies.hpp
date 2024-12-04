#pragma once

namespace Qounters::Copies {
    enum Copy {
        Position,
        Type,
        TextSource,
        Fill,
        Color,
        Gradient,
        Enable,
        CopyMax,
    };

    void Copy(enum Copy id);
    void Paste(enum Copy id);
    bool HasCopy(enum Copy id);
}
