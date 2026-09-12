/*
    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KIO_DNDACTIONSUGGEST_H
#define KIO_DNDACTIONSUGGEST_H

#include <QList>

#include "kiowidgets_export.h"

class QUrl;

namespace KIO
{

/*!
 * \brief What action a drop should perform, independent of Qt's drop-action enum.
 *
 * This is the decision a drop-target makes while hovering, before the drop
 * happens. It is derived from where the sources live relative to the
 * destination, so the same answer drives both the cursor glyph (sent over
 * wl_data_offer.set_actions) and the actual drop job.
 *
 * \since 6.24
 */
enum class DndActionGuess {
    Move, // every source is local and on the same block device as the destination
    Copy, // at least one source is on a different device, or not local
    Ask, // a safe fallback when we cannot tell (non-local destinations, etc.)
};

/*!
 * \brief Suggest the DnD action for dragging \a srcUrls into the folder \a destUrl.
 *
 * The logic mirrors what DropJob does on drop (KMountPoint-based same-device
 * detection) so that the hover glyph and the final behavior are guaranteed to
 * agree.
 *
 * This is a pure function: it performs no KConfig lookup, no KJob allocation and
 * does not cache across calls (callers that need it should cache the mount-point
 * table on the side, e.g. once per drag).
 *
 * \return Move only when every srcUrl is local and on the same mounted block
 *         device as destUrl. Otherwise Copy, or Ask if the destination cannot
 *         be resolved to a local mount point.
 *
 * \since 6.24
 */
KIOWIDGETS_EXPORT DndActionGuess suggestActionForDrop(const QList<QUrl> &srcUrls, const QUrl &destUrl);

}

#endif
