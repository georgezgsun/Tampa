#include "RadarInterface/CoordTransforms.h"
#include <stdio.h>
#include <math.h>

// This class provides coordinate transformations between the roadway, radar and
// video perspective view frames of reference.
// Call InitCoordTransforms() before calling other class members to initialize
// the transformation parameters.  If any of these change during program execution,
// call InitCoordTransforms() again to set the new transform parameters.

CCoordTransforms::CCoordTransforms(void)
{
}


CCoordTransforms::~CCoordTransforms(void)
{
}

void CCoordTransforms::InitCoordTransforms(float xs, float zs, float zt, float fovh, float fovv)
{
// The following parameters cannot be deduced from data provided by the sensor and must be
// specified as part of system setup
    Xs = xs;
    Zs = zs;
    Zt = zt;
    FOVh = fovh;
    FOVv = fovv;
    Theta_hs = 0.0f;
    Theta_vs = 0.0f;
//    printf("Coordinate Transforms initialized as:\nXs = %f\nZs = %f\nZt = %f\nTheta_hs = %f\nTheta_vs = %f\nFOVh = %f\nFOVv = %f\n\n", Xs, Zs, Zt, Theta_hs, Theta_vs, FOVh, FOVv);
}

void CCoordTransforms::UpdateSensor(float theta_vs, float theta_rs, float theta_hs)
{
    Theta_vs = theta_vs;
    Theta_rs = theta_rs;
    Theta_hs = theta_hs;
}

bool CCoordTransforms::Transform(coord_struct * InputCoordinates, coord_struct * OutputCoordinates)
{
    float Dr;
    float Theta_rr;

    float Xrvh;
    float Yrvh;
    float Zrvh;

    float Xrh;
    float Yrh;
    float Zrh;

    float Xrt;
    float Yrt;
    float Zrt;

    if(InputCoordinates->type == OutputCoordinates->type)
    {
        // Input and Output are same type - just copy input to output
        // Note:  Vz and Theta_Vz are included for generality but the roadway
        //        Vz and Theta_Vz will be set to zero for the Oculii
        //        sensor since it does not provide elevation speeds.
        OutputCoordinates->X = InputCoordinates->X;
        OutputCoordinates->Y = InputCoordinates->Y;
        OutputCoordinates->Z = InputCoordinates->Z;
        OutputCoordinates->R = InputCoordinates->R;
        OutputCoordinates->Theta_Y = InputCoordinates->Theta_Y;
        OutputCoordinates->Theta_Z = InputCoordinates->Theta_Z;
        OutputCoordinates->Vx = InputCoordinates->Vx;
        OutputCoordinates->Vy = InputCoordinates->Vy;
        OutputCoordinates->Vz = InputCoordinates->Vz;
        OutputCoordinates->V = InputCoordinates->V;
        OutputCoordinates->Theta_Vy = InputCoordinates->Theta_Vy;
        OutputCoordinates->Theta_Vz = InputCoordinates->Theta_Vz;
        OutputCoordinates->Ix = InputCoordinates->Ix;
        OutputCoordinates->Iz = InputCoordinates->Iz;
        return false;
    }
    else switch(InputCoordinates->type)
    {
        case roadway:
        {
            // We only have to handle roadway -> radar & roadway -> video.
            // Other cases are invalid or handled above
            if(OutputCoordinates->type == radar)
            {
                Dr = sqrtf((InputCoordinates->X - Xs) * (InputCoordinates->X - Xs) +
                                 InputCoordinates->Y * InputCoordinates->Y +
                                 InputCoordinates->Z * InputCoordinates->Z);		// Equation (1)
                Theta_rr = asinf(-Zs/Dr);						// Equation (2)
		/* float Theta_ht = atan2(InputCoordinates->Y, InputCoordinates->X - Xs);	// Equation (4)
		float Vtr = InputCoordinates->V * cosf(Theta_ht) *  cosf(Theta_rr);	// Equation (3)
		*/
                Xrt = InputCoordinates->X - Xs;						// Equation (5)
                Yrt = InputCoordinates->Y;						// Equation (6)
                Zrt = InputCoordinates->Z - Zs;						// Equation (7)

                Xrh = Xrt * cosf(Theta_hs) - Yrt * sinf(Theta_hs);			// Equation (8)
                Yrh = Xrt * sinf(Theta_hs) + Yrt * cosf(Theta_hs);			// Equation (9)
                Zrh = Zrt;	                        				// Equation (10)

                Xrvh = Xrh;								// Equation (11)
                Yrvh = Yrh * cosf(-Theta_vs) - Zrh * sinf(-Theta_vs);			// Equation (12)
                Zrvh = Yrh * sinf(-Theta_vs) + Zrh * cosf(-Theta_vs);       		// Equation (13)

                OutputCoordinates->X = Xrvh * cosf(Theta_rs) - Zrvh * sinf(Theta_rs);	// Equation (14)
                OutputCoordinates->Y = Yrvh;						// Equation (15)
                OutputCoordinates->Z = Xrvh * sinf(Theta_rs) + Zrvh * cosf(Theta_rs);	// Equation (16)
// Should this use Vtr instead?
                OutputCoordinates->V = sqrtf(InputCoordinates->Vx * InputCoordinates->Vx +	// Equation (32)
                    InputCoordinates->Vy * InputCoordinates->Vy);  // Report flat-plane speed ignoring height
                OutputCoordinates->Theta_Vy = atan2(InputCoordinates->Vx, InputCoordinates->Vy) +
                    Theta_hs;									// Equation (33)
                if(OutputCoordinates->Theta_Vy > PI) OutputCoordinates->Theta_Vy =
                                                        2.0f * PI - OutputCoordinates->Theta_Vy;
                else if (OutputCoordinates->Theta_Vy < -PI) OutputCoordinates->Theta_Vy =
                                                              -2.0f * PI + OutputCoordinates->Theta_Vy;
                OutputCoordinates->Theta_Vz = InputCoordinates->Theta_Vz - Theta_vs;
                if(OutputCoordinates->Theta_Vz > PI) OutputCoordinates->Theta_Vz =
                                                        2.0f * PI - OutputCoordinates->Theta_Vz;
                else if (OutputCoordinates->Theta_Vz < -PI) OutputCoordinates->Theta_Vz =
                                                              -2.0f * PI + OutputCoordinates->Theta_Vz;
                OutputCoordinates->Vx = OutputCoordinates->V * sin(OutputCoordinates->Theta_Vy) *
                    cos(OutputCoordinates->Theta_Vz);
                OutputCoordinates->Vy = OutputCoordinates->V * cos(OutputCoordinates->Theta_Vy) *
                    cos(OutputCoordinates->Theta_Vz);
                OutputCoordinates->Vz = OutputCoordinates->V * sin(OutputCoordinates->Theta_Vz);
                OutputCoordinates->Ix = 0.0f;
                OutputCoordinates->Iz = 0.0f;
            }
            else if(OutputCoordinates->type == video)
            {
                coord_struct radar_c;
                radar_c.type = radar;
                Transform(InputCoordinates, &radar_c);
                Transform(&radar_c, OutputCoordinates);
            }
            else
            {
                return true;	// Error - Invalid output type
            }
            break;
        }
        case radar:
        {
            // We only have to handle radar -> roadway & radar -> video.
            // Other cases are invalid or handled above
            if(OutputCoordinates->type == roadway)
            {
                Dr = sqrtf((InputCoordinates->X - Xs) * (InputCoordinates->X - Xs) +
                                 InputCoordinates->Y * InputCoordinates->Y +
                                 InputCoordinates->Z * InputCoordinates->Z);	// Equation (1)
                Theta_rr = asinf(-Zs/Dr);					// Equation (2)

// Per the iterative method described in the HH1 Coordinate Transformation.doc, find Zr as needed in equations (17) and (19)

                // 1. Initial guesses
                Zrt = Zt - Zs;							// Equation (7)
                Zrh = Zrt;	                        			// Equation (10)
                // 2.
                Yrh = InputCoordinates->Y;					// (Yr)
                // 3.
                Zrvh = Yrh * sinf(-Theta_vs) + Zrh * cosf(-Theta_vs);		// Equation (13)
                // 4.
                Xrvh = InputCoordinates->X;	// (Xr)
                // 5. Begin iteration
                float Zr = Xrvh * sinf(Theta_rs) + Zrvh * cosf(Theta_rs);	// Equation (16)
                // 6.
                float Zerr;
                int Zcount = 0;
                do
                {
                    float Zguess = Zr;
                    // Xr = InputCoordinates->X
                    // Yr = InputCoordinates->Y
                    // Zr -> InputCoordinates->Z
                    Xrvh = InputCoordinates->X * cosf(-Theta_rs) -
                           Zr * sinf(-Theta_rs);				// Equation (17)
                    Yrvh = InputCoordinates->Y;					// Equation (18)
                    Zrvh = InputCoordinates->X * sinf(-Theta_rs) +
                           Zr * cosf(-Theta_rs);				// Equation (19)
                // 7.
                    Xrh = Xrvh;							// Equation (20)
                    Yrh = Yrvh * cosf(Theta_vs) - Zrvh * sinf(Theta_vs);	// Equation (21)
                    Zrh = Zrt;							// Equation (10)
                // 8.
                    Xrvh = Xrh;							// Equation (11)
                    Yrvh = Yrh * cosf(-Theta_vs) - Zrh * sinf(-Theta_vs);	// Equation (12)
                    Zrvh = Yrh * sinf(-Theta_vs) + Zrh * cosf(-Theta_vs);       // Equation (13)

                    Zr = Xrvh * sinf(Theta_rs) + Zrvh * cosf(Theta_rs);		// Equation (16)
                    Zerr = Zr - Zguess;
                    if(Zerr < 0) Zerr = -Zerr;
                    Zcount++;
                } while((Zerr > .001f) && (Zcount < 10));
                InputCoordinates->Z = Zr;
                // Calculate final values using estimated Zr
                Xrvh = InputCoordinates->X * cosf(-Theta_rs) -
                       Zr * sinf(-Theta_rs);					// Equation (17)
                Yrvh = InputCoordinates->Y;					// Equation (18)
                Zrvh = InputCoordinates->X * sinf(-Theta_rs) +
                           Zr * cosf(-Theta_rs);				// Equation (19)

                Xrh = Xrvh;							// Equation (20)
                Yrh = Yrvh * cosf(Theta_vs) - Zrvh * sinf(Theta_vs);		// Equation (21)
                Zrh = Yrvh * sinf(Theta_vs) + Zrvh * cosf(Theta_vs);		// Equation (22)

                Xrt = Xrh * cosf(-Theta_hs) - Yrh * sinf(-Theta_hs);		// Equation (23)
                Yrt = Xrh * sinf(-Theta_hs) + Yrh * cosf(-Theta_hs);		// Equation (24)
                Zrt = Zrh;							// Equation (25)

//                printf("In %s Line %d:\n", __FILE__, __LINE__);
//                printf(" Xrh = %f, Yrh = %f, Zrh = %f\n", Xrh, Yrh, Zrh);
//                printf(" Xrt = %f, Yrt = %f, Zrt = %f\n", Xrt, Yrt, Zrt);

                OutputCoordinates->X = Xrt + Xs;				// Equation (26)
                OutputCoordinates->Y = Yrt;					    // Equation (27)
                OutputCoordinates->Z = Zrt + Zs;				// Equation (28)

                OutputCoordinates->R = Dr * cosf(Theta_rr);  // Report roadway-plane distance ignoring height
                OutputCoordinates->Theta_Y = atan2f(OutputCoordinates->X, OutputCoordinates->Y);
                OutputCoordinates->Theta_Z = Theta_rr;
                float Theta_ht = atan2(OutputCoordinates->Y, OutputCoordinates->X - Xs);	// Equation (4)
                float V = InputCoordinates->V / (cosf(Theta_ht) * cosf(Theta_rr));		// Equation (29)
                OutputCoordinates->Theta_Vy = atan2(InputCoordinates->Vx, InputCoordinates->Vy) - Theta_hs;
                OutputCoordinates->Theta_Vz = InputCoordinates->Theta_Vz + Theta_vs;
                OutputCoordinates->V = V * cosf(OutputCoordinates->Theta_Vz);  // Report flat-plane speed ignoring height
                OutputCoordinates->Vx = OutputCoordinates->V * sinf(Theta_ht);			// Equation (30)
                OutputCoordinates->Vy = OutputCoordinates->V * cosf(Theta_ht);			// Equation (31)
                OutputCoordinates->Vz = 0.0f;
                OutputCoordinates->Ix = 0.0f;
                OutputCoordinates->Iz = 0.0f;
            }
            else if(OutputCoordinates->type == video)
            {
                // First map the infinity point
                float IRxp = -2.0f * Theta_hs/FOVh;						// Equation (34)
                float IRzp = -2.0f * Theta_vs/FOVv;						// Equation (35)

//                printf("In %s Line %d, IRxp = %f, IRzp = %f\n", __FILE__, __LINE__, IRxp, IRzp);

                OutputCoordinates->Ix = IRxp * cosf(-Theta_rs) - IRzp * sinf(-Theta_rs);	// Equation (36)
                OutputCoordinates->Iz = IRxp * sinf(-Theta_rs) + IRzp * cosf(-Theta_rs);	// Equation (37)

//                printf("In %s Line %d, Ix = %f, Iz = %f\n", __FILE__, __LINE__, OutputCoordinates->Ix, OutputCoordinates->Iz);

                float Txp;
                float Tzp;
                if(InputCoordinates->Y > 0.0f)
                {
                    Txp = 2.0f * InputCoordinates->X/(InputCoordinates->Y * FOVh);		// Equation (38)
                    Tzp = 2.0f * InputCoordinates->Z/(InputCoordinates->Y * FOVv);		// Equation (39)
                }
                else
                {
                    Txp = 1.0f;
                    Tzp = 1.0f;
                }

//                printf("In %s Line %d, Txp = %f, Tzp = %f\n", __FILE__, __LINE__, Txp, Tzp);

                OutputCoordinates->X = Txp * cosf(-Theta_rs) - Tzp * sinf(-Theta_rs);		// Equation (40)
                OutputCoordinates->Y = 0.0f;			// There is no Y axis on the perspective view
                OutputCoordinates->Z = Txp * sinf(-Theta_rs) + Tzp * cosf(-Theta_rs);		// Equation (41)
//                printf("In %s Line %d:\n", __FILE__, __LINE__);
//                printf("X = %f\n", OutputCoordinates->X);
//                printf("Y = %f\n", OutputCoordinates->Y);
//                printf("Z = %f\n", OutputCoordinates->Z);

                OutputCoordinates->R = InputCoordinates->R;					// Keep true distance
                OutputCoordinates->Theta_Y = 0.0f;		// There is no Y axis on the perspective view
                // Note:  Reconsider the following.  We might instead want to report the angle
                // to the infinity point Ix, Iy?
                OutputCoordinates->Theta_Z = atan2f(OutputCoordinates->X, OutputCoordinates->Z);
                OutputCoordinates->Vx = InputCoordinates->Vx;		// Video mapping does not change speeds
                OutputCoordinates->Vy = InputCoordinates->Vy;
                OutputCoordinates->Vz = InputCoordinates->Vz;
                OutputCoordinates->V = InputCoordinates->V;
                OutputCoordinates->Theta_Vy = 0.0f;			// There is no Y axis on the perspective view
                OutputCoordinates->Theta_Vz = atan2f(OutputCoordinates->Vx, OutputCoordinates->Vz); // Is this what we want?

            }
            else
            {
                return true;	// Error - Invalid output type
            }
            break;
        }
        case video:
        {
            return true;	// Error - Cannot do reverse transformation from video
        }
        default:
        {
            return true;	// Error - Invalid input coordinates type
        }
    }
    return false;
}
