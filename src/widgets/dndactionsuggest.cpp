/*
    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dndactionsuggest.h"

#include <KFileItem>
#include <KMountPoint>

#include <QUrl>

#include <algorithm>

namespace KIO
{

DndActionGuess suggestActionForDrop(const QList<QUrl> &srcUrls, const QUrl &destUrl)
{
    if (srcUrls.isEmpty()) {
        return DndActionGuess::Ask;
    }

    // Every source is already directly inside destUrl (e.g. dropping back into
    // the folder it's already in, or hovering the empty space of the window
    // that's already showing that folder). This is a same-location no-op
    // regardless of scheme, so it doesn't need a mount-point lookup and must
    // be checked before the "not local" bail-out below, or remote (e.g. sftp)
    // destinations would always fall through to Copy even for this case.
    // Mirrors DropJob's own equalDestination check so the hover glyph and the
    // actual (no-op) drop decision agree.
    const bool equalDestination = std::all_of(srcUrls.cbegin(), srcUrls.cend(), [&destUrl](const QUrl &url) {
        return destUrl.matches(url.adjusted(QUrl::RemoveFilename), QUrl::StripTrailingSlash);
    });
    if (equalDestination) {
        return DndActionGuess::Move;
    }

    // A destination we can't resolve to a local mount point (remote, trash,
    // non-URL) is never a same-device move; fall back conservatively.
    if (!destUrl.isLocalFile()) {
        return DndActionGuess::Ask;
    }

    const KMountPoint::List mountPoints = KMountPoint::currentMountPoints();

    const KMountPoint::Ptr destMountPoint = mountPoints.findByPath(destUrl.path());
    const QString destDevice = destMountPoint ? destMountPoint->mountedFrom() : QString();
    if (destDevice.isEmpty()) {
        // Local destination with no matching mount entry; be conservative.
        return DndActionGuess::Ask;
    }

    for (const QUrl &url : srcUrls) {
        if (!url.isLocalFile()) {
            return DndActionGuess::Copy;
        }

        const KMountPoint::Ptr sourceMountPoint = mountPoints.findByPath(url.path());
        const QString sourceDevice = sourceMountPoint ? sourceMountPoint->mountedFrom() : QString();
        if (sourceDevice.isEmpty()) {
            // Local file we can't resolve a mount for; be conservative.
            return DndActionGuess::Ask;
        }

        // A symlink crossing to a different device is still a "move" in the
        // user's mental model; treat it as same-device like DropJob does.
        if (sourceDevice != destDevice && !KFileItem(url).isLink()) {
            return DndActionGuess::Copy;
        }
    }

    return DndActionGuess::Move;
}

}
