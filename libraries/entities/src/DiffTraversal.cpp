//
//  DiffTraversal.cpp
//  assignment-client/src/entities
//
//  Created by Andrew Meadows 2017.08.08
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "DiffTraversal.h"

#include <OctreeUtils.h>

#include "EntityPriorityQueue.h"

DiffTraversal::Waypoint::Waypoint(EntityTreeElementPointer& element) : _nextIndex(0) {
    assert(element);
    _weakElement = element;
}

void DiffTraversal::Waypoint::getNextVisibleElementFirstTime(DiffTraversal::VisibleElement& next,
        const DiffTraversal::View& view) {
    // NOTE: no need to set next.intersection in the "FirstTime" context
    if (_nextIndex == -1) {
        // root case is special:
        // its intersection is always INTERSECT,
        // we never bother checking for LOD culling, and
        // we can skip it if the content hasn't changed
        ++_nextIndex;
        next.element = _weakElement.lock();
        return;
    } else if (_nextIndex < NUMBER_OF_CHILDREN) {
        EntityTreeElementPointer element = _weakElement.lock();
        if (element) {
            while (_nextIndex < NUMBER_OF_CHILDREN) {
                EntityTreeElementPointer nextElement = element->getChildAtIndex(_nextIndex);
                ++_nextIndex;
                if (nextElement && view.shouldTraverseElement(*nextElement)) {
                    next.element = nextElement;
                    return;
                }
            }
        }
    }
    next.element.reset();
}

void DiffTraversal::Waypoint::getNextVisibleElementRepeat(
        DiffTraversal::VisibleElement& next, const DiffTraversal::View& view, uint64_t lastTime) {
    if (_nextIndex == -1) {
        // root case is special
        ++_nextIndex;
        EntityTreeElementPointer element = _weakElement.lock();
        if (element->getLastChangedContent() > lastTime) {
            next.element = element;
            return;
        }
    }
    if (_nextIndex < NUMBER_OF_CHILDREN) {
        EntityTreeElementPointer element = _weakElement.lock();
        if (element) {
            while (_nextIndex < NUMBER_OF_CHILDREN) {
                EntityTreeElementPointer nextElement = element->getChildAtIndex(_nextIndex);
                ++_nextIndex;
                if (nextElement &&
                    nextElement->getLastChanged() > lastTime &&
                    view.shouldTraverseElement(*nextElement)) {

                    next.element = nextElement;
                    return;
                }
            }
        }
    }
    next.element.reset();
}

void DiffTraversal::Waypoint::getNextVisibleElementDifferential(DiffTraversal::VisibleElement& next,
        const DiffTraversal::View& view, const DiffTraversal::View& lastView) {
    if (_nextIndex == -1) {
        // root case is special
        ++_nextIndex;
        EntityTreeElementPointer element = _weakElement.lock();
        next.element = element;
        return;
    } else if (_nextIndex < NUMBER_OF_CHILDREN) {
        EntityTreeElementPointer element = _weakElement.lock();
        if (element) {
            while (_nextIndex < NUMBER_OF_CHILDREN) {
                EntityTreeElementPointer nextElement = element->getChildAtIndex(_nextIndex);
                ++_nextIndex;
                if (nextElement && view.shouldTraverseElement(*nextElement)) {
                    next.element = nextElement;
                    return;
                }
            }
        }
    }
    next.element.reset();
}

bool DiffTraversal::View::usesViewFrustums() const {
    return !viewFrustums.empty();
}

bool DiffTraversal::View::isVerySimilar(const View& view) const {
    auto size = view.viewFrustums.size();

    if (view.lodScaleFactor != lodScaleFactor ||
        viewFrustums.size() != size) {
        return false;
    }

    for (size_t i = 0; i < size; ++i) {
        if (!viewFrustums[i].isVerySimilar(view.viewFrustums[i])) {
            return false;
        }
    }
    return true;
}

float DiffTraversal::View::computePriority(const EntityItemPointer& entity) const {
    if (!entity) {
        return PrioritizedEntity::DO_NOT_SEND;
    }

    if (!usesViewFrustums()) {
        return PrioritizedEntity::WHEN_IN_DOUBT_PRIORITY;
    }

    bool success = false;
    auto cube = entity->getQueryAACube(success);
    if (!success) {
        return PrioritizedEntity::WHEN_IN_DOUBT_PRIORITY;
    }

    auto center = cube.calcCenter(); // center of bounding sphere
    auto radius = 0.5f * SQRT_THREE * cube.getScale(); // radius of bounding sphere

    auto priority = PrioritizedEntity::DO_NOT_SEND;

    for (const auto& frustum : viewFrustums) {
        auto position = center - frustum.getPosition(); // position of bounding sphere in view-frame
        float distance = glm::length(position); // distance to center of bounding sphere

        // Check the size of the entity, it's possible that a "too small to see" entity is included in a
        // larger octree cell because of its position (for example if it crosses the boundary of a cell it
        // pops to the next higher cell. So we want to check to see that the entity is large enough to be seen
        // before we consider including it.
        float angularSize = frustum.getAngularSize(distance, radius);
        if (angularSize > lodScaleFactor * MIN_ENTITY_ANGULAR_DIAMETER &&
            frustum.intersects(position, distance, radius)) {

            // use the angular size as priority
            // we compute the max priority for all frustums
            priority = std::max(priority, angularSize);
        }
    }

    return priority;
}

