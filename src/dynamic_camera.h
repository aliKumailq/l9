#ifndef DYNAMIC_CAMERA_H
#define DYNAMIC_CAMERA_H

#include <iostream>
#include <limits>

#include "camera.h"

constexpr float INF = std::numeric_limits<float>::infinity();

// AABB in 1D
struct Interval {
    float min;
    float max;

    constexpr bool contains(const Interval& other) const {
        return (min <= other.min) && (max >= other.max);
    }

    constexpr bool contains(float x) const { return (min <= x) && (x <= max); }

    // signedDistance returns a positive smallest distance of x from the interval boundary
    // if it is not contained, else it returns a smallest (in magnitude) negative distance
    // from the interval boundary.
    constexpr float signedDistance(float x) const {
        if (contains(x)) {
            // 0 or negative
            return std::max(x - max, min - x);
        }
        // positive
        return std::max(x - max, min - x);
    }
};

struct DynamicCamera {
    inline void update(Camera& camera,                           // the camera to update
                const sf::Vector2f& trackedPosWorld,      // world coords of player to track
                const sf::Vector2f& trackedVelocityWorld  // world velocity of player to track
    ) {
        // TODO: (HINTS/outline)
        // collect statistics on the player position and velocity in (normalized, if you like)
        // window coordinates. Compute lookAhead distance in world coords, adjusting for
        // if the player is moving left or right.
        // Now update lookAhead state using an EMA over its previous state and the lookAhead world
        // vector. Then determine if the player is in the dead zone, soft rejection zone, or hard
        // rejection zone. For each, perform EMA on the delta of the camera displacement (ignoring
        // lookahead) appropriately. Next,
        // set the camera center by offsetting the cameraUnadjustedCenter, combining lookAhead.
        // Finally update cameraUnadjustedCenter, but without lookahead.
        auto lookAheadDistance = camera.windowPercentToWorldDist(sf::Vector2f{lookAhead, 0}).x * (trackedVelocityWorld.x < 0 ? -1 : 1);
        if (trackedVelocityWorld.x == 0) lookAheadDistance = 0;

          lookaheadState = lookaheadState + alpha * (lookAheadDistance - lookaheadState);
                                                    // target is the lookAheadDistance + current camera center.
                                                    //
          
          auto percentWindow = camera.worldToWindowPercent(trackedPosWorld);
          if (!softZoneX.contains(percentWindow.x)) {
            // if in the hard exclusion zone
            auto hard_boundary = percentWindow.x > 0.5 ? softZoneX.max : softZoneX.min;
            auto a = sf::Vector2f{hard_boundary, percentWindow.y};
            camera.setCameraPercent(trackedPosWorld, {hard_boundary, percentWindow.y});
            return;

          } else if (!deadZoneX.contains(percentWindow.x)) {

            auto hard_boundary = percentWindow.x > 0.5 ? deadZoneX.max : deadZoneX.min;
            auto dir_push = -hard_boundary  + percentWindow.x;
            dir_push *= 1000;


            cameraUnadjustedCenter = cameraUnadjustedCenter + (alpha) * ((cameraUnadjustedCenter + sf::Vector2f{dir_push, 0.0f}) - cameraUnadjustedCenter);



          }
         camera.setCameraCenter({lookaheadState + trackedPosWorld.x, camera.getCameraCenter().y});



        auto screenPosPerc = camera.worldToWindowPercent(trackedPosWorld);
        auto screenVelPerc = camera.worldToWindowPercentDist(trackedVelocityWorld);



        

    }

    float alpha;  // alpha to use for damping of camera position, *as well as* for smoothing of
                  // lookahead offset.
    Interval deadZoneX;  // percentage interval along x-axis to use for dead zone in [0, 1]
    // percentage interval along x-axis to use for soft rejection zone in [0, 1]
    Interval softZoneX = {-INF, INF};
    // (everything beyond softZone is considered hard zone).
    float lookAhead;  // screen-percentage lookahead
    mutable sf::Vector2f
        cameraUnadjustedCenter;      // the camera world center from last frame, ignoring lookahead
    mutable float lookaheadState{};  // the EMA of the accumulated lookahead offset in world coords
};

#endif // DYNAMIC_CAMERA_H
