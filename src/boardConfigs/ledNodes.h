#ifdef TESSERATICA_SEGMENT
#include "../lib/shared.h"
const StructureDef tesseraticaStrip = {
    "Tesseratica",
    {{"A", {{-85.182, -90.626, 90.98}, {85.182, -90.626, 90.98}, {33.4, -39.42, 39.1}, {-33.4, -39, 39.4}, {-85.182, -90.626, 90.98}}},
     {"B", {}}},
    {{"C", {}}},
    {{"D", {}}},
    {"E", {{-90.5, -84.9, 90.8}, {-90.5, 84.9, 90.87}, {-39, 33.4, 39.4}, {-39, -33.4, 39.4}, {-90.5, -84.9, 90.8}}},
    {{"F", {}}},
    {{"G", {}}},
    {{"H", {}}},
    {"I", {{-85.182, 90.6, 91}, {85.182, 91, 90.6}, {33.4, 39, 39.421}, {-33.4, 39.42, 39.1}, {-85.182, 90.6, 91}}},
    {"J", {}},
    {"K", {}},
    {"L", {}}};

// fill in the additional segments by rotating around the y axis 90 degrees  each

// Create
const std::vector<Vec3D> bigPoints = {
    {{-85.182, -90.626, 90.98}, {85.182, -90.626, 90.98}, {33.4, -39.42, 39.1}, {-33.4, -39, 39.4}, {-85.182, -90.626, 90.98}}};

StructureDef generateFullTesseratica2(std::vector<Vec3D> pointsForA)
{
    Segment sA("A", pointsForA);
    // Bottom ones
    Segment sB("B", rotateSegmentY(sA, 1)); // 90° Y rotation
    Segment sC("C", rotateSegmentY(sA, 2)); // 180° Y rotation
    Segment sD("D", rotateSegmentY(sA, 3)); // 270° Y rotation

    Segment sE("E", rotateSegmentZ(sA, 1)); // 90° Z rotation
    Segment sF("F", rotateSegmentY(sE, 1)); // 90° Y rotation from E
    Segment sG("G", rotateSegmentY(sE, 2)); // 180° Y
    Segment sH("H", rotateSegmentY(sE, 3)); // 270° Y rotation from E

    Segment sI("I", rotateSegmentZ(sA, 2)); // 180° Z rotation
    Segment sJ("J", rotateSegmentY(sI, 1)); // 90° Y rotation from I
    Segment sK("K", rotateSegmentY(sI, 2)); // 180° Y rotation from I
    Segment sL("L", rotateSegmentY(sI, 3)); // 270° Y rotation from I

    StructureDef def = {
        "Tesseratica",
        {sA, sB, sC, sD, sE, sF, sG, sH, sI, sJ, sK, sL} // All segments
    };

    return def;
}

const StructureDef &generateFullTesseratica()
{
    static const StructureDef tesseratica = generateFullTesseratica2(bigPoints);
    return tesseratica;
}
#endif