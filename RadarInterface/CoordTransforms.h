#pragma once
#include "RadarTypes.h"

// This class provides coordinate transformations between the roadway, radar and
// video perspective view frames of reference.
// Call InitCoordTransforms() before calling other class members to initialize
// the transformation parameters.  If any of these change during program execution,
// call InitCoordTransforms() again to set the new transform parameters.

class CCoordTransforms
{
public:
    CCoordTransforms(void);
    ~CCoordTransforms(void);
    void InitCoordTransforms(float Xs, float Zs, float Zt, float FOVh, float FOVv);
    void UpdateSensor(float Theta_vs, float Theta_rs, float Theta_hs);
    bool Transform(coord_struct * InputCoordinates, coord_struct * OutputCoordinates);
private:
    float Xs;			// X axis sensor offset in meters
    float Zs;			// Z axis sensor offset in meters
    float Zt;			// Height of target above roadway
    float Theta_vs;		//  Sensor vertical tilt angle relative to Y axis in radians
    float Theta_rs;		//  Sensor roll angle relative to X axis in radians
    float Theta_hs;		//  Sensor horizontal tilt angle relative to Y axis in radians
    float FOVh;			// Camera horizontal field of view in radians
    float FOVv;			// Camera vertical field of view in radians
};
