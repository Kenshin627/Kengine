#include "cone.h"

Cone::Cone(float radius, float height, int radialSegments, int heightSegments, bool openEnded, float thetaStart, float thetaLength)
	:Cylinder(0, radius, height, radialSegments, heightSegments, openEnded, thetaStart, thetaLength)
{
}
