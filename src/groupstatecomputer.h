#ifndef GROUPSTATECOMPUTER_H
#define GROUPSTATECOMPUTER_H

#include "groupstate.h"

#include <QtCore/QScopedPointer>

namespace KGeoMap
{

class GroupStateComputer
{
public:

    GroupStateComputer();
    virtual ~GroupStateComputer();

    GroupState getState() const;

    void clear();

    void addState(const GroupState state);
    void addSelectedState(const GroupState state);
    void addFilteredPositiveState(const GroupState state);
    void addRegionSelectedState(const GroupState state);

private:

    class Private;
    const QScopedPointer<Private> d;
};

} /* namespace KGeoMap */

#endif // GROUPSTATECOMPUTER_H