bool DiffTraversal::View::shouldTraverseElement(const EntityTreeElement& element) const {
    if (!usesViewFrustums()) {
        return true;
    }

    const auto& cube = element.getAACube();

    auto center = cube.calcCenter(); // center of bounding sphere
    auto radius = 0.5f * SQRT_THREE * cube.getScale(); // radius of bounding sphere


    return any_of(begin(viewFrustums), end(viewFrustums), [&](const ConicalViewFrustum& frustum) {
        auto position = center - frustum.getPosition(); // position of bounding sphere in view-frame
        float distance = glm::length(position); // distance to center of bounding sphere

        // Check the size of the entity, it's possible that a "too small to see" entity is included in a
        // larger octree cell because of its position (for example if it crosses the boundary of a cell it
        // pops to the next higher cell. So we want to check to see that the entity is large enough to be seen
        // before we consider including it.
        float angularSize = frustum.getAngularSize(distance, radius);

        return angularSize > lodScaleFactor * MIN_ELEMENT_ANGULAR_DIAMETER &&
               frustum.intersects(position, distance, radius);
    });
}

DiffTraversal::DiffTraversal() {
    const int32_t MIN_PATH_DEPTH = 16;
    _path.reserve(MIN_PATH_DEPTH);
}

DiffTraversal::Type DiffTraversal::prepareNewTraversal(const DiffTraversal::View& view, EntityTreeElementPointer root,
                                                       bool forceFirstPass) {
    assert(root);
    // there are three types of traversal:
    //
    //   (1) First = fresh view --> find all elements in view
    //   (2) Repeat = view hasn't changed --> find elements changed since last complete traversal
    //   (3) Differential = view has changed --> find elements changed or in new view but not old
    //
    // for each traversal type we assign the appropriate _getNextVisibleElementCallback
    //
    //   _getNextVisibleElementCallback = identifies elements that need to be traversed,
    //        updates VisibleElement ref argument with pointer-to-element and view-intersection
    //        (INSIDE, INTERSECT, or OUTSIDE)
    //
    // external code should update the _scanElementCallback after calling prepareNewTraversal
    //

    Type type;
    // If usesViewFrustum changes, treat it as a First traversal
    if (forceFirstPass || _completedView.startTime == 0 || _currentView.usesViewFrustums() != _completedView.usesViewFrustums()) {
        type = Type::First;
        _currentView.viewFrustums = view.viewFrustums;
        _currentView.lodScaleFactor = view.lodScaleFactor;
        _getNextVisibleElementCallback = [this](DiffTraversal::VisibleElement& next) {
            _path.back().getNextVisibleElementFirstTime(next, _currentView);
        };
    } else if (!_currentView.usesViewFrustums() || _completedView.isVerySimilar(view)) {
        type = Type::Repeat;
        _getNextVisibleElementCallback = [this](DiffTraversal::VisibleElement& next) {
            _path.back().getNextVisibleElementRepeat(next, _completedView, _completedView.startTime);
        };
    } else {
        type = Type::Differential;
        _currentView.viewFrustums = view.viewFrustums;
        _currentView.lodScaleFactor = view.lodScaleFactor;
        _getNextVisibleElementCallback = [this](DiffTraversal::VisibleElement& next) {
            _path.back().getNextVisibleElementDifferential(next, _currentView, _completedView);
        };
    }

    _path.clear();
    _path.push_back(DiffTraversal::Waypoint(root));
    // set root fork's index such that root element returned at getNextElement()
    _path.back().initRootNextIndex();

    _currentView.startTime = usecTimestampNow();

    return type;
}

void DiffTraversal::getNextVisibleElement(DiffTraversal::VisibleElement& next) {
    if (_path.empty()) {
        next.element.reset();
        return;
    }
    _getNextVisibleElementCallback(next);
    if (next.element) {
        int8_t nextIndex = _path.back().getNextIndex();
        if (nextIndex > 0) {
            _path.push_back(DiffTraversal::Waypoint(next.element));
        }
    } else {
        // we're done at this level
        while (!next.element) {
            // pop one level
            _path.pop_back();
            if (_path.empty()) {
                // we've traversed the entire tree
                _completedView = _currentView;
                return;
            }
            // keep looking for next
            _getNextVisibleElementCallback(next);
            if (next.element) {
                // we've descended one level so add it to the path
                _path.push_back(DiffTraversal::Waypoint(next.element));
            }
        }
    }
}

void DiffTraversal::setScanCallback(std::function<void (DiffTraversal::VisibleElement&)> cb) {
    if (!cb) {
        _scanElementCallback = [](DiffTraversal::VisibleElement& a){};
    } else {
        _scanElementCallback = cb;
    }
}

void DiffTraversal::traverse(uint64_t timeBudget) {
    uint64_t expiry = usecTimestampNow() + timeBudget;
    DiffTraversal::VisibleElement next;
    getNextVisibleElement(next);
    while (next.element) {
        if (next.element->hasContent()) {
            _scanElementCallback(next);
        }
        if (usecTimestampNow() > expiry) {
            break;
        }
        getNextVisibleElement(next);
    }
}
