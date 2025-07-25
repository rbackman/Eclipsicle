

import math
from dataclasses import dataclass
import json
from typing import List


@dataclass
class Vec3D:
    x: float
    y: float
    z: float

    def to_dict(self):
        return [self.x, self.y, self.z]  # as a list for compact JSON


@dataclass
class Segment:
    name: str
    points: List[Vec3D]

    def to_dict(self):
        return {
            "name": self.name,
            "points": [p.to_dict() for p in self.points]
        }


@dataclass
class StructureDef:
    name: str
    node_segments: List[Segment]

    def to_dict(self):
        return {
            "name": self.name,
            "segments": [s.to_dict() for s in self.node_segments]
        }


def rotateY(p: Vec3D, times90: int) -> Vec3D:
    t = times90 % 4
    if t == 1:
        return Vec3D(p.z, p.y, -p.x)
    elif t == 2:
        return Vec3D(-p.x, p.y, -p.z)
    elif t == 3:
        return Vec3D(-p.z, p.y, p.x)
    else:
        return p


def rotateZ(p: Vec3D, times90: int) -> Vec3D:
    t = times90 % 4
    if t == 1:
        return Vec3D(-p.y, p.x, p.z)
    elif t == 2:
        return Vec3D(-p.x, -p.y, p.z)
    elif t == 3:
        return Vec3D(p.y, -p.x, p.z)
    else:
        return p


def rotateSegmentY(segment: Segment, times90: int) -> List[Vec3D]:
    return [rotateY(p, times90) for p in segment.points]


def rotateSegmentZ(segment: Segment, times90: int) -> List[Vec3D]:
    return [rotateZ(p, times90) for p in segment.points]


def generate_full_tesseratica(points_for_A: List[Vec3D]) -> StructureDef:
    sA = Segment("A", points_for_A)

    # Y-axis rotations of A
    sB = Segment("B", rotateSegmentY(sA, 1))
    sC = Segment("C", rotateSegmentY(sA, 2))
    sD = Segment("D", rotateSegmentY(sA, 3))

    # Z-rotation of A for E, then Y-rotations of E
    sE = Segment("E", rotateSegmentZ(sA, 1))
    sF = Segment("F", rotateSegmentY(sE, 1))
    sG = Segment("G", rotateSegmentY(sE, 2))
    sH = Segment("H", rotateSegmentY(sE, 3))

    # Z-rotation of A for I, then Y-rotations of I
    sI = Segment("I", rotateSegmentZ(sA, 2))
    sJ = Segment("J", rotateSegmentY(sI, 1))
    sK = Segment("K", rotateSegmentY(sI, 2))
    sL = Segment("L", rotateSegmentY(sI, 3))

    return StructureDef("Tesseratica", [sA, sB, sC, sD, sE, sF, sG, sH, sI, sJ, sK, sL])


def distance(p1: Vec3D, p2: Vec3D) -> float:
    dx = p2.x - p1.x
    dy = p2.y - p1.y
    dz = p2.z - p1.z
    return math.sqrt(dx**2 + dy**2 + dz**2)


def generate_node_file(structure: StructureDef, segment_names: List[str], leds_per_inch: float) -> str:
    lines = []

    # Build a lookup of segments by name
    segment_map = {seg.name: seg for seg in structure.node_segments}

    for name in segment_names:
        if name not in segment_map:
            print(f"Warning: Segment {name} not found")
            continue

        seg = segment_map[name]
        led_line = f"nodes:{name}:"
        cumulative = 0.0

        for i in range(len(seg.points)):
            if i > 0:
                cumulative += distance(seg.points[i - 1], seg.points[i])

            led_index = round(cumulative * leds_per_inch)
            p = seg.points[i]
            led_line += f"{led_index},{p.x},{p.y},{p.z}:"

        lines.append(led_line)

    return "\n".join(lines)


def makeTess():

    points_A = [
        Vec3D(-85.182, -90.626, 90.98),
        Vec3D(85.182, -90.626, 90.98),
        Vec3D(33.4, -39.42, 39.1),
        Vec3D(-33.4, -39, 39.4),
        Vec3D(-85.182, -90.626, 90.98)
    ]

    structure = generate_full_tesseratica(points_A)
    res = generate_node_file(structure, ["A", "B", "C", "D", "E", "F",
                                         "G", "H", "I", "J", "K", "L"], 0.3)
    with open("led_nodes_tess.txt", "w") as f:
        f.write(res)

    with open("led_structure.json", "w") as f:
        json.dump(structure.to_dict(), f, indent=4)
    print("Tesseratica structure and node file generated.")
