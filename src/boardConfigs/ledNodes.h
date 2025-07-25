#ifdef TESSERATICA_SEGMENT
#pragma once
#include "../lib/shared.h"

Vec3D rotateY(const Vec3D &p, int times90)
{
    // times90: 1 = 90°, 2 = 180°, 3 = 270°
    int t = times90 % 4;
    switch (t)
    {
    case 1:
        return {p.z, p.y, -p.x};
    case 2:
        return {-p.x, p.y, -p.z};
    case 3:
        return {-p.z, p.y, p.x};
    default:
        return p; // 0° rotation
    }
}

Vec3D rotateZ(const Vec3D &p, int times90)
{
    int t = times90 % 4;
    switch (t)
    {
    case 1:
        return {-p.y, p.x, p.z};
    case 2:
        return {-p.x, -p.y, p.z};
    case 3:
        return {p.y, -p.x, p.z};
    default:
        return p; // 0° rotation
    }
}

std::vector<Vec3D> rotateSegmentZ(const Segment &segment, int times90)
{
    std::vector<Vec3D> points = segment.points;
    for (auto &point : points)
    {
        point = rotateZ(point, times90);
    }
    return points;
}
std::vector<Vec3D> rotateSegmentY(const Segment &segment, int times90)
{
    std::vector<Vec3D> points = segment.points;
    for (auto &point : points)
    {
        point = rotateY(point, times90);
    }
    return points;
}
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